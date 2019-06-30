#include "pch.h"
#include "debug_server.h"

#include <Kore/Log.h>
#include <Kore/Threads/Thread.h>
#include <Kore/Threads/Mutex.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include <vector>

#ifdef KORE_WINDOWS
#include <winsock.h>
#include <io.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

void signalSemaphore(v8::base::Semaphore* semaphore);

void(*receiveMessageCallback)(char*) = nullptr;

std::string sha1(const char* data, int length);

namespace {
	int PORT = 0;
	const int RCVBUFSIZE = 4096;

	Kore::Mutex mutex;
	std::vector<std::string> queuedMessages;
	volatile int step = 0;

#ifdef KORE_WINDOWS
	SOCKET client_socket;
#else
	int client_socket;
#endif

	//v8::base::Semaphore* ready_semaphore;

	bool readHttp(const char* http) {
		Kore::log(Kore::Info, "Http message: %s", http);

		if (step == 0) {
			const char* httpheader =
				"HTTP/1.1 200 OK\r\n\
Server: Krom\r\n\
Content-Length: 371\r\n\
Content-Language: en\r\n\
Connection: close\r\n\
Content-Type: text/json\r\n\
\r\n\r\n";

			char portstring[24];
			sprintf(portstring, "%d", PORT);

			const char* httpdata1 =
				"[{\r\n\
\"description\": \"\",\r\n\
\"devtoolsFrontendUrl\": \"/devtools/inspector.html?ws=localhost:";

			const char* httpdata2 = "/devtools/page/dc5c7352-a3c4-40d2-9bec-30a329ef40e0\",\r\n\
\"id\": \"dc5c7352-a3c4-40d2-9bec-30a329ef40e0\",\r\n\
\"title\": \"localhost:";
			
			const char* httpdata3 = "/json\",\r\n\
\"type\": \"page\",\r\n\
\"url\": \"http://krom\",\r\n\
\"webSocketDebuggerUrl\": \"ws://localhost:";
			
			const char* httpdata4 = "/devtools/page/dc5c7352-a3c4-40d2-9bec-30a329ef40e0\"\r\n\
}]";

			char data[4096];
			strcpy(data, httpheader);
			strcat(data, httpdata1);
			strcat(data, portstring);
			strcat(data, httpdata2);
			strcat(data, portstring);
			strcat(data, httpdata3);
			strcat(data, portstring);
			strcat(data, httpdata4);
			send(client_socket, data, strlen(data), 0);

			++step;

			return false;
		}
		else if (step == 1) {
			std::string buffer = http;
			std::string search = "Sec-WebSocket-Key: ";
			size_t start = buffer.find(search, 0);
			size_t end = buffer.find_first_of('\r', start);
			std::string key = buffer.substr(start + search.length(), end - start - search.length()) + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			std::string sha = sha1(key.c_str(), key.length());
			while (sha[sha.length() - 1] == '\n' || sha[sha.length() - 1] == '\r') {
				sha = sha.substr(0, sha.length() - 1);
			}

			char data[4096];
			strcpy(data,
				"HTTP/1.1 101 Switching Protocols\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Accept: ");
			strcat(data, sha.c_str());
			strcat(data, "\r\n");
			strcat(data, "Sec-WebSocket-Extensions:");
			strcat(data, "\r\n\r\n");
			send(client_socket, data, strlen(data), 0);

			++step;

			return true;
		}
		else {
			return true;
		}
	}

	char httpBuffer[4096];
	int httpBufferIndex = 0;
	int httpBufferStep = 0;

	bool collectHttp(char* data, int size) {
		bool keepOpen = true;
		for (int i = 0; i < size; ++i) {
			httpBuffer[httpBufferIndex++] = data[i];
			if (httpBufferStep == 0 && data[i] == '\r') ++httpBufferStep;
			else if (httpBufferStep == 1 && data[i] == '\n') ++httpBufferStep;
			else if (httpBufferStep == 2 && data[i] == '\r') ++httpBufferStep;
			else if (httpBufferStep == 3 && data[i] == '\n') {
				httpBufferStep = 0;
				httpBuffer[httpBufferIndex] = 0;
				keepOpen = readHttp(httpBuffer);
				httpBufferIndex = 0;
			}
			else {
				httpBufferStep = 0;
			}
		}
		return keepOpen;
	}

	int readWebSocket(unsigned char* data, int size) {
		if (size < 2) return -1;

		unsigned char fin = data[0] >> 7;
		unsigned char opcode = data[0] & 0xf;
		unsigned char maskbit = data[1] >> 7;
		unsigned char payload1 = data[1] & 0x7f;

		if (fin == 0) {
			int a = 3;
			++a;
		}

		if (opcode == 1) {
			int position = 0;

			Kore::u64 payload = 0;
			if (payload1 <= 125) {
				payload = payload1;
				position = 2;
			}
			else if (payload1 == 126) {
				if (size < 4) return -1;
				unsigned short payload2 = (data[2] << 8) | data[3];
				payload = payload2;
				position = 4;
			}
			else {
				assert(payload1 == 127);
				if (size < 10) return -1;
				payload = *(Kore::u64*)&data[2];
				position = 10;
			}

			if (size < position + 4) return -1;
			unsigned char mask[4];
			if (maskbit) {
				for (int i = 0; i < 4; ++i) {
					mask[i] = data[position++];
				}
			}

			if (size < position + payload) return -1;
			unsigned char* encoded = &data[position];
			unsigned char decoded[RCVBUFSIZE];
			for (Kore::u64 i = 0; i < payload; ++i) {
				decoded[i] = encoded[i] ^ mask[i % 4];
			}
			decoded[payload] = 0;

			Kore::log(Kore::Info, "WebSocket message: %s", decoded);

			mutex.lock();
			queuedMessages.push_back((char*)decoded);
			//**receiveMessageCallback((char*)decoded);
			mutex.unlock();

			step = 3;

			if (strcmp((char*)decoded, "{\"id\":4,\"method\":\"Runtime.run\"}") == 0) {
				//signalSemaphore(ready_semaphore);
			}
			return position + payload;
		}
		else {
			int a = 3;
			++a;
			return 0;
		}
	}

	unsigned char webSocketBuffer[4096];
	int webSocketBufferIndex = 0;

	void collectWebSocket(char* data, int size) {
		unsigned char* buffer = (unsigned char*)data;
		for (int i = 0; i < size; ++i) {
			webSocketBuffer[webSocketBufferIndex++] = buffer[i];
		}
		while (webSocketBufferIndex > 0) {
			int decoded = readWebSocket(webSocketBuffer, webSocketBufferIndex);
			if (decoded > 0) {
				for (int i = decoded; i < size; ++i) {
					webSocketBuffer[i - decoded] = webSocketBuffer[i];
				}
				webSocketBufferIndex = size - decoded;
				size -= decoded;
			}
			else {
				return;
			}
		}
	}

	static void error_exit(const char *error_message) {
#ifdef KORE_WINDOWS
		fprintf(stderr, "%s: %d\n", error_message, WSAGetLastError());
#else
		fprintf(stderr, "%s: %s\n", error_message, strerror(errno));
#endif
		exit(EXIT_FAILURE);
	}
	
#ifdef KORE_WINDOWS
	static void echo(SOCKET client_socket)
#else
	static void echo(int client_socket)
#endif
	{
		for (;;) {
			::client_socket = client_socket;
			char echo_buffer[RCVBUFSIZE];
			int recv_size;
			time_t zeit;

			if ((recv_size = recv(client_socket, echo_buffer, RCVBUFSIZE, 0)) < 0) error_exit("recv() error");

			if (step < 2) {
				bool keepOpen = collectHttp(echo_buffer, recv_size);
				if (!keepOpen) return;
			}
			else {
				collectWebSocket(echo_buffer, recv_size);
			}
		}
	}

	std::string encodeMessage(std::string message) {
		std::string encoded;
		encoded += (unsigned char)0x81;
		if (message.length() <= 125) {
			encoded += (unsigned char)message.length();
		}
		else {
			encoded += (unsigned char)126;
			unsigned short payload = (unsigned short)message.length();
			unsigned char* payloadparts = (unsigned char*)&payload;
			encoded += payloadparts[1];
			encoded += payloadparts[0];
		}
		encoded += message;
		return encoded;
	}


	void startServerInThread(void*) {
		struct sockaddr_in server, client;

#ifdef KORE_WINDOWS
		SOCKET sock, fd;
#else
		int sock, fd;
#endif

#ifdef KORE_WINDOWS
		int len;
#else
		unsigned int len;
#endif

#ifdef KORE_WINDOWS
		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD(1, 1);
		if (WSAStartup(wVersionRequested, &wsaData) != 0) error_exit("Winsock initialization error");
#endif

		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock < 0) error_exit("Socket error");

		memset(&server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = htonl(INADDR_ANY);
		server.sin_port = htons(PORT);

		if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
			error_exit("Could not bind socket");

		if (listen(sock, 5) == -1) error_exit("listen() error");

		printf("Server started\n");
		for (;;) {
			len = sizeof(client);
			fd = accept(sock, (struct sockaddr*)&client, &len);
			if (fd < 0) error_exit("accept() error");
			Kore::log(Kore::Info, "Data from address: %s\n", inet_ntoa(client.sin_addr));
			echo(fd);

#ifdef KORE_WINDOWS
			closesocket(fd);
#else
			close(fd);
#endif
		}
	}
}

void sendMessage(const char* message) {
	std::string encoded = encodeMessage(message);
	send(client_socket, encoded.c_str(), encoded.length(), 0);
}

std::string receiveMessage() {
	mutex.lock();
	if (queuedMessages.size() < 1) {
		mutex.unlock();
		return "";
	}
	else {
		std::string message = queuedMessages[0];
		queuedMessages.erase(queuedMessages.begin());
		mutex.unlock();
		return message;
	}
}

void startServer(int port) { // v8::base::Semaphore* semaphore) {
	PORT = port;
	//ready_semaphore = semaphore;
	//Kore::threadsInit();
	mutex.create();
	Kore::createAndRunThread(startServerInThread, nullptr);

	//while (step < 3) { }
}
