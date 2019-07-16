#include "pch.h"
#include <Kore/IO/FileReader.h>
#include <Kore/IO/FileWriter.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Compute/Compute.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Pen.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Audio2/Audio.h>
#include <Kore/Audio1/Audio.h>
#include <Kore/Audio1/Sound.h>
#include <Kore/Audio1/SoundStream.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>
#include <Kore/Display.h>
#include <Kore/Log.h>
#include <Kore/Threads/Thread.h>
#include <Kore/Threads/Mutex.h>
#include <kinc/io/filereader.h>

#include "../V8/include/libplatform/libplatform.h"
#include "../V8/include/v8.h"

#include <map>
#include <string>
#include <vector>
#include <stdarg.h>

#ifdef KORE_WINDOWS
#include <Windows.h> // AttachConsole
#endif

#ifdef KORE_DIRECT3D
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <strstream>
#endif
#include <nfd.h>

using namespace v8;

const int KROM_API = 3;

#ifdef KORE_MACOS
const char* macgetresourcepath();
#endif

Global<Context> globalContext;
Isolate* isolate;

bool saveAndQuit = false;
void armorySaveAndQuit() { saveAndQuit = true; }

namespace {
	int _argc;
	char** _argv;
	bool enableSound = false;
	bool nowindow = false;

	std::unique_ptr<Platform> plat;
	Global<Function> updateFunction;
	Global<Function> dropFilesFunction;
	Global<Function> cutFunction;
	Global<Function> copyFunction;
	Global<Function> pasteFunction;
	Global<Function> foregroundFunction;
	Global<Function> resumeFunction;
	Global<Function> pauseFunction;
	Global<Function> backgroundFunction;
	Global<Function> shutdownFunction;
	Global<Function> keyboardDownFunction;
	Global<Function> keyboardUpFunction;
	Global<Function> keyboardPressFunction;
	Global<Function> mouseDownFunction;
	Global<Function> mouseUpFunction;
	Global<Function> mouseMoveFunction;
	Global<Function> mouseWheelFunction;
	Global<Function> penDownFunction;
	Global<Function> penUpFunction;
	Global<Function> penMoveFunction;
	Global<Function> gamepadAxisFunction;
	Global<Function> gamepadButtonFunction;
	Global<Function> audioFunction;
	Global<Function> saveAndQuitFunction;

	Kore::Mutex mutex;

	void update();
	void initAudioBuffer();
	void mix(int samples);
	void dropFiles(wchar_t* filePath);
	char* cut();
	char* copy();
	void paste(char* data);
	void foreground();
	void resume();
	void pause();
	void background();
	void shutdown();
	void keyDown(Kore::KeyCode code);
	void keyUp(Kore::KeyCode code);
    void keyPress(wchar_t character);
	void mouseMove(int window, int x, int y, int mx, int my);
	void mouseDown(int window, int button, int x, int y);
	void mouseUp(int window, int button, int x, int y);
	void mouseWheel(int window, int delta);
	void penDown(int window, int x, int y, float pressure);
	void penUp(int window, int x, int y, float pressure);
	void penMove(int window, int x, int y, float pressure);
	void gamepad1Axis(int axis, float value);
	void gamepad1Button(int button, float value);
	void gamepad2Axis(int axis, float value);
	void gamepad2Button(int button, float value);
	void gamepad3Axis(int axis, float value);
	void gamepad3Button(int button, float value);
	void gamepad4Axis(int axis, float value);
	void gamepad4Button(int button, float value);

	const int tempStringSize = 1024 * 1024 - 1;
	char tempStringVS[tempStringSize + 1];
	char tempStringFS[tempStringSize + 1];

	void sendLogMessageArgs(const char* format, va_list args) {
		char message[4096];
		vsnprintf(message, sizeof(message) - 2, format, args);
		Kore::log(Kore::Info, "%s", message);
	}

	void sendLogMessage(const char* format, ...) {
		va_list args;
		va_start(args, format);
		sendLogMessageArgs(format, args);
		va_end(args);
	}

	void krom_init(const v8::FunctionCallbackInfo<v8::Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value title(isolate, arg);
		int width = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int samplesPerPixel = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool vSync = args[4]->ToBoolean(isolate)->Value();
		int windowMode = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int windowFeatures = args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int apiVersion = args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		if (apiVersion != KROM_API) {
			const char* outdated;
			if (apiVersion < KROM_API) {
				outdated = "Kha";
			}
			else if (KROM_API < apiVersion) {
				outdated = "Krom";
			}
			sendLogMessage("Krom uses API version %i but Kha targets API version %i. Please update %s.", KROM_API, apiVersion, outdated);
			exit(1);
		}

		Kore::WindowOptions win;
		win.title = *title;
		win.width = width;
		win.height = height;
		win.x = -1;
		win.y = -1;
		win.visible = !nowindow;
		win.mode = Kore::WindowMode(windowMode);
		win.windowFeatures = windowFeatures;
		Kore::FramebufferOptions frame;
		frame.verticalSync = vSync;
		frame.samplesPerPixel = samplesPerPixel;
		Kore::System::init(*title, width, height, &win, &frame);

		mutex.create();
		if (enableSound) {
			Kore::Audio2::audioCallback = mix;
			Kore::Audio2::init();
			initAudioBuffer();
		}
		Kore::Random::init((int)(Kore::System::time() * 1000));

		Kore::System::setCallback(update);
		Kore::System::setDropFilesCallback(dropFiles);
		Kore::System::setCopyCallback(copy);
		Kore::System::setCutCallback(cut);
		Kore::System::setPasteCallback(paste);
		Kore::System::setForegroundCallback(foreground);
		Kore::System::setResumeCallback(resume);
		Kore::System::setPauseCallback(pause);
		Kore::System::setBackgroundCallback(background);
		Kore::System::setShutdownCallback(shutdown);

		Kore::Keyboard::the()->KeyDown = keyDown;
		Kore::Keyboard::the()->KeyUp = keyUp;
		Kore::Keyboard::the()->KeyPress = keyPress;
		Kore::Mouse::the()->Move = mouseMove;
		Kore::Mouse::the()->Press = mouseDown;
		Kore::Mouse::the()->Release = mouseUp;
		Kore::Mouse::the()->Scroll = mouseWheel;
		Kore::Pen::the()->Press = penDown;
		Kore::Pen::the()->Release = penUp;
		Kore::Pen::the()->Move = penMove;
		Kore::Gamepad::get(0)->Axis = gamepad1Axis;
		Kore::Gamepad::get(0)->Button = gamepad1Button;
		Kore::Gamepad::get(1)->Axis = gamepad2Axis;
		Kore::Gamepad::get(1)->Button = gamepad2Button;
		Kore::Gamepad::get(2)->Axis = gamepad3Axis;
		Kore::Gamepad::get(2)->Button = gamepad3Button;
		Kore::Gamepad::get(3)->Axis = gamepad4Axis;
		Kore::Gamepad::get(3)->Button = gamepad4Button;
	}

	void krom_log(const v8::FunctionCallbackInfo<v8::Value>& args) {
		if (args.Length() < 1) return;
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value value(isolate, arg);
		sendLogMessage(*value);
	}

	void graphics_clear(const v8::FunctionCallbackInfo<v8::Value>& args) {
		HandleScope scope(args.GetIsolate());
		int flags = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float depth = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int stencil = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::clear(flags, color, depth, stencil);
	}

	void krom_set_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		updateFunction.Reset(isolate, func);
	}

	void krom_set_drop_files_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		dropFilesFunction.Reset(isolate, func);
	}

	void krom_set_cut_copy_paste_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> cutArg = args[0];
		Local<Function> cutFunc = Local<Function>::Cast(cutArg);
		cutFunction.Reset(isolate, cutFunc);
		Local<Value> copyArg = args[1];
		Local<Function> copyFunc = Local<Function>::Cast(copyArg);
		copyFunction.Reset(isolate, copyFunc);
		Local<Value> pasteArg = args[2];
		Local<Function> pasteFunc = Local<Function>::Cast(pasteArg);
		pasteFunction.Reset(isolate, pasteFunc);
	}

	void krom_set_application_state_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> foregroundArg = args[0];
		Local<Function> foregroundFunc = Local<Function>::Cast(foregroundArg);
		foregroundFunction.Reset(isolate, foregroundFunc);
		Local<Value> resumeArg = args[1];
		Local<Function> resumeFunc = Local<Function>::Cast(resumeArg);
		resumeFunction.Reset(isolate, resumeFunc);
		Local<Value> pauseArg = args[2];
		Local<Function> pauseFunc = Local<Function>::Cast(pauseArg);
		pauseFunction.Reset(isolate, pauseFunc);
		Local<Value> backgroundArg = args[3];
		Local<Function> backgroundFunc = Local<Function>::Cast(backgroundArg);
		backgroundFunction.Reset(isolate, backgroundFunc);
		Local<Value> shutdownArg = args[4];
		Local<Function> shutdownFunc = Local<Function>::Cast(shutdownArg);
		shutdownFunction.Reset(isolate, shutdownFunc);
	}

	void krom_set_keyboard_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboardDownFunction.Reset(isolate, func);
	}

	void krom_set_keyboard_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboardUpFunction.Reset(isolate, func);
	}

	void krom_set_keyboard_press_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboardPressFunction.Reset(isolate, func);
	}

	void krom_set_mouse_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouseDownFunction.Reset(isolate, func);
	}

	void krom_set_mouse_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouseUpFunction.Reset(isolate, func);
	}

	void krom_set_mouse_move_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouseMoveFunction.Reset(isolate, func);
	}

	void krom_set_mouse_wheel_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouseWheelFunction.Reset(isolate, func);
	}

	void krom_set_pen_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		penDownFunction.Reset(isolate, func);
	}

	void krom_set_pen_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		penUpFunction.Reset(isolate, func);
	}

	void krom_set_pen_move_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		penMoveFunction.Reset(isolate, func);
	}

	void krom_set_gamepad_axis_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepadAxisFunction.Reset(isolate, func);
	}

	void krom_set_gamepad_button_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepadButtonFunction.Reset(isolate, func);
	}

	void krom_lock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Mouse::the()->lock(0);
	}

	void krom_unlock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Mouse::the()->unlock(0);
	}

	void krom_can_lock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, Kore::Mouse::the()->canLock(0)));
	}

	void krom_is_mouse_locked(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, Kore::Mouse::the()->isLocked(0)));
	}

	void krom_show_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Mouse::the()->show(args[0]->ToBoolean(isolate)->Value());
	}

	void krom_set_audio_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		audioFunction.Reset(isolate, func);
	}

	// TODO: krom_audio_lock
	void audio_thread(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		bool lock = args[0]->ToBoolean(isolate)->Value();

		if (lock) mutex.lock();    //v8::Locker::Locker(isolate);
		else mutex.unlock();       //v8::Unlocker(args.GetIsolate());
	}

	void krom_create_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, new Kore::Graphics4::IndexBuffer(args[0]->Int32Value(isolate->GetCurrentContext()).FromJust())));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::IndexBuffer* buffer = (Kore::Graphics4::IndexBuffer*)field->Value();
		delete buffer;
	}

	void krom_lock_index_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::IndexBuffer* buffer = (Kore::Graphics4::IndexBuffer*)field->Value();
		int* vertices = buffer->lock();
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, vertices, buffer->count() * sizeof(int));
		args.GetReturnValue().Set(Uint32Array::New(abuffer, 0, buffer->count()));
	}

	void krom_unlock_index_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::IndexBuffer* buffer = (Kore::Graphics4::IndexBuffer*)field->Value();
		buffer->unlock();
	}

	void krom_set_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::IndexBuffer* buffer = (Kore::Graphics4::IndexBuffer*)field->Value();
		Kore::Graphics4::setIndexBuffer(*buffer);
	}

	Kore::Graphics4::VertexData convertVertexData(int num) {
		switch (num) {
		case 0:
			return Kore::Graphics4::Float1VertexData;
		case 1:
			return Kore::Graphics4::Float2VertexData;
		case 2:
			return Kore::Graphics4::Float3VertexData;
		case 3:
			return Kore::Graphics4::Float4VertexData;
		case 4:
			return Kore::Graphics4::Float4x4VertexData;
		case 5:
			return Kore::Graphics4::Short2NormVertexData;
		case 6:
			return Kore::Graphics4::Short4NormVertexData;
		}
		return Kore::Graphics4::Float1VertexData;
	}

	void krom_create_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();

		Local<Object> jsstructure = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::VertexStructure structure;
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> element = jsstructure->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<Value> str = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			String::Utf8Value utf8_value(isolate, str);
			int32_t data = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "data").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			char* name = new char[32]; // TODO
			strcpy(name, *utf8_value);
			structure.add(name, convertVertexData(data));
		}

		obj->SetInternalField(0, External::New(isolate, new Kore::Graphics4::VertexBuffer(args[0]->Int32Value(isolate->GetCurrentContext()).FromJust(), structure, (Kore::Graphics4::Usage)args[2]->Int32Value(isolate->GetCurrentContext()).FromJust(), args[3]->Int32Value(isolate->GetCurrentContext()).FromJust())));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::VertexBuffer* buffer = (Kore::Graphics4::VertexBuffer*)field->Value();
		delete buffer;
	}

	void krom_lock_vertex_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::VertexBuffer* buffer = (Kore::Graphics4::VertexBuffer*)field->Value();
		float* vertices = buffer->lock();
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, vertices, buffer->count() * buffer->stride());
		args.GetReturnValue().Set(Float32Array::New(abuffer, 0, buffer->count() * buffer->stride() / 4));
	}

	void krom_unlock_vertex_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::VertexBuffer* buffer = (Kore::Graphics4::VertexBuffer*)field->Value();
		buffer->unlock();
	}

	void krom_set_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::VertexBuffer* buffer = (Kore::Graphics4::VertexBuffer*)field->Value();
		Kore::Graphics4::setVertexBuffer(*buffer);
	}

	void krom_set_vertexbuffers(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Graphics4::VertexBuffer* vertexBuffers[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		Local<Object> jsarray = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> bufferobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "buffer").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> bufferfield = Local<External>::Cast(bufferobj->GetInternalField(0));
			Kore::Graphics4::VertexBuffer* buffer = (Kore::Graphics4::VertexBuffer*)bufferfield->Value();
			vertexBuffers[i] = buffer;
		}
		Kore::Graphics4::setVertexBuffers(vertexBuffers, length);
	}

	void krom_draw_indexed_vertices(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int start = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (count < 0) Kore::Graphics4::drawIndexedVertices();
		else Kore::Graphics4::drawIndexedVertices(start, count);
	}

	void krom_draw_indexed_vertices_instanced(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int instanceCount = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int start = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (count < 0) Kore::Graphics4::drawIndexedVerticesInstanced(instanceCount);
		else Kore::Graphics4::drawIndexedVerticesInstanced(instanceCount, start, count);
	}

	void krom_create_vertex_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(content.Data(), (int)content.ByteLength(), Kore::Graphics4::VertexShader);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_vertex_shader_from_source(const FunctionCallbackInfo<Value>& args) {

		#ifdef KORE_DIRECT3D

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		//char* tempStringVS = new char[strlen(*utf8_value) + 1];
		strcpy(tempStringVS, *utf8_value);
		
		ID3DBlob* errorMessage;
		ID3DBlob* shaderBuffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(tempStringVS, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, &shaderBuffer, &errorMessage);
		if (hr != S_OK) {
			Kore::log(Kore::Info, "%s", (char*)errorMessage->GetBufferPointer());
			return;
		}

		// bool hasBone = strstr(tempStringVS, "bone :") != NULL;
		// bool hasCol = strstr(tempStringVS, "col :") != NULL;
		bool hasNor = strstr(tempStringVS, "nor :") != NULL;
		bool hasPos = strstr(tempStringVS, "pos :") != NULL;
		// bool hasTang = strstr(tempStringVS, "tang :") != NULL;
		bool hasTex = strstr(tempStringVS, "tex :") != NULL;
		// bool hasWeight = strstr(tempStringVS, "weight :") != NULL;

		std::map<std::string, int> attributes;
		int index = 0;
		// if (hasBone) attributes["bone"] = index++;
		// if (hasCol) attributes["col"] = index++;
		if (hasNor) attributes["nor"] = index++;
		if (hasPos) attributes["pos"] = index++;
		// if (hasTang) attributes["tang"] = index++;
		if (hasTex) attributes["tex"] = index++;
		// if (hasWeight) attributes["weight"] = index++;

		char* output = tempStringVS;
		std::ostrstream file(output, 1024 * 1024);
		int outputlength = 0;

		file.put((char)attributes.size()); outputlength += 1;
		for (std::map<std::string, int>::const_iterator attribute = attributes.begin(); attribute != attributes.end(); ++attribute) {
			(file) << attribute->first.c_str(); outputlength += attribute->first.length();
			file.put(0); outputlength += 1;
			file.put(attribute->second); outputlength += 1;
		}

		ID3D11ShaderReflection* reflector = nullptr;
		D3DReflect(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

		D3D11_SHADER_DESC desc;
		reflector->GetDesc(&desc);

		file.put(desc.BoundResources); outputlength += 1;
		for (unsigned i = 0; i < desc.BoundResources; ++i) {
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			reflector->GetResourceBindingDesc(i, &bindDesc);
			(file) << bindDesc.Name; outputlength += strlen(bindDesc.Name);
			file.put(0); outputlength += 1;
			file.put(bindDesc.BindPoint); outputlength += 1;
		}

		ID3D11ShaderReflectionConstantBuffer* constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables); outputlength += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable* variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name; outputlength += strlen(variableDesc.Name);
					file.put(0); outputlength += 1;
					file.write((char*)&variableDesc.StartOffset, 4); outputlength += 4;
					file.write((char*)&variableDesc.Size, 4); outputlength += 4;
					D3D11_SHADER_TYPE_DESC typeDesc;
					hr = variable->GetType()->GetDesc(&typeDesc);
					if (hr == S_OK) {
						file.put(typeDesc.Columns); outputlength += 1;
						file.put(typeDesc.Rows); outputlength += 1;
					}
					else {
						file.put(0); outputlength += 1;
						file.put(0); outputlength += 1;
					}
				}
			}
		}
		else {
			file.put(0); outputlength += 1;
		}
		file.write((char*)shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize()); outputlength += shaderBuffer->GetBufferSize();
		shaderBuffer->Release();
		reflector->Release();

		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(output, outputlength, Kore::Graphics4::VertexShader);

		#else

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		char* source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(source, Kore::Graphics4::VertexShader);

		#endif

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		Local<String> name = String::NewFromUtf8(isolate, "").ToLocalChecked();
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_fragment_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(content.Data(), (int)content.ByteLength(), Kore::Graphics4::FragmentShader);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_fragment_shader_from_source(const FunctionCallbackInfo<Value>& args) {

		#ifdef KORE_DIRECT3D

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		//char* tempStringFS = new char[strlen(*utf8_value) + 1];
		strcpy(tempStringFS, *utf8_value);
		
		ID3DBlob* errorMessage;
		ID3DBlob* shaderBuffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(tempStringFS, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, &shaderBuffer, &errorMessage);
		if (hr != S_OK) {
			Kore::log(Kore::Info, "%s", (char*)errorMessage->GetBufferPointer());
			return;
		}

		std::map<std::string, int> attributes;

		char* output = tempStringFS;
		std::ostrstream file(output, 1024 * 1024);
		int outputlength = 0;

		file.put((char)attributes.size()); outputlength += 1;

		ID3D11ShaderReflection* reflector = nullptr;
		D3DReflect(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

		D3D11_SHADER_DESC desc;
		reflector->GetDesc(&desc);

		file.put(desc.BoundResources); outputlength += 1;
		for (unsigned i = 0; i < desc.BoundResources; ++i) {
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			reflector->GetResourceBindingDesc(i, &bindDesc);
			(file) << bindDesc.Name; outputlength += strlen(bindDesc.Name);
			file.put(0); outputlength += 1;
			file.put(bindDesc.BindPoint); outputlength += 1;
		}

		ID3D11ShaderReflectionConstantBuffer* constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables); outputlength += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable* variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name; outputlength += strlen(variableDesc.Name);
					file.put(0); outputlength += 1;
					file.write((char*)&variableDesc.StartOffset, 4); outputlength += 4;
					file.write((char*)&variableDesc.Size, 4); outputlength += 4;
					D3D11_SHADER_TYPE_DESC typeDesc;
					hr = variable->GetType()->GetDesc(&typeDesc);
					if (hr == S_OK) {
						file.put(typeDesc.Columns); outputlength += 1;
						file.put(typeDesc.Rows); outputlength += 1;
					}
					else {
						file.put(0); outputlength += 1;
						file.put(0); outputlength += 1;
					}
				}
			}
		}
		else {
			file.put(0); outputlength += 1;
		}
		file.write((char*)shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize()); outputlength += shaderBuffer->GetBufferSize();
		shaderBuffer->Release();
		reflector->Release();

		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(output, outputlength, Kore::Graphics4::FragmentShader);

		#else

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		char* source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(source, Kore::Graphics4::FragmentShader);

		#endif

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		Local<String> name = String::NewFromUtf8(isolate, "").ToLocalChecked();
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_geometry_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(content.Data(), (int)content.ByteLength(), Kore::Graphics4::GeometryShader);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_tessellation_control_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(content.Data(), (int)content.ByteLength(), Kore::Graphics4::TessellationControlShader);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_tessellation_evaluation_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::Graphics4::Shader* shader = new Kore::Graphics4::Shader(content.Data(), (int)content.ByteLength(), Kore::Graphics4::TessellationEvaluationShader);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Shader* shader = (Kore::Graphics4::Shader*)field->Value();
		delete shader;
	}

	void krom_create_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Graphics4::PipelineState* pipeline = new Kore::Graphics4::PipelineState;

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(8);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, pipeline));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> progobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> progfield = Local<External>::Cast(progobj->GetInternalField(0));
		Kore::Graphics4::PipelineState* pipeline = (Kore::Graphics4::PipelineState*)progfield->Value();
		delete pipeline;
	}

	void krom_compile_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<Object> progobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		Local<External> progfield = Local<External>::Cast(progobj->GetInternalField(0));
		Kore::Graphics4::PipelineState* pipeline = (Kore::Graphics4::PipelineState*)progfield->Value();

		Kore::Graphics4::VertexStructure s0, s1, s2, s3;
		Kore::Graphics4::VertexStructure* structures[4] = { &s0, &s1, &s2, &s3 };

		int32_t size = args[5]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i1 = 0; i1 < size; ++i1) {
			Local<Object> jsstructure = args[i1 + 1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			structures[i1]->instanced = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "instanced").ToLocalChecked()).ToLocalChecked()->ToBoolean(isolate)->Value();
			Local<Object> elements = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			int32_t length = elements->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			for (int32_t i2 = 0; i2 < length; ++i2) {
				Local<Object> element = elements->Get(isolate->GetCurrentContext(), i2).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				Local<Value> str = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
				String::Utf8Value utf8_value(isolate, str);
				int32_t data = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "data").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
				char* name = new char[32]; // TODO
				strcpy(name, *utf8_value);
				structures[i1]->add(name, convertVertexData(data));
			}
		}

		progobj->SetInternalField(1, External::New(isolate, structures));
		progobj->SetInternalField(2, External::New(isolate, &size));

		Local<External> vsfield = Local<External>::Cast(args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Shader* vertexShader = (Kore::Graphics4::Shader*)vsfield->Value();
		progobj->SetInternalField(3, External::New(isolate, vertexShader));
		progobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "vsname").ToLocalChecked(), args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());

		Local<External> fsfield = Local<External>::Cast(args[7]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Shader* fragmentShader = (Kore::Graphics4::Shader*)fsfield->Value();
		progobj->SetInternalField(4, External::New(isolate, fragmentShader));
		progobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "fsname").ToLocalChecked(), args[7]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());

		pipeline->vertexShader = vertexShader;
		pipeline->fragmentShader = fragmentShader;

		if (!args[8]->IsNull() && !args[8]->IsUndefined()) {
			Local<External> gsfield = Local<External>::Cast(args[8]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			Kore::Graphics4::Shader* geometryShader = (Kore::Graphics4::Shader*)gsfield->Value();
			progobj->SetInternalField(5, External::New(isolate, geometryShader));
			progobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "gsname").ToLocalChecked(), args[8]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
			pipeline->geometryShader = geometryShader;
		}

		if (!args[9]->IsNull() && !args[9]->IsUndefined()) {
			Local<External> tcsfield = Local<External>::Cast(args[9]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			Kore::Graphics4::Shader* tessellationControlShader = (Kore::Graphics4::Shader*)tcsfield->Value();
			progobj->SetInternalField(6, External::New(isolate, tessellationControlShader));
			progobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "tcsname").ToLocalChecked(), args[9]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
			pipeline->tessellationControlShader = tessellationControlShader;
		}

		if (!args[10]->IsNull() && !args[10]->IsUndefined()) {
			Local<External> tesfield = Local<External>::Cast(args[10]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			Kore::Graphics4::Shader* tessellationEvaluationShader = (Kore::Graphics4::Shader*)tesfield->Value();
			progobj->SetInternalField(7, External::New(isolate, tessellationEvaluationShader));
			progobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "tesname").ToLocalChecked(), args[10]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
			pipeline->tessellationEvaluationShader = tessellationEvaluationShader;
		}

		for (int i = 0; i < size; ++i) {
			pipeline->inputLayout[i] = structures[i];
		}
		pipeline->inputLayout[size] = nullptr;

		pipeline->cullMode = (Kore::Graphics4::CullMode)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "cullMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->depthWrite = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depthWrite").ToLocalChecked()).ToLocalChecked()->BooleanValue(isolate);
		pipeline->depthMode = (Kore::Graphics4::ZCompareMode)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depthMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->stencilMode = (Kore::Graphics4::ZCompareMode)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencilBothPass = (Kore::Graphics4::StencilAction)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilBothPass").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencilDepthFail = (Kore::Graphics4::StencilAction)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilDepthFail").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencilFail = (Kore::Graphics4::StencilAction)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilFail").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencilReferenceValue = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilReferenceValue").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencilReadMask = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilReadMask").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencilWriteMask = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilWriteMask").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->blendSource = (Kore::Graphics4::BlendingOperation)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->blendDestination = (Kore::Graphics4::BlendingOperation)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alphaBlendSource = (Kore::Graphics4::BlendingOperation)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alphaBlendDestination = (Kore::Graphics4::BlendingOperation)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		Local<Object> maskRedArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskRed").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskGreenArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskGreen").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskBlueArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskBlue").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskAlphaArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskAlpha").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		for (int i = 0; i < 8; ++i) {
			pipeline->colorWriteMaskRed[i] = maskRedArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->colorWriteMaskGreen[i] = maskGreenArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->colorWriteMaskBlue[i] = maskBlueArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->colorWriteMaskAlpha[i] = maskAlphaArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
		}

		pipeline->conservativeRasterization = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "conservativeRasterization").ToLocalChecked()).ToLocalChecked()->BooleanValue(isolate);

		pipeline->compile();
	}

	void krom_set_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> progobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> progfield = Local<External>::Cast(progobj->GetInternalField(0));
		Kore::Graphics4::PipelineState* pipeline = (Kore::Graphics4::PipelineState*)progfield->Value();
		Kore::Graphics4::setPipeline(pipeline);
	}

	void krom_load_image(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		bool readable = args[1]->ToBoolean(isolate)->Value();
		Kore::Graphics4::Texture* texture = new Kore::Graphics4::Texture(*utf8_value, readable);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->texWidth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->texHeight));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "filename").ToLocalChecked(), args[0]);
		args.GetReturnValue().Set(obj);
	}

	void krom_unload_image(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNull() || args[0]->IsUndefined()) return;
		Local<Object> image = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> tex = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		Local<Value> rt = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();

		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)texfield->Value();
			delete texture;
		}
		else if (rt->IsObject()) {
			Local<External> rtfield = Local<External>::Cast(rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();
			delete renderTarget;
		}
	}

	void krom_load_sound(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		Kore::Sound* sound = new Kore::Sound(*utf8_value);
		Local<ArrayBuffer> buffer;
		ArrayBuffer::Contents content;
		buffer = ArrayBuffer::New(isolate, sound->size * 2 * sizeof(float));
		content = buffer->Externalize();
		float* to = (float*)content.Data();

		Kore::s16* left = (Kore::s16*)&sound->left[0];
		Kore::s16* right = (Kore::s16*)&sound->right[0];
		for (int i = 0; i < sound->size; i += 1) {
			to[i * 2 + 0] = (float)(left [i] / 32767.0);
			to[i * 2 + 1] = (float)(right[i] / 32767.0);
		}

		args.GetReturnValue().Set(buffer);
		delete sound;
	}

	void write_audio_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		float value = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		*(float*)&Kore::Audio2::buffer.data[Kore::Audio2::buffer.writeLocation] = value;
		Kore::Audio2::buffer.writeLocation += 4;
		if (Kore::Audio2::buffer.writeLocation >= Kore::Audio2::buffer.dataSize) Kore::Audio2::buffer.writeLocation = 0;
	}

	void krom_load_blob(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		Kore::FileReader reader;
		reader.open(*utf8_value);

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader.size());
		ArrayBuffer::Contents contents = buffer->Externalize();

		unsigned char* from = (unsigned char*)reader.readAll();
		unsigned char* to = (unsigned char*)contents.Data();
		for (int i = 0; i < reader.size(); ++i) {
			to[i] = from[i];
		}

		reader.close();

		args.GetReturnValue().Set(buffer);
	}

	void krom_get_constant_location(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> progfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::PipelineState* pipeline = (Kore::Graphics4::PipelineState*)progfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		Kore::Graphics4::ConstantLocation location = pipeline->getConstantLocation(*utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, new Kore::Graphics4::ConstantLocation(location)));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> progfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::PipelineState* pipeline = (Kore::Graphics4::PipelineState*)progfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		Kore::Graphics4::TextureUnit unit = pipeline->getTextureUnit(*utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, new Kore::Graphics4::TextureUnit(unit)));
		args.GetReturnValue().Set(obj);
	}

	void krom_set_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();

		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)texfield->Value();

		Kore::Graphics4::setTexture(*unit, texture);
	}

	void krom_set_render_target(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();

		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();
		
		renderTarget->useColorAsTexture(*unit);
	}

	void krom_set_texture_depth(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();

		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();
		
		renderTarget->useDepthAsTexture(*unit);
	}

	void krom_set_image_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();

		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)texfield->Value();

		Kore::Graphics4::setImageTexture(*unit, texture);
	}

	Kore::Graphics4::TextureAddressing convertTextureAddressing(int index) {
		switch (index) {
		default:
		case 0: // Repeat
			return Kore::Graphics4::Repeat;
		case 1: // Mirror
			return Kore::Graphics4::Mirror;
		case 2: // Clamp
			return Kore::Graphics4::Clamp;
		}
	}

	Kore::Graphics4::TextureFilter convertTextureFilter(int index) {
		switch (index) {
		default:
		case 0: // PointFilter
			return Kore::Graphics4::PointFilter;
		case 1: // LinearFilter
			return Kore::Graphics4::LinearFilter;
		case 2: // AnisotropicFilter
			return Kore::Graphics4::AnisotropicFilter;
		}
	}

	Kore::Graphics4::MipmapFilter convertMipmapFilter(int index) {
		switch (index) {
		default:
		case 0: // NoMipFilter
			return Kore::Graphics4::NoMipFilter;
		case 1: // PointMipFilter
			return Kore::Graphics4::PointMipFilter;
		case 2: // LinearMipFilter
			return Kore::Graphics4::LinearMipFilter;
		}
	}

	void krom_set_texture_parameters(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();
		Kore::Graphics4::setTextureAddressing(*unit, Kore::Graphics4::U, convertTextureAddressing(args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTextureAddressing(*unit, Kore::Graphics4::V, convertTextureAddressing(args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTextureMinificationFilter(*unit, convertTextureFilter(args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTextureMagnificationFilter(*unit, convertTextureFilter(args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTextureMipmapFilter(*unit, convertMipmapFilter(args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
	}

	void krom_set_texture_3d_parameters(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();
		Kore::Graphics4::setTexture3DAddressing(*unit, Kore::Graphics4::U, convertTextureAddressing(args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTexture3DAddressing(*unit, Kore::Graphics4::V, convertTextureAddressing(args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTexture3DAddressing(*unit, Kore::Graphics4::W, convertTextureAddressing(args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTexture3DMinificationFilter(*unit, convertTextureFilter(args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTexture3DMagnificationFilter(*unit, convertTextureFilter(args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Graphics4::setTexture3DMipmapFilter(*unit, convertMipmapFilter(args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
	}

	void krom_set_texture_compare_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();
		Kore::Graphics4::setTextureCompareMode(*unit, args[1]->ToBoolean(isolate)->Value());
	}

	void krom_set_cube_map_compare_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::TextureUnit* unit = (Kore::Graphics4::TextureUnit*)unitfield->Value();
		Kore::Graphics4::setCubeMapCompareMode(*unit, args[1]->ToBoolean(isolate)->Value());
	}

	void krom_set_bool(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::setBool(*location, value != 0);
	}

	void krom_set_int(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::setInt(*location, value);
	}

	void krom_set_float(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::setFloat(*location, value);
	}

	void krom_set_float2(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::setFloat2(*location, value1, value2);
	}

	void krom_set_float3(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::setFloat3(*location, value1, value2, value3);
	}

	void krom_set_float4(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Graphics4::setFloat4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		float* from = (float*)content.Data();

		Kore::Graphics4::setFloats(*location, from, int(content.ByteLength() / 4));
	}

	void krom_set_matrix(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		float* from = (float*)content.Data();
		Kore::mat4 m;
		m.Set(0, 0, from[0]); m.Set(1, 0, from[1]); m.Set(2, 0, from[2]); m.Set(3, 0, from[3]);
		m.Set(0, 1, from[4]); m.Set(1, 1, from[5]); m.Set(2, 1, from[6]); m.Set(3, 1, from[7]);
		m.Set(0, 2, from[8]); m.Set(1, 2, from[9]); m.Set(2, 2, from[10]); m.Set(3, 2, from[11]);
		m.Set(0, 3, from[12]); m.Set(1, 3, from[13]); m.Set(2, 3, from[14]); m.Set(3, 3, from[15]);

		Kore::Graphics4::setMatrix(*location, m);
	}

	void krom_set_matrix3(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::ConstantLocation* location = (Kore::Graphics4::ConstantLocation*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		float* from = (float*)content.Data();
		Kore::mat3 m;
		m.Set(0, 0, from[0]); m.Set(1, 0, from[1]); m.Set(2, 0, from[2]);
		m.Set(0, 1, from[3]); m.Set(1, 1, from[4]); m.Set(2, 1, from[5]);
		m.Set(0, 2, from[6]); m.Set(1, 2, from[7]); m.Set(2, 2, from[8]);

		Kore::Graphics4::setMatrix(*location, m);
	}

	void krom_get_time(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Number::New(isolate, Kore::System::time()));
	}

	void krom_window_width(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, Kore::System::windowWidth(windowId)));
	}

	void krom_window_height(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, Kore::System::windowHeight(windowId)));
	}

	void krom_set_window_title(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value title(isolate, args[1]);
		Kore::Window::get(windowId)->setTitle(*title);
	}

	void krom_screen_dpi(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, Kore::Display::primary()->pixelsPerInch()));
	}

	void krom_system_id(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, Kore::System::systemId()).ToLocalChecked());
	}

	void krom_request_shutdown(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::System::stop();
	}

	void krom_display_count(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, Kore::Display::count()));
	}

	void krom_display_width(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, Kore::Display::get(index)->width()));
	}

	void krom_display_height(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, Kore::Display::get(index)->height()));
	}

	void krom_display_x(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, Kore::Display::get(index)->x()));
	}

	void krom_display_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, Kore::Display::get(index)->y()));
	}

	void krom_display_is_primary(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Boolean::New(isolate, Kore::Display::get(index) == Kore::Display::primary()));
	}

	void krom_write_storage(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();

		Kore::FileWriter writer;
		if (!writer.open(*utf8_name)) return;
		writer.write(content.Data(), (int)content.ByteLength());
	}

	void krom_read_storage(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		Kore::FileReader reader;
		if (!reader.open(*utf8_name, Kore::FileReader::Save)) return;

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader.size());
		ArrayBuffer::Contents contents = buffer->Externalize();
		unsigned char* from = (unsigned char*)reader.readAll();
		unsigned char* to = (unsigned char*)contents.Data();
		for (int i = 0; i < reader.size(); ++i) {
			to[i] = from[i];
		}
		reader.close();
		args.GetReturnValue().Set(buffer);
	}

	void krom_create_render_target(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Graphics4::RenderTarget* renderTarget = new Kore::Graphics4::RenderTarget(args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false, (Kore::Graphics4::RenderTargetFormat)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, renderTarget));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, renderTarget->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, renderTarget->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_render_target_cube_map(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Graphics4::RenderTarget* renderTarget = new Kore::Graphics4::RenderTarget(args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false, (Kore::Graphics4::RenderTargetFormat)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, renderTarget));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, renderTarget->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, renderTarget->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Graphics4::Texture* texture = new Kore::Graphics4::Texture(args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (Kore::Graphics4::Image::Format)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->texWidth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->texHeight));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_3d(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Kore::Graphics4::Texture* texture = new Kore::Graphics4::Texture(args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (Kore::Graphics4::Image::Format)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->depth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->texWidth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->texHeight));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::Graphics4::Texture* texture = new Kore::Graphics4::Texture(content.Data(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (Kore::Graphics4::Image::Format)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[4]->ToBoolean(isolate)->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->texWidth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->texHeight));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes_3d(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::Graphics4::Texture* texture = new Kore::Graphics4::Texture(content.Data(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (Kore::Graphics4::Image::Format)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[5]->ToBoolean(isolate)->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->depth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->texWidth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->texHeight));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_encoded_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		String::Utf8Value format(isolate, args[1]);
		Kore::Graphics4::Texture* texture = new Kore::Graphics4::Texture(content.Data(), (int)content.ByteLength(), *format, args[2]->ToBoolean(isolate)->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->texWidth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->texHeight));
		args.GetReturnValue().Set(obj);
	}

	int formatByteSize(Kore::Graphics4::Image::Format format) {
		switch (format) {
		case Kore::Graphics4::Image::RGBA128:
			return 16;
		case Kore::Graphics4::Image::RGBA64:
			return 8;
		case Kore::Graphics4::Image::RGB24:
			return 4;
		case Kore::Graphics4::Image::A32:
			return 4;
		case Kore::Graphics4::Image::A16:
			return 2;
		case Kore::Graphics4::Image::Grey8:
			return 1;
		case Kore::Graphics4::Image::BGRA32:
		case Kore::Graphics4::Image::RGBA32:
		default:
			return 4;
		}
	}

	void krom_get_texture_pixels(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)field->Value();

		Kore::u8* data = texture->getPixels();
		int byteLength = formatByteSize(texture->format) * texture->width * texture->height * texture->depth;
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, data, byteLength);
		args.GetReturnValue().Set(buffer);
	}

	void krom_get_render_target_pixels(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* rt = (Kore::Graphics4::RenderTarget*)field->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();

		Kore::u8* b = (Kore::u8*)content.Data();
		rt->getPixels(b);
	}

	void krom_lock_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)field->Value();
		Kore::u8* tex = texture->lock();

		int byteLength = formatByteSize(texture->format) * texture->width * texture->height * texture->depth;
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, tex, byteLength);
		args.GetReturnValue().Set(abuffer);
	}

	void krom_unlock_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)field->Value();
		texture->unlock();
	}

	void krom_clear_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)field->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int width = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int depth = args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		texture->clear(x, y, z, width, height, depth, color);
	}

	void krom_generate_texture_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)field->Value();
		texture->generateMipmaps(args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_generate_render_target_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* rt = (Kore::Graphics4::RenderTarget*)field->Value();
		rt->generateMipmaps(args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_set_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)field->Value();

		Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> mipmapobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> mipmapfield = Local<External>::Cast(mipmapobj->GetInternalField(0));
			Kore::Graphics4::Texture* mipmap = (Kore::Graphics4::Texture*)mipmapfield->Value();
			texture->setMipmap(mipmap, i + 1);
		}
	}

	void krom_set_depth_stencil_from(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> targetfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)targetfield->Value();
		Local<External> sourcefield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* sourceTarget = (Kore::Graphics4::RenderTarget*)sourcefield->Value();
		renderTarget->setDepthStencilFrom(sourceTarget);
	}

	void krom_viewport(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		Kore::Graphics4::viewport(x, y, w, h);
	}

	void krom_scissor(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		Kore::Graphics4::scissor(x, y, w, h);
	}

	void krom_disable_scissor(const FunctionCallbackInfo<Value>& args) {
		Kore::Graphics4::disableScissor();
	}

	void krom_render_targets_inverted_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, Kore::Graphics4::renderTargetsInvertedY()));
	}

	void krom_begin(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNull() || args[0]->IsUndefined()) {
			Kore::Graphics4::restoreRenderTarget();
		}
		else {
			Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
			Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();

			if (args[1]->IsNull() || args[1]->IsUndefined()) {
				Kore::Graphics4::setRenderTarget(renderTarget);
			}
			else {
				Kore::Graphics4::RenderTarget* renderTargets[8] = { renderTarget, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
				Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
				if (length > 7) length = 7;
				for (int32_t i = 0; i < length; ++i) {
					Local<Object> artobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
					Local<External> artfield = Local<External>::Cast(artobj->GetInternalField(0));
					Kore::Graphics4::RenderTarget* art = (Kore::Graphics4::RenderTarget*)artfield->Value();
					renderTargets[i + 1] = art;
				}
				Kore::Graphics4::setRenderTargets(renderTargets, length + 1);
			}
		}
	}

	void krom_begin_face(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
		Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();
		int face = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		Kore::Graphics4::setRenderTargetFace(renderTarget, face);
	}

	void krom_end(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
	}

	void krom_file_save_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_path(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();

		FILE* file = fopen(*utf8_path, "wb");
		if (file == nullptr) return;
		fwrite(content.Data(), 1, (int)content.ByteLength(), file);
		fclose(file);
	}

	void krom_sys_command(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_cmd(isolate, args[0]);
		int result = system(*utf8_cmd);
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_save_path(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, Kore::System::savePath()).ToLocalChecked());
	}

	void krom_get_arg_count(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, _argc));
	}

	void krom_get_arg(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, _argv[index]).ToLocalChecked());
	}

	void krom_get_files_location(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_internal_get_files_location()).ToLocalChecked());
	}

	void krom_set_bool_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Compute::setBool(*location, value != 0);
	}

	void krom_set_int_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Compute::setInt(*location, value);
	}

	void krom_set_float_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Compute::setFloat(*location, value);
	}

	void krom_set_float2_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Compute::setFloat2(*location, value1, value2);
	}

	void krom_set_float3_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Compute::setFloat3(*location, value1, value2, value3);
	}

	void krom_set_float4_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Compute::setFloat4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		float* from = (float*)content.Data();

		Kore::Compute::setFloats(*location, from, int(content.ByteLength() / 4));
	}

	void krom_set_matrix_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		float* from = (float*)content.Data();
		Kore::mat4 m;
		m.Set(0, 0, from[0]); m.Set(1, 0, from[1]); m.Set(2, 0, from[2]); m.Set(3, 0, from[3]);
		m.Set(0, 1, from[4]); m.Set(1, 1, from[5]); m.Set(2, 1, from[6]); m.Set(3, 1, from[7]);
		m.Set(0, 2, from[8]); m.Set(1, 2, from[9]); m.Set(2, 2, from[10]); m.Set(3, 2, from[11]);
		m.Set(0, 3, from[12]); m.Set(1, 3, from[13]); m.Set(2, 3, from[14]); m.Set(3, 3, from[15]);

		Kore::Compute::setMatrix(*location, m);
	}

	void krom_set_matrix3_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeConstantLocation* location = (Kore::ComputeConstantLocation*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		float* from = (float*)content.Data();
		Kore::mat3 m;
		m.Set(0, 0, from[0]); m.Set(1, 0, from[1]); m.Set(2, 0, from[2]);
		m.Set(0, 1, from[3]); m.Set(1, 1, from[4]); m.Set(2, 1, from[5]);
		m.Set(0, 2, from[6]); m.Set(1, 2, from[7]); m.Set(2, 2, from[8]);

		Kore::Compute::setMatrix(*location, m);
	}

	void krom_set_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeTextureUnit* unit = (Kore::ComputeTextureUnit*)unitfield->Value();

		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)texfield->Value();

		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		Kore::Compute::setTexture(*unit, texture, (Kore::Compute::Access)access);
	}

	void krom_set_render_target_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeTextureUnit* unit = (Kore::ComputeTextureUnit*)unitfield->Value();

		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();

		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		Kore::Compute::setTexture(*unit, renderTarget, (Kore::Compute::Access)access);
	}

	void krom_set_sampled_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeTextureUnit* unit = (Kore::ComputeTextureUnit*)unitfield->Value();

		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::Texture* texture = (Kore::Graphics4::Texture*)texfield->Value();

		Kore::Compute::setSampledTexture(*unit, texture);
	}

	void krom_set_sampled_render_target_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeTextureUnit* unit = (Kore::ComputeTextureUnit*)unitfield->Value();

		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();

		Kore::Compute::setSampledTexture(*unit, renderTarget);
	}

	void krom_set_sampled_depth_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeTextureUnit* unit = (Kore::ComputeTextureUnit*)unitfield->Value();

		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::Graphics4::RenderTarget* renderTarget = (Kore::Graphics4::RenderTarget*)rtfield->Value();

		Kore::Compute::setSampledDepthTexture(*unit, renderTarget);
	}

	void krom_set_texture_parameters_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeTextureUnit* unit = (Kore::ComputeTextureUnit*)unitfield->Value();
		Kore::Compute::setTextureAddressing(*unit, Kore::Graphics4::U, convertTextureAddressing(args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTextureAddressing(*unit, Kore::Graphics4::V, convertTextureAddressing(args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTextureMinificationFilter(*unit, convertTextureFilter(args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTextureMagnificationFilter(*unit, convertTextureFilter(args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTextureMipmapFilter(*unit, convertMipmapFilter(args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
	}

	void krom_set_texture_3d_parameters_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeTextureUnit* unit = (Kore::ComputeTextureUnit*)unitfield->Value();
		Kore::Compute::setTexture3DAddressing(*unit, Kore::Graphics4::U, convertTextureAddressing(args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTexture3DAddressing(*unit, Kore::Graphics4::V, convertTextureAddressing(args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTexture3DAddressing(*unit, Kore::Graphics4::W, convertTextureAddressing(args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTexture3DMinificationFilter(*unit, convertTextureFilter(args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTexture3DMagnificationFilter(*unit, convertTextureFilter(args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
		Kore::Compute::setTexture3DMipmapFilter(*unit, convertMipmapFilter(args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust()));
	}

	void krom_set_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeShader* shader = (Kore::ComputeShader*)shaderfield->Value();
		Kore::Compute::setShader(shader);
	}

	void krom_create_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		Kore::ComputeShader* shader = new Kore::ComputeShader(content.Data(), (int)content.ByteLength());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> shaderobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> shaderfield = Local<External>::Cast(shaderobj->GetInternalField(0));
		Kore::ComputeShader* shader = (Kore::ComputeShader*)shaderfield->Value();
		delete shader;
	}

	void krom_get_constant_location_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeShader* shader = (Kore::ComputeShader*)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		Kore::ComputeConstantLocation location = shader->getConstantLocation(*utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, new Kore::ComputeConstantLocation(location)));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		Kore::ComputeShader* shader = (Kore::ComputeShader*)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		Kore::ComputeTextureUnit unit = shader->getTextureUnit(*utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, new Kore::ComputeTextureUnit(unit)));
		args.GetReturnValue().Set(obj);
	}

	void krom_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Kore::Compute::compute(x, y, z);
	}

	void krom_open_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value filterList(isolate, args[0]);
		String::Utf8Value defaultPath(isolate, args[1]);
		nfdchar_t *outPath = NULL;
		nfdresult_t result = NFD_OpenDialog(*filterList, *defaultPath, &outPath);
		if (result == NFD_OKAY) {
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, outPath).ToLocalChecked());
			free(outPath);
		}
		else if (result == NFD_CANCEL) {}
		else {
			Kore::log(Kore::Info, "Error: %s\n", NFD_GetError());
		}
	}

	void krom_save_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value filterList(isolate, args[0]);
		String::Utf8Value defaultPath(isolate, args[1]);
		nfdchar_t *outPath = NULL;
		nfdresult_t result = NFD_SaveDialog(*filterList, *defaultPath, &outPath);
		if (result == NFD_OKAY) {
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, outPath).ToLocalChecked());
			free(outPath);
		}
		else if (result == NFD_CANCEL) {}
		else {
			Kore::log(Kore::Info, "Error: %s\n", NFD_GetError());
		}
	}

	void krom_set_save_and_quit_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		saveAndQuitFunction.Reset(isolate, func);
	}

	void startV8(const char* bindir) {
#if defined(KORE_MACOS)
		char filepath[256];
		strcpy(filepath, macgetresourcepath());
		strcat(filepath, "/");
		strcat(filepath, "macos");
		strcat(filepath, "/");
		V8::InitializeExternalStartupData(filepath);
#else
		// V8::InitializeExternalStartupData(bindir);
		// V8::InitializeExternalStartupData("./");
#endif

		plat = platform::NewDefaultPlatform();
		V8::InitializePlatform(plat.get());
		V8::Initialize();

		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		isolate = Isolate::New(create_params);

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);

		Local<ObjectTemplate> krom = ObjectTemplate::New(isolate);
		krom->Set(String::NewFromUtf8(isolate, "init").ToLocalChecked(), FunctionTemplate::New(isolate, krom_init));
		krom->Set(String::NewFromUtf8(isolate, "log").ToLocalChecked(), FunctionTemplate::New(isolate, krom_log));
		krom->Set(String::NewFromUtf8(isolate, "clear").ToLocalChecked(), FunctionTemplate::New(isolate, graphics_clear));
		krom->Set(String::NewFromUtf8(isolate, "setCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_callback));
		krom->Set(String::NewFromUtf8(isolate, "setDropFilesCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_drop_files_callback));
		krom->Set(String::NewFromUtf8(isolate, "setCutCopyPasteCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_cut_copy_paste_callback));
		krom->Set(String::NewFromUtf8(isolate, "setApplicationStateCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_application_state_callback));
		krom->Set(String::NewFromUtf8(isolate, "setKeyboardDownCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_keyboard_down_callback));
		krom->Set(String::NewFromUtf8(isolate, "setKeyboardUpCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_keyboard_up_callback));
		krom->Set(String::NewFromUtf8(isolate, "setKeyboardPressCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_keyboard_press_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseDownCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_down_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseUpCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_up_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseMoveCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_move_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseWheelCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_wheel_callback));
		krom->Set(String::NewFromUtf8(isolate, "setPenDownCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pen_down_callback));
		krom->Set(String::NewFromUtf8(isolate, "setPenUpCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pen_up_callback));
		krom->Set(String::NewFromUtf8(isolate, "setPenMoveCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pen_move_callback));
		krom->Set(String::NewFromUtf8(isolate, "setGamepadAxisCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_gamepad_axis_callback));
		krom->Set(String::NewFromUtf8(isolate, "setGamepadButtonCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_gamepad_button_callback));
		krom->Set(String::NewFromUtf8(isolate, "lockMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_mouse));
		krom->Set(String::NewFromUtf8(isolate, "unlockMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_mouse));
		krom->Set(String::NewFromUtf8(isolate, "canLockMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_can_lock_mouse));
		krom->Set(String::NewFromUtf8(isolate, "isMouseLocked").ToLocalChecked(), FunctionTemplate::New(isolate, krom_is_mouse_locked));
		krom->Set(String::NewFromUtf8(isolate, "showMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_show_mouse));
		krom->Set(String::NewFromUtf8(isolate, "createIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "deleteIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "lockIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_index_buffer));
		krom->Set(String::NewFromUtf8(isolate, "unlockIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_index_buffer));
		krom->Set(String::NewFromUtf8(isolate, "setIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "createVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_vertexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "deleteVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_vertexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "lockVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_vertex_buffer));
		krom->Set(String::NewFromUtf8(isolate, "unlockVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_vertex_buffer));
		krom->Set(String::NewFromUtf8(isolate, "setVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_vertexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "setVertexBuffers").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_vertexbuffers));
		krom->Set(String::NewFromUtf8(isolate, "drawIndexedVertices").ToLocalChecked(), FunctionTemplate::New(isolate, krom_draw_indexed_vertices));
		krom->Set(String::NewFromUtf8(isolate, "drawIndexedVerticesInstanced").ToLocalChecked(), FunctionTemplate::New(isolate, krom_draw_indexed_vertices_instanced));
		krom->Set(String::NewFromUtf8(isolate, "createVertexShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_vertex_shader));
		krom->Set(String::NewFromUtf8(isolate, "createVertexShaderFromSource").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_vertex_shader_from_source));
		krom->Set(String::NewFromUtf8(isolate, "createFragmentShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_fragment_shader));
		krom->Set(String::NewFromUtf8(isolate, "createFragmentShaderFromSource").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_fragment_shader_from_source));
		krom->Set(String::NewFromUtf8(isolate, "createGeometryShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_geometry_shader));
		krom->Set(String::NewFromUtf8(isolate, "createTessellationControlShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_tessellation_control_shader));
		krom->Set(String::NewFromUtf8(isolate, "createTessellationEvaluationShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_tessellation_evaluation_shader));
		krom->Set(String::NewFromUtf8(isolate, "deleteShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_shader));
		krom->Set(String::NewFromUtf8(isolate, "createPipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "deletePipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "compilePipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_compile_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "setPipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "loadImage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_image));
		krom->Set(String::NewFromUtf8(isolate, "unloadImage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unload_image));
		krom->Set(String::NewFromUtf8(isolate, "loadSound").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_sound));
		krom->Set(String::NewFromUtf8(isolate, "setAudioCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_audio_callback));
		krom->Set(String::NewFromUtf8(isolate, "audioThread").ToLocalChecked(), FunctionTemplate::New(isolate, audio_thread));
		krom->Set(String::NewFromUtf8(isolate, "writeAudioBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, write_audio_buffer));
		krom->Set(String::NewFromUtf8(isolate, "loadBlob").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_blob));
		krom->Set(String::NewFromUtf8(isolate, "getConstantLocation").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_constant_location));
		krom->Set(String::NewFromUtf8(isolate, "getTextureUnit").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_unit));
		krom->Set(String::NewFromUtf8(isolate, "setTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture));
		krom->Set(String::NewFromUtf8(isolate, "setRenderTarget").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_render_target));
		krom->Set(String::NewFromUtf8(isolate, "setTextureDepth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_depth));
		krom->Set(String::NewFromUtf8(isolate, "setImageTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_image_texture));
		krom->Set(String::NewFromUtf8(isolate, "setTextureParameters").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_parameters));
		krom->Set(String::NewFromUtf8(isolate, "setTexture3DParameters").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_3d_parameters));
		krom->Set(String::NewFromUtf8(isolate, "setTextureCompareMode").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_compare_mode));
		krom->Set(String::NewFromUtf8(isolate, "setCubeMapCompareMode").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_cube_map_compare_mode));
		krom->Set(String::NewFromUtf8(isolate, "setBool").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_bool));
		krom->Set(String::NewFromUtf8(isolate, "setInt").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_int));
		krom->Set(String::NewFromUtf8(isolate, "setFloat").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float));
		krom->Set(String::NewFromUtf8(isolate, "setFloat2").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float2));
		krom->Set(String::NewFromUtf8(isolate, "setFloat3").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float3));
		krom->Set(String::NewFromUtf8(isolate, "setFloat4").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float4));
		krom->Set(String::NewFromUtf8(isolate, "setFloats").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_floats));
		krom->Set(String::NewFromUtf8(isolate, "setMatrix").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix));
		krom->Set(String::NewFromUtf8(isolate, "setMatrix3").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix3));
		krom->Set(String::NewFromUtf8(isolate, "getTime").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_time));
		krom->Set(String::NewFromUtf8(isolate, "windowWidth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_width));
		krom->Set(String::NewFromUtf8(isolate, "windowHeight").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_height));
		krom->Set(String::NewFromUtf8(isolate, "setWindowTitle").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_window_title));
		krom->Set(String::NewFromUtf8(isolate, "screenDpi").ToLocalChecked(), FunctionTemplate::New(isolate, krom_screen_dpi));
		krom->Set(String::NewFromUtf8(isolate, "systemId").ToLocalChecked(), FunctionTemplate::New(isolate, krom_system_id));
		krom->Set(String::NewFromUtf8(isolate, "requestShutdown").ToLocalChecked(), FunctionTemplate::New(isolate, krom_request_shutdown));
		krom->Set(String::NewFromUtf8(isolate, "displayCount").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_count));
		krom->Set(String::NewFromUtf8(isolate, "displayWidth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_width));
		krom->Set(String::NewFromUtf8(isolate, "displayHeight").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_height));
		krom->Set(String::NewFromUtf8(isolate, "displayX").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_x));
		krom->Set(String::NewFromUtf8(isolate, "displayY").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_y));
		krom->Set(String::NewFromUtf8(isolate, "displayIsPrimary").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_is_primary));
		krom->Set(String::NewFromUtf8(isolate, "writeStorage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_write_storage));
		krom->Set(String::NewFromUtf8(isolate, "readStorage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_read_storage));
		krom->Set(String::NewFromUtf8(isolate, "createRenderTarget").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_render_target));
		krom->Set(String::NewFromUtf8(isolate, "createRenderTargetCubeMap").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_render_target_cube_map));
		krom->Set(String::NewFromUtf8(isolate, "createTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture));
		krom->Set(String::NewFromUtf8(isolate, "createTexture3D").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_3d));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromBytes").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_bytes));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromBytes3D").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_bytes_3d));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromEncodedBytes").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_encoded_bytes));
		krom->Set(String::NewFromUtf8(isolate, "getTexturePixels").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_pixels));
		krom->Set(String::NewFromUtf8(isolate, "getRenderTargetPixels").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_render_target_pixels));
		krom->Set(String::NewFromUtf8(isolate, "lockTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_texture));
		krom->Set(String::NewFromUtf8(isolate, "unlockTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_texture));
		krom->Set(String::NewFromUtf8(isolate, "clearTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_clear_texture));
		krom->Set(String::NewFromUtf8(isolate, "generateTextureMipmaps").ToLocalChecked(), FunctionTemplate::New(isolate, krom_generate_texture_mipmaps));
		krom->Set(String::NewFromUtf8(isolate, "generateRenderTargetMipmaps").ToLocalChecked(), FunctionTemplate::New(isolate, krom_generate_render_target_mipmaps));
		krom->Set(String::NewFromUtf8(isolate, "setMipmaps").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mipmaps));
		krom->Set(String::NewFromUtf8(isolate, "setDepthStencilFrom").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_depth_stencil_from));
		krom->Set(String::NewFromUtf8(isolate, "viewport").ToLocalChecked(), FunctionTemplate::New(isolate, krom_viewport));
		krom->Set(String::NewFromUtf8(isolate, "scissor").ToLocalChecked(), FunctionTemplate::New(isolate, krom_scissor));
		krom->Set(String::NewFromUtf8(isolate, "disableScissor").ToLocalChecked(), FunctionTemplate::New(isolate, krom_disable_scissor));
		krom->Set(String::NewFromUtf8(isolate, "renderTargetsInvertedY").ToLocalChecked(), FunctionTemplate::New(isolate, krom_render_targets_inverted_y));
		krom->Set(String::NewFromUtf8(isolate, "begin").ToLocalChecked(), FunctionTemplate::New(isolate, krom_begin));
		krom->Set(String::NewFromUtf8(isolate, "beginFace").ToLocalChecked(), FunctionTemplate::New(isolate, krom_begin_face));
		krom->Set(String::NewFromUtf8(isolate, "end").ToLocalChecked(), FunctionTemplate::New(isolate, krom_end));
		krom->Set(String::NewFromUtf8(isolate, "fileSaveBytes").ToLocalChecked(), FunctionTemplate::New(isolate, krom_file_save_bytes));
		krom->Set(String::NewFromUtf8(isolate, "sysCommand").ToLocalChecked(), FunctionTemplate::New(isolate, krom_sys_command));
		krom->Set(String::NewFromUtf8(isolate, "savePath").ToLocalChecked(), FunctionTemplate::New(isolate, krom_save_path));
		krom->Set(String::NewFromUtf8(isolate, "getArgCount").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_arg_count));
		krom->Set(String::NewFromUtf8(isolate, "getArg").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_arg));
		krom->Set(String::NewFromUtf8(isolate, "getFilesLocation").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_files_location));
		krom->Set(String::NewFromUtf8(isolate, "setBoolCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_bool_compute));
		krom->Set(String::NewFromUtf8(isolate, "setIntCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_int_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloatCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloat2Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float2_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloat3Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float3_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloat4Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float4_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloatsCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_floats_compute));
		krom->Set(String::NewFromUtf8(isolate, "setMatrixCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix_compute));
		krom->Set(String::NewFromUtf8(isolate, "setMatrix3Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix3_compute));
		krom->Set(String::NewFromUtf8(isolate, "setTextureCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_compute));
		krom->Set(String::NewFromUtf8(isolate, "setRenderTargetCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_render_target_compute));
		krom->Set(String::NewFromUtf8(isolate, "setSampledTextureCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_sampled_texture_compute));
		krom->Set(String::NewFromUtf8(isolate, "setSampledRenderTargetCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_sampled_render_target_compute));
		krom->Set(String::NewFromUtf8(isolate, "setSampledDepthTextureCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_sampled_depth_texture_compute));
		krom->Set(String::NewFromUtf8(isolate, "setTextureParametersCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_parameters_compute));
		krom->Set(String::NewFromUtf8(isolate, "setTexture3DParametersCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_3d_parameters_compute));
		krom->Set(String::NewFromUtf8(isolate, "setShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "deleteShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "createShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "getConstantLocationCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_constant_location_compute));
		krom->Set(String::NewFromUtf8(isolate, "getTextureUnitCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_unit_compute));
		krom->Set(String::NewFromUtf8(isolate, "compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_compute));
		krom->Set(String::NewFromUtf8(isolate, "openDialog").ToLocalChecked(), FunctionTemplate::New(isolate, krom_open_dialog));
		krom->Set(String::NewFromUtf8(isolate, "saveDialog").ToLocalChecked(), FunctionTemplate::New(isolate, krom_save_dialog));
		krom->Set(String::NewFromUtf8(isolate, "setSaveAndQuitCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_save_and_quit_callback));

		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		global->Set(String::NewFromUtf8(isolate, "Krom").ToLocalChecked(), krom);

		Local<Context> context = Context::New(isolate, NULL, global);
		globalContext.Reset(isolate, context);
	}

	bool startKrom(char* scriptfile) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		Local<String> source = String::NewFromUtf8(isolate, scriptfile, NewStringType::kNormal).ToLocalChecked();

		TryCatch try_catch(isolate);
		Local<Script> compiled_script = Script::Compile(isolate->GetCurrentContext(), source).ToLocalChecked();

		Local<Value> result;
		if (!compiled_script->Run(context).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
			return false;
		}

		return true;
	}

	void runV8() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		v8::MicrotasksScope microtasks_scope(isolate, v8::MicrotasksScope::kRunMicrotasks);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<v8::Function> func = Local<v8::Function>::New(isolate, updateFunction);
		Local<Value> result;

		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}

		if (saveAndQuit) {
			v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, saveAndQuitFunction);
			Local<Value> result;
			if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
				v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
				sendLogMessage("Trace: %s", *stack_trace);
			}
			saveAndQuit = false;
		}
	}

	void endV8() {
		updateFunction.Reset();
		globalContext.Reset();
		isolate->Dispose();
		V8::Dispose();
		V8::ShutdownPlatform();
	}

	void initAudioBuffer() {
		for (int i = 0; i < Kore::Audio2::buffer.dataSize; i++) {
			*(float*)&Kore::Audio2::buffer.data[i] = 0;
		}
	}

	void updateAudio(int samples) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		v8::MicrotasksScope microtasks_scope(isolate, v8::MicrotasksScope::kRunMicrotasks);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<v8::Function> func = Local<v8::Function>::New(isolate, audioFunction);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, samples)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void mix(int samples) {
		//mutex.Lock();
		updateAudio(samples);
		//mutex.Unlock();
	}

	void update() {
		if (enableSound ) Kore::Audio2::update();
		Kore::Graphics4::begin();

		//mutex.Lock();
		runV8();
		//mutex.Unlock();

		Kore::Graphics4::end();
		Kore::Graphics4::swapBuffers();
	}

	void dropFiles(wchar_t* filePath) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, dropFilesFunction);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc];
		if (sizeof(wchar_t) == 2) {
			argv[0] = {String::NewFromTwoByte(isolate, (const uint16_t*)filePath).ToLocalChecked()};
		}
		else {
			size_t len = wcslen(filePath);
			uint16_t* str = new uint16_t[len + 1];
			for (int i = 0; i < len; i++) str[i] = filePath[i];
			str[len] = 0;
			argv[0] = {String::NewFromTwoByte(isolate, str).ToLocalChecked()};
			delete str;
		}
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	char cutCopyString[4096];

 	char* copy() {
 		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, copyFunction);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
		String::Utf8Value cutCopyString(isolate, result);
		return *cutCopyString;
	}

 	char* cut() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, cutFunction);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
		String::Utf8Value cutCopyString(isolate, result);
		return *cutCopyString;
	}

 	void paste(char* data) {
 		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pasteFunction);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {String::NewFromUtf8(isolate, data).ToLocalChecked()};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		} 		
	}

	void foreground() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, foregroundFunction);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void resume() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, resumeFunction);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void pause() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pauseFunction);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void background() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, backgroundFunction);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void shutdown() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, shutdownFunction);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void keyDown(Kore::KeyCode code) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboardDownFunction);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, (int)code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void keyUp(Kore::KeyCode code) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboardUpFunction);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, (int)code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

    void keyPress(wchar_t character) {
        v8::Locker locker{isolate};

        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
        Context::Scope context_scope(context);

        TryCatch try_catch(isolate);
        v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboardPressFunction);
        Local<Value> result;
        const int argc = 1;
        Local<Value> argv[argc] = {Int32::New(isolate, (int)character)};
        if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
            v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
            sendLogMessage("Trace: %s", *stack_trace);
        }
    }

	void mouseMove(int window, int x, int y, int mx, int my) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouseMoveFunction);
		Local<Value> result;
		const int argc = 4;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Int32::New(isolate, mx), Int32::New(isolate, my)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void mouseDown(int window, int button, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouseDownFunction);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void mouseUp(int window, int button, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouseUpFunction);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void mouseWheel(int window, int delta) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouseWheelFunction);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, delta)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void penDown(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, penDownFunction);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void penUp(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, penUpFunction);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void penMove(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, penMoveFunction);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void gamepadAxis(int gamepad, int axis, float value) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, gamepadAxisFunction);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, axis), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void gamepadButton(int gamepad, int button, float value) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, globalContext);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, gamepadButtonFunction);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, button), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			sendLogMessage("Trace: %s", *stack_trace);
		}
	}

	void gamepad1Axis(int axis, float value) {
		gamepadAxis(0, axis, value);
	}

	void gamepad1Button(int button, float value) {
		gamepadButton(0, button, value);
	}

	void gamepad2Axis(int axis, float value) {
		gamepadAxis(1, axis, value);
	}

	void gamepad2Button(int button, float value) {
		gamepadButton(1, button, value);
	}

	void gamepad3Axis(int axis, float value) {
		gamepadAxis(2, axis, value);
	}

	void gamepad3Button(int button, float value) {
		gamepadButton(2, button, value);
	}

	void gamepad4Axis(int axis, float value) {
		gamepadAxis(3, axis, value);
	}

	void gamepad4Button(int button, float value) {
		gamepadButton(3, button, value);
	}

	std::string assetsdir;
}

int kickstart(int argc, char** argv) {
	_argc = argc;
	_argv = argv;
	std::string bindir(argv[0]);
#ifdef KORE_WINDOWS
	bindir = bindir.substr(0, bindir.find_last_of("\\"));
#else
	bindir = bindir.substr(0, bindir.find_last_of("/"));
#endif
	assetsdir = argc > 1 ? argv[1] : bindir;
	// shadersdir = argc > 2 ? argv[2] : bindir;

	bool readStdoutPath = false;
	bool readConsolePid = false;
	for (int i = 3; i < argc; ++i) {
		if (strcmp(argv[i], "--sound") == 0) {
			enableSound = true;
		}
		else if (strcmp(argv[i], "--nowindow") == 0) {
			nowindow = true;
		}
		else if (readStdoutPath) {
			freopen(argv[i], "w", stdout);
			readStdoutPath = false;
		}
		else if (strcmp(argv[i], "--stdout") == 0) {
			readStdoutPath = true;
		}
		else if (readConsolePid) {
			#ifdef KORE_WINDOWS
			AttachConsole(atoi(argv[i]));
			#endif
			readConsolePid = false;
		}
		else if (strcmp(argv[i], "--consolepid") == 0) {
			readConsolePid = true;
		}
	}

	kinc_internal_set_files_location(&assetsdir[0u]);

	Kore::FileReader reader;
	if (!reader.open("krom.bin")) {
		if (!reader.open("krom.js")) {
			fprintf(stderr, "Could not load krom.js, aborting.\n");
			exit(1);
		}
	}

	char* code = new char[reader.size() + 1];
	memcpy(code, reader.readAll(), reader.size());
	code[reader.size()] = 0;
	reader.close();

	#ifdef KORE_WINDOWS
	char dirsep = '\\';
	#else
	char dirsep = '/';
	#endif
	startV8((bindir + dirsep).c_str());

	Kore::threadsInit();

	startKrom(code);
	Kore::System::start();

	exit(0); // TODO
	endV8();
	return 0;
}
