#include <kinc/log.h>
#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>
#include <kinc/audio2/audio.h>
#include <kinc/audio1/sound.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <kinc/display.h>
#include <kinc/input/mouse.h>
#include <kinc/input/surface.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/pen.h>
#include <kinc/input/gamepad.h>
#include <kinc/math/random.h>
#include <kinc/math/core.h>
#include <kinc/threads/thread.h>
#include <kinc/threads/mutex.h>
#include <kinc/network/http.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/compute/compute.h>
#include <kinc/libs/stb_image.h>
#ifdef KORE_LZ4X
extern "C" int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxOutputSize);
#else
#include <kinc/io/lz4/lz4.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <kinc/libs/stb_image.h>

#include <libplatform/libplatform.h>
#ifdef KORE_LINUX // xlib defines conflicting with v8
#undef True
#undef False
#undef None
#undef Status
#endif
#include <v8.h>

#include <map>
#include <string>
#include <vector>

#ifdef KORE_WINDOWS
#include <Windows.h> // AttachConsole
extern "C" { struct HWND__ *kinc_windows_window_handle(int window_index); } // Kore/Windows.h

//Enable visual styles for controls.
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifdef WITH_D3DCOMPILER
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <strstream>
#endif
#ifdef KORE_RAYTRACE
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/raytrace.h>
#endif
#ifdef KORE_VR
#include <kinc/vr/vrinterface.h>
#endif
#ifdef WITH_NFD
#include <nfd.h>
#elif KORE_ANDROID
#include "AndroidFileDialog.h"
#elif KORE_IOS
#include "IOSFileDialog.h"
#endif
#ifdef WITH_TINYDIR
#include <tinydir.h>
#endif
#ifdef WITH_ZLIB
#include <zlib.h>
#endif
#ifdef WITH_STB_IMAGE_WRITE
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif
#ifdef WITH_TEXSYNTH
#include <texsynth.h>
#endif
#ifdef WITH_ONNX
#include <onnxruntime_c_api.h>
#ifdef KORE_WINDOWS
#include <dml_provider_factory.h>
#endif
#endif
#ifdef IDLE_SLEEP
#include <unistd.h>
#endif
#ifdef WITH_WORKER
#include "worker.h"
#endif

#ifdef KORE_MACOS
extern const char* macgetresourcepath();
#endif

#ifdef KORE_IOS
extern const char* iphonegetresourcepath();
#endif

using namespace v8;

const int KROM_API = 5;

int save_and_quit = 0; // off, save, nosave
void armory_save_and_quit(bool save) { save_and_quit = save ? 1 : 2; }

#if defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)
extern void krafix_compile(const char* source, char* output, int* length, const char* targetlang, const char* system, const char* shadertype);
#endif

#ifdef KORE_RAYTRACE
#ifdef __cplusplus
extern "C" {
#endif
	extern kinc_g5_command_list_t commandList;
#ifdef __cplusplus
}
#endif
static kinc_g5_constant_buffer_t constant_buffer;
static kinc_g4_render_target_t* render_target;
static kinc_raytrace_pipeline_t pipeline;
static kinc_raytrace_acceleration_structure_t accel;
static bool accel_created = false;
const int constant_buffer_size = 24;
#endif

namespace {
	int _argc;
	char** _argv;
	bool enable_sound = true;
	bool enable_window = true;
	bool profile = false;
	bool snapshot = false;
	bool stderr_created = false;
	bool paused = false;
	int pausedFrames = 0;
	bool armorcore = false;

	Isolate* isolate;
	std::unique_ptr<Platform> plat;
	Global<Context> global_context;
	Global<Function> update_func;
	Global<Function> drop_files_func;
	Global<Function> cut_func;
	Global<Function> copy_func;
	Global<Function> paste_func;
	Global<Function> foreground_func;
	Global<Function> resume_func;
	Global<Function> pause_func;
	Global<Function> background_func;
	Global<Function> shutdown_func;
	Global<Function> keyboard_down_func;
	Global<Function> keyboard_up_func;
	Global<Function> keyboard_press_func;
	Global<Function> mouse_down_func;
	Global<Function> mouse_up_func;
	Global<Function> mouse_move_func;
	Global<Function> touch_down_func;
	Global<Function> touch_up_func;
	Global<Function> touch_move_func;
	Global<Function> mouse_wheel_func;
	Global<Function> pen_down_func;
	Global<Function> pen_up_func;
	Global<Function> pen_move_func;
	Global<Function> gamepad_axis_func;
	Global<Function> gamepad_button_func;
	Global<Function> audio_func;
	Global<Function> http_func;
	Global<Function> save_and_quit_func;

	kinc_mutex_t mutex;
	kinc_a2_buffer_t audio_buffer;
	int audio_read_location = 0;

	void update();
	void update_audio(kinc_a2_buffer_t *buffer, int samples);
	void drop_files(wchar_t* file_path);
	char* cut();
	char* copy();
	void paste(char* data);
	void foreground();
	void resume();
	void pause();
	void background();
	void shutdown();
	void key_down(int code);
	void key_up(int code);
	void key_press(unsigned int character);
	void mouse_move(int window, int x, int y, int mx, int my);
	void mouse_down(int window, int button, int x, int y);
	void mouse_up(int window, int button, int x, int y);
	void mouse_wheel(int window, int delta);
	void touch_move(int index, int x, int y);
	void touch_down(int index, int x, int y);
	void touch_up(int index, int x, int y);
	void pen_down(int window, int x, int y, float pressure);
	void pen_up(int window, int x, int y, float pressure);
	void pen_move(int window, int x, int y, float pressure);
	void gamepad_axis(int gamepad, int axis, float value);
	void gamepad_button(int gamepad, int button, float value);

	char temp_string[4096];
	char temp_string_vs[1024 * 1024];
	char temp_string_fs[1024 * 1024];
	char temp_string_vstruct[4][32][32];
	std::string assetsdir;
	#ifdef KORE_WINDOWS
	wchar_t temp_wstring[1024];
	wchar_t temp_wstring1[1024];
	#endif
	#ifdef ARM_PROFILE
	double startup_time = 0.0;
	#endif
	int32_t http_result_size = 0;

	void write_stack_trace(const char* stack_trace) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", stack_trace);

		#ifdef KORE_WINDOWS
		FILE* file = fopen("stderr.txt", stderr_created ? "a" : "w");
		if (file != nullptr) {
			stderr_created = true;
			fwrite(stack_trace, 1, strlen(stack_trace), file);
			fwrite("\n", 1, 1, file);
			fclose(file);
		}
		#endif
	}

	void handle_exception(TryCatch *try_catch) {
		MaybeLocal<Value> trace = try_catch->StackTrace(isolate->GetCurrentContext());
		if (trace.IsEmpty()) {
			v8::String::Utf8Value stack_trace(isolate, try_catch->Message()->Get());
			write_stack_trace(*stack_trace);
		}
		else {
			v8::String::Utf8Value stack_trace(isolate, trace.ToLocalChecked());
			write_stack_trace(*stack_trace);
		}
	}

	void krom_init(const v8::FunctionCallbackInfo<v8::Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value title(isolate, arg);
		int width = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int samples_per_pixel = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool vertical_sync = args[4]->ToBoolean(isolate)->Value();
		int window_mode = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int window_features = args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int api_version = args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int x = -1;
		int y = -1;
		int frequency = 60;
		if (args.Length() > 8) {
			armorcore = true;
			x = args[8]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			y = args[9]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			frequency = args[10]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		}

		if (api_version != KROM_API) {
			const char* outdated;
			if (api_version < KROM_API) {
				outdated = "Kha";
			}
			else if (KROM_API < api_version) {
				outdated = "Krom";
			}
			kinc_log(KINC_LOG_LEVEL_INFO, "Krom uses API version %i but Kha targets API version %i. Please update %s.", KROM_API, api_version, outdated);
			exit(1);
		}

		kinc_window_options_t win;
		win.title = *title;
		win.x = x;
		win.y = y;
		win.width = width;
		win.height = height;
		win.display_index = -1;
		win.visible = enable_window;
		win.window_features = window_features;
		win.mode = (kinc_window_mode_t)window_mode;
		kinc_framebuffer_options_t frame;
		frame.frequency = frequency;
		frame.vertical_sync = vertical_sync;
		frame.color_bits = 32;
		frame.depth_bits = 0;
		frame.stencil_bits = 0;
		frame.samples_per_pixel = samples_per_pixel;
		kinc_init(*title, width, height, &win, &frame);
		kinc_mutex_init(&mutex);
		kinc_random_init((int)(kinc_time() * 1000));

		#ifdef WITH_AUDIO
		if (enable_sound) {
			kinc_a2_init();
			kinc_a2_set_callback(update_audio);
			audio_buffer.read_location = 0;
			audio_buffer.write_location = 0;
			audio_buffer.data_size = 128 * 1024;
			audio_buffer.data = new uint8_t[audio_buffer.data_size];
		}
		#endif

		kinc_set_update_callback(update);
		kinc_set_drop_files_callback(drop_files);
		kinc_set_copy_callback(copy);
		kinc_set_cut_callback(cut);
		kinc_set_paste_callback(paste);
		kinc_set_foreground_callback(foreground);
		kinc_set_resume_callback(resume);
		kinc_set_pause_callback(pause);
		kinc_set_background_callback(background);
		kinc_set_shutdown_callback(shutdown);

		kinc_keyboard_key_down_callback = key_down;
		kinc_keyboard_key_up_callback = key_up;
		kinc_keyboard_key_press_callback = key_press;
		kinc_mouse_move_callback = mouse_move;
		kinc_mouse_press_callback = mouse_down;
		kinc_mouse_release_callback = mouse_up;
		kinc_mouse_scroll_callback = mouse_wheel;
		kinc_surface_move_callback = touch_move;
		kinc_surface_touch_start_callback = touch_down;
		kinc_surface_touch_end_callback = touch_up;
		kinc_pen_press_callback = pen_down;
		kinc_pen_move_callback = pen_move;
		kinc_pen_release_callback = pen_up;
		kinc_gamepad_axis_callback = gamepad_axis;
		kinc_gamepad_button_callback = gamepad_button;

		#if KORE_ANDROID
		android_check_permissions();
		#endif
	}

	void krom_set_application_name(const v8::FunctionCallbackInfo<v8::Value>& args) {
		// Name used by kinc_internal_save_path()
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[0]);
		kinc_set_application_name(*name);
	}

	void krom_log(const v8::FunctionCallbackInfo<v8::Value>& args) {
		if (args.Length() < 1) return;
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value value(isolate, arg);
		size_t len = strlen(*value);
		if (len < 2048) {
			kinc_log(KINC_LOG_LEVEL_INFO, *value);
		}
		else {
			int pos = 0;
			while (pos < len) {
				strncpy(temp_string, *value + pos, 2047);
				temp_string[2047] = 0;
				kinc_log(KINC_LOG_LEVEL_INFO, temp_string);
				pos += 2047;
			}
		}
	}

	void krom_clear(const v8::FunctionCallbackInfo<v8::Value>& args) {
		HandleScope scope(args.GetIsolate());
		int flags = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float depth = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int stencil = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_clear(flags, color, depth, stencil);
	}

	void krom_set_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		update_func.Reset(isolate, func);
	}

	void krom_set_drop_files_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		drop_files_func.Reset(isolate, func);
	}

	void krom_set_cut_copy_paste_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> cutArg = args[0];
		Local<Function> cutFunc = Local<Function>::Cast(cutArg);
		cut_func.Reset(isolate, cutFunc);
		Local<Value> copyArg = args[1];
		Local<Function> copyFunc = Local<Function>::Cast(copyArg);
		copy_func.Reset(isolate, copyFunc);
		Local<Value> pasteArg = args[2];
		Local<Function> pasteFunc = Local<Function>::Cast(pasteArg);
		paste_func.Reset(isolate, pasteFunc);
	}

	void krom_set_application_state_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> foregroundArg = args[0];
		Local<Function> foregroundFunc = Local<Function>::Cast(foregroundArg);
		foreground_func.Reset(isolate, foregroundFunc);
		Local<Value> resumeArg = args[1];
		Local<Function> resumeFunc = Local<Function>::Cast(resumeArg);
		resume_func.Reset(isolate, resumeFunc);
		Local<Value> pauseArg = args[2];
		Local<Function> pauseFunc = Local<Function>::Cast(pauseArg);
		pause_func.Reset(isolate, pauseFunc);
		Local<Value> backgroundArg = args[3];
		Local<Function> backgroundFunc = Local<Function>::Cast(backgroundArg);
		background_func.Reset(isolate, backgroundFunc);
		Local<Value> shutdownArg = args[4];
		Local<Function> shutdownFunc = Local<Function>::Cast(shutdownArg);
		shutdown_func.Reset(isolate, shutdownFunc);
	}

	void krom_set_keyboard_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_down_func.Reset(isolate, func);
	}

	void krom_set_keyboard_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_up_func.Reset(isolate, func);
	}

	void krom_set_keyboard_press_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_press_func.Reset(isolate, func);
	}

	void krom_set_mouse_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_down_func.Reset(isolate, func);
	}

	void krom_set_mouse_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_up_func.Reset(isolate, func);
	}

	void krom_set_mouse_move_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_move_func.Reset(isolate, func);
	}

	void krom_set_touch_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		touch_down_func.Reset(isolate, func);
	}

	void krom_set_touch_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		touch_up_func.Reset(isolate, func);
	}

	void krom_set_touch_move_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		touch_move_func.Reset(isolate, func);
	}

	void krom_set_mouse_wheel_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_wheel_func.Reset(isolate, func);
	}

	void krom_set_pen_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_down_func.Reset(isolate, func);
	}

	void krom_set_pen_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_up_func.Reset(isolate, func);
	}

	void krom_set_pen_move_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_move_func.Reset(isolate, func);
	}

	void krom_set_gamepad_axis_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepad_axis_func.Reset(isolate, func);
	}

	void krom_set_gamepad_button_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepad_button_func.Reset(isolate, func);
	}

	void krom_lock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_mouse_lock(0);
	}

	void krom_unlock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_mouse_unlock(0);
	}

	void krom_can_lock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, kinc_mouse_can_lock(0)));
	}

	void krom_is_mouse_locked(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, kinc_mouse_is_locked(0)));
	}

	void krom_set_mouse_position(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_mouse_set_position(0, x, y);
	}

	void krom_show_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args[0]->ToBoolean(isolate)->Value() ? kinc_mouse_show() : kinc_mouse_hide();
	}

	void krom_show_keyboard(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args[0]->ToBoolean(isolate)->Value() ? kinc_keyboard_show() : kinc_keyboard_hide();
	}

	void krom_set_audio_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		audio_func.Reset(isolate, func);
	}

	void krom_audio_thread(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		bool lock = args[0]->ToBoolean(isolate)->Value();
		if (lock) kinc_mutex_lock(&mutex);    //v8::Locker::Locker(isolate);
		else kinc_mutex_unlock(&mutex);       //v8::Unlocker(args.GetIsolate());
	}

	void krom_create_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)malloc(sizeof(kinc_g4_index_buffer_t));
		kinc_g4_index_buffer_init(buffer, args[0]->Int32Value(isolate->GetCurrentContext()).FromJust(), KINC_G4_INDEX_BUFFER_FORMAT_32BIT);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, buffer));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		kinc_g4_index_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_lock_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		int* vertices = kinc_g4_index_buffer_lock(buffer);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, vertices, kinc_g4_index_buffer_count(buffer) * sizeof(int));
		args.GetReturnValue().Set(Uint32Array::New(abuffer, 0, kinc_g4_index_buffer_count(buffer)));
	}

	void krom_unlock_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		kinc_g4_index_buffer_unlock(buffer);
	}

	void krom_set_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		kinc_g4_set_index_buffer(buffer);
	}

	kinc_g4_vertex_data_t convert_vertex_data(int num) {
		switch (num) {
		case 0:
			return KINC_G4_VERTEX_DATA_FLOAT1;
		case 1:
			return KINC_G4_VERTEX_DATA_FLOAT2;
		case 2:
			return KINC_G4_VERTEX_DATA_FLOAT3;
		case 3:
			return KINC_G4_VERTEX_DATA_FLOAT4;
		case 4:
			return KINC_G4_VERTEX_DATA_FLOAT4X4;
		case 5:
			return KINC_G4_VERTEX_DATA_SHORT2_NORM;
		case 6:
			return KINC_G4_VERTEX_DATA_SHORT4_NORM;
		}
		return KINC_G4_VERTEX_DATA_FLOAT1;
	}

	void krom_create_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> jsstructure = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_vertex_structure_t structure;
		kinc_g4_vertex_structure_init(&structure);
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> element = jsstructure->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<Value> str = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			String::Utf8Value utf8_value(isolate, str);
			int32_t data = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "data").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			strcpy(temp_string_vstruct[0][i], *utf8_value);
			kinc_g4_vertex_structure_add(&structure, temp_string_vstruct[0][i], convert_vertex_data(data));
		}
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)malloc(sizeof(kinc_g4_vertex_buffer_t));
		kinc_g4_vertex_buffer_init(buffer, args[0]->Int32Value(isolate->GetCurrentContext()).FromJust(), &structure, (kinc_g4_usage_t)args[2]->Int32Value(isolate->GetCurrentContext()).FromJust(), args[3]->Int32Value(isolate->GetCurrentContext()).FromJust());
		obj->SetInternalField(0, External::New(isolate, buffer));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		kinc_g4_vertex_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_lock_vertex_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		int start = 0;
		int count = kinc_g4_vertex_buffer_count(buffer);
		if (armorcore) {
			start = args[1]->Int32Value(isolate->GetCurrentContext()).FromJust();
			count = args[2]->Int32Value(isolate->GetCurrentContext()).FromJust();
		}
		float* vertices = kinc_g4_vertex_buffer_lock(buffer, start, count);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, vertices, (count - start) * kinc_g4_vertex_buffer_stride(buffer));
		args.GetReturnValue().Set(Float32Array::New(abuffer, 0, (count - start) * kinc_g4_vertex_buffer_stride(buffer) / 4));
	}

	void krom_unlock_vertex_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		int count = kinc_g4_vertex_buffer_count(buffer);
		if (armorcore) {
			count = args[1]->Int32Value(isolate->GetCurrentContext()).FromJust();
		}
		kinc_g4_vertex_buffer_unlock(buffer, count);
	}

	void krom_set_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		kinc_g4_set_vertex_buffer(buffer);
	}

	void krom_set_vertexbuffers(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_vertex_buffer_t* vertex_buffers[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		Local<Object> jsarray = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> bufferobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "buffer").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> bufferfield = Local<External>::Cast(bufferobj->GetInternalField(0));
			kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)bufferfield->Value();
			vertex_buffers[i] = buffer;
		}
		kinc_g4_set_vertex_buffers(vertex_buffers, length);
	}

	void krom_draw_indexed_vertices(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int start = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (count < 0) kinc_g4_draw_indexed_vertices();
		else kinc_g4_draw_indexed_vertices_from_to(start, count);
	}

	void krom_draw_indexed_vertices_instanced(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int instance_count = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int start = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (count < 0) kinc_g4_draw_indexed_vertices_instanced(instance_count);
		else kinc_g4_draw_indexed_vertices_instanced_from_to(instance_count, start, count);
	}

	void krom_create_vertex_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_VERTEX);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_vertex_shader_from_source(const FunctionCallbackInfo<Value>& args) {

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		#ifdef WITH_D3DCOMPILER

		strcpy(temp_string_vs, *utf8_value);

		ID3DBlob* error_message;
		ID3DBlob* shader_buffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(temp_string_vs, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, &shader_buffer, &error_message);
		if (hr != S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char*)error_message->GetBufferPointer());
			return;
		}

		bool hasBone = strstr(temp_string_vs, "bone :") != NULL;
		bool hasCol = strstr(temp_string_vs, "col :") != NULL;
		bool hasNor = strstr(temp_string_vs, "nor :") != NULL;
		bool hasPos = strstr(temp_string_vs, "pos :") != NULL;
		bool hasTex = strstr(temp_string_vs, "tex :") != NULL;

		std::map<std::string, int> attributes;
		int index = 0;
		if (hasBone) attributes["bone"] = index++;
		if (hasCol) attributes["col"] = index++;
		if (hasNor) attributes["nor"] = index++;
		if (hasPos) attributes["pos"] = index++;
		if (hasTex) attributes["tex"] = index++;
		if (hasBone) attributes["weight"] = index++;

		char* output = temp_string_vs;
		std::ostrstream file(output, 1024 * 1024);
		size_t output_len = 0;

		file.put((char)attributes.size());
		output_len += 1;
		for (std::map<std::string, int>::const_iterator attribute = attributes.begin(); attribute != attributes.end(); ++attribute) {
			(file) << attribute->first.c_str();
			output_len += attribute->first.length();
			file.put(0);
			output_len += 1;
			file.put(attribute->second);
			output_len += 1;
		}

		ID3D11ShaderReflection* reflector = nullptr;
		D3DReflect(shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

		D3D11_SHADER_DESC desc;
		reflector->GetDesc(&desc);

		file.put(desc.BoundResources);
		output_len += 1;
		for (unsigned i = 0; i < desc.BoundResources; ++i) {
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			reflector->GetResourceBindingDesc(i, &bindDesc);
			(file) << bindDesc.Name;
			output_len += strlen(bindDesc.Name);
			file.put(0);
			output_len += 1;
			file.put(bindDesc.BindPoint);
			output_len += 1;
		}

		ID3D11ShaderReflectionConstantBuffer* constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables);
			output_len += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable* variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name;
					output_len += strlen(variableDesc.Name);
					file.put(0);
					output_len += 1;
					file.write((char*)&variableDesc.StartOffset, 4);
					output_len += 4;
					file.write((char*)&variableDesc.Size, 4);
					output_len += 4;
					D3D11_SHADER_TYPE_DESC typeDesc;
					hr = variable->GetType()->GetDesc(&typeDesc);
					if (hr == S_OK) {
						file.put(typeDesc.Columns);
						output_len += 1;
						file.put(typeDesc.Rows);
						output_len += 1;
					}
					else {
						file.put(0);
						output_len += 1;
						file.put(0);
						output_len += 1;
					}
				}
			}
		}
		else {
			file.put(0);
			output_len += 1;
		}
		file.write((char*)shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize());
		output_len += shader_buffer->GetBufferSize();
		shader_buffer->Release();
		reflector->Release();

		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, (int)output_len, KINC_G4_SHADER_TYPE_VERTEX);

		#elif KORE_METAL

		strcpy(temp_string_vs, "// my_main\n");
		strcat(temp_string_vs, *utf8_value);
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, temp_string_vs, strlen(temp_string_vs), KINC_G4_SHADER_TYPE_VERTEX);

		#elif defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)

		char* output = new char[1024 * 1024];
		int length;
		krafix_compile(*utf8_value, output, &length, "spirv", "windows", "vert");
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_VERTEX);

		#else

		char* source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_VERTEX);

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
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_FRAGMENT);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_fragment_shader_from_source(const FunctionCallbackInfo<Value>& args) {

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		#ifdef WITH_D3DCOMPILER

		strcpy(temp_string_fs, *utf8_value);

		ID3DBlob* error_message;
		ID3DBlob* shader_buffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(temp_string_fs, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, &shader_buffer, &error_message);
		if (hr != S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char*)error_message->GetBufferPointer());
			return;
		}

		std::map<std::string, int> attributes;

		char* output = temp_string_fs;
		std::ostrstream file(output, 1024 * 1024);
		size_t output_len = 0;

		file.put((char)attributes.size());
		output_len += 1;

		ID3D11ShaderReflection* reflector = nullptr;
		D3DReflect(shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

		D3D11_SHADER_DESC desc;
		reflector->GetDesc(&desc);

		file.put(desc.BoundResources);
		output_len += 1;
		for (unsigned i = 0; i < desc.BoundResources; ++i) {
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			reflector->GetResourceBindingDesc(i, &bindDesc);
			(file) << bindDesc.Name;
			output_len += strlen(bindDesc.Name);
			file.put(0);
			output_len += 1;
			file.put(bindDesc.BindPoint);
			output_len += 1;
		}

		ID3D11ShaderReflectionConstantBuffer* constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables);
			output_len += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable* variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name;
					output_len += strlen(variableDesc.Name);
					file.put(0);
					output_len += 1;
					file.write((char*)&variableDesc.StartOffset, 4);
					output_len += 4;
					file.write((char*)&variableDesc.Size, 4);
					output_len += 4;
					D3D11_SHADER_TYPE_DESC typeDesc;
					hr = variable->GetType()->GetDesc(&typeDesc);
					if (hr == S_OK) {
						file.put(typeDesc.Columns);
						output_len += 1;
						file.put(typeDesc.Rows);
						output_len += 1;
					}
					else {
						file.put(0);
						output_len += 1;
						file.put(0);
						output_len += 1;
					}
				}
			}
		}
		else {
			file.put(0);
			output_len += 1;
		}
		file.write((char*)shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize());
		output_len += shader_buffer->GetBufferSize();
		shader_buffer->Release();
		reflector->Release();

		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, (int)output_len, KINC_G4_SHADER_TYPE_FRAGMENT);

		#elif KORE_METAL

		strcpy(temp_string_fs, "// my_main\n");
		strcat(temp_string_fs, *utf8_value);
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, temp_string_fs, strlen(temp_string_fs), KINC_G4_SHADER_TYPE_FRAGMENT);

		#elif defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)

		char* output = new char[1024 * 1024];
		int length;
		krafix_compile(*utf8_value, output, &length, "spirv", "windows", "frag");
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_FRAGMENT);

		#else

		char* source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_FRAGMENT);

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
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_GEOMETRY);

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
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL);

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
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION);

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
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)field->Value();
		kinc_g4_shader_destroy(shader);
		free(shader);
	}

	void krom_create_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)malloc(sizeof(kinc_g4_pipeline_t));
		kinc_g4_pipeline_init(pipeline);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(8);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, pipeline));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();
		kinc_g4_pipeline_destroy(pipeline);
		free(pipeline);
	}

	void krom_compile_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();

		kinc_g4_vertex_structure_t s0, s1, s2, s3;
		kinc_g4_vertex_structure_init(&s0);
		kinc_g4_vertex_structure_init(&s1);
		kinc_g4_vertex_structure_init(&s2);
		kinc_g4_vertex_structure_init(&s3);
		kinc_g4_vertex_structure_t* structures[4] = { &s0, &s1, &s2, &s3 };

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

				strcpy(temp_string_vstruct[i1][i2], *utf8_value);
				kinc_g4_vertex_structure_add(structures[i1], temp_string_vstruct[i1][i2], convert_vertex_data(data));
			}
		}

		pipeobj->SetInternalField(1, External::New(isolate, structures));
		pipeobj->SetInternalField(2, External::New(isolate, &size));

		Local<External> vsfield = Local<External>::Cast(args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_shader_t* vertexShader = (kinc_g4_shader_t*)vsfield->Value();
		pipeobj->SetInternalField(3, External::New(isolate, vertexShader));

		Local<External> fsfield = Local<External>::Cast(args[7]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_shader_t* fragmentShader = (kinc_g4_shader_t*)fsfield->Value();
		pipeobj->SetInternalField(4, External::New(isolate, fragmentShader));

		pipeline->vertex_shader = vertexShader;
		pipeline->fragment_shader = fragmentShader;

		if (!args[8]->IsNullOrUndefined()) {
			Local<External> gsfield = Local<External>::Cast(args[8]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t* geometryShader = (kinc_g4_shader_t*)gsfield->Value();
			pipeobj->SetInternalField(5, External::New(isolate, geometryShader));
			pipeline->geometry_shader = geometryShader;
		}

		if (!args[9]->IsNullOrUndefined()) {
			Local<External> tcsfield = Local<External>::Cast(args[9]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t* tessellationControlShader = (kinc_g4_shader_t*)tcsfield->Value();
			pipeobj->SetInternalField(6, External::New(isolate, tessellationControlShader));
			pipeline->tessellation_control_shader = tessellationControlShader;
		}

		if (!args[10]->IsNullOrUndefined()) {
			Local<External> tesfield = Local<External>::Cast(args[10]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t* tessellationEvaluationShader = (kinc_g4_shader_t*)tesfield->Value();
			pipeobj->SetInternalField(7, External::New(isolate, tessellationEvaluationShader));
			pipeline->tessellation_evaluation_shader = tessellationEvaluationShader;
		}

		for (int i = 0; i < size; ++i) {
			pipeline->input_layout[i] = structures[i];
		}
		pipeline->input_layout[size] = nullptr;

		Local<Object> args11 = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		pipeline->cull_mode = (kinc_g4_cull_mode_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "cullMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->depth_write = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depthWrite").ToLocalChecked()).ToLocalChecked()->BooleanValue(isolate);
		pipeline->depth_mode = (kinc_g4_compare_mode_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depthMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->blend_source = (kinc_g4_blending_operation_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->blend_destination = (kinc_g4_blending_operation_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alpha_blend_source = (kinc_g4_blending_operation_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alpha_blend_destination = (kinc_g4_blending_operation_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		Local<Object> maskRedArray = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskRed").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskGreenArray = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskGreen").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskBlueArray = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskBlue").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskAlphaArray = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskAlpha").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		for (int i = 0; i < 8; ++i) {
			pipeline->color_write_mask_red[i] = maskRedArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->color_write_mask_green[i] = maskGreenArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->color_write_mask_blue[i] = maskBlueArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->color_write_mask_alpha[i] = maskAlphaArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
		}

		if (armorcore) {
			pipeline->color_attachment_count = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorAttachmentCount").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
			Local<Object> colorAttachmentArray = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorAttachments").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			for (int i = 0; i < 8; ++i) {
				pipeline->color_attachment[i] = (kinc_g4_render_target_format_t)colorAttachmentArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
			}

			pipeline->depth_attachment_bits = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depthAttachmentBits").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
			pipeline->stencil_attachment_bits = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilAttachmentBits").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		}

		pipeline->conservative_rasterization = args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "conservativeRasterization").ToLocalChecked()).ToLocalChecked()->BooleanValue(isolate);

		kinc_g4_pipeline_compile(pipeline);
	}

	void krom_set_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();
		kinc_g4_set_pipeline(pipeline);
	}

	bool ends_with(const char* str, const char* suffix) {
		if (!str || !suffix) return 0;
		size_t lenstr = strlen(str);
		size_t lensuffix = strlen(suffix);
		if (lensuffix > lenstr) return 0;
		return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
	}

	void load_image(kinc_file_reader_t& reader, const char* filename, unsigned char*& output, int& width, int& height, kinc_image_format_t& format) {
		format = KINC_IMAGE_FORMAT_RGBA32;
		int size = (int)kinc_file_reader_size(&reader);
		int comp;
		unsigned char* data = (unsigned char*)malloc(size);
		kinc_file_reader_read(&reader, data, size);
		kinc_file_reader_close(&reader);

		if (ends_with(filename, "k")) {
			width = kinc_read_s32le(data);
			height = kinc_read_s32le(data + 4);
			char fourcc[5];
			fourcc[0] = data[8];
			fourcc[1] = data[9];
			fourcc[2] = data[10];
			fourcc[3] = data[11];
			fourcc[4] = 0;
			int compressedSize = size - 12;
			if (strcmp(fourcc, "LZ4 ") == 0) {
				int outputSize = width * height * 4;
				output = (unsigned char*)malloc(outputSize);
				LZ4_decompress_safe((char *)(data + 12), (char *)output, compressedSize, outputSize);
			}
			else if (strcmp(fourcc, "LZ4F") == 0) {
				int outputSize = width * height * 16;
				output = (unsigned char*)malloc(outputSize);
				LZ4_decompress_safe((char *)(data + 12), (char *)output, compressedSize, outputSize);
				format = KINC_IMAGE_FORMAT_RGBA128;
			}
		}
		else if (ends_with(filename, "hdr")) {
			output = (unsigned char*)stbi_loadf_from_memory(data, size, &width, &height, &comp, 4);
			if (output == nullptr) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			}
			format = KINC_IMAGE_FORMAT_RGBA128;
		}
		else { // jpg, png, ..
			output = stbi_load_from_memory(data, size, &width, &height, &comp, 4);
			if (output == nullptr) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			}
		}
		free(data);
	}

	void krom_load_image(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		bool readable = args[1]->ToBoolean(isolate)->Value();

		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t* image = (kinc_image_t*)malloc(sizeof(kinc_image_t));

		if (armorcore) {
			kinc_file_reader_t reader;
			if (kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) {
				unsigned char* image_data;
				int image_width;
				int image_height;
				kinc_image_format_t image_format;
				load_image(reader, *utf8_value, image_data, image_width, image_height, image_format);
				kinc_image_init(image, image_data, image_width, image_height, image_format);
			}
			else return;
		}
		else {
			// TODO: make kinc_image load faster
			size_t byte_size = kinc_image_size_from_file(*utf8_value);
			if (byte_size == 0) return;
			void* memory = malloc(byte_size);
			kinc_image_init_from_file(image, memory, *utf8_value);
		}

		kinc_g4_texture_init_from_image(texture, image);
		if (!readable) {
			delete[] image->data;
			kinc_image_destroy(image);
			free(image);
			// free(memory);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	void krom_unload_image(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNullOrUndefined()) return;
		Local<Object> image = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> tex = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		Local<Value> rt = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();

		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
			kinc_g4_texture_destroy(texture);
			free(texture);
		}
		else if (rt->IsObject()) {
			Local<External> rtfield = Local<External>::Cast(rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
			kinc_g4_render_target_destroy(render_target);
			free(render_target);
		}
	}

	void krom_load_sound(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		kinc_a1_sound_t* sound = kinc_a1_sound_create(*utf8_value);
		Local<ArrayBuffer> buffer;
		ArrayBuffer::Contents content;
		buffer = ArrayBuffer::New(isolate, sound->size * 2 * sizeof(float));
		content = buffer->GetContents();
		float* to = (float*)content.Data();

		int16_t* left = (int16_t*)&sound->left[0];
		int16_t* right = (int16_t*)&sound->right[0];
		for (int i = 0; i < sound->size; i += 1) {
			to[i * 2 + 0] = (float)(left [i] / 32767.0);
			to[i * 2 + 1] = (float)(right[i] / 32767.0);
		}

		args.GetReturnValue().Set(buffer);
		kinc_a1_sound_destroy(sound);
	}

	void krom_write_audio_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		int samples = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		for (int i = 0; i < samples; ++i) {
			float* values = (float*)content.Data();
			float value = values[audio_read_location / 4];
			audio_read_location += 4;
			if (audio_read_location >= content.ByteLength()) audio_read_location = 0;

			*(float*)&audio_buffer.data[audio_buffer.write_location] = value;
			audio_buffer.write_location += 4;
			if (audio_buffer.write_location >= audio_buffer.data_size) audio_buffer.write_location = 0;
		}
	}

	void krom_load_blob(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		kinc_file_reader_t reader;
		if (!kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) return;
		int reader_size = (int)kinc_file_reader_size(&reader);

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader_size);
		ArrayBuffer::Contents contents = buffer->GetContents();
		kinc_file_reader_read(&reader, contents.Data(), reader_size);
		kinc_file_reader_close(&reader);

		args.GetReturnValue().Set(buffer);
	}

	void krom_load_url(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_load_url(*utf8_value);
	}

	void krom_get_constant_location(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> pipefield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_g4_constant_location_t* location_copy = (kinc_g4_constant_location_t*)malloc(sizeof(kinc_g4_constant_location_t));
		memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
		obj->SetInternalField(0, External::New(isolate, location_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> pipefield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_g4_texture_unit_t* unit_copy = (kinc_g4_texture_unit_t*)malloc(sizeof(kinc_g4_texture_unit_t));
		memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
		obj->SetInternalField(0, External::New(isolate, unit_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_set_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		#ifdef KORE_METAL
		if (unit->impl._unit.impl.index == -1) return;
		#endif
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		kinc_g4_set_texture(*unit, texture);
	}

	void krom_set_render_target(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		#ifdef KORE_METAL
		if (unit->impl._unit.impl.index == -1) return;
		#endif
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_g4_render_target_use_color_as_texture(render_target, *unit);
	}

	void krom_set_texture_depth(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		#ifdef KORE_METAL
		if (unit->impl._unit.impl.index == -1) return;
		#endif
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_g4_render_target_use_depth_as_texture(render_target, *unit);
	}

	void krom_set_image_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		kinc_g4_set_image_texture(*unit, texture);
	}

	void krom_set_texture_parameters(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture3d_parameters(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture_compare_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_texture_compare_mode(*unit, args[1]->ToBoolean(isolate)->Value());
	}

	void krom_set_cube_map_compare_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_cubemap_compare_mode(*unit, args[1]->ToBoolean(isolate)->Value());
	}

	void krom_set_bool(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_bool(*location, value != 0);
	}

	void krom_set_int(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_int(*location, value);
	}

	void krom_set_float(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float(*location, value);
	}

	void krom_set_float2(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float2(*location, value1, value2);
	}

	void krom_set_float3(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float3(*location, value1, value2, value3);
	}

	void krom_set_float4(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		float* from = (float*)content.Data();
		kinc_g4_set_floats(*location, from, int(content.ByteLength() / 4));
	}

	void krom_set_matrix(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_g4_set_matrix4(*location, (kinc_matrix4x4_t*)from);
	}

	void krom_set_matrix3(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_g4_set_matrix3(*location, (kinc_matrix3x3_t*)from);
	}

	void krom_get_time(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Number::New(isolate, kinc_time()));
	}

	void krom_window_width(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_width(windowId)));
	}

	void krom_window_height(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_height(windowId)));
	}

	void krom_set_window_title(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value title(isolate, args[1]);
		kinc_window_set_title(windowId, *title);
	}

	void krom_get_window_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_get_mode(windowId)));
	}

	void krom_set_window_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_mode_t windowMode = (kinc_window_mode_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_change_mode(windowId, windowMode);
	}

	void krom_resize_window(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int width = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_resize(windowId, width, height);
	}

	void krom_move_window(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_move(windowId, x, y);
	}

	void krom_screen_dpi(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int ppi = kinc_display_current_mode(kinc_primary_display()).pixels_per_inch;
		args.GetReturnValue().Set(Int32::New(isolate, ppi));
	}

	void krom_system_id(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_system_id()).ToLocalChecked());
	}

	void krom_request_shutdown(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_stop();
	}

	void krom_display_count(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, kinc_count_displays()));
	}

	void krom_display_width(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).width));
	}

	void krom_display_height(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).height));
	}

	void krom_display_x(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).x));
	}

	void krom_display_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).y));
	}

	void krom_display_frequency(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).frequency));
	}

	void krom_display_is_primary(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Boolean::New(isolate, index == kinc_primary_display()));
	}

	void krom_write_storage(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		kinc_file_writer_t writer;
		kinc_file_writer_open(&writer, *utf8_name);
		kinc_file_writer_write(&writer, content.Data(), (int)content.ByteLength());
		kinc_file_writer_close(&writer);
	}

	void krom_read_storage(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		kinc_file_reader_t reader;
		if (!kinc_file_reader_open(&reader, *utf8_name, KINC_FILE_TYPE_SAVE)) return;
		int reader_size = (int)kinc_file_reader_size(&reader);

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader_size);
		ArrayBuffer::Contents contents = buffer->GetContents();
		kinc_file_reader_read(&reader, contents.Data(), reader_size);
		kinc_file_reader_close(&reader);

		args.GetReturnValue().Set(buffer);
	}

	void krom_create_render_target(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)malloc(sizeof(kinc_g4_render_target_t));
		kinc_g4_render_target_init(render_target, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false, (kinc_g4_render_target_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), 0);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, render_target));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, render_target->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, render_target->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_render_target_cube_map(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)malloc(sizeof(kinc_g4_render_target_t));
		kinc_g4_render_target_init_cube(render_target, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false, (kinc_g4_render_target_format_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), 0);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, render_target));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, render_target->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, render_target->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init(texture, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture3d(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init3d(texture, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->tex_depth));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		// content = buffer->GetContents();
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t* image = (kinc_image_t*)malloc(sizeof(kinc_image_t));
		kinc_image_init(image, content.Data(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
		kinc_g4_texture_init_from_image(texture, image);
		bool readable = args[4]->ToBoolean(isolate)->Value();
		if (!readable) {
			delete[] content.Data();
			kinc_image_destroy(image);
			free(image);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes3d(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		// content = buffer->GetContents();
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t image;
		kinc_image_init3d(&image, content.Data(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
		kinc_g4_texture_init_from_image3d(texture, &image);
		kinc_image_destroy(&image);
		bool readable = args[5]->ToBoolean(isolate)->Value();
		if (!readable) {
			delete[] content.Data();
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->tex_depth));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_encoded_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		String::Utf8Value format(isolate, args[1]);
		bool readable = args[2]->ToBoolean(isolate)->Value();

		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t* image = (kinc_image_t*)malloc(sizeof(kinc_image_t));

		unsigned char *content_data = (unsigned char *)content.Data();
		int content_length = (int)content.ByteLength();
		unsigned char *image_data;
		kinc_image_format_t image_format;
		int image_width;
		int image_height;

		if (ends_with(*format, "k")) {
			image_width = kinc_read_s32le(content_data);
			image_height = kinc_read_s32le(content_data + 4);
			char fourcc[5];
			fourcc[0] = content_data[8];
			fourcc[1] = content_data[9];
			fourcc[2] = content_data[10];
			fourcc[3] = content_data[11];
			fourcc[4] = 0;
			int compressedSize = (int)content.ByteLength() - 12;
			if (strcmp(fourcc, "LZ4 ") == 0) {
				int outputSize = image_width * image_height * 4;
				image_data = (unsigned char*)malloc(outputSize);
				LZ4_decompress_safe((char*)content_data + 12, (char *)image_data, compressedSize, outputSize);
				image_format = KINC_IMAGE_FORMAT_RGBA32;
			}
			else if (strcmp(fourcc, "LZ4F") == 0) {
				int outputSize = image_width * image_height * 16;
				image_data = (unsigned char*)malloc(outputSize);
				LZ4_decompress_safe((char*)content_data + 12, (char *)image_data, compressedSize, outputSize);
				image_format = KINC_IMAGE_FORMAT_RGBA128;
			}
		}
		else if (ends_with(*format, "hdr")) {
			int comp;
			image_data = (unsigned char*)stbi_loadf_from_memory(content_data, content_length, &image_width, &image_height, &comp, 4);
			image_format = KINC_IMAGE_FORMAT_RGBA128;
		}
		else { // jpg, png, ..
			int comp;
			image_data = stbi_load_from_memory(content_data, content_length, &image_width, &image_height, &comp, 4);
			image_format = KINC_IMAGE_FORMAT_RGBA32;
		}

		kinc_image_init(image, image_data, image_width, image_height, image_format);
		kinc_g4_texture_init_from_image(texture, image);
		if (!readable) {
			delete[] image->data;
			kinc_image_destroy(image);
			free(image);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	int format_byte_size(kinc_image_format_t format) {
		switch (format) {
		case KINC_IMAGE_FORMAT_RGBA128:
			return 16;
		case KINC_IMAGE_FORMAT_RGBA64:
			return 8;
		case KINC_IMAGE_FORMAT_RGB24:
			return 4;
		case KINC_IMAGE_FORMAT_A32:
			return 4;
		case KINC_IMAGE_FORMAT_A16:
			return 2;
		case KINC_IMAGE_FORMAT_GREY8:
			return 1;
		case KINC_IMAGE_FORMAT_BGRA32:
		case KINC_IMAGE_FORMAT_RGBA32:
		default:
			return 4;
		}
	}

	void krom_get_texture_pixels(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(1));
		kinc_image_t* image = (kinc_image_t*)field->Value();

		uint8_t* data = kinc_image_get_pixels(image);
		int byteLength = format_byte_size(image->format) * image->width * image->height * image->depth;
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, data, byteLength);
		args.GetReturnValue().Set(buffer);
	}

	void krom_get_render_target_pixels(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* rt = (kinc_g4_render_target_t*)field->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		uint8_t* b = (uint8_t*)content.Data();
		kinc_g4_render_target_get_pixels(rt, b);
	}

	void krom_lock_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		uint8_t* tex = kinc_g4_texture_lock(texture);

		int byteLength = kinc_g4_texture_stride(texture) * texture->tex_height * texture->tex_depth;
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, tex, byteLength);
		args.GetReturnValue().Set(abuffer);
	}

	void krom_unlock_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		kinc_g4_texture_unlock(texture);
	}

	void krom_clear_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int width = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int depth = args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_texture_clear(texture, x, y, z, width, height, depth, color);
	}

	void krom_generate_texture_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		kinc_g4_texture_generate_mipmaps(texture, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_generate_render_target_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* rt = (kinc_g4_render_target_t*)field->Value();
		kinc_g4_render_target_generate_mipmaps(rt, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_set_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();

		Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> mipmapobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> mipmapfield = Local<External>::Cast(mipmapobj->GetInternalField(1));
			kinc_image_t* mipmap = (kinc_image_t*)mipmapfield->Value();
			kinc_g4_texture_set_mipmap(texture, mipmap, i + 1);
		}
	}

	void krom_set_depth_stencil_from(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> targetfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)targetfield->Value();
		Local<External> sourcefield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* source_target = (kinc_g4_render_target_t*)sourcefield->Value();
		kinc_g4_render_target_set_depth_stencil_from(render_target, source_target);
	}

	void krom_viewport(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_viewport(x, y, w, h);
	}

	void krom_scissor(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_scissor(x, y, w, h);
	}

	void krom_disable_scissor(const FunctionCallbackInfo<Value>& args) {
		kinc_g4_disable_scissor();
	}

	void krom_render_targets_inverted_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, kinc_g4_render_targets_inverted_y()));
	}

	void krom_begin(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNullOrUndefined()) {
			kinc_g4_restore_render_target();
		}
		else {
			Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
			kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();

			int32_t length = 1;
			kinc_g4_render_target_t* render_targets[8] = { render_target, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
			if (!args[1]->IsNullOrUndefined()) {
				Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value() + 1;
				if (length > 8) length = 8;
				for (int32_t i = 1; i < length; ++i) {
					Local<Object> artobj = jsarray->Get(isolate->GetCurrentContext(), i - 1).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
					Local<External> artfield = Local<External>::Cast(artobj->GetInternalField(0));
					kinc_g4_render_target_t* art = (kinc_g4_render_target_t*)artfield->Value();
					render_targets[i] = art;
				}
			}
			kinc_g4_set_render_targets(render_targets, length);
		}
	}

	void krom_begin_face(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		int face = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_set_render_target_face(render_target, face);
	}

	void krom_end(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
	}

	void krom_file_save_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_path(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		bool hasLengthArg = args.Length() > 2 && !args[2]->IsNullOrUndefined();
		int byteLength = hasLengthArg ? args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value() : (int)content.ByteLength();
		if (byteLength > (int)content.ByteLength()) byteLength = (int)content.ByteLength();

		#ifdef KORE_WINDOWS
		MultiByteToWideChar(CP_UTF8, 0, *utf8_path, -1, temp_wstring, 1024);
		FILE* file = _wfopen(temp_wstring, L"wb");
		#else
		FILE* file = fopen(*utf8_path, "wb");
		#endif
		if (file == nullptr) return;
		fwrite(content.Data(), 1, byteLength, file);
		fclose(file);
	}

	void krom_sys_command(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_cmd(isolate, args[0]);
		
		#ifdef KORE_WINDOWS
		int wlen = MultiByteToWideChar(CP_UTF8, 0, *utf8_cmd, -1, NULL, 0);
		wchar_t* wcmd = new wchar_t[wlen];
		MultiByteToWideChar(CP_UTF8, 0, *utf8_cmd, -1, wcmd, wlen);
		
		wchar_t* cmdCommand = new wchar_t[wlen + 11];
		wsprintf(cmdCommand,L"cmd.exe /C %s", wcmd);
		delete[] wcmd;

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		ZeroMemory(&pi, sizeof(pi));
		int result = 1;
		// Start the child process.
		if (CreateProcess(NULL,   // No module name (use command line)
			cmdCommand,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory
			&si,            // Pointer to STARTUPINFO structure
			&pi)           // Pointer to PROCESS_INFORMATION structure
			)
		{
			DWORD dwResult;

			// Wait until child process exits.
			WaitForSingleObject(pi.hProcess, INFINITE);
			GetExitCodeProcess(pi.hProcess, &dwResult); //grab the exit code
			// Close process and thread handles.
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			result = dwResult;
		}

		delete[] cmdCommand;
		#elif KORE_IOS
		int result = 0;
		#else
		int result = system(*utf8_cmd);
		#endif
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_delete_file(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_file(isolate, args[0]);
		#ifdef KORE_WINDOWS
		int wlen = MultiByteToWideChar(CP_UTF8, 0, *utf8_file, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[wlen];
		MultiByteToWideChar(CP_UTF8, 0, *utf8_file, -1, wstr, wlen);
		int result = ~DeleteFileW(wstr);
		delete[] wstr;
		#else
		int result = remove(*utf8_file);
		#endif
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_copy_file(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_srcPath(isolate, args[0]);
		String::Utf8Value utf8_dstPath(isolate, args[1]);
		#ifdef KORE_WINDOWS
		int wlen = MultiByteToWideChar(CP_UTF8, 0, *utf8_srcPath, -1, NULL, 0);
		wchar_t* wsrcPath = new wchar_t[wlen];
		MultiByteToWideChar(CP_UTF8, 0, *utf8_srcPath, -1, wsrcPath, wlen);
		wlen = MultiByteToWideChar(CP_UTF8, 0, *utf8_dstPath, -1, NULL, 0);
		wchar_t* wdstPath = new wchar_t[wlen];
		MultiByteToWideChar(CP_UTF8, 0, *utf8_dstPath, -1, wdstPath, wlen);

		int result = ~CopyFileW(wsrcPath, wdstPath, FALSE);
		delete[] wsrcPath;
		delete[] wdstPath;
		#else
		char* cmd = new char[utf8_srcPath.length() + utf8_dstPath.length() + 8]; //cp 'PATH' 'PATH'
		sprintf(cmd, "cp '%s' '%s'", *utf8_srcPath, *utf8_dstPath);
		int result = system(cmd);
		delete[] cmd;
		#endif
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_open_in_std_app(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_path(isolate, args[0]);
		#if defined(KORE_WINDOWS)
		int wlen = MultiByteToWideChar(CP_UTF8, 0, *utf8_path, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[wlen];
		MultiByteToWideChar(CP_UTF8, 0, *utf8_path, -1, wstr, wlen);
		SHELLEXECUTEINFOW info{ sizeof(info) };
		info.fMask = SEE_MASK_NOASYNC; 
		info.lpVerb = L"open";
		info.lpFile = wstr;
		info.nShow = SW_SHOWDEFAULT;
		int result = 0;

		result = ~ShellExecuteExW(&info);
		delete[] wstr;
		
		#elif defined(KORE_LINUX)
		char* cmd = new char[utf8_path.length()+11]; //xdg-open 'PATH'
		sprintf(cmd, "xdg-open '%s'", *utf8_path);
		int result = system(cmd);
		delete[] cmd;
		#elif defined(KORE_ANDROID) || defined(KORE_IOS)
		kinc_load_url(*utf8_path);
		int result = 0;
		#else
		char* cmd = new char[utf8_path.length() + 7]; //open 'PATH'
		sprintf(cmd, "open '%s'", *utf8_path);
		int result = system(cmd);
		delete[] cmd;
		#endif
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_create_directory(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_dir(isolate, args[0]);
		#ifdef KORE_WINDOWS
		int wlen = MultiByteToWideChar(CP_UTF8, 0, *utf8_dir, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[wlen];
		MultiByteToWideChar(CP_UTF8, 0, *utf8_dir, -1, wstr, wlen);
		int result = ~CreateDirectoryW(wstr,NULL);
		delete[] wstr;
		#else
		int result = mkdir(*utf8_dir);
		#endif
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_working_dir(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		#ifdef KORE_WINDOWS
		DWORD dwBufferLength = GetCurrentDirectoryW(0, NULL);
		wchar_t* wcwd = new wchar_t[dwBufferLength];
		GetCurrentDirectoryW(dwBufferLength, wcwd);
		int utf8len = WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, NULL, 0, NULL, NULL);
		char* cwd = new char[utf8len];
		WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, cwd, utf8len, NULL, NULL);
		delete[] wcwd;
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, cwd).ToLocalChecked());
		delete[] cwd;
		#else
		char cwd[PATH_MAX];
		getcwd(cwd, PATH_MAX);
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, cwd).ToLocalChecked());
		#endif
	}


	void krom_save_path(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_internal_save_path()).ToLocalChecked());
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
		#ifdef KORE_MACOS
		char path[1024];
		strcpy(path, macgetresourcepath());
		strcat(path, "/");
		strcat(path, KORE_DEBUGDIR);
		strcat(path, "/");
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, path).ToLocalChecked());
		#elif KORE_IOS
		char path[1024];
		strcpy(path, iphonegetresourcepath());
		strcat(path, "/");
		strcat(path, KORE_DEBUGDIR);
		strcat(path, "/");
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, path).ToLocalChecked());
		#else
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_internal_get_files_location()).ToLocalChecked());
		#endif
	}

	void krom_http_callback(int error, int response, const char *body, void *callbackdata) {
		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, http_func);
		TryCatch try_catch(isolate);
		Local<Value> result;
		Local<Value> argv[1];
		if (body != NULL) argv[0] = ArrayBuffer::New(isolate, (void *)body, http_result_size);
		if (!func->Call(context, context->Global(), body != NULL ? 1 : 0, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void krom_http_request(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value url(isolate, args[0]);
		// TODO: assuming krom_http_request is synchronous for now
		http_result_size = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<Function> func = Local<Function>::Cast(args[2]);
		http_func.Reset(isolate, func);
		char url_base[512];
		char url_path[512];
		const char* curl = *url;
		int i = 0;
		for (; i < 512; ++i) {
			if (curl[i + 8] == '/') break;
			url_base[i] = curl[i + 8]; // Strip https://
		}
		url_base[i] = 0;
		int j = 0;
		for (; j < 512; ++j) {
			if (curl[i + 8 + j] == 0) break;
			url_path[j] = curl[i + 8 + j];
		}
		url_path[j] = 0;
		kinc_http_request(url_base, url_path, NULL, 443, true, 0, NULL, &krom_http_callback, NULL);
	}

	void krom_set_bool_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_bool(*location, value != 0);
	}

	void krom_set_int_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_int(*location, value);
	}

	void krom_set_float_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float(*location, value);
	}

	void krom_set_float2_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float2(*location, value1, value2);
	}

	void krom_set_float3_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float3(*location, value1, value2, value3);
	}

	void krom_set_float4_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_compute_set_floats(*location, from, int(content.ByteLength() / 4));
	}

	void krom_set_matrix_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_compute_set_matrix4(*location, (kinc_matrix4x4_t*)from);
	}

	void krom_set_matrix3_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_compute_set_matrix3(*location, (kinc_matrix3x3_t*)from);
	}

	void krom_set_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_compute_set_texture(*unit, texture, (kinc_compute_access_t)access);
	}

	void krom_set_render_target_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_compute_set_render_target(*unit, render_target, (kinc_compute_access_t)access);
	}

	void krom_set_sampled_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		kinc_compute_set_sampled_texture(*unit, texture);
	}

	void krom_set_sampled_render_target_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_compute_set_sampled_render_target(*unit, render_target);
	}

	void krom_set_sampled_depth_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_compute_set_sampled_depth_from_render_target(*unit, render_target);
	}

	void krom_set_texture_parameters_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		kinc_compute_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture3d_parameters_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();
		kinc_compute_set_shader(shader);
	}

	void krom_create_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_compute_shader* shader = (kinc_compute_shader*)malloc(sizeof(kinc_compute_shader));
		kinc_compute_shader_init(shader, content.Data(), (int)content.ByteLength());

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
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();
		kinc_compute_shader_destroy(shader);
		free(shader);
	}

	void krom_get_constant_location_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_compute_constant_location_t location = kinc_compute_shader_get_constant_location(shader, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_compute_constant_location_t* location_copy = (kinc_compute_constant_location_t*)malloc(sizeof(kinc_compute_constant_location_t)); // TODO
		memcpy(location_copy, &location, sizeof(kinc_compute_constant_location_t));
		obj->SetInternalField(0, External::New(isolate, location_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_compute_texture_unit_t unit = kinc_compute_shader_get_texture_unit(shader, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_compute_texture_unit_t* unit_copy = (kinc_compute_texture_unit_t*)malloc(sizeof(kinc_compute_texture_unit_t)); // TODO
		memcpy(unit_copy, &unit, sizeof(kinc_compute_texture_unit_t));
		obj->SetInternalField(0, External::New(isolate, unit_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute(x, y, z);
	}

	void krom_set_save_and_quit_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		save_and_quit_func.Reset(isolate, func);
	}

	void krom_set_mouse_cursor(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int id = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_mouse_set_cursor(id);
		#ifdef KORE_WINDOWS
		// Set hand icon for drag even when mouse button is pressed
		if (id == 1) SetCursor(LoadCursor(NULL, IDC_HAND));
		#endif
	}

	#ifdef WITH_NFD
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
			kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
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
			kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
		}
	}
	#elif KORE_ANDROID
	void krom_open_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		AndroidFileDialogOpen();
	}

	void krom_save_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		wchar_t* outPath = AndroidFileDialogSave();
		// args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t*)outPath).ToLocalChecked());
	}
	#elif KORE_IOS
	void krom_open_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		// String::Utf8Value filterList(isolate, args[0]);
		// String::Utf8Value defaultPath(isolate, args[1]);
		// Once finished drop_files callback is called
		IOSFileDialogOpen();
	}

	void krom_save_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		// String::Utf8Value filterList(isolate, args[0]);
		// String::Utf8Value defaultPath(isolate, args[1]);
		// Path to app document directory
		wchar_t* outPath = IOSFileDialogSave();
		size_t len = wcslen(outPath);
		uint16_t* str = new uint16_t[len + 1];
		for (int i = 0; i < len; i++) str[i] = outPath[i];
		str[len] = 0;
		args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t*)str).ToLocalChecked());
		delete str;
	}
	#endif

	#ifdef WITH_TINYDIR
	void krom_read_directory(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value path(isolate, args[0]);
		bool foldersOnly = args[1]->ToBoolean(isolate)->Value();

		tinydir_dir dir;
		#ifdef KORE_WINDOWS
		MultiByteToWideChar(CP_UTF8, 0, *path, -1, temp_wstring, 1023);
		tinydir_open_sorted(&dir, temp_wstring);
		#else
		tinydir_open_sorted(&dir, *path);
		#endif

		#ifdef KORE_WINDOWS
		temp_wstring[0] = 0;
		#else
		temp_string[0] = 0;
		#endif

		for (int i = 0; i < dir.n_files; i++) {
			tinydir_file file;
			tinydir_readfile_n(&dir, &file, i);

			if (!file.is_dir || !foldersOnly) {
				#ifdef KORE_WINDOWS
				if (wcscmp(file.name, L".") == 0 || wcscmp(file.name, L"..") == 0) continue;
				if (wcslen(temp_wstring) + wcslen(file.name) + 1 > 1023) break;
				wcscat(temp_wstring, file.name);
				if (i < dir.n_files - 1) wcscat(temp_wstring, L"\n"); // Separator
				#else
				if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) continue;
				if(strlen(temp_string) + strlen(file.name) + 1 > 1023) break;
				strcat(temp_string, file.name);
				if (i < dir.n_files - 1) strcat(temp_string, "\n"); // Separator
				#endif
			}
		}

		tinydir_close(&dir);
		#ifdef KORE_WINDOWS
		args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t*)temp_wstring).ToLocalChecked());
		#else
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, temp_string).ToLocalChecked());
		#endif
	}
	#endif

	void krom_file_exists(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		bool exists = false;
		String::Utf8Value utf8_value(isolate, args[0]);

		kinc_file_reader_t reader;
		if (kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) {
			exists = true;
			kinc_file_reader_close(&reader);
		}

		args.GetReturnValue().Set(Boolean::New(isolate, exists));
	}

	#ifdef WITH_ZLIB
	void krom_inflate(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		bool raw = args[1]->ToBoolean(isolate)->Value();

		unsigned char* inflated = (unsigned char*)malloc(content.ByteLength());

		z_stream infstream;
		infstream.zalloc = Z_NULL;
		infstream.zfree = Z_NULL;
		infstream.opaque = Z_NULL;
		infstream.avail_in = (uInt)content.ByteLength();
		infstream.next_in = (Bytef *)content.Data();
		infstream.avail_out = (uInt)content.ByteLength();
		infstream.next_out = (Bytef *)inflated;

		inflateInit2(&infstream, raw ? -15 : 15 + 32);

		int i = 2;
		while (true) {
			int res = inflate(&infstream, Z_NO_FLUSH);
			if (res == Z_STREAM_END) break;
			if (infstream.avail_out == 0) {
				inflated = (unsigned char*)realloc(inflated, content.ByteLength() * i);
				infstream.avail_out = (uInt)content.ByteLength();
				infstream.next_out = (Bytef *)(inflated + content.ByteLength() * (i - 1));
				i++;
			}
		}

		inflateEnd(&infstream);

		Local<ArrayBuffer> output = ArrayBuffer::New(isolate, infstream.total_out);
		ArrayBuffer::Contents outputContent = output->GetContents();
		memcpy(outputContent.Data(), inflated, infstream.total_out);
		free(inflated);

		args.GetReturnValue().Set(output);
	}

	void krom_deflate(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		bool raw = args[1]->ToBoolean(isolate)->Value();

		int deflatedSize = compressBound((uInt)content.ByteLength());
		void* deflated = malloc(deflatedSize);

		z_stream defstream;
		defstream.zalloc = Z_NULL;
		defstream.zfree = Z_NULL;
		defstream.opaque = Z_NULL;
		defstream.avail_in = (uInt)content.ByteLength();
		defstream.next_in = (Bytef *)content.Data();
		defstream.avail_out = deflatedSize;
		defstream.next_out = (Bytef *)deflated;

		deflateInit2(&defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, raw ? -15 : 15, 5, Z_DEFAULT_STRATEGY);
		deflate(&defstream, Z_FINISH);
		deflateEnd(&defstream);

		Local<ArrayBuffer> output = ArrayBuffer::New(isolate, defstream.total_out);
		ArrayBuffer::Contents outputContent = output->GetContents();
		memcpy(outputContent.Data(), deflated, defstream.total_out);
		free(deflated);

		args.GetReturnValue().Set(output);
	}
	#endif

	#ifdef WITH_STB_IMAGE_WRITE
	void krom_write_png(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
	}

	void krom_write_jpg(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
	}
	#endif

	#ifdef WITH_TEXSYNTH
	void krom_texsynth_inpaint(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int32_t w = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int32_t h = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<ArrayBuffer> bufferOut = Local<ArrayBuffer>::Cast(args[2]);
		ArrayBuffer::Contents contentOut = bufferOut->GetContents();
		Local<ArrayBuffer> bufferImage = Local<ArrayBuffer>::Cast(args[3]);
		ArrayBuffer::Contents contentImage = bufferImage->GetContents();
		Local<ArrayBuffer> bufferMask = Local<ArrayBuffer>::Cast(args[4]);
		ArrayBuffer::Contents contentMask = bufferMask->GetContents();
		bool tiling = args[5]->ToBoolean(isolate)->Value();
		texsynth_inpaint(w, h, contentOut.Data(), contentImage.Data(), contentMask.Data(), tiling);
	}
	#endif

	#ifdef WITH_ONNX
	void krom_ml_inference(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		const OrtApi *ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
		OrtEnv *env;
		ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "armorcore", &env);

		OrtSessionOptions *session_options;
		ort->CreateSessionOptions(&session_options);
		ort->SetIntraOpNumThreads(session_options, 8);
		ort->SetInterOpNumThreads(session_options, 8);

		#ifdef KORE_WINDOWS
		// ort->SetSessionExecutionMode(session_options, ORT_SEQUENTIAL);
		// ort->DisableMemPattern(session_options);
		// OrtSessionOptionsAppendExecutionProvider_DML(session_options, 0);
		#endif

		Local<ArrayBuffer> model_buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents model_content;
		model_content = model_buffer->GetContents();

		Local<ArrayBuffer> tensor_buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents tensor_content;
		tensor_content = tensor_buffer->GetContents();

		OrtSession *session;
		ort->CreateSessionFromArray(env, model_content.Data(), (int)model_content.ByteLength(), session_options, &session);
		OrtAllocator *allocator;
		ort->GetAllocatorWithDefaultOptions(&allocator);
		char *input_node_name;
		ort->SessionGetInputName(session, 0, allocator, &input_node_name);
		char *output_node_name;
		ort->SessionGetOutputName(session, 0, allocator, &output_node_name);

		OrtTypeInfo *input_type_info;
		ort->SessionGetInputTypeInfo(session, 0, &input_type_info);
		const OrtTensorTypeAndShapeInfo *input_tensor_info;
		ort->CastTypeInfoToTensorInfo(input_type_info, &input_tensor_info);
		size_t num_input_dims;
		ort->GetDimensionsCount(input_tensor_info, &num_input_dims);
		std::vector<int64_t> input_node_dims(num_input_dims);
		ort->GetDimensions(input_tensor_info, (int64_t *)input_node_dims.data(), num_input_dims);
		ort->ReleaseTypeInfo(input_type_info);

		OrtTypeInfo *output_type_info;
		ort->SessionGetOutputTypeInfo(session, 0, &output_type_info);
		const OrtTensorTypeAndShapeInfo *output_tensor_info;
		ort->CastTypeInfoToTensorInfo(output_type_info, &output_tensor_info);
		size_t num_output_dims;
		ort->GetDimensionsCount(output_tensor_info, &num_output_dims);
		std::vector<int64_t> output_node_dims(num_output_dims);
		ort->GetDimensions(output_tensor_info, (int64_t *)output_node_dims.data(), num_output_dims);
		ort->ReleaseTypeInfo(output_type_info);
		size_t output_byte_length = 4 * output_node_dims[0];
		for (int i = 0; i < num_output_dims; ++i) output_byte_length *= output_node_dims[i];

		OrtMemoryInfo *memory_info;
		ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);
		OrtValue *input_tensor = NULL;
		ort->CreateTensorWithDataAsOrtValue(memory_info, tensor_content.Data(), (int)tensor_content.ByteLength(), input_node_dims.data(), 4, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input_tensor);
		ort->ReleaseMemoryInfo(memory_info);

		OrtValue *output_tensor = NULL;
		ort->Run(session, NULL, &input_node_name, (const OrtValue* const*)&input_tensor, 1, &output_node_name, 1, &output_tensor);

		float *float_array;
		ort->GetTensorMutableData(output_tensor, (void**)&float_array);

		Local<ArrayBuffer> output = ArrayBuffer::New(isolate, output_byte_length);
		ArrayBuffer::Contents output_content = output->GetContents();
		memcpy(output_content.Data(), float_array, output_byte_length);

		ort->ReleaseValue(output_tensor);
		ort->ReleaseValue(input_tensor);
		ort->ReleaseSession(session);
		ort->ReleaseSessionOptions(session_options);
		ort->ReleaseEnv(env);
		args.GetReturnValue().Set(output);
	}
	#endif

	#ifdef KORE_RAYTRACE
	void krom_raytrace_init(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		if (accel_created) {
			kinc_g5_constant_buffer_destroy(&constant_buffer);
			kinc_raytrace_acceleration_structure_destroy(&accel);
			kinc_raytrace_pipeline_destroy(&pipeline);
		}

		Local<ArrayBuffer> shader_buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents shader_content;
		shader_content = shader_buffer->GetContents();

		Local<External> vb_field = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* vertex_buffer4 = (kinc_g4_vertex_buffer_t*)vb_field->Value();
		kinc_g5_vertex_buffer_t* vertex_buffer = &vertex_buffer4->impl._buffer;

		Local<External> ib_field = Local<External>::Cast(args[2]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* index_buffer4 = (kinc_g4_index_buffer_t*)ib_field->Value();
		kinc_g5_index_buffer_t* index_buffer = &index_buffer4->impl._buffer;

		float scale = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		kinc_g5_constant_buffer_init(&constant_buffer, constant_buffer_size * 4);

		kinc_raytrace_pipeline_init(&pipeline, &commandList, shader_content.Data(), (int)shader_content.ByteLength(), &constant_buffer);

		kinc_raytrace_acceleration_structure_init(&accel, &commandList, vertex_buffer, index_buffer, scale);
		accel_created = true;
	}

	void krom_raytrace_set_textures(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> rtfield0 = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texpaint0 = (kinc_g4_render_target_t*)rtfield0->Value();

		Local<External> rtfield1 = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texpaint1 = (kinc_g4_render_target_t*)rtfield1->Value();

		Local<External> rtfield2 = Local<External>::Cast(args[2]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texpaint2 = (kinc_g4_render_target_t*)rtfield2->Value();

		Local<External> envfield = Local<External>::Cast(args[3]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texenv = (kinc_g4_texture_t*)envfield->Value();

		Local<External> sobolfield = Local<External>::Cast(args[4]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texsobol = (kinc_g4_texture_t*)sobolfield->Value();

		Local<External> scramblefield = Local<External>::Cast(args[5]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texscramble = (kinc_g4_texture_t*)scramblefield->Value();

		Local<External> rankfield = Local<External>::Cast(args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texrank = (kinc_g4_texture_t*)rankfield->Value();

		if (!texenv->impl._uploaded) {
			kinc_g5_command_list_upload_texture(&commandList, &texenv->impl._texture);
			texenv->impl._uploaded = true;
		}
		if (!texsobol->impl._uploaded) {
			kinc_g5_command_list_upload_texture(&commandList, &texsobol->impl._texture);
			texsobol->impl._uploaded = true;
		}
		if (!texscramble->impl._uploaded) {
			kinc_g5_command_list_upload_texture(&commandList, &texscramble->impl._texture);
			texscramble->impl._uploaded = true;
		}
		if (!texrank->impl._uploaded) {
			kinc_g5_command_list_upload_texture(&commandList, &texrank->impl._texture);
			texrank->impl._uploaded = true;
		}

		kinc_raytrace_set_textures(&texpaint0->impl._renderTarget, &texpaint1->impl._renderTarget, &texpaint2->impl._renderTarget, &texenv->impl._texture, &texsobol->impl._texture, &texscramble->impl._texture, &texrank->impl._texture);
	}

	void krom_raytrace_dispatch_rays(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> rtfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		render_target = (kinc_g4_render_target_t*)rtfield->Value();

		Local<ArrayBuffer> cb_buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents cb_content;
		cb_content = cb_buffer->GetContents();
		float* cb = (float*)cb_content.Data();

		kinc_g5_constant_buffer_lock_all(&constant_buffer);
		for (int i = 0; i < constant_buffer_size; ++i) {
			kinc_g5_constant_buffer_set_float(&constant_buffer, i * 4, cb[i]);
		}
		kinc_g5_constant_buffer_unlock(&constant_buffer);

		kinc_raytrace_set_acceleration_structure(&accel);
		kinc_raytrace_set_pipeline(&pipeline);
		kinc_raytrace_set_target(&render_target->impl._renderTarget);
		kinc_raytrace_dispatch_rays(&commandList);
	}
	#endif

	#ifdef KORE_VR
	void krom_vr_begin(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_vr_interface_begin();
	}

	void krom_vr_begin_render(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int eye = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_vr_interface_begin_render(eye);
	}

	void krom_vr_end_render(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int eye = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_vr_interface_end_render(eye);
	}

	void krom_vr_warp_swap(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_vr_interface_warp_swap();
	}

	void krom_vr_get_sensor_state_view(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int eye = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_vr_sensor_state_t state = kinc_vr_interface_get_sensor_state(eye);
		kinc_matrix4x4_t view = state.pose.vrPose.eye;
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_00").ToLocalChecked(), Number::New(isolate, view.m[0]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_01").ToLocalChecked(), Number::New(isolate, view.m[1]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_02").ToLocalChecked(), Number::New(isolate, view.m[2]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_03").ToLocalChecked(), Number::New(isolate, view.m[3]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_10").ToLocalChecked(), Number::New(isolate, view.m[4]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_11").ToLocalChecked(), Number::New(isolate, view.m[5]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_12").ToLocalChecked(), Number::New(isolate, view.m[6]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_13").ToLocalChecked(), Number::New(isolate, view.m[7]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_20").ToLocalChecked(), Number::New(isolate, view.m[8]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_21").ToLocalChecked(), Number::New(isolate, view.m[9]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_22").ToLocalChecked(), Number::New(isolate, view.m[10]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_23").ToLocalChecked(), Number::New(isolate, view.m[11]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_30").ToLocalChecked(), Number::New(isolate, view.m[12]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_31").ToLocalChecked(), Number::New(isolate, view.m[13]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_32").ToLocalChecked(), Number::New(isolate, view.m[14]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_33").ToLocalChecked(), Number::New(isolate, view.m[15]));
		args.GetReturnValue().Set(obj);
	}

	void krom_vr_get_sensor_state_projection(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int eye = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_vr_sensor_state_t state = kinc_vr_interface_get_sensor_state(eye);
		kinc_matrix4x4_t proj = state.pose.vrPose.projection;
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_00").ToLocalChecked(), Number::New(isolate, proj.m[0]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_01").ToLocalChecked(), Number::New(isolate, proj.m[1]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_02").ToLocalChecked(), Number::New(isolate, proj.m[2]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_03").ToLocalChecked(), Number::New(isolate, proj.m[3]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_10").ToLocalChecked(), Number::New(isolate, proj.m[4]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_11").ToLocalChecked(), Number::New(isolate, proj.m[5]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_12").ToLocalChecked(), Number::New(isolate, proj.m[6]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_13").ToLocalChecked(), Number::New(isolate, proj.m[7]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_20").ToLocalChecked(), Number::New(isolate, proj.m[8]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_21").ToLocalChecked(), Number::New(isolate, proj.m[9]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_22").ToLocalChecked(), Number::New(isolate, proj.m[10]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_23").ToLocalChecked(), Number::New(isolate, proj.m[11]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_30").ToLocalChecked(), Number::New(isolate, proj.m[12]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_31").ToLocalChecked(), Number::New(isolate, proj.m[13]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_32").ToLocalChecked(), Number::New(isolate, proj.m[14]));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "_33").ToLocalChecked(), Number::New(isolate, proj.m[15]));
		args.GetReturnValue().Set(obj);
	}

	void krom_vr_get_sensor_state_hmd_mounted(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_vr_sensor_state_t state = kinc_vr_interface_get_sensor_state(0);
		args.GetReturnValue().Set(Boolean::New(isolate, state.pose.hmdMounted));
	}
	#endif

	void krom_window_x(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		// args.GetReturnValue().Set(Int32::New(isolate, kinc_window_x(windowId))); // Returns window creation pos
		#ifdef KORE_WINDOWS
		RECT rect;
		GetWindowRect(kinc_windows_window_handle(windowId), &rect);
		args.GetReturnValue().Set(Int32::New(isolate, rect.left));
		#endif
	}

	void krom_window_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		// args.GetReturnValue().Set(Int32::New(isolate, kinc_window_y(windowId)));
		#ifdef KORE_WINDOWS
		RECT rect;
		GetWindowRect(kinc_windows_window_handle(windowId), &rect);
		args.GetReturnValue().Set(Int32::New(isolate, rect.top));
		#endif
	}

	void krom_language(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_language()).ToLocalChecked());
	}

	void start_v8(char *krom_bin, int krom_bin_size) {
		plat = platform::NewDefaultPlatform();
		V8::InitializePlatform(plat.get());

		std::string flags = "";
		#ifdef KORE_IOS
		flags += "--jitless ";
		#endif
		if (profile) flags += "--logfile=krom-v8.log --prof --log-source-code ";
		V8::SetFlagsFromString(flags.c_str(), (int)flags.size());

		V8::Initialize();

		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		StartupData blob;
		if (krom_bin_size > 0) {
			blob.data = krom_bin;
			blob.raw_size = krom_bin_size;
			create_params.snapshot_blob = &blob;
		}
		isolate = Isolate::New(create_params);

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);

		Local<ObjectTemplate> krom = ObjectTemplate::New(isolate);
		krom->Set(String::NewFromUtf8(isolate, "init").ToLocalChecked(), FunctionTemplate::New(isolate, krom_init));
		krom->Set(String::NewFromUtf8(isolate, "setApplicationName").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_application_name));
		krom->Set(String::NewFromUtf8(isolate, "log").ToLocalChecked(), FunctionTemplate::New(isolate, krom_log));
		krom->Set(String::NewFromUtf8(isolate, "clear").ToLocalChecked(), FunctionTemplate::New(isolate, krom_clear));
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
		krom->Set(String::NewFromUtf8(isolate, "setTouchDownCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_touch_down_callback));
		krom->Set(String::NewFromUtf8(isolate, "setTouchUpCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_touch_up_callback));
		krom->Set(String::NewFromUtf8(isolate, "setTouchMoveCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_touch_move_callback));
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
		krom->Set(String::NewFromUtf8(isolate, "setMousePosition").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_position));
		krom->Set(String::NewFromUtf8(isolate, "showMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_show_mouse));
		krom->Set(String::NewFromUtf8(isolate, "showKeyboard").ToLocalChecked(), FunctionTemplate::New(isolate, krom_show_keyboard));
		krom->Set(String::NewFromUtf8(isolate, "createIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "deleteIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "lockIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "unlockIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_indexbuffer));
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
		krom->Set(String::NewFromUtf8(isolate, "audioThread").ToLocalChecked(), FunctionTemplate::New(isolate, krom_audio_thread));
		krom->Set(String::NewFromUtf8(isolate, "writeAudioBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_write_audio_buffer));
		krom->Set(String::NewFromUtf8(isolate, "loadBlob").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_blob));
		krom->Set(String::NewFromUtf8(isolate, "loadUrl").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_url));
		krom->Set(String::NewFromUtf8(isolate, "getConstantLocation").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_constant_location));
		krom->Set(String::NewFromUtf8(isolate, "getTextureUnit").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_unit));
		krom->Set(String::NewFromUtf8(isolate, "setTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture));
		krom->Set(String::NewFromUtf8(isolate, "setRenderTarget").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_render_target));
		krom->Set(String::NewFromUtf8(isolate, "setTextureDepth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_depth));
		krom->Set(String::NewFromUtf8(isolate, "setImageTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_image_texture));
		krom->Set(String::NewFromUtf8(isolate, "setTextureParameters").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_parameters));
		krom->Set(String::NewFromUtf8(isolate, "setTexture3DParameters").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture3d_parameters));
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
		krom->Set(String::NewFromUtf8(isolate, "getWindowMode").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_window_mode));
		krom->Set(String::NewFromUtf8(isolate, "setWindowMode").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_window_mode));
		krom->Set(String::NewFromUtf8(isolate, "resizeWindow").ToLocalChecked(), FunctionTemplate::New(isolate, krom_resize_window));
		krom->Set(String::NewFromUtf8(isolate, "moveWindow").ToLocalChecked(), FunctionTemplate::New(isolate, krom_move_window));
		krom->Set(String::NewFromUtf8(isolate, "screenDpi").ToLocalChecked(), FunctionTemplate::New(isolate, krom_screen_dpi));
		krom->Set(String::NewFromUtf8(isolate, "systemId").ToLocalChecked(), FunctionTemplate::New(isolate, krom_system_id));
		krom->Set(String::NewFromUtf8(isolate, "requestShutdown").ToLocalChecked(), FunctionTemplate::New(isolate, krom_request_shutdown));
		krom->Set(String::NewFromUtf8(isolate, "displayCount").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_count));
		krom->Set(String::NewFromUtf8(isolate, "displayWidth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_width));
		krom->Set(String::NewFromUtf8(isolate, "displayHeight").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_height));
		krom->Set(String::NewFromUtf8(isolate, "displayX").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_x));
		krom->Set(String::NewFromUtf8(isolate, "displayY").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_y));
		krom->Set(String::NewFromUtf8(isolate, "displayFrequency").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_frequency));
		krom->Set(String::NewFromUtf8(isolate, "displayIsPrimary").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_is_primary));
		krom->Set(String::NewFromUtf8(isolate, "writeStorage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_write_storage));
		krom->Set(String::NewFromUtf8(isolate, "readStorage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_read_storage));
		krom->Set(String::NewFromUtf8(isolate, "createRenderTarget").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_render_target));
		krom->Set(String::NewFromUtf8(isolate, "createRenderTargetCubeMap").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_render_target_cube_map));
		krom->Set(String::NewFromUtf8(isolate, "createTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture));
		krom->Set(String::NewFromUtf8(isolate, "createTexture3D").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture3d));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromBytes").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_bytes));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromBytes3D").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_bytes3d));
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
		krom->Set(String::NewFromUtf8(isolate, "deleteFile").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_file));
		krom->Set(String::NewFromUtf8(isolate, "copyFile").ToLocalChecked(), FunctionTemplate::New(isolate, krom_copy_file));
		krom->Set(String::NewFromUtf8(isolate, "createDirectory").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_directory));
		krom->Set(String::NewFromUtf8(isolate, "openInStdApp").ToLocalChecked(), FunctionTemplate::New(isolate, krom_open_in_std_app));
		krom->Set(String::NewFromUtf8(isolate, "workingDir").ToLocalChecked(), FunctionTemplate::New(isolate, krom_working_dir));
		krom->Set(String::NewFromUtf8(isolate, "savePath").ToLocalChecked(), FunctionTemplate::New(isolate, krom_save_path));
		krom->Set(String::NewFromUtf8(isolate, "getArgCount").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_arg_count));
		krom->Set(String::NewFromUtf8(isolate, "getArg").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_arg));
		krom->Set(String::NewFromUtf8(isolate, "getFilesLocation").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_files_location));
		krom->Set(String::NewFromUtf8(isolate, "httpRequest").ToLocalChecked(), FunctionTemplate::New(isolate, krom_http_request));
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
		krom->Set(String::NewFromUtf8(isolate, "setTexture3DParametersCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture3d_parameters_compute));
		krom->Set(String::NewFromUtf8(isolate, "setShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "deleteShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "createShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "getConstantLocationCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_constant_location_compute));
		krom->Set(String::NewFromUtf8(isolate, "getTextureUnitCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_unit_compute));
		krom->Set(String::NewFromUtf8(isolate, "compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_compute));
		//
		krom->Set(String::NewFromUtf8(isolate, "setSaveAndQuitCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_save_and_quit_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseCursor").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_cursor));
		#if defined(WITH_NFD) || defined(KORE_IOS) || defined(KORE_ANDROID)
		krom->Set(String::NewFromUtf8(isolate, "openDialog").ToLocalChecked(), FunctionTemplate::New(isolate, krom_open_dialog));
		krom->Set(String::NewFromUtf8(isolate, "saveDialog").ToLocalChecked(), FunctionTemplate::New(isolate, krom_save_dialog));
		#endif
		#ifdef WITH_TINYDIR
		krom->Set(String::NewFromUtf8(isolate, "readDirectory").ToLocalChecked(), FunctionTemplate::New(isolate, krom_read_directory));
		#endif
		krom->Set(String::NewFromUtf8(isolate, "fileExists").ToLocalChecked(), FunctionTemplate::New(isolate, krom_file_exists));
		#ifdef WITH_ZLIB
		krom->Set(String::NewFromUtf8(isolate, "inflate").ToLocalChecked(), FunctionTemplate::New(isolate, krom_inflate));
		krom->Set(String::NewFromUtf8(isolate, "deflate").ToLocalChecked(), FunctionTemplate::New(isolate, krom_deflate));
		#endif
		#ifdef WITH_STB_IMAGE_WRITE
		krom->Set(String::NewFromUtf8(isolate, "writePng").ToLocalChecked(), FunctionTemplate::New(isolate, krom_write_png));
		krom->Set(String::NewFromUtf8(isolate, "writeJpg").ToLocalChecked(), FunctionTemplate::New(isolate, krom_write_jpg));
		#endif
		#ifdef WITH_TEXSYNTH
		krom->Set(String::NewFromUtf8(isolate, "texsynthInpaint").ToLocalChecked(), FunctionTemplate::New(isolate, krom_texsynth_inpaint));
		#endif
		#ifdef WITH_ONNX
		krom->Set(String::NewFromUtf8(isolate, "mlInference").ToLocalChecked(), FunctionTemplate::New(isolate, krom_ml_inference));
		#endif
		#ifdef KORE_RAYTRACE
		krom->Set(String::NewFromUtf8(isolate, "raytraceInit").ToLocalChecked(), FunctionTemplate::New(isolate, krom_raytrace_init));
		krom->Set(String::NewFromUtf8(isolate, "raytraceSetTextures").ToLocalChecked(), FunctionTemplate::New(isolate, krom_raytrace_set_textures));
		krom->Set(String::NewFromUtf8(isolate, "raytraceDispatchRays").ToLocalChecked(), FunctionTemplate::New(isolate, krom_raytrace_dispatch_rays));
		#endif
		#ifdef KORE_VR
		krom->Set(String::NewFromUtf8(isolate, "vrBegin").ToLocalChecked(), FunctionTemplate::New(isolate, krom_vr_begin));
		krom->Set(String::NewFromUtf8(isolate, "vrBeginRender").ToLocalChecked(), FunctionTemplate::New(isolate, krom_vr_begin_render));
		krom->Set(String::NewFromUtf8(isolate, "vrEndRender").ToLocalChecked(), FunctionTemplate::New(isolate, krom_vr_end_render));
		krom->Set(String::NewFromUtf8(isolate, "vrWarpSwap").ToLocalChecked(), FunctionTemplate::New(isolate, krom_vr_warp_swap));
		krom->Set(String::NewFromUtf8(isolate, "vrGetSensorStateView").ToLocalChecked(), FunctionTemplate::New(isolate, krom_vr_get_sensor_state_view));
		krom->Set(String::NewFromUtf8(isolate, "vrGetSensorStateProjection").ToLocalChecked(), FunctionTemplate::New(isolate, krom_vr_get_sensor_state_projection));
		krom->Set(String::NewFromUtf8(isolate, "vrGetSensorStateHmdMounted").ToLocalChecked(), FunctionTemplate::New(isolate, krom_vr_get_sensor_state_hmd_mounted));
		#endif
		krom->Set(String::NewFromUtf8(isolate, "windowX").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_x));
		krom->Set(String::NewFromUtf8(isolate, "windowY").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_y));
		krom->Set(String::NewFromUtf8(isolate, "language").ToLocalChecked(), FunctionTemplate::New(isolate, krom_language));

		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		global->Set(String::NewFromUtf8(isolate, "Krom").ToLocalChecked(), krom);

		Local<Context> context = Context::New(isolate, NULL, global);
		global_context.Reset(isolate, context);
	}

	void start_krom(char* scriptfile) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		if (scriptfile != NULL) {
			Local<String> source = String::NewFromUtf8(isolate, scriptfile, NewStringType::kNormal).ToLocalChecked();

			TryCatch try_catch(isolate);
			Local<Script> compiled_script = Script::Compile(isolate->GetCurrentContext(), source).ToLocalChecked();

			Local<Value> result;
			if (!compiled_script->Run(context).ToLocal(&result)) {
				handle_exception(&try_catch);
			}
		}
		else {
			TryCatch try_catch(isolate);
			v8::Local<v8::Value> js_kickstart = context->Global()->Get(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "kickstart").ToLocalChecked()).ToLocalChecked();
			if (!js_kickstart->IsNullOrUndefined()) {
				Local<Value> result;
				if (!js_kickstart->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->CallAsFunction(context, context->Global(), 0, nullptr).ToLocal(&result)) {
					handle_exception(&try_catch);
				}
			}
		}
	}

	void run_v8() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		v8::MicrotasksScope microtasks_scope(isolate, v8::MicrotasksScope::kRunMicrotasks);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<v8::Function> func = Local<v8::Function>::New(isolate, update_func);
		Local<Value> result;

		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		if (save_and_quit > 0) {
			v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, save_and_quit_func);
			Local<Value> result;
			const int argc = 1;
			Local<Value> argv[argc] = {Boolean::New(isolate, save_and_quit == 1)};
			if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
				handle_exception(&try_catch);
			}
			save_and_quit = 0;
		}
	}

	void end_v8() {
		update_func.Reset();
		global_context.Reset();
		isolate->Dispose();
		V8::Dispose();
		V8::ShutdownPlatform();
	}

	#ifdef WITH_AUDIO
	void update_audio(kinc_a2_buffer_t *buffer, int samples) {
		// kinc_mutex_lock(&mutex);
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		v8::MicrotasksScope microtasks_scope(isolate, v8::MicrotasksScope::kRunMicrotasks);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<v8::Function> func = Local<v8::Function>::New(isolate, audio_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, samples)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		for (int i = 0; i < samples; ++i) {
			float sample = *(float*)&audio_buffer.data[audio_buffer.read_location];
			audio_buffer.read_location += 4;
			if (audio_buffer.read_location >= audio_buffer.data_size) {
				audio_buffer.read_location = 0;
			}

			*(float*)&buffer->data[buffer->write_location] = sample;
			buffer->write_location += 4;
			if (buffer->write_location >= buffer->data_size) {
				buffer->write_location = 0;
			}
		}

		// kinc_mutex_unlock(&mutex);
	}
	#endif

	void update() {
		#ifdef KORE_WINDOWS
		if (paused && ++pausedFrames > 3 && armorcore) { Sleep(1); return; }
		#endif

		#ifdef IDLE_SLEEP
		if (++pausedFrames > 120) { usleep(1000); return; }
		#endif

		#ifdef WITH_AUDIO
		if (enable_sound) kinc_a2_update();
		#endif

		#ifdef WITH_WORKER
		handle_worker_messages(isolate, global_context);
		#endif

		kinc_g4_begin(0);
		run_v8();
		kinc_g4_end(0);
		kinc_g4_swap_buffers();

		#ifdef ARM_PROFILE
		if (startup_time > 0) {
			kinc_log(KINC_LOG_LEVEL_INFO, "Startup time: %f", kinc_time() - startup_time);
			startup_time = 0.0;
		}
		#endif
	}

	void drop_files(wchar_t* file_path) {
		// Update mouse position
		#ifdef KORE_WINDOWS
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(kinc_windows_window_handle(0), &p);
		mouse_move(0, p.x, p.y, 0, 0);
		#endif

		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, drop_files_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc];
		if (sizeof(wchar_t) == 2) {
			argv[0] = {String::NewFromTwoByte(isolate, (const uint16_t*)file_path).ToLocalChecked()};
		}
		else {
			size_t len = wcslen(file_path);
			uint16_t* str = new uint16_t[len + 1];
			for (int i = 0; i < len; i++) str[i] = file_path[i];
			str[len] = 0;
			argv[0] = {String::NewFromTwoByte(isolate, str).ToLocalChecked()};
			delete str;
		}
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		paused = false;
	}

	char* copy() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, copy_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
		String::Utf8Value cutCopyString(isolate, result);
		strcpy(temp_string, *cutCopyString);
		return temp_string;
	}

	char* cut() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, cut_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
		String::Utf8Value cutCopyString(isolate, result);
		strcpy(temp_string, *cutCopyString);
		return temp_string;
	}

	void paste(char* data) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, paste_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {String::NewFromUtf8(isolate, data).ToLocalChecked()};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void foreground() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, foreground_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		paused = false;
	}

	void resume() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, resume_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void pause() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pause_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void background() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, background_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		paused = true;
		pausedFrames = 0;
	}

	void shutdown() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, shutdown_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void key_down(int code) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboard_down_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void key_up(int code) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboard_up_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void key_press(unsigned int character) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboard_press_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, character)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void mouse_move(int window, int x, int y, int mx, int my) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_move_func);
		Local<Value> result;
		const int argc = 4;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Int32::New(isolate, mx), Int32::New(isolate, my)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void mouse_down(int window, int button, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void mouse_up(int window, int button, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void mouse_wheel(int window, int delta) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_wheel_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, delta)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void touch_move(int index, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, touch_move_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void touch_down(int index, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, touch_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void touch_up(int index, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, touch_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void pen_down(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pen_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void pen_up(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pen_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void pen_move(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pen_move_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void gamepad_axis(int gamepad, int axis, float value) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, gamepad_axis_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, axis), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}

	void gamepad_button(int gamepad, int button, float value) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, gamepad_button_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, button), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		pausedFrames = 0;
		#endif
	}
}

int kickstart(int argc, char** argv) {
	_argc = argc;
	_argv = argv;
#ifdef KORE_ANDROID
	std::string bindir("/");
#elif KORE_IOS
	std::string bindir("");
#else
	std::string bindir(argv[0]);
#endif

#ifdef KORE_WINDOWS // Handle non-ascii path
	HMODULE hModule = GetModuleHandleW(NULL);
	GetModuleFileNameW(hModule, temp_wstring, 1024);
	WideCharToMultiByte(CP_UTF8, 0, temp_wstring, -1, temp_string, 4096, nullptr, nullptr);
	bindir = temp_string;
#endif

#ifdef KORE_WINDOWS
	bindir = bindir.substr(0, bindir.find_last_of("\\"));
#else
	bindir = bindir.substr(0, bindir.find_last_of("/"));
#endif
	assetsdir = argc > 1 ? argv[1] : bindir;

	// Opening a file
	int l = (int)assetsdir.length();
	if ((l > 6 && assetsdir[l - 6] == '.') ||
		(l > 5 && assetsdir[l - 5] == '.') ||
		(l > 4 && assetsdir[l - 4] == '.')) {
		assetsdir = bindir;
	}

	bool read_console_pid = false;
	for (int i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "--nosound") == 0) {
			enable_sound = false;
		}
		else if (strcmp(argv[i], "--nowindow") == 0) {
			enable_window = false;
		}
		else if (strcmp(argv[i], "--prof") == 0) {
			profile = true;
		}
		else if (strcmp(argv[i], "--snapshot") == 0) {
			snapshot = true;
		}
		else if (read_console_pid) {
			#ifdef KORE_WINDOWS
			AttachConsole(atoi(argv[i]));
			#endif
			read_console_pid = false;
		}
		else if (strcmp(argv[i], "--consolepid") == 0) {
			read_console_pid = true;
		}
	}

#if !defined(KORE_MACOS) && !defined(KORE_IOS)
	kinc_internal_set_files_location(&assetsdir[0u]);
#endif

#ifdef ARM_PROFILE
	startup_time = kinc_time();
#endif

	bool snapshot_found = true;
	kinc_file_reader_t reader;
	if (snapshot || !kinc_file_reader_open(&reader, "krom.bin", KINC_FILE_TYPE_ASSET)) {
		if (!kinc_file_reader_open(&reader, "krom.js", KINC_FILE_TYPE_ASSET)) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Could not load krom.js, aborting.");
			exit(1);
		}
		snapshot_found = false;
	}

	int reader_size = (int)kinc_file_reader_size(&reader);
	char* code = (char*)malloc(reader_size + 1);
	kinc_file_reader_read(&reader, code, reader_size);
	code[reader_size] = 0;
	kinc_file_reader_close(&reader);

	if (snapshot) {
		plat = platform::NewDefaultPlatform();
		V8::InitializePlatform(plat.get());
		V8::Initialize();

		std::string flags = "--nolazy";
		V8::SetFlagsFromString(flags.c_str(), (int)flags.size());

		v8::ScriptCompiler::CachedData* cache;
		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		v8::Isolate* isolate_cache = Isolate::New(create_params);
		{
			v8::HandleScope handle_scope(isolate_cache);
			{
				v8::Local<v8::Context> context = Context::New(isolate_cache);
				v8::Context::Scope context_scope(context);

				v8::ScriptOrigin origin(String::NewFromUtf8(isolate_cache, "krom_cache").ToLocalChecked());
				v8::ScriptCompiler::Source source(String::NewFromUtf8(isolate_cache, code).ToLocalChecked(), origin);

				Local<Script> compiled_script = v8::ScriptCompiler::Compile(context, &source, v8::ScriptCompiler::kEagerCompile).ToLocalChecked();
				cache = v8::ScriptCompiler::CreateCodeCache(compiled_script->GetUnboundScript());
			}
		}

		v8::SnapshotCreator creator_cold;
		v8::Isolate* isolate_cold = creator_cold.GetIsolate();
		{
			v8::HandleScope handle_scope(isolate_cold);
			{
				v8::Local<v8::Context> context = Context::New(isolate_cold);
				v8::Context::Scope context_scope(context);

				const size_t line_size = 512;
				char line[line_size];
				strcpy(line, assetsdir.c_str());
				strcat(line, "/data/embed.txt");
				FILE *fp = fopen (line, "r");
				if (fp != NULL) {
					while (fgets(line, line_size, fp) != NULL)  {
						line[strlen(line) - 1] = 0; // Trim \n
						kinc_file_reader_t reader;
						if (!kinc_file_reader_open(&reader, line, KINC_FILE_TYPE_ASSET)) continue;
						int reader_size = (int)kinc_file_reader_size(&reader);

						Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate_cold, reader_size);
						ArrayBuffer::Contents contents = buffer->GetContents();
						kinc_file_reader_read(&reader, contents.Data(), reader_size);
						kinc_file_reader_close(&reader);

						context->Global()->Set(context, String::NewFromUtf8(isolate_cold, line).ToLocalChecked(), buffer);
					}
					fclose (fp);
				}

				v8::ScriptOrigin origin(String::NewFromUtf8(isolate_cold, "krom_cold").ToLocalChecked());
				v8::ScriptCompiler::Source source(String::NewFromUtf8(isolate_cold, code).ToLocalChecked(), origin, cache);

				Local<Script> compiled_script = v8::ScriptCompiler::Compile(context, &source, v8::ScriptCompiler::kConsumeCodeCache).ToLocalChecked();
				compiled_script->Run(context);

				creator_cold.SetDefaultContext(context);
			}
		}
		StartupData coldData = creator_cold.CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kKeep);

		// SnapshotCreator creator_warm(nullptr, &coldData);
		// Isolate* isolate_warm = creator_warm.GetIsolate();
		// {
		// 	HandleScope handle_scope(isolate_warm);
		// 	{
		// 		Local<Context> context = Context::New(isolate_warm);
		// 		v8::Context::Scope context_scope(context);

		// 		// std::string code_warm("Main.main();");
		// 		v8::ScriptOrigin origin(String::NewFromUtf8(isolate_warm, "krom_warm").ToLocalChecked());
		// 		v8::ScriptCompiler::Source source(String::NewFromUtf8(isolate_warm, code).ToLocalChecked(), origin);

		// 		Local<Script> compiled_script = v8::ScriptCompiler::Compile(context, &source, v8::ScriptCompiler::kEagerCompile).ToLocalChecked();
		// 		compiled_script->Run(context);
		// 	}
		// }
		// {
		//   HandleScope handle_scope(isolate_warm);
		//   isolate_warm->ContextDisposedNotification(false);
		//   Local<Context> context = Context::New(isolate_warm);
		//   creator_warm.SetDefaultContext(context);
		// }
		// StartupData warmData = creator_warm.CreateBlob(SnapshotCreator::FunctionCodeHandling::kKeep);

		std::string krombin = assetsdir + "/krom.bin";
		FILE* file = fopen(&krombin[0u], "wb");
		if (file != nullptr) {
			// fwrite(warmData.data, 1, warmData.raw_size, file);
			fwrite(coldData.data, 1, coldData.raw_size, file);
			fclose(file);
		}
		exit(0);
	}

	kinc_threads_init();
	kinc_display_init();

	start_v8(snapshot_found ? code : NULL, snapshot_found ? reader_size : 0);
	#ifdef WITH_WORKER
	bind_worker_class(isolate, global_context);
	#endif
	start_krom(snapshot_found ? NULL : code);

	#ifdef ARM_PROFILE
	kinc_log(KINC_LOG_LEVEL_INFO, "Parse time: %f", kinc_time() - startup_time);
	#endif

	kinc_start();

	// #ifdef WITH_AUDIO
	// if (enable_sound) {
		// kinc_a2_shutdown();
		// kinc_mutex_lock(&mutex); // Prevent audio thread from running
	// }
	// #endif

	exit(0); // TODO

	free(code);
	end_v8();
	return 0;
}
