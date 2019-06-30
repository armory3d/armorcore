#include "pch.h"

#ifdef KORE_WINDOWS

#include <Kore/Threads/Thread.h>
#include <Kore/Log.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

extern "C" void filechanged(char* file);

namespace {
	void watch(void* data) {
		HANDLE handle = ::CreateFileA((char*)data, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		
		union {
			FILE_NOTIFY_INFORMATION i;
			char d[sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH];
		} info;

		char path[MAX_PATH];

		DWORD bytesReturned = 0;

		for (;;) {
			ReadDirectoryChangesW(handle, &info, sizeof(info), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, &bytesReturned, nullptr, nullptr);
			info.i.FileName[info.i.FileNameLength] = 0;
			for (unsigned i = 0; i < info.i.FileNameLength; ++i) {
				path[i] = (char)info.i.FileName[i];
			}
			path[info.i.FileNameLength / 2] = 0;
			filechanged(path);
		}
	}
}

extern "C" void watchDirectories(char* path1, char* path2) {
	Kore::createAndRunThread(watch, path1);
	Kore::createAndRunThread(watch, path2);
}

#endif
