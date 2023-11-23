#include <map>
#include <string>
#include <vector>
#include <kinc/log.h>
#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>
#ifdef WITH_AUDIO
#include <kinc/audio1/audio.h>
#include <kinc/audio1/sound.h>
#include <kinc/audio2/audio.h>
#endif
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
#ifdef WITH_COMPUTE
#include <kinc/compute/compute.h>
#endif
#ifdef KORE_LZ4X
extern "C" int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxOutputSize);
#else
#include <kinc/io/lz4/lz4.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <kinc/libs/stb_image.h>
#ifdef KORE_DIRECT3D11
#include <d3d11.h>
#endif
#ifdef KORE_DIRECT3D12
#include <d3d12.h>
extern "C" extern bool waitAfterNextDraw;
#endif
#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/raytrace.h>
#endif
#ifdef KORE_VR
#include <kinc/vr/vrinterface.h>
#endif

#include <libplatform/libplatform.h>
#ifdef KORE_LINUX // xlib defines conflicting with v8
#undef True
#undef False
#undef None
#undef Status
#endif
#include <v8.h>
#include <v8-fast-api-calls.h>

#ifdef KORE_WINDOWS
#include <Windows.h> // AttachConsole
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
extern "C" { struct HWND__ *kinc_windows_window_handle(int window_index); } // Kore/Windows.h
bool show_window = false;
// Enable visual styles for ui controls
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifdef WITH_D3DCOMPILER
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <sstream>
#endif
#ifdef WITH_NFD
#include <nfd.h>
#elif defined(KORE_ANDROID)
#include "android/android_file_dialog.h"
#include "android/android_http_request.h"
#elif defined(KORE_IOS)
#include "ios/ios_file_dialog.h"
#endif
#ifdef WITH_TINYDIR
#include <tinydir.h>
#endif
#ifdef WITH_STB_IMAGE_WRITE
#ifdef WITH_ZLIB
extern "C" unsigned char *stbiw_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality);
#define STBIW_ZLIB_COMPRESS stbiw_zlib_compress
#endif
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif
#ifdef WITH_MPEG_WRITE
#include <jo_mpeg.h>
#endif
#ifdef WITH_ZLIB
#include <zlib.h>
#endif
#ifdef WITH_ONNX
#include <onnxruntime_c_api.h>
#ifdef KORE_WINDOWS
#include <dml_provider_factory.h>
#elif defined(KORE_MACOS)
#include <coreml_provider_factory.h>
#endif
#endif
#if defined(IDLE_SLEEP) && !defined(KORE_WINDOWS)
#include <unistd.h>
#endif
#ifdef WITH_WORKER
#include "worker.h"
#endif
#ifdef WITH_G2
extern "C" {
	#include "g2/g2.h"
	#include "g2/g2_ext.h"
}
#endif
#ifdef WITH_IRON
extern "C" {
	#include "iron/io_obj.h"
	#include "iron/iron_armpack.h"
}
#endif
#ifdef WITH_ZUI
extern "C" {
	#include "zui/zui.h"
	#include "zui/zui_ext.h"
	#include "zui/zui_nodes.h"
}
#endif

using namespace v8;

#ifdef WITH_PLUGIN_EMBED
void plugin_embed(v8::Isolate *isolate, Local<ObjectTemplate> global);
#endif
#ifdef KORE_MACOS
extern "C" const char *macgetresourcepath();
#endif
#ifdef KORE_IOS
extern "C" const char *iphonegetresourcepath();
#endif

const int KROM_API = 6;

#if defined(KORE_IOS) || defined(KORE_ANDROID)
char mobile_title[1024];
#endif

#if defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)
extern "C" extern int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
#endif

#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
extern "C" {
	extern kinc_g5_command_list_t commandList;
}
static kinc_g5_constant_buffer_t constant_buffer;
static kinc_g4_render_target_t *render_target;
static kinc_raytrace_pipeline_t pipeline;
static kinc_raytrace_acceleration_structure_t accel;
static bool accel_created = false;
const int constant_buffer_size = 24;
#endif

namespace {
	int _argc;
	char **_argv;
	bool enable_window = true;
	#ifdef WITH_AUDIO
	bool enable_audio = true;
	#endif
	bool snapshot = false;
	bool stderr_created = false;
	bool in_background = false;
	int paused_frames = 0;
	bool armorcore = false;
	#ifdef IDLE_SLEEP
	bool input_down = false;
	int last_window_width = 0;
	int last_window_height = 0;
	#endif

	Isolate *isolate;
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
	Global<Function> save_and_quit_func;
	Global<Function> picker_func;
	#ifdef WITH_ZUI
	Global<Function> on_border_hover_func;
	Global<Function> on_text_hover_func;
	Global<Function> on_deselect_text_func;
	Global<Function> on_tab_drop_func;
	#endif

	bool save_and_quit_func_set = false;
	void update(void *data);
	void drop_files(wchar_t *file_path, void *data);
	char *cut(void *data);
	char *copy(void *data);
	void paste(char *text, void *data);
	void foreground(void *data);
	void resume(void *data);
	void pause(void *data);
	void background(void *data);
	void shutdown(void *data);
	void key_down(int code, void *data);
	void key_up(int code, void *data);
	void key_press(unsigned int character, void *data);
	void mouse_move(int window, int x, int y, int mx, int my, void *data);
	void mouse_down(int window, int button, int x, int y, void *data);
	void mouse_up(int window, int button, int x, int y, void *data);
	void mouse_wheel(int window, int delta, void *data);
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
	#ifdef WITH_ONNX
	const OrtApi *ort = NULL;
	OrtEnv *ort_env;
	OrtSessionOptions *ort_session_options;
	OrtSession *session = NULL;
	#endif

	class KromCallbackdata {
	public:
		int32_t size;
		Global<Function> func;
	};

	void write_stack_trace(const char *stack_trace) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", stack_trace);

		#ifdef KORE_WINDOWS
		FILE *file = fopen("stderr.txt", stderr_created ? "a" : "w");
		if (file == nullptr) { // Running from protected path
			strcpy(temp_string, kinc_internal_save_path());
			strcat(temp_string, "\\stderr.txt");
			file = fopen(temp_string, stderr_created ? "a" : "w");
		}
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
			String::Utf8Value stack_trace(isolate, try_catch->Message()->Get());
			write_stack_trace(*stack_trace);
		}
		else {
			String::Utf8Value stack_trace(isolate, trace.ToLocalChecked());
			write_stack_trace(*stack_trace);
		}
	}

	void krom_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value title(isolate, arg);
		int width = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int samples_per_pixel = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool vertical_sync = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
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
			const char *outdated;
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
		#ifdef KORE_WINDOWS
		win.visible = false; // Prevent white flicker when opening the window
		#else
		win.visible = enable_window;
		#endif
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
		kinc_random_init((int)(kinc_time() * 1000));

		#ifdef KORE_WINDOWS
		// Maximized window has x < -1, prevent window centering done by kinc
		if (x < -1 && y < -1) {
			kinc_window_move(0, x, y);
		}

		char vdata[4];
		DWORD cbdata = 4 * sizeof(char);
		RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, vdata, &cbdata);
		BOOL use_dark_mode = int(vdata[3] << 24 | vdata[2] << 16 | vdata[1] << 8 | vdata[0]) != 1;
		DwmSetWindowAttribute(kinc_windows_window_handle(0), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));

		show_window = true;
		#endif

		#ifdef WITH_AUDIO
		if (enable_audio) {
			kinc_a1_init();
			kinc_a2_init();
		}
		#endif

		kinc_set_update_callback(update, NULL);
		kinc_set_drop_files_callback(drop_files, NULL);
		kinc_set_copy_callback(copy, NULL);
		kinc_set_cut_callback(cut, NULL);
		kinc_set_paste_callback(paste, NULL);
		kinc_set_foreground_callback(foreground, NULL);
		kinc_set_resume_callback(resume, NULL);
		kinc_set_pause_callback(pause, NULL);
		kinc_set_background_callback(background, NULL);
		kinc_set_shutdown_callback(shutdown, NULL);

		kinc_keyboard_set_key_down_callback(key_down, NULL);
		kinc_keyboard_set_key_up_callback(key_up, NULL);
		kinc_keyboard_set_key_press_callback(key_press, NULL);
		kinc_mouse_set_move_callback(mouse_move, NULL);
		kinc_mouse_set_press_callback(mouse_down, NULL);
		kinc_mouse_set_release_callback(mouse_up, NULL);
		kinc_mouse_set_scroll_callback(mouse_wheel, NULL);
		kinc_surface_set_move_callback(touch_move);
		kinc_surface_set_touch_start_callback(touch_down);
		kinc_surface_set_touch_end_callback(touch_up);
		kinc_pen_set_press_callback(pen_down);
		kinc_pen_set_move_callback(pen_move);
		kinc_pen_set_release_callback(pen_up);
		kinc_gamepad_set_axis_callback(gamepad_axis);
		kinc_gamepad_set_button_callback(gamepad_button);

		#ifdef KORE_ANDROID
		android_check_permissions();
		#endif
	}

	void krom_set_application_name(const FunctionCallbackInfo<Value> &args) {
		// Name used by kinc_internal_save_path()
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[0]);
		kinc_set_application_name(*name);
	}

	void krom_log(const FunctionCallbackInfo<Value> &args) {
		if (args.Length() < 1) return;
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value value(isolate, arg);
		size_t len = strlen(*value);
        kinc_log_level_t level;
        if (args.Length() > 1) {
            level = (kinc_log_level_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
        }
        else {
            level = KINC_LOG_LEVEL_INFO;
        }
		if (len < 2048) {
			kinc_log(level, *value);
		}
		else {
			int pos = 0;
			while (pos < len) {
				strncpy(temp_string, *value + pos, 2047);
				temp_string[2047] = 0;
				kinc_log(level, temp_string);
				pos += 2047;
			}
		}
	}

	void krom_clear_fast(Local<Object> receiver, int flags, int color, float depth, int stencil) {
		kinc_g4_clear(flags, color, depth, stencil);
	}

	void krom_clear(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int flags = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float depth = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int stencil = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		krom_clear_fast(args.This(), flags, color, depth, stencil);
	}

	void krom_set_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		update_func.Reset(isolate, func);
	}

	void krom_set_drop_files_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		drop_files_func.Reset(isolate, func);
	}

	void krom_set_cut_copy_paste_callback(const FunctionCallbackInfo<Value> &args) {
		// HandleScope scope(args.GetIsolate());
		// Local<Value> cutArg = args[0];
		// Local<Function> cutFunc = Local<Function>::Cast(cutArg);
		// cut_func.Reset(isolate, cutFunc);
		// Local<Value> copyArg = args[1];
		// Local<Function> copyFunc = Local<Function>::Cast(copyArg);
		// copy_func.Reset(isolate, copyFunc);
		// Local<Value> pasteArg = args[2];
		// Local<Function> pasteFunc = Local<Function>::Cast(pasteArg);
		// paste_func.Reset(isolate, pasteFunc);
	}

	void krom_set_application_state_callback(const FunctionCallbackInfo<Value> &args) {
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

	void krom_set_keyboard_down_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_down_func.Reset(isolate, func);
	}

	void krom_set_keyboard_up_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_up_func.Reset(isolate, func);
	}

	void krom_set_keyboard_press_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_press_func.Reset(isolate, func);
	}

	void krom_set_mouse_down_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_down_func.Reset(isolate, func);
	}

	void krom_set_mouse_up_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_up_func.Reset(isolate, func);
	}

	void krom_set_mouse_move_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_move_func.Reset(isolate, func);
	}

	void krom_set_touch_down_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		touch_down_func.Reset(isolate, func);
	}

	void krom_set_touch_up_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		touch_up_func.Reset(isolate, func);
	}

	void krom_set_touch_move_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		touch_move_func.Reset(isolate, func);
	}

	void krom_set_mouse_wheel_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_wheel_func.Reset(isolate, func);
	}

	void krom_set_pen_down_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_down_func.Reset(isolate, func);
	}

	void krom_set_pen_up_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_up_func.Reset(isolate, func);
	}

	void krom_set_pen_move_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_move_func.Reset(isolate, func);
	}

	void krom_set_gamepad_axis_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepad_axis_func.Reset(isolate, func);
	}

	void krom_set_gamepad_button_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepad_button_func.Reset(isolate, func);
	}

	void krom_lock_mouse_fast(Local<Object> receiver) {
		kinc_mouse_lock(0);
	}

	void krom_lock_mouse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		krom_lock_mouse_fast(args.This());
	}

	void krom_unlock_mouse_fast(Local<Object> receiver) {
		kinc_mouse_unlock();
	}

	void krom_unlock_mouse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		krom_unlock_mouse_fast(args.This());
	}

	int krom_can_lock_mouse_fast(Local<Object> receiver) {
		return kinc_mouse_can_lock();
	}

	void krom_can_lock_mouse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, krom_can_lock_mouse_fast(args.This())));
	}

	int krom_is_mouse_locked_fast(Local<Object> receiver) {
		return kinc_mouse_is_locked();
	}

	void krom_is_mouse_locked(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, krom_is_mouse_locked_fast(args.This())));
	}

	void krom_set_mouse_position_fast(Local<Object> receiver, int windowId, int x, int y) {
		kinc_mouse_set_position(windowId, x, y);
	}

	void krom_set_mouse_position(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		krom_set_mouse_position_fast(args.This(), windowId, x, y);
	}

	void krom_show_mouse_fast(Local<Object> receiver, int show) {
		show ? kinc_mouse_show() : kinc_mouse_hide();
	}

	void krom_show_mouse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int show = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		krom_show_mouse_fast(args.This(), show);
	}

	void krom_show_keyboard_fast(Local<Object> receiver, int show) {
		show ? kinc_keyboard_show() : kinc_keyboard_hide();
	}

	void krom_show_keyboard(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int show = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		krom_show_keyboard_fast(args.This(), show);
	}

	void krom_create_indexbuffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)malloc(sizeof(kinc_g4_index_buffer_t));
		kinc_g4_index_buffer_init(buffer, args[0]->Int32Value(isolate->GetCurrentContext()).FromJust(), KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, buffer));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_indexbuffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)field->Value();
		kinc_g4_index_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_lock_indexbuffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)field->Value();
		int *vertices = (int *)kinc_g4_index_buffer_lock_all(buffer);
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)vertices, kinc_g4_index_buffer_count(buffer) * sizeof(int), [](void *, size_t, void *) {}, nullptr);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(Uint32Array::New(abuffer, 0, kinc_g4_index_buffer_count(buffer)));
	}

	void krom_unlock_indexbuffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)field->Value();
		kinc_g4_index_buffer_unlock_all(buffer);
	}

	void krom_set_indexbuffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)field->Value();
		kinc_g4_set_index_buffer(buffer);
	}

	kinc_g4_vertex_data_t convert_vertex_data(int num) {
		switch (num) {
		case 0: // Float32_1X
			return KINC_G4_VERTEX_DATA_F32_1X;
		case 1: // Float32_2X
			return KINC_G4_VERTEX_DATA_F32_2X;
		case 2: // Float32_3X
			return KINC_G4_VERTEX_DATA_F32_3X;
		case 3: // Float32_4X
			return KINC_G4_VERTEX_DATA_F32_4X;
		case 4: // Float32_4X4
			return KINC_G4_VERTEX_DATA_F32_4X4;
		case 5: // Int8_1X
			return KINC_G4_VERTEX_DATA_I8_1X;
		case 6: // UInt8_1X
			return KINC_G4_VERTEX_DATA_U8_1X;
		case 7: // Int8_1X_Normalized
			return KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED;
		case 8: // UInt8_1X_Normalized
			return KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED;
		case 9: // Int8_2X
			return KINC_G4_VERTEX_DATA_I8_2X;
		case 10: // UInt8_2X
			return KINC_G4_VERTEX_DATA_U8_2X;
		case 11: // Int8_2X_Normalized
			return KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED;
		case 12: // UInt8_2X_Normalized
			return KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED;
		case 13: // Int8_4X
			return KINC_G4_VERTEX_DATA_I8_4X;
		case 14: // UInt8_4X
			return KINC_G4_VERTEX_DATA_U8_4X;
		case 15: // Int8_4X_Normalized
			return KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED;
		case 16: // UInt8_4X_Normalized
			return KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED;
		case 17: // Int16_1X
			return KINC_G4_VERTEX_DATA_I16_1X;
		case 18: // UInt16_1X
			return KINC_G4_VERTEX_DATA_U16_1X;
		case 19: // Int16_1X_Normalized
			return KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED;
		case 20: // UInt16_1X_Normalized
			return KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED;
		case 21: // Int16_2X
			return KINC_G4_VERTEX_DATA_I16_2X;
		case 22: // UInt16_2X
			return KINC_G4_VERTEX_DATA_U16_2X;
		case 23: // Int16_2X_Normalized
			return KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED;
		case 24: // UInt16_2X_Normalized
			return KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED;
		case 25: // Int16_4X
			return KINC_G4_VERTEX_DATA_I16_4X;
		case 26: // UInt16_4X
			return KINC_G4_VERTEX_DATA_U16_4X;
		case 27: // Int16_4X_Normalized
			return KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED;
		case 28: // UInt16_4X_Normalized
			return KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED;
		case 29: // Int32_1X
			return KINC_G4_VERTEX_DATA_I32_1X;
		case 30: // UInt32_1X
			return KINC_G4_VERTEX_DATA_U32_1X;
		case 31: // Int32_2X
			return KINC_G4_VERTEX_DATA_I32_2X;
		case 32: // UInt32_2X
			return KINC_G4_VERTEX_DATA_U32_2X;
		case 33: // Int32_3X
			return KINC_G4_VERTEX_DATA_I32_3X;
		case 34: // UInt32_3X
			return KINC_G4_VERTEX_DATA_U32_3X;
		case 35: // Int32_4X
			return KINC_G4_VERTEX_DATA_I32_4X;
		case 36: // UInt32_4X
			return KINC_G4_VERTEX_DATA_U32_4X;
		}
		return KINC_G4_VERTEX_DATA_NONE;
	}

	void krom_create_vertexbuffer(const FunctionCallbackInfo<Value> &args) {
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
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)malloc(sizeof(kinc_g4_vertex_buffer_t));
		kinc_g4_vertex_buffer_init(buffer, args[0]->Int32Value(isolate->GetCurrentContext()).FromJust(), &structure, (kinc_g4_usage_t)args[2]->Int32Value(isolate->GetCurrentContext()).FromJust(), args[3]->Int32Value(isolate->GetCurrentContext()).FromJust());
		obj->SetInternalField(0, External::New(isolate, buffer));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_vertexbuffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)field->Value();
		kinc_g4_vertex_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_lock_vertex_buffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)field->Value();
		int start = args[1]->Int32Value(isolate->GetCurrentContext()).FromJust();
		int count = args[2]->Int32Value(isolate->GetCurrentContext()).FromJust();
		float *vertices = kinc_g4_vertex_buffer_lock(buffer, start, count);
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)vertices, (uint32_t)(count * kinc_g4_vertex_buffer_stride(buffer)), [](void *, size_t, void *) {}, nullptr);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(abuffer);
	}

	void krom_unlock_vertex_buffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)field->Value();
		int count = args[1]->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_vertex_buffer_unlock(buffer, count);
	}

	void krom_set_vertexbuffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)field->Value();
		kinc_g4_set_vertex_buffer(buffer);
	}

	void krom_set_vertexbuffers(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_vertex_buffer_t *vertex_buffers[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		Local<Object> jsarray = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> bufferobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "buffer").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> bufferfield = Local<External>::Cast(bufferobj->GetInternalField(0));
			kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)bufferfield->Value();
			vertex_buffers[i] = buffer;
		}
		kinc_g4_set_vertex_buffers(vertex_buffers, length);
	}

	void krom_draw_indexed_vertices_fast(Local<Object> receiver, int start, int count) {
		#ifdef KORE_DIRECT3D12
		// TODO: Prevent heapIndex overflow in texture.c.h/kinc_g5_internal_set_textures
		waitAfterNextDraw = true;
		#endif
		if (count < 0) kinc_g4_draw_indexed_vertices();
		else kinc_g4_draw_indexed_vertices_from_to(start, count);
	}

	void krom_draw_indexed_vertices(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int start = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		krom_draw_indexed_vertices_fast(args.This(), start, count);
	}

	void krom_draw_indexed_vertices_instanced_fast(Local<Object> receiver, int instance_count, int start, int count) {
		if (count < 0) kinc_g4_draw_indexed_vertices_instanced(instance_count);
		else kinc_g4_draw_indexed_vertices_instanced_from_to(instance_count, start, count);
	}

	void krom_draw_indexed_vertices_instanced(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int instance_count = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int start = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		krom_draw_indexed_vertices_instanced_fast(args.This(), instance_count, start, count);
	}

	void krom_create_vertex_shader(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content->Data(), (int)content->ByteLength(), KINC_G4_SHADER_TYPE_VERTEX);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_vertex_shader_from_source(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		#ifdef WITH_D3DCOMPILER

		strcpy(temp_string_vs, *utf8_value);

		ID3DBlob *error_message;
		ID3DBlob *shader_buffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(temp_string_vs, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, &shader_buffer, &error_message);
		if (hr != S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char *)error_message->GetBufferPointer());
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

		std::ostringstream file;
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

		ID3D11ShaderReflection *reflector = nullptr;
		D3DReflect(shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&reflector);

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

		ID3D11ShaderReflectionConstantBuffer *constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables);
			output_len += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable *variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name;
					output_len += strlen(variableDesc.Name);
					file.put(0);
					output_len += 1;
					file.write((char *)&variableDesc.StartOffset, 4);
					output_len += 4;
					file.write((char *)&variableDesc.Size, 4);
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
		file.write((char *)shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize());
		output_len += shader_buffer->GetBufferSize();
		shader_buffer->Release();
		reflector->Release();

		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, (void *)file.str().c_str(), (int)output_len, KINC_G4_SHADER_TYPE_VERTEX);

		#elif defined(KORE_METAL)

		strcpy(temp_string_vs, "// my_main\n");
		strcat(temp_string_vs, *utf8_value);
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, temp_string_vs, strlen(temp_string_vs), KINC_G4_SHADER_TYPE_VERTEX);

		#elif defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)

		char *output = new char[1024 * 1024];
		int length;
		krafix_compile(*utf8_value, output, &length, "spirv", "windows", "vert", -1);
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_VERTEX);

		#else

		char *source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, source, strlen(source), KINC_G4_SHADER_TYPE_VERTEX);

		#endif

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		Local<String> name = String::NewFromUtf8(isolate, "").ToLocalChecked();
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_fragment_shader(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content->Data(), (int)content->ByteLength(), KINC_G4_SHADER_TYPE_FRAGMENT);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_fragment_shader_from_source(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		#ifdef WITH_D3DCOMPILER

		strcpy(temp_string_fs, *utf8_value);

		ID3DBlob *error_message;
		ID3DBlob *shader_buffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(temp_string_fs, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, &shader_buffer, &error_message);
		if (hr != S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char *)error_message->GetBufferPointer());
			return;
		}

		std::map<std::string, int> attributes;

		std::ostringstream file;
		size_t output_len = 0;

		file.put((char)attributes.size());
		output_len += 1;

		ID3D11ShaderReflection *reflector = nullptr;
		D3DReflect(shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&reflector);

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

		ID3D11ShaderReflectionConstantBuffer *constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables);
			output_len += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable *variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name;
					output_len += strlen(variableDesc.Name);
					file.put(0);
					output_len += 1;
					file.write((char *)&variableDesc.StartOffset, 4);
					output_len += 4;
					file.write((char *)&variableDesc.Size, 4);
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
		file.write((char *)shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize());
		output_len += shader_buffer->GetBufferSize();
		shader_buffer->Release();
		reflector->Release();

		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, (void *)file.str().c_str(), (int)output_len, KINC_G4_SHADER_TYPE_FRAGMENT);

		#elif defined(KORE_METAL)

		strcpy(temp_string_fs, "// my_main\n");
		strcat(temp_string_fs, *utf8_value);
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, temp_string_fs, strlen(temp_string_fs), KINC_G4_SHADER_TYPE_FRAGMENT);

		#elif defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)

		char *output = new char[1024 * 1024];
		int length;
		krafix_compile(*utf8_value, output, &length, "spirv", "windows", "frag", -1);
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_FRAGMENT);

		#else

		char *source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, source, strlen(source), KINC_G4_SHADER_TYPE_FRAGMENT);

		#endif

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		Local<String> name = String::NewFromUtf8(isolate, "").ToLocalChecked();
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_geometry_shader(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content->Data(), (int)content->ByteLength(), KINC_G4_SHADER_TYPE_GEOMETRY);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_tessellation_control_shader(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content->Data(), (int)content->ByteLength(), KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_tessellation_evaluation_shader(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content->Data(), (int)content->ByteLength(), KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_shader(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)field->Value();
		kinc_g4_shader_destroy(shader);
		free(shader);
	}

	void krom_create_pipeline(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)malloc(sizeof(kinc_g4_pipeline_t));
		kinc_g4_pipeline_init(pipeline);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(8);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, pipeline));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_pipeline(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)pipefield->Value();
		kinc_g4_pipeline_destroy(pipeline);
		free(pipeline);
	}

	void krom_compile_pipeline(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)pipefield->Value();

		kinc_g4_vertex_structure_t s0, s1, s2, s3;
		kinc_g4_vertex_structure_init(&s0);
		kinc_g4_vertex_structure_init(&s1);
		kinc_g4_vertex_structure_init(&s2);
		kinc_g4_vertex_structure_init(&s3);
		kinc_g4_vertex_structure_t *structures[4] = { &s0, &s1, &s2, &s3 };

		int32_t size = args[5]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i1 = 0; i1 < size; ++i1) {
			Local<Object> jsstructure = args[i1 + 1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			structures[i1]->instanced = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "instanced").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
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
		kinc_g4_shader_t *vertexShader = (kinc_g4_shader_t *)vsfield->Value();
		pipeobj->SetInternalField(3, External::New(isolate, vertexShader));

		Local<External> fsfield = Local<External>::Cast(args[7]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_shader_t *fragmentShader = (kinc_g4_shader_t *)fsfield->Value();
		pipeobj->SetInternalField(4, External::New(isolate, fragmentShader));

		pipeline->vertex_shader = vertexShader;
		pipeline->fragment_shader = fragmentShader;

		if (!args[8]->IsNullOrUndefined()) {
			Local<External> gsfield = Local<External>::Cast(args[8]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t *geometryShader = (kinc_g4_shader_t *)gsfield->Value();
			pipeobj->SetInternalField(5, External::New(isolate, geometryShader));
			pipeline->geometry_shader = geometryShader;
		}

		if (!args[9]->IsNullOrUndefined()) {
			Local<External> tcsfield = Local<External>::Cast(args[9]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t *tessellationControlShader = (kinc_g4_shader_t *)tcsfield->Value();
			pipeobj->SetInternalField(6, External::New(isolate, tessellationControlShader));
			pipeline->tessellation_control_shader = tessellationControlShader;
		}

		if (!args[10]->IsNullOrUndefined()) {
			Local<External> tesfield = Local<External>::Cast(args[10]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t *tessellationEvaluationShader = (kinc_g4_shader_t *)tesfield->Value();
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

		pipeline->blend_source = (kinc_g4_blending_factor_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->blend_destination = (kinc_g4_blending_factor_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alpha_blend_source = (kinc_g4_blending_factor_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alpha_blend_destination = (kinc_g4_blending_factor_t)args11->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

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

	void krom_set_pipeline(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)pipefield->Value();
		kinc_g4_set_pipeline(pipeline);
	}

	bool ends_with(const char *str, const char *suffix) {
		if (!str || !suffix) return 0;
		size_t lenstr = strlen(str);
		size_t lensuffix = strlen(suffix);
		if (lensuffix > lenstr) return 0;
		return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
	}

	bool load_image(kinc_file_reader_t &reader, const char *filename, unsigned char *&output, int &width, int &height, kinc_image_format_t &format) {
		format = KINC_IMAGE_FORMAT_RGBA32;
		int size = (int)kinc_file_reader_size(&reader);
		int comp;
		bool success = true;
		unsigned char *data = (unsigned char *)malloc(size);
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
				output = (unsigned char *)malloc(outputSize);
				LZ4_decompress_safe((char *)(data + 12), (char *)output, compressedSize, outputSize);
			}
			else if (strcmp(fourcc, "LZ4F") == 0) {
				int outputSize = width * height * 16;
				output = (unsigned char *)malloc(outputSize);
				LZ4_decompress_safe((char *)(data + 12), (char *)output, compressedSize, outputSize);
				format = KINC_IMAGE_FORMAT_RGBA128;

				#ifdef KORE_IOS // No RGBA128 filtering, convert to RGBA64
				uint32_t *_output32 = (uint32_t *)output;
				unsigned char *_output = (unsigned char *)malloc(outputSize / 2);
				uint16_t *_output16 = (uint16_t *)_output;
				for (int i = 0; i < outputSize / 4; ++i) {
					uint32_t x = *((uint32_t *)&_output32[i]);
					_output16[i] = ((x >> 16) & 0x8000) | ((((x & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((x >> 13) & 0x03ff);
				}
				format = KINC_IMAGE_FORMAT_RGBA64;
				free(output);
				output = _output;
				#endif
			}
			else {
				success = false;
			}
		}
		else if (ends_with(filename, "hdr")) {
			output = (unsigned char *)stbi_loadf_from_memory(data, size, &width, &height, &comp, 4);
			if (output == nullptr) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
				success = false;
			}
			format = KINC_IMAGE_FORMAT_RGBA128;
		}
		else { // jpg, png, ..
			output = stbi_load_from_memory(data, size, &width, &height, &comp, 4);
			if (output == nullptr) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
				success = false;
			}
		}
		free(data);
		return success;
	}

	void krom_load_image(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		bool readable = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool success = true;

		kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));

		if (armorcore) {
			kinc_file_reader_t reader;
			if (kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) {
				unsigned char *image_data;
				int image_width;
				int image_height;
				kinc_image_format_t image_format;
				success = load_image(reader, *utf8_value, image_data, image_width, image_height, image_format);
				if (success) {
					kinc_image_init(image, image_data, image_width, image_height, image_format);
				}
			}
			else {
				success = false;
			}
		}
		else {
			// TODO: make kinc_image load faster
			size_t byte_size = kinc_image_size_from_file(*utf8_value);
			if (byte_size == 0) {
				success = false;
			}
			else {
				void *memory = malloc(byte_size);
				kinc_image_init_from_file(image, memory, *utf8_value);
			}
		}

		if (!success) {
			free(image);
			return;
		}

		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init_from_image(texture, image);
		if (!readable) {
			free(image->data);
			kinc_image_destroy(image);
			free(image);
			// free(memory);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		if (!armorcore) {
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		}
		args.GetReturnValue().Set(obj);
	}

	void krom_unload_image(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNullOrUndefined()) return;
		Local<Object> image = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> tex = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		Local<Value> rt = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();

		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
			kinc_g4_texture_destroy(texture);
			free(texture);
		}
		else if (rt->IsObject()) {
			Local<External> rtfield = Local<External>::Cast(rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
			kinc_g4_render_target_destroy(render_target);
			free(render_target);
		}
	}

	#ifdef WITH_AUDIO
	void krom_load_sound(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_a1_sound_t *sound = kinc_a1_sound_create(*utf8_value);
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, sound));
		args.GetReturnValue().Set(obj);
	}

	void krom_unload_sound(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_a1_sound_t *sound = (kinc_a1_sound_t *)field->Value();
		kinc_a1_sound_destroy(sound);
	}

	void krom_play_sound(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_a1_sound_t *sound = (kinc_a1_sound_t *)field->Value();
		bool loop = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_a1_play_sound(sound, loop, 1.0, false);
	}

	void krom_stop_sound(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_a1_sound_t *sound = (kinc_a1_sound_t *)field->Value();
		kinc_a1_stop_sound(sound);
	}
	#endif

	void krom_load_blob(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		kinc_file_reader_t reader;
		if (!kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) return;
		uint32_t reader_size = (uint32_t)kinc_file_reader_size(&reader);

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader_size);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_file_reader_read(&reader, content->Data(), reader_size);
		kinc_file_reader_close(&reader);

		args.GetReturnValue().Set(buffer);
	}

	void krom_load_url(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_load_url(*utf8_value);
	}

	void krom_copy_to_clipboard(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_copy_to_clipboard(*utf8_value);
	}

	void krom_get_constant_location(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> pipefield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)pipefield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_g4_constant_location_t *location_copy = (kinc_g4_constant_location_t *)malloc(sizeof(kinc_g4_constant_location_t));
		memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
		obj->SetInternalField(0, External::New(isolate, location_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> pipefield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)pipefield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_g4_texture_unit_t *unit_copy = (kinc_g4_texture_unit_t *)malloc(sizeof(kinc_g4_texture_unit_t));
		memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
		obj->SetInternalField(0, External::New(isolate, unit_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_set_texture(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
		kinc_g4_set_texture(*unit, texture);
	}

	void krom_set_render_target(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
		kinc_g4_render_target_use_color_as_texture(render_target, *unit);
	}

	void krom_set_texture_depth(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
		kinc_g4_render_target_use_depth_as_texture(render_target, *unit);
	}

	void krom_set_image_texture(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
		kinc_g4_set_image_texture(*unit, texture);
	}

	void krom_set_texture_parameters(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture3d_parameters(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture_compare_mode(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		kinc_g4_set_texture_compare_mode(*unit, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_set_cube_map_compare_mode(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)unitfield->Value();
		kinc_g4_set_cubemap_compare_mode(*unit, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_set_bool(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_bool(*location, value != 0);
	}

	void krom_set_int(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_int(*location, value);
	}

	void krom_set_float(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float(*location, value);
	}

	void krom_set_float2(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float2(*location, value1, value2);
	}

	void krom_set_float3(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float3(*location, value1, value2, value3);
	}

	void krom_set_float4(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();

		float *from = (float *)content->Data();
		kinc_g4_set_floats(*location, from, int(content->ByteLength() / 4));
	}

	void krom_set_matrix(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		float *from = (float *)content->Data();
		kinc_g4_set_matrix4(*location, (kinc_matrix4x4_t *)from);
	}

	void krom_set_matrix3(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		float *from = (float *)content->Data();
		kinc_g4_set_matrix3(*location, (kinc_matrix3x3_t *)from);
	}

	double krom_get_time_fast(Local<Object> receiver) {
		return kinc_time();
	}

	void krom_get_time(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Number::New(isolate, krom_get_time_fast(args.This())));
	}

	int krom_window_width_fast(Local<Object> receiver, int windowId) {
		return kinc_window_width(windowId);
	}

	void krom_window_width(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, krom_window_width_fast(args.This(), windowId)));
	}

	int krom_window_height_fast(Local<Object> receiver, int windowId) {
		return kinc_window_height(windowId);
	}

	void krom_window_height(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, krom_window_height_fast(args.This(), windowId)));
	}

	void krom_set_window_title(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value title(isolate, args[1]);
		kinc_window_set_title(windowId, *title);
		#if defined(KORE_IOS) || defined(KORE_ANDROID)
		strcpy(mobile_title, *title);
		#endif
	}

	void krom_get_window_mode(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_get_mode(windowId)));
	}

	void krom_set_window_mode(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_mode_t windowMode = (kinc_window_mode_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_change_mode(windowId, windowMode);
	}

	void krom_resize_window(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int width = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_resize(windowId, width, height);
	}

	void krom_move_window(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_window_move(windowId, x, y);
	}

	void krom_screen_dpi(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int ppi = kinc_display_current_mode(kinc_primary_display()).pixels_per_inch;
		args.GetReturnValue().Set(Int32::New(isolate, ppi));
	}

	void krom_system_id(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_system_id()).ToLocalChecked());
	}

	void krom_request_shutdown(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_stop();
	}

	void krom_display_count(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, kinc_count_displays()));
	}

	void krom_display_width(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).width));
	}

	void krom_display_height(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).height));
	}

	void krom_display_x(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).x));
	}

	void krom_display_y(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).y));
	}

	void krom_display_frequency(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).frequency));
	}

	void krom_display_is_primary(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		#ifdef KORE_LINUX // TODO: Primary display detection broken in Kinc
		args.GetReturnValue().Set(Int32::New(isolate, true));
		#else
		args.GetReturnValue().Set(Int32::New(isolate, index == kinc_primary_display()));
		#endif
	}

	void krom_write_storage(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();

		kinc_file_writer_t writer;
		kinc_file_writer_open(&writer, *utf8_name);
		kinc_file_writer_write(&writer, content->Data(), (int)content->ByteLength());
		kinc_file_writer_close(&writer);
	}

	void krom_read_storage(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		kinc_file_reader_t reader;
		if (!kinc_file_reader_open(&reader, *utf8_name, KINC_FILE_TYPE_SAVE)) return;
		int reader_size = (int)kinc_file_reader_size(&reader);

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader_size);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_file_reader_read(&reader, content->Data(), reader_size);
		kinc_file_reader_close(&reader);

		args.GetReturnValue().Set(buffer);
	}

	void krom_create_render_target(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
		kinc_g4_render_target_init(render_target, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_g4_render_target_format_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, render_target));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, render_target->width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, render_target->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_render_target_cube_map(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
		kinc_g4_render_target_init_cube(render_target, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_g4_render_target_format_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, render_target));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, render_target->width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, render_target->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init(texture, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		if (!armorcore) {
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		}
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture3d(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init3d(texture, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->tex_depth));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));

		bool readable = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		void *image_data;
		if (readable) {
			image_data = malloc(content->ByteLength());
			memcpy(image_data, content->Data(), content->ByteLength());
		}
		else {
			image_data = content->Data();
		}

		kinc_image_init(image, image_data, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
		kinc_g4_texture_init_from_image(texture, image);

		if (!readable) {
			kinc_image_destroy(image);
			free(image);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		if (!armorcore) {
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		}
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes3d(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t *image = (kinc_image_t*)malloc(sizeof(kinc_image_t));

		bool readable = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		void *image_data;
		if (readable) {
			image_data = malloc(content->ByteLength());
			memcpy(image_data, content->Data(), content->ByteLength());
		}
		else {
			image_data = content->Data();
		}

		kinc_image_init3d(image, image_data, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
		kinc_g4_texture_init_from_image3d(texture, image);

		if (!readable) {
			kinc_image_destroy(image);
			free(image);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->tex_depth));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_encoded_bytes(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		String::Utf8Value format(isolate, args[1]);
		bool readable = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));

		unsigned char *content_data = (unsigned char *)content->Data();
		int content_length = (int)content->ByteLength();
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
			int compressedSize = (int)content->ByteLength() - 12;
			if (strcmp(fourcc, "LZ4 ") == 0) {
				int outputSize = image_width * image_height * 4;
				image_data = (unsigned char *)malloc(outputSize);
				LZ4_decompress_safe((char *)content_data + 12, (char *)image_data, compressedSize, outputSize);
				image_format = KINC_IMAGE_FORMAT_RGBA32;
			}
			else if (strcmp(fourcc, "LZ4F") == 0) {
				int outputSize = image_width * image_height * 16;
				image_data = (unsigned char *)malloc(outputSize);
				LZ4_decompress_safe((char *)content_data + 12, (char *)image_data, compressedSize, outputSize);
				image_format = KINC_IMAGE_FORMAT_RGBA128;
			}
		}
		else if (ends_with(*format, "hdr")) {
			int comp;
			image_data = (unsigned char *)stbi_loadf_from_memory(content_data, content_length, &image_width, &image_height, &comp, 4);
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
			free(image->data);
			kinc_image_destroy(image);
			free(image);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
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

	void krom_get_texture_pixels(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(1));
		kinc_image_t *image = (kinc_image_t *)field->Value();

		uint8_t *data = kinc_image_get_pixels(image);
		int byteLength = format_byte_size(image->format) * image->width * image->height * image->depth;
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)data, byteLength, [](void *, size_t, void *) {}, nullptr);
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(buffer);
	}

	void krom_get_render_target_pixels(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *rt = (kinc_g4_render_target_t *)field->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();

		uint8_t *b = (uint8_t *)content->Data();
		kinc_g4_render_target_get_pixels(rt, b);

		// Release staging texture immediately to save memory
		#ifdef KORE_DIRECT3D11
		rt->impl.textureStaging->Release();
		rt->impl.textureStaging = NULL;
		#elif defined(KORE_DIRECT3D12)
		rt->impl._renderTarget.impl.renderTargetReadback->Release();
		rt->impl._renderTarget.impl.renderTargetReadback = NULL;
		#elif defined(KORE_METAL)
		// id<MTLTexture> texReadback = (__bridge_transfer id<MTLTexture>)rt->impl._renderTarget.impl._texReadback;
		// texReadback = nil;
		// rt->impl._renderTarget.impl._texReadback = NULL;
		#endif
	}

	void krom_lock_texture(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> textureobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> field = Local<External>::Cast(textureobj->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)field->Value();
		uint8_t *tex = kinc_g4_texture_lock(texture);

		int stride = kinc_g4_texture_stride(texture);
		(void) textureobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stride").ToLocalChecked(), Int32::New(isolate, stride));

		int byteLength = stride * texture->tex_height * texture->tex_depth;
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)tex, byteLength, [](void *, size_t, void *) {}, nullptr);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(abuffer);
	}

	void krom_unlock_texture(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)field->Value();
		kinc_g4_texture_unlock(texture);
	}

	void krom_clear_texture(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)field->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int width = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int depth = args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_texture_clear(texture, x, y, z, width, height, depth, color);
	}

	void krom_generate_texture_mipmaps(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)field->Value();
		kinc_g4_texture_generate_mipmaps(texture, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_generate_render_target_mipmaps(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *rt = (kinc_g4_render_target_t *)field->Value();
		kinc_g4_render_target_generate_mipmaps(rt, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_set_mipmaps(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)field->Value();

		Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> mipmapobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> mipmapfield = Local<External>::Cast(mipmapobj->GetInternalField(1));
			kinc_image_t *mipmap = (kinc_image_t *)mipmapfield->Value();
			kinc_g4_texture_set_mipmap(texture, mipmap, i + 1);
		}
	}

	void krom_set_depth_stencil_from(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> targetfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)targetfield->Value();
		Local<External> sourcefield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *source_target = (kinc_g4_render_target_t *)sourcefield->Value();
		kinc_g4_render_target_set_depth_stencil_from(render_target, source_target);
	}

	void krom_viewport_fast(Local<Object> receiver, int x, int y, int w, int h) {
		kinc_g4_viewport(x, y, w, h);
	}

	void krom_viewport(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		krom_viewport_fast(args.This(), x, y, w, h);
	}

	void krom_scissor_fast(Local<Object> receiver, int x, int y, int w, int h) {
		kinc_g4_scissor(x, y, w, h);
	}

	void krom_scissor(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		krom_scissor_fast(args.This(), x, y, w, h);
	}

	void krom_disable_scissor_fast(Local<Object> receiver) {
		kinc_g4_disable_scissor();
	}

	void krom_disable_scissor(const FunctionCallbackInfo<Value> &args) {
		krom_disable_scissor_fast(args.This());
	}

	int krom_render_targets_inverted_y_fast(Local<Object> receiver) {
		return kinc_g4_render_targets_inverted_y();
	}

	void krom_render_targets_inverted_y(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, krom_render_targets_inverted_y_fast(args.This())));
	}

	void krom_begin(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNullOrUndefined()) {
			kinc_g4_restore_render_target();
		}
		else {
			Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
			kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();

			int32_t length = 1;
			kinc_g4_render_target_t *render_targets[8] = { render_target, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
			if (!args[1]->IsNullOrUndefined()) {
				Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value() + 1;
				if (length > 8) length = 8;
				for (int32_t i = 1; i < length; ++i) {
					Local<Object> artobj = jsarray->Get(isolate->GetCurrentContext(), i - 1).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
					Local<External> artfield = Local<External>::Cast(artobj->GetInternalField(0));
					kinc_g4_render_target_t *art = (kinc_g4_render_target_t *)artfield->Value();
					render_targets[i] = art;
				}
			}
			kinc_g4_set_render_targets(render_targets, length);
		}
	}

	void krom_begin_face(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
		int face = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_set_render_target_face(render_target, face);
	}

	void krom_end(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
	}

	void krom_file_save_bytes(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_path(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();

		bool hasLengthArg = args.Length() > 2 && !args[2]->IsNullOrUndefined();
		int byteLength = hasLengthArg ? args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value() : (int)content->ByteLength();
		if (byteLength > (int)content->ByteLength()) byteLength = (int)content->ByteLength();

		#ifdef KORE_WINDOWS
		MultiByteToWideChar(CP_UTF8, 0, *utf8_path, -1, temp_wstring, 1024);
		FILE *file = _wfopen(temp_wstring, L"wb");
		#else
		FILE *file = fopen(*utf8_path, "wb");
		#endif
		if (file == nullptr) return;
		fwrite(content->Data(), 1, byteLength, file);
		fclose(file);
	}

	int sys_command(const char *cmd) {
		#ifdef KORE_WINDOWS
		int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd, -1, NULL, 0);
		wchar_t *wstr = new wchar_t[wlen];
		MultiByteToWideChar(CP_UTF8, 0, cmd, -1, wstr, wlen);
		int result = _wsystem(wstr);
		delete[] wstr;
		#elif defined(KORE_IOS)
		int result = 0;
		#else
		int result = system(cmd);
		#endif
		return result;
	}

	void krom_sys_command(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_cmd(isolate, args[0]);
		int result = sys_command(*utf8_cmd);
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_save_path(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_internal_save_path()).ToLocalChecked());
	}

	void krom_get_arg_count(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, _argc));
	}

	void krom_get_arg(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, _argv[index]).ToLocalChecked());
	}

	void krom_get_files_location(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		#ifdef KORE_MACOS
		char path[1024];
		strcpy(path, macgetresourcepath());
		strcat(path, "/");
		strcat(path, KORE_DEBUGDIR);
		strcat(path, "/");
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, path).ToLocalChecked());
		#elif defined(KORE_IOS)
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
		#if defined(KORE_MACOS) || defined(KORE_IOS)
		Locker locker{isolate};
		#endif

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Value> result;
		Local<Value> argv[1];
		KromCallbackdata *cbd = (KromCallbackdata *)callbackdata;
		if (body != NULL) {
			std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)body, cbd->size > 0 ? cbd->size : strlen(body), [](void *, size_t, void *) {}, nullptr);
			argv[0] = ArrayBuffer::New(isolate, std::move(backing));
		}
		Local<Function> func = Local<Function>::New(isolate, cbd->func);
		if (!func->Call(context, context->Global(), body != NULL ? 1 : 0, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
		delete cbd;
	}

	void krom_http_request(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value url(isolate, args[0]);

		KromCallbackdata *cbd = new KromCallbackdata();
		cbd->size = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<Function> func = Local<Function>::Cast(args[2]);
		cbd->func.Reset(isolate, func);

		char url_base[512];
		char url_path[512];
		const char *curl = *url;
		int i = 0;
		for (; i < strlen(curl) - 8; ++i) {
			if (curl[i + 8] == '/') break;
			url_base[i] = curl[i + 8]; // Strip https://
		}
		url_base[i] = 0;
		int j = 0;
		if (strlen(url_base) < strlen(curl) - 8) ++i; // Skip /
		for (; j < strlen(curl) - 8 - i; ++j) {
			if (curl[i + 8 + j] == 0) break;
			url_path[j] = curl[i + 8 + j];
		}
		url_path[j] = 0;
		#ifdef KORE_ANDROID // TODO: move to Kinc
		android_http_request(curl, url_path, NULL, 443, true, 0, NULL, &krom_http_callback, cbd);
		#else
		kinc_http_request(url_base, url_path, NULL, 443, true, 0, NULL, &krom_http_callback, cbd);
		#endif
	}

	#ifdef WITH_COMPUTE
	void krom_set_bool_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_bool(*location, value != 0);
	}

	void krom_set_int_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_int(*location, value);
	}

	void krom_set_float_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float(*location, value);
	}

	void krom_set_float2_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float2(*location, value1, value2);
	}

	void krom_set_float3_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float3(*location, value1, value2, value3);
	}

	void krom_set_float4_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		float *from = (float *)content->Data();
		kinc_compute_set_floats(*location, from, int(content->ByteLength() / 4));
	}

	void krom_set_matrix_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		float *from = (float *)content->Data();
		kinc_compute_set_matrix4(*location, (kinc_matrix4x4_t *)from);
	}

	void krom_set_matrix3_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t *location = (kinc_compute_constant_location_t *)locationfield->Value();
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		float *from = (float *)content->Data();
		kinc_compute_set_matrix3(*location, (kinc_matrix3x3_t *)from);
	}

	void krom_set_texture_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t *unit = (kinc_compute_texture_unit_t *)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_compute_set_texture(*unit, texture, (kinc_compute_access_t)access);
	}

	void krom_set_render_target_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t *unit = (kinc_compute_texture_unit_t *)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_compute_set_render_target(*unit, render_target, (kinc_compute_access_t)access);
	}

	void krom_set_sampled_texture_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t *unit = (kinc_compute_texture_unit_t *)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
		kinc_compute_set_sampled_texture(*unit, texture);
	}

	void krom_set_sampled_render_target_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t *unit = (kinc_compute_texture_unit_t *)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
		kinc_compute_set_sampled_render_target(*unit, render_target);
	}

	void krom_set_sampled_depth_texture_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t *unit = (kinc_compute_texture_unit_t *)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
		kinc_compute_set_sampled_depth_from_render_target(*unit, render_target);
	}

	void krom_set_texture_parameters_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t *unit = (kinc_compute_texture_unit_t *)unitfield->Value();
		kinc_compute_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture3d_parameters_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t *unit = (kinc_compute_texture_unit_t *)unitfield->Value();
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_shader_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader *shader = (kinc_compute_shader *)shaderfield->Value();
		kinc_compute_set_shader(shader);
	}

	void krom_create_shader_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		kinc_compute_shader *shader = (kinc_compute_shader *)malloc(sizeof(kinc_compute_shader));
		kinc_compute_shader_init(shader, content->Data(), (int)content->ByteLength());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_shader_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> shaderobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> shaderfield = Local<External>::Cast(shaderobj->GetInternalField(0));
		kinc_compute_shader *shader = (kinc_compute_shader *)shaderfield->Value();
		kinc_compute_shader_destroy(shader);
		free(shader);
	}

	void krom_get_constant_location_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader *shader = (kinc_compute_shader *)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_compute_constant_location_t location = kinc_compute_shader_get_constant_location(shader, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_compute_constant_location_t *location_copy = (kinc_compute_constant_location_t *)malloc(sizeof(kinc_compute_constant_location_t)); // TODO
		memcpy(location_copy, &location, sizeof(kinc_compute_constant_location_t));
		obj->SetInternalField(0, External::New(isolate, location_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader *shader = (kinc_compute_shader *)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_compute_texture_unit_t unit = kinc_compute_shader_get_texture_unit(shader, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_compute_texture_unit_t *unit_copy = (kinc_compute_texture_unit_t *)malloc(sizeof(kinc_compute_texture_unit_t)); // TODO
		memcpy(unit_copy, &unit, sizeof(kinc_compute_texture_unit_t));
		obj->SetInternalField(0, External::New(isolate, unit_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_compute(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute(x, y, z);
	}
	#endif

	#ifdef WITH_G2
	void krom_g2_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer_image_vert = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content_image_vert = buffer_image_vert->GetBackingStore();
		Local<ArrayBuffer> buffer_image_frag = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content_image_frag = buffer_image_frag->GetBackingStore();
		Local<ArrayBuffer> buffer_colored_vert = Local<ArrayBuffer>::Cast(args[2]);
		std::shared_ptr<BackingStore> content_colored_vert = buffer_colored_vert->GetBackingStore();
		Local<ArrayBuffer> buffer_colored_frag = Local<ArrayBuffer>::Cast(args[3]);
		std::shared_ptr<BackingStore> content_colored_frag = buffer_colored_frag->GetBackingStore();
		Local<ArrayBuffer> buffer_text_vert = Local<ArrayBuffer>::Cast(args[4]);
		std::shared_ptr<BackingStore> content_text_vert = buffer_text_vert->GetBackingStore();
		Local<ArrayBuffer> buffer_text_frag = Local<ArrayBuffer>::Cast(args[5]);
		std::shared_ptr<BackingStore> content_text_frag = buffer_text_frag->GetBackingStore();

		g2_init(content_image_vert->Data(), (int)content_image_vert->ByteLength(), content_image_frag->Data(), (int)content_image_frag->ByteLength(), content_colored_vert->Data(), (int)content_colored_vert->ByteLength(), content_colored_frag->Data(), (int)content_colored_frag->ByteLength(), content_text_vert->Data(), (int)content_text_vert->ByteLength(), content_text_frag->Data(), (int)content_text_frag->ByteLength());
	}

	void krom_g2_begin(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		g2_begin();
	}

	void krom_g2_end(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		g2_end();
	}

	void krom_g2_draw_scaled_sub_image(const FunctionCallbackInfo<Value> &args) {
		#ifdef KORE_DIRECT3D12
		waitAfterNextDraw = true;
		#endif
		HandleScope scope(args.GetIsolate());
		Local<Object> image = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		float sx = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float sy = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float sw = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float sh = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float dx = (float)args[5]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float dy = (float)args[6]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float dw = (float)args[7]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float dh = (float)args[8]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<Value> tex = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
			g2_draw_scaled_sub_image(texture, sx, sy, sw, sh, dx, dy, dw, dh);
		}
		else {
			Local<Value> rt = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();
			Local<External> rtfield = Local<External>::Cast(rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();
			g2_draw_scaled_sub_render_target(render_target, sx, sy, sw, sh, dx, dy, dw, dh);
		}
	}

	void krom_g2_fill_triangle(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float x0 = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y0 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float x1 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y1 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float x2 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y2 = (float)args[5]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_fill_triangle(x0, y0, x1, y1, x2, y2);
	}

	void krom_g2_fill_rect(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float x = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float width = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float height = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_fill_rect(x, y, width, height);
	}

	void krom_g2_draw_rect(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float x = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float width = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float height = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float strength = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_draw_rect(x, y, width, height, strength);
	}

	void krom_g2_draw_line(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float x0 = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y0 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float x1 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y1 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float strength = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_draw_line(x0, y0, x1, y1, strength);
	}

	void krom_g2_draw_string(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value text(isolate, args[0]);
		float x = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_draw_string(*text, x, y);
	}

	void krom_g2_set_font(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		g2_font_t *font = (g2_font_t *)field->Value();
		int size = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_set_font(font, size);
	}

	void krom_g2_font_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> blob_buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> blob_content = blob_buffer->GetBackingStore();

		g2_font_t *font = (g2_font_t *)malloc(sizeof(g2_font_t));
		g2_font_init(font, blob_content->Data(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, font));
		args.GetReturnValue().Set(obj);
	}

	void krom_g2_font_13(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> blob_buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> blob_content = blob_buffer->GetBackingStore();

		g2_font_t *font = (g2_font_t *)malloc(sizeof(g2_font_t));
		g2_font_13(font, blob_content->Data());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, font));
		args.GetReturnValue().Set(obj);
	}

	void krom_g2_font_set_glyphs(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> jsarray = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int *ar = (int *)malloc(sizeof(int) * length);
		for (int i = 0; i < length; ++i) {
			int32_t j = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			ar[i] = j;
		}
		g2_font_set_glyphs(ar, length);
		free(ar);
	}

	void krom_g2_font_count(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		g2_font_t *font = (g2_font_t *)field->Value();
		int i = g2_font_count(font);
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_g2_font_height(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		g2_font_t *font = (g2_font_t *)field->Value();
		int size = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int i = (int)g2_font_height(font, size);
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_g2_string_width(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		g2_font_t *font = (g2_font_t *)field->Value();
		int size = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value text(isolate, args[2]);
		int i = (int)g2_string_width(font, size, *text);
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_g2_set_bilinear_filter(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		g2_set_bilinear_filter(args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_g2_restore_render_target(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		g2_restore_render_target();
	}

	void krom_g2_set_render_target(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t *rt = (kinc_g4_render_target_t *)field->Value();
		g2_set_render_target(rt);
	}

	void krom_g2_set_color(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int color = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_set_color(color);
	}

	void krom_g2_set_pipeline(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNullOrUndefined()) {
			g2_set_pipeline(NULL);
		}
		else {
			Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
			kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)pipefield->Value();
			g2_set_pipeline(pipeline);
		}
	}

	void krom_g2_set_transform(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNullOrUndefined()) {
			g2_set_transform(NULL);
		}
		else {
			Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
			std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
			float *from = (float *)content->Data();
			g2_set_transform((kinc_matrix3x3_t *)from);
		}
	}

	void krom_g2_fill_circle(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float cx = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float cy = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float radius = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int segments = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_fill_circle(cx, cy, radius, segments);
	}

	void krom_g2_draw_circle(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float cx = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float cy = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float radius = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int segments = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float strength = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		g2_draw_circle(cx, cy, radius, segments, strength);
	}

	void krom_g2_draw_cubic_bezier(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		Local<Object> jsarray_x = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray_x->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float *x = (float *)malloc(sizeof(float) * length);
		for (int i = 0; i < length; ++i) {
			float j = (float)jsarray_x->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			x[i] = j;
		}

		Local<Object> jsarray_y = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		length = jsarray_y->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float *y = (float *)malloc(sizeof(float) * length);
		for (int i = 0; i < length; ++i) {
			float j = (float)jsarray_y->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			y[i] = j;
		}

		int segments = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float strength = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		g2_draw_cubic_bezier(x, y, segments, strength);
		free(x);
		free(y);
	}
	#endif

	bool window_close_callback(void *data) {
		#ifdef KORE_WINDOWS
		bool save = false;
		Locker locker{isolate};
		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, save_and_quit_func);
		Local<Value> result;
		const int argc = 1;

		wchar_t title[1024];
		GetWindowTextW(kinc_windows_window_handle(0), title, sizeof(title));
		bool dirty = wcsstr(title, L"* - ArmorPaint") != NULL;
		if (dirty) {
			int res = MessageBox(kinc_windows_window_handle(0), L"Project has been modified, save changes?", L"Save Changes?", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
			if (res == IDYES)
				save = true;
			else if (res == IDNO)
				save = false;
			else // Cancel
				return false;
		}

		if (save_and_quit_func_set) {
			Local<Value> argv[argc] = { Int32::New(isolate, save) };
			if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
				handle_exception(&try_catch);
			}
			return false;
		}
		#endif

		return true;
	}

	void krom_set_save_and_quit_callback(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		save_and_quit_func.Reset(isolate, func);
		save_and_quit_func_set = true;
		kinc_window_set_close_callback(0, window_close_callback, NULL);
	}

	void krom_set_mouse_cursor(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int id = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_mouse_set_cursor(id);
		#ifdef KORE_WINDOWS
		// Set hand icon for drag even when mouse button is pressed
		if (id == 1) SetCursor(LoadCursor(NULL, IDC_HAND));
		#endif
	}

	void krom_delay_idle_sleep_fast(Local<Object> receiver) {
		paused_frames = 0;
	}

	void krom_delay_idle_sleep(const FunctionCallbackInfo<Value> &args) {
		krom_delay_idle_sleep_fast(args.This());
	}

	#ifdef WITH_NFD
	void krom_open_dialog(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value filterList(isolate, args[0]);
		String::Utf8Value defaultPath(isolate, args[1]);
		bool openMultiple = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		nfdpathset_t outPaths;
		nfdchar_t* outPath;
		nfdresult_t result = openMultiple ? NFD_OpenDialogMultiple(*filterList, *defaultPath, &outPaths) : NFD_OpenDialog(*filterList, *defaultPath, &outPath);

		if (result == NFD_OKAY) {
			int pathCount = openMultiple ? (int)NFD_PathSet_GetCount(&outPaths) : 1;
			Local<Array> result = Array::New(isolate, pathCount);

			if (openMultiple) {
				for (int i = 0; i < pathCount; ++i) {
					nfdchar_t* outPath = NFD_PathSet_GetPath(&outPaths, i);
					(void)result->Set(isolate->GetCurrentContext(), i, String::NewFromUtf8(isolate, outPath).ToLocalChecked());
				}
				NFD_PathSet_Free(&outPaths);
			}
			else {
				(void)result->Set(isolate->GetCurrentContext(), 0, String::NewFromUtf8(isolate, outPath).ToLocalChecked());
				free(outPath);
			}

			args.GetReturnValue().Set(result);
		}
		else if (result == NFD_CANCEL) {}
		else {
			kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
		}
	}

	void krom_save_dialog(const FunctionCallbackInfo<Value> &args) {
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
	#elif defined(KORE_ANDROID)
	void krom_open_dialog(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		AndroidFileDialogOpen();
	}

	void krom_save_dialog(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		wchar_t *outPath = AndroidFileDialogSave();
		size_t len = wcslen(outPath);
		uint16_t *str = new uint16_t[len + 1];
		for (int i = 0; i < len; i++) str[i] = outPath[i];
		str[len] = 0;
		args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t *)str).ToLocalChecked());
		delete[] str;
	}
	#elif defined(KORE_IOS)
	void krom_open_dialog(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		// String::Utf8Value filterList(isolate, args[0]);
		// String::Utf8Value defaultPath(isolate, args[1]);
		// Once finished drop_files callback is called
		IOSFileDialogOpen();
	}

	void krom_save_dialog(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		// String::Utf8Value filterList(isolate, args[0]);
		// String::Utf8Value defaultPath(isolate, args[1]);
		// Path to app document directory
		wchar_t *outPath = IOSFileDialogSave();
		size_t len = wcslen(outPath);
		uint16_t *str = new uint16_t[len + 1];
		for (int i = 0; i < len; i++) str[i] = outPath[i];
		str[len] = 0;
		args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t *)str).ToLocalChecked());
		delete[] str;
	}
	#endif

	#ifdef WITH_TINYDIR
	void krom_read_directory(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value path(isolate, args[0]);
		bool foldersOnly = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		tinydir_dir dir;
		#ifdef KORE_WINDOWS
		MultiByteToWideChar(CP_UTF8, 0, *path, -1, temp_wstring, 1023);
		tinydir_open_sorted(&dir, temp_wstring);
		#else
		tinydir_open_sorted(&dir, *path);
		#endif

		#ifdef KORE_WINDOWS
		std::wstring files;
		#else
		std::string files;
		#endif

		for (int i = 0; i < dir.n_files; i++) {
			tinydir_file file;
			tinydir_readfile_n(&dir, &file, i);

			if (!file.is_dir || !foldersOnly) {
				#ifdef KORE_WINDOWS
				if (FILE_ATTRIBUTE_HIDDEN & GetFileAttributesW(file.path)) continue; // Skip hidden files
				if (wcscmp(file.name, L".") == 0 || wcscmp(file.name, L"..") == 0) continue;
				files += file.name;

				if (i < dir.n_files - 1) files += L"\n"; // Separator
				#else
				if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) continue;
				files += file.name;
				if (i < dir.n_files - 1) files += "\n";
				#endif
			}
		}

		tinydir_close(&dir);
		#ifdef KORE_WINDOWS
		args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t *)files.c_str()).ToLocalChecked());
		#else
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, files.c_str()).ToLocalChecked());
		#endif
	}
	#endif

	void krom_file_exists(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		bool exists = false;
		String::Utf8Value utf8_value(isolate, args[0]);

		kinc_file_reader_t reader;
		if (kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) {
			exists = true;
			kinc_file_reader_close(&reader);
		}

		args.GetReturnValue().Set(Int32::New(isolate, exists));
	}

	void krom_delete_file(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		#ifdef KORE_IOS
		IOSDeleteFile(*utf8_value);
		#elif defined(KORE_WINDOWS)
		char path[1024];
		strcpy(path, "del /f \"");
		strcat(path, *utf8_value);
		strcat(path, "\"");
		sys_command(path);
		#else
		char path[1024];
		strcpy(path, "rm \"");
		strcat(path, *utf8_value);
		strcat(path, "\"");
		sys_command(path);
		#endif
	}

	#ifdef WITH_ZLIB
	void krom_inflate(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		bool raw = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		unsigned char *inflated = (unsigned char *)malloc(content->ByteLength());

		z_stream infstream;
		infstream.zalloc = Z_NULL;
		infstream.zfree = Z_NULL;
		infstream.opaque = Z_NULL;
		infstream.avail_in = (uInt)content->ByteLength();
		infstream.next_in = (Bytef *)content->Data();
		infstream.avail_out = (uInt)content->ByteLength();
		infstream.next_out = (Bytef *)inflated;

		inflateInit2(&infstream, raw ? -15 : 15 + 32);

		int i = 2;
		while (true) {
			int res = inflate(&infstream, Z_NO_FLUSH);
			if (res == Z_STREAM_END) break;
			if (infstream.avail_out == 0) {
				inflated = (unsigned char *)realloc(inflated, content->ByteLength() * i);
				infstream.avail_out = (uInt)content->ByteLength();
				infstream.next_out = (Bytef *)(inflated + content->ByteLength() * (i - 1));
				i++;
			}
		}

		inflateEnd(&infstream);

		Local<ArrayBuffer> output = ArrayBuffer::New(isolate, infstream.total_out);
		std::shared_ptr<BackingStore> output_content = output->GetBackingStore();
		memcpy(output_content->Data(), inflated, infstream.total_out);
		free(inflated);

		args.GetReturnValue().Set(output);
	}

	void krom_deflate(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		bool raw = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		int deflatedSize = compressBound((uInt)content->ByteLength());
		void *deflated = malloc(deflatedSize);

		z_stream defstream;
		defstream.zalloc = Z_NULL;
		defstream.zfree = Z_NULL;
		defstream.opaque = Z_NULL;
		defstream.avail_in = (uInt)content->ByteLength();
		defstream.next_in = (Bytef *)content->Data();
		defstream.avail_out = deflatedSize;
		defstream.next_out = (Bytef *)deflated;

		deflateInit2(&defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, raw ? -15 : 15, 5, Z_DEFAULT_STRATEGY);
		deflate(&defstream, Z_FINISH);
		deflateEnd(&defstream);

		Local<ArrayBuffer> output = ArrayBuffer::New(isolate, defstream.total_out);
		std::shared_ptr<BackingStore> output_content = output->GetBackingStore();
		memcpy(output_content->Data(), deflated, defstream.total_out);
		free(deflated);

		args.GetReturnValue().Set(output);
	}
	#endif

	#ifdef WITH_STB_IMAGE_WRITE
	void write_image(const FunctionCallbackInfo<Value> &args, int imageFormat, int quality) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_path(isolate, args[0]);
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int format = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		int comp = 0;
		unsigned char *pixels = NULL;
		unsigned char *rgba = (unsigned char *)content->Data();
		if (format == 0) { // RGBA
			comp = 4;
			pixels = rgba;
		}
		else if (format == 1) { // R
			comp = 1;
			pixels = rgba;
		}
		else if (format == 2) { // RGB1
			comp = 3;
			pixels = (unsigned char *)malloc(w * h * comp);
			for (int i = 0; i < w * h; ++i) {
				#if defined(KORE_METAL) || defined(KORE_VULKAN)
				pixels[i * 3    ] = rgba[i * 4 + 2];
				pixels[i * 3 + 1] = rgba[i * 4 + 1];
				pixels[i * 3 + 2] = rgba[i * 4    ];
				#else
				pixels[i * 3    ] = rgba[i * 4    ];
				pixels[i * 3 + 1] = rgba[i * 4 + 1];
				pixels[i * 3 + 2] = rgba[i * 4 + 2];
				#endif
			}
		}
		else if (format > 2) { // RRR1, GGG1, BBB1, AAA1
			comp = 1;
			pixels = (unsigned char *)malloc(w * h * comp);
			int off = format - 3;
			#if defined(KORE_METAL) || defined(KORE_VULKAN)
			off = 2 - off;
			#endif
			for (int i = 0; i < w * h; ++i) pixels[i] = rgba[i * 4 + off];
		}

		imageFormat == 0 ?
			stbi_write_jpg(*utf8_path, w, h, comp, pixels, quality) :
			stbi_write_png(*utf8_path, w, h, comp, pixels, w * comp);

		if (pixels != rgba) free(pixels);
	}

	void krom_write_jpg(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int quality = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		write_image(args, 0, quality);
	}

	void krom_write_png(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		write_image(args, 1, 100);
	}

	unsigned char *encode_data;
	int encode_size;
	void encode_image_func(void *context, void *data, int size) {
		memcpy(encode_data + encode_size, data, size);
		encode_size += size;
	}

	void encode_image(const FunctionCallbackInfo<Value> &args, int imageFormat, int quality) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		int w = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int h = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int format = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		encode_data = (unsigned char *)malloc(w * h * 4);
		encode_size = 0;

		imageFormat == 0 ?
			stbi_write_jpg_to_func(&encode_image_func, NULL, w, h, 4, content->Data(), quality) :
			stbi_write_png_to_func(&encode_image_func, NULL, w, h, 4, content->Data(), w * 4);

		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)encode_data, encode_size, [](void *, size_t, void *) {}, nullptr);
		Local<ArrayBuffer> out = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(out);
	}

	void krom_encode_jpg(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int quality = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		encode_image(args, 0, quality);
	}

	void krom_encode_png(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		encode_image(args, 1, 100);
	}

	#ifdef WITH_ZLIB
	extern "C" unsigned char *stbiw_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality) {
		int deflatedSize = compressBound((uInt)data_len);
		void *deflated = malloc(deflatedSize);
		z_stream defstream;
		defstream.zalloc = Z_NULL;
		defstream.zfree = Z_NULL;
		defstream.opaque = Z_NULL;
		defstream.avail_in = (uInt)data_len;
		defstream.next_in = (Bytef *)data;
		defstream.avail_out = deflatedSize;
		defstream.next_out = (Bytef *)deflated;
		deflateInit2(&defstream, Z_BEST_SPEED, Z_DEFLATED, 15, 5, Z_DEFAULT_STRATEGY);
		deflate(&defstream, Z_FINISH);
		deflateEnd(&defstream);
		*out_len = defstream.total_out;
		return (unsigned char *)deflated;
	}
	#endif
	#endif

	#ifdef WITH_MPEG_WRITE
	void krom_write_mpeg(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

	}
	#endif

	#ifdef WITH_ONNX
	void krom_ml_inference(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		#ifdef ARM_PROFILE
		double inference_time = kinc_time();
		#endif

		OrtStatus *onnx_status = NULL;

		static bool use_gpu_last = false;
		bool use_gpu = !(args.Length() > 4 && !args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
		if (ort == NULL || use_gpu_last != use_gpu) {
			use_gpu_last = use_gpu;
			ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
			ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "armorcore", &ort_env);

			ort->CreateSessionOptions(&ort_session_options);
			ort->SetIntraOpNumThreads(ort_session_options, 8);
			ort->SetInterOpNumThreads(ort_session_options, 8);

			if (use_gpu) {
				#ifdef KORE_WINDOWS
				ort->SetSessionExecutionMode(ort_session_options, ORT_SEQUENTIAL);
				ort->DisableMemPattern(ort_session_options);
				onnx_status = OrtSessionOptionsAppendExecutionProvider_DML(ort_session_options, 0);
				#elif defined(KORE_LINUX)
				// onnx_status = OrtSessionOptionsAppendExecutionProvider_CUDA(ort_session_options, 0);
				#elif defined(KORE_MACOS)
				onnx_status = OrtSessionOptionsAppendExecutionProvider_CoreML(ort_session_options, 0);
				#endif
				if (onnx_status != NULL) {
					const char *msg = ort->GetErrorMessage(onnx_status);
					kinc_log(KINC_LOG_LEVEL_ERROR, "%s", msg);
					ort->ReleaseStatus(onnx_status);
				}
			}
		}

		Local<ArrayBuffer> model_buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> model_content = model_buffer->GetBackingStore();

		static void *model_content_last = 0;
		if (model_content_last != model_content->Data() || session == NULL) {
			if (session != NULL) {
				ort->ReleaseSession(session);
				session = NULL;
			}
			onnx_status = ort->CreateSessionFromArray(ort_env, model_content->Data(), (int)model_content->ByteLength(), ort_session_options, &session);
			if (onnx_status != NULL) {
				const char* msg = ort->GetErrorMessage(onnx_status);
				kinc_log(KINC_LOG_LEVEL_ERROR, "%s", msg);
				ort->ReleaseStatus(onnx_status);
			}
		}
		model_content_last = model_content->Data();

		OrtAllocator *allocator;
		ort->GetAllocatorWithDefaultOptions(&allocator);
		OrtMemoryInfo *memory_info;
		ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);

		Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (length > 4) length = 4;
		char *input_node_names[4];
		OrtValue *input_tensors[4];
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> tensorobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<ArrayBuffer> tensor_buffer = Local<ArrayBuffer>::Cast(tensorobj);
			std::shared_ptr<BackingStore> tensor_content = tensor_buffer->GetBackingStore();

			ort->SessionGetInputName(session, i, allocator, &input_node_names[i]);

			OrtTypeInfo *input_type_info;
			ort->SessionGetInputTypeInfo(session, i, &input_type_info);
			const OrtTensorTypeAndShapeInfo *input_tensor_info;
			ort->CastTypeInfoToTensorInfo(input_type_info, &input_tensor_info);
			size_t num_input_dims;
			ort->GetDimensionsCount(input_tensor_info, &num_input_dims);
			std::vector<int64_t> input_node_dims(num_input_dims);

			if (args.Length() > 2 && !args[2]->IsNullOrUndefined()) {
				Local<Object> jsarray = args[2]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				Local<Object> jsarray2 = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				for (int32_t i = 0; i < num_input_dims; ++i) {
					int32_t j = jsarray2->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
					input_node_dims[i] = j;
				}
			}
			else {
				ort->GetDimensions(input_tensor_info, (int64_t *)input_node_dims.data(), num_input_dims);
			}
			ONNXTensorElementDataType tensor_element_type;
			ort->GetTensorElementType(input_tensor_info, &tensor_element_type);

			ort->CreateTensorWithDataAsOrtValue(memory_info, tensor_content->Data(), (int)tensor_content->ByteLength(), input_node_dims.data(), num_input_dims,  tensor_element_type, &input_tensors[i]);
			ort->ReleaseTypeInfo(input_type_info);
		}

		char *output_node_name;
		ort->SessionGetOutputName(session, 0, allocator, &output_node_name);
		OrtValue *output_tensor = NULL;
		onnx_status = ort->Run(session, NULL, input_node_names, input_tensors, length, &output_node_name, 1, &output_tensor);
		if (onnx_status != NULL) {
			const char* msg = ort->GetErrorMessage(onnx_status);
			kinc_log(KINC_LOG_LEVEL_ERROR, "%s", msg);
			ort->ReleaseStatus(onnx_status);
		}
		float *float_array;
		ort->GetTensorMutableData(output_tensor, (void **)&float_array);

		size_t output_byte_length = 4;
		if (args.Length() > 3 && !args[3]->IsNullOrUndefined()) {
			Local<Object> jsarray = args[3]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			for (int i = 0; i < length; ++i) {
				int32_t j = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
				output_byte_length *= j;
			}
		}
		else {
			OrtTypeInfo *output_type_info;
			ort->SessionGetOutputTypeInfo(session, 0, &output_type_info);
			const OrtTensorTypeAndShapeInfo *output_tensor_info;
			ort->CastTypeInfoToTensorInfo(output_type_info, &output_tensor_info);
			size_t num_output_dims;
			ort->GetDimensionsCount(output_tensor_info, &num_output_dims);
			std::vector<int64_t> output_node_dims(num_output_dims);
			ort->GetDimensions(output_tensor_info, (int64_t *)output_node_dims.data(), num_output_dims);
			ort->ReleaseTypeInfo(output_type_info);
			for (int i = 0; i < num_output_dims; ++i) if (output_node_dims[i] > 1) output_byte_length *= output_node_dims[i];
		}

		Local<ArrayBuffer> output = ArrayBuffer::New(isolate, output_byte_length);
		std::shared_ptr<BackingStore> output_content = output->GetBackingStore();
		memcpy(output_content->Data(), float_array, output_byte_length);

		ort->ReleaseMemoryInfo(memory_info);
		ort->ReleaseValue(output_tensor);
		for (int i = 0; i < length; ++i) ort->ReleaseValue(input_tensors[i]);
		args.GetReturnValue().Set(output);
	}

	void krom_ml_unload(const FunctionCallbackInfo<Value> &args) {
		if (session != NULL) {
			ort->ReleaseSession(session);
			session = NULL;
		}
	}
	#endif

	#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
	void krom_raytrace_supported(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		#ifdef KORE_METAL
		bool supported = kinc_raytrace_supported();
		#else
		bool supported = true;
		#endif
		args.GetReturnValue().Set(Int32::New(isolate, supported));
	}

	void krom_raytrace_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		if (accel_created) {
			kinc_g5_constant_buffer_destroy(&constant_buffer);
			kinc_raytrace_acceleration_structure_destroy(&accel);
			kinc_raytrace_pipeline_destroy(&pipeline);
		}

		Local<ArrayBuffer> shader_buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> shader_content = shader_buffer->GetBackingStore();

		Local<External> vb_field = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t *vertex_buffer4 = (kinc_g4_vertex_buffer_t *)vb_field->Value();
		kinc_g5_vertex_buffer_t *vertex_buffer = &vertex_buffer4->impl._buffer;

		Local<External> ib_field = Local<External>::Cast(args[2]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t *index_buffer4 = (kinc_g4_index_buffer_t *)ib_field->Value();
		kinc_g5_index_buffer_t *index_buffer = &index_buffer4->impl._buffer;

		float scale = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		kinc_g5_constant_buffer_init(&constant_buffer, constant_buffer_size * 4);

		kinc_raytrace_pipeline_init(&pipeline, &commandList, shader_content->Data(), (int)shader_content->ByteLength(), &constant_buffer);

		kinc_raytrace_acceleration_structure_init(&accel, &commandList, vertex_buffer, index_buffer, scale);
		accel_created = true;
	}

	void krom_raytrace_set_textures(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		kinc_g4_render_target_t *texpaint0;
		kinc_g4_render_target_t *texpaint1;
		kinc_g4_render_target_t *texpaint2;

		Local<Object> texpaint0_image = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> texpaint0_tex = texpaint0_image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		Local<Value> texpaint0_rt = texpaint0_image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();

		if (texpaint0_tex->IsObject()) {
			#ifdef KORE_DIRECT3D12
			Local<External> texfield = Local<External>::Cast(texpaint0_tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
			if (!texture->impl._uploaded) {
				kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
				texture->impl._uploaded = true;
			}
			texpaint0 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
			texpaint0->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
			#endif
		}
		else {
			Local<External> rtfield = Local<External>::Cast(texpaint0_rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			texpaint0 = (kinc_g4_render_target_t *)rtfield->Value();
		}

		Local<Object> texpaint1_image = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> texpaint1_tex = texpaint1_image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		Local<Value> texpaint1_rt = texpaint1_image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();

		if (texpaint1_tex->IsObject()) {
			#ifdef KORE_DIRECT3D12
			Local<External> texfield = Local<External>::Cast(texpaint1_tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
			if (!texture->impl._uploaded) {
				kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
				texture->impl._uploaded = true;
			}
			texpaint1 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
			texpaint1->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
			#endif
		}
		else {
			Local<External> rtfield = Local<External>::Cast(texpaint1_rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			texpaint1 = (kinc_g4_render_target_t *)rtfield->Value();
		}

		Local<Object> texpaint2_image = args[2]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> texpaint2_tex = texpaint2_image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		Local<Value> texpaint2_rt = texpaint2_image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();

		if (texpaint2_tex->IsObject()) {
			#ifdef KORE_DIRECT3D12
			Local<External> texfield = Local<External>::Cast(texpaint2_tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texfield->Value();
			if (!texture->impl._uploaded) {
				kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
				texture->impl._uploaded = true;
			}
			texpaint2 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
			texpaint2->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
			#endif
		}
		else {
			Local<External> rtfield = Local<External>::Cast(texpaint2_rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			texpaint2 = (kinc_g4_render_target_t *)rtfield->Value();
		}

		Local<External> envfield = Local<External>::Cast(args[3]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texenv = (kinc_g4_texture_t *)envfield->Value();

		Local<External> sobolfield = Local<External>::Cast(args[4]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texsobol = (kinc_g4_texture_t *)sobolfield->Value();

		Local<External> scramblefield = Local<External>::Cast(args[5]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texscramble = (kinc_g4_texture_t *)scramblefield->Value();

		Local<External> rankfield = Local<External>::Cast(args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t *texrank = (kinc_g4_texture_t *)rankfield->Value();

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

		if (texpaint0_tex->IsObject()) {
			free(texpaint0);
		}

		if (texpaint1_tex->IsObject()) {
			free(texpaint1);
		}

		if (texpaint2_tex->IsObject()) {
			free(texpaint2);
		}
	}

	void krom_raytrace_dispatch_rays(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		Local<External> rtfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		render_target = (kinc_g4_render_target_t *)rtfield->Value();

		Local<ArrayBuffer> cb_buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> cb_content = cb_buffer->GetBackingStore();
		float *cb = (float *)cb_content->Data();

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
	void krom_vr_begin(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_vr_interface_begin();
	}

	void krom_vr_begin_render(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int eye = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_vr_interface_begin_render(eye);
	}

	void krom_vr_end_render(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int eye = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_vr_interface_end_render(eye);
	}

	void krom_vr_warp_swap(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_vr_interface_warp_swap();
	}

	void krom_vr_get_sensor_state_view(const FunctionCallbackInfo<Value> &args) {
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

	void krom_vr_get_sensor_state_projection(const FunctionCallbackInfo<Value> &args) {
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

	void krom_vr_get_sensor_state_hmd_mounted(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		kinc_vr_sensor_state_t state = kinc_vr_interface_get_sensor_state(0);
		args.GetReturnValue().Set(Int32::New(isolate, state.pose.hmdMounted));
	}
	#endif

	void krom_window_x(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_x(windowId)));
	}

	void krom_window_y(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_y(windowId)));
	}

	void krom_language(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_language()).ToLocalChecked());
	}

	#ifdef WITH_IRON
	void krom_io_obj_parse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();

		int split_code = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int start_pos = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int udim = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		obj_part_t *part = io_obj_parse((uint8_t *)content->Data(), split_code, start_pos, udim);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();

		std::unique_ptr<v8::BackingStore> backing_posa = v8::ArrayBuffer::NewBackingStore((void *)part->posa, part->vertex_count * 4 * 2, [](void *data, size_t, void *) { free(data); }, nullptr);
		Local<ArrayBuffer> buffer_posa = ArrayBuffer::New(isolate, std::move(backing_posa));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "posa").ToLocalChecked(), Int16Array::New(buffer_posa, 0, buffer_posa->ByteLength() / 2));

		std::unique_ptr<v8::BackingStore> backing_nora = v8::ArrayBuffer::NewBackingStore((void *)part->nora, part->vertex_count * 2 * 2, [](void *data, size_t, void *) { free(data); }, nullptr);
		Local<ArrayBuffer> buffer_nora = ArrayBuffer::New(isolate, std::move(backing_nora));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "nora").ToLocalChecked(), Int16Array::New(buffer_nora, 0, buffer_nora->ByteLength() / 2));

		if (part->texa != NULL) {
			std::unique_ptr<v8::BackingStore> backing_texa = v8::ArrayBuffer::NewBackingStore((void *)part->texa, part->vertex_count * 2 * 2, [](void *data, size_t, void *) { free(data); }, nullptr);
			Local<ArrayBuffer> buffer_texa = ArrayBuffer::New(isolate, std::move(backing_texa));
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texa").ToLocalChecked(), Int16Array::New(buffer_texa, 0, buffer_texa->ByteLength() / 2));
		}

		std::unique_ptr<v8::BackingStore> backing_inda = v8::ArrayBuffer::NewBackingStore((void *)part->inda, part->index_count * 4, [](void *data, size_t, void *) { free(data); }, nullptr);
		Local<ArrayBuffer> buffer_inda = ArrayBuffer::New(isolate, std::move(backing_inda));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "inda").ToLocalChecked(), Uint32Array::New(buffer_inda, 0, buffer_inda->ByteLength() / 4));

		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), String::NewFromUtf8(isolate, part->name).ToLocalChecked());
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "scalePos").ToLocalChecked(), Number::New(isolate, part->scale_pos));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "has_next").ToLocalChecked(), Number::New(isolate, part->has_next));
		(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "pos").ToLocalChecked(), Number::New(isolate, (int)part->pos));

		if (udim) {
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "udims_u").ToLocalChecked(), Number::New(isolate, part->udims_u));

			Local<Array> udims = Array::New(isolate, part->udims_u * part->udims_v);
			for (int i = 0; i < part->udims_u * part->udims_v; ++i) {
				std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)part->udims[i], part->udims_count[i] * 4, [](void *data, size_t, void *) { free(data); }, nullptr);
				Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing));
				(void) udims->Set(isolate->GetCurrentContext(), i, Uint32Array::New(buffer, 0, buffer->ByteLength() / 4));
			}
			(void) obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "udims").ToLocalChecked(), udims);
		}

		args.GetReturnValue().Set(obj);
	}
	#endif

	#ifdef WITH_ZUI
	extern "C" float krom_js_eval(char *str) {
		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		Local<String> source = String::NewFromUtf8(isolate, str, NewStringType::kNormal).ToLocalChecked();
		TryCatch try_catch(isolate);
		Local<Script> compiled_script = Script::Compile(isolate->GetCurrentContext(), source).ToLocalChecked();
		Local<Value> result;
		if (!compiled_script->Run(context).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
		return (float)result->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
	}

	void krom_zui_on_border_hover(zui_handle_t *handle, int side) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, on_border_hover_func);
		Local<Value> result;
		const int argc = 2;
		Local<Value> argv[argc] = {Number::New(isolate, (size_t)handle), Int32::New(isolate, side)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void krom_zui_on_text_hover() {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, on_text_hover_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void krom_zui_on_deselect_text() {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, on_deselect_text_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void krom_zui_on_tab_drop(zui_handle_t *to, int to_pos, zui_handle_t *from, int from_pos) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, on_tab_drop_func);
		Local<Value> result;
		const int argc = 4;
		Local<Value> argv[argc] = {Number::New(isolate, (size_t)to), Int32::New(isolate, to_pos), Number::New(isolate, (size_t)from), Int32::New(isolate, from_pos)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void krom_zui_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> arg0 = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		zui_options_t ops;
		ops.scale_factor = (float)arg0->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "scale_factor").ToLocalChecked()).ToLocalChecked()->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		Local<Value> theme = arg0->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "theme").ToLocalChecked()).ToLocalChecked();
		Local<External> themefield = Local<External>::Cast(theme->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		ops.theme = (zui_theme_t *)themefield->Value();

		Local<Value> font = arg0->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "font").ToLocalChecked()).ToLocalChecked();
		Local<External> fontfield = Local<External>::Cast(font->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		ops.font = (g2_font_t *)fontfield->Value();

		Local<Value> colorwheel = arg0->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "color_wheel").ToLocalChecked()).ToLocalChecked();
		Local<External> colorwheelfield = Local<External>::Cast(colorwheel->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		ops.color_wheel = (kinc_g4_texture_t *)colorwheelfield->Value();

		Local<Value> blackwhitegradient = arg0->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "black_white_gradient").ToLocalChecked()).ToLocalChecked();
		Local<External> blackwhitegradientfield = Local<External>::Cast(blackwhitegradient->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		ops.black_white_gradient = (kinc_g4_texture_t *)blackwhitegradientfield->Value();

		zui_t *ui = (zui_t *)malloc(sizeof(zui_t));
		zui_init(ui, ops);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, ui));
		args.GetReturnValue().Set(obj);

		zui_on_border_hover = &krom_zui_on_border_hover;
		zui_on_text_hover = &krom_zui_on_text_hover;
		zui_on_deselect_text = &krom_zui_on_deselect_text;
		zui_on_tab_drop = &krom_zui_on_tab_drop;
	}

	void krom_zui_get_scale(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_t *ui = (zui_t *)field->Value();
		zui_t *current = zui_get_current();
		zui_set_current(ui);
		args.GetReturnValue().Set(Number::New(isolate, ZUI_SCALE()));
		zui_set_current(current);
	}

	void krom_zui_set_scale(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_t *ui = (zui_t *)field->Value();
		zui_t *current = zui_get_current();
		zui_set_current(ui);
		float factor = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_set_scale(factor);
		zui_set_current(current);
	}

	void krom_zui_set_font(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_t *ui = (zui_t *)field->Value();
		Local<External> fontfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		g2_font_t *font = (g2_font_t *)fontfield->Value();
		ui->ops.font = font;
	}

	void krom_zui_begin(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_t *ui = (zui_t *)field->Value();
		zui_begin(ui);
	}

	void krom_zui_end(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		bool last = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_end(last);
	}

	void krom_zui_begin_region(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_t *ui = (zui_t *)field->Value();
		int x = (int)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = (int)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int w = (int)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_begin_region(ui, x, y, w);
	}

	void krom_zui_end_region(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		bool last = (bool)args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_end_region(last);
	}

	void krom_zui_begin_sticky(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		zui_begin_sticky();
	}

	void krom_zui_end_sticky(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		zui_end_sticky();
	}

	void krom_zui_end_input(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		zui_end_input();
	}

	void krom_zui_end_window(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		bool bing_global_g = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_end_window(bing_global_g);
	}

	void krom_zui_end_element(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float element_size = args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (element_size < 0) zui_end_element();
		else zui_end_element_of_size(element_size);
	}

	void krom_zui_start_text_edit(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		int align = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_start_text_edit(handle, align);
	}

	void krom_zui_input_in_rect(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float x = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float w = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float h = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool b = zui_input_in_rect(x, y, w, h);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	void krom_zui_window(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int w = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int h = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int drag = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool b = zui_window(handle, x, y, w, h, drag);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	void krom_zui_button(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value text(isolate, args[0]);
		int align = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value label(isolate, args[2]);
		bool b = zui_button(*text, align, *label);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	void krom_zui_check(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		String::Utf8Value text(isolate, args[1]);
		String::Utf8Value label(isolate, args[2]);
		bool b = zui_check(handle, *text, *label);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	void krom_zui_radio(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		int position = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value text(isolate, args[2]);
		String::Utf8Value label(isolate, args[3]);
		bool b = zui_radio(handle, position, *text, *label);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	char combo_texts_data[32][64];
	char *combo_texts[32];
	char combo_label[32];
	void krom_zui_combo(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		String::Utf8Value label(isolate, args[2]);
		int show_label = (int)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int align = (int)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int search_bar = (int)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		assert(length <= 32);

		int i;
		if (zui_get_current()->combo_selected_handle == NULL) {
			for (int i = 0; i < length; ++i) {
				String::Utf8Value str(isolate, jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked());
				strcpy(combo_texts_data[i], *str);
				combo_texts[i] = combo_texts_data[i];
			}
			strcpy(combo_label, *label);
			i = zui_combo(handle, combo_texts, length, combo_label, show_label, align, search_bar);
		}
		else {
			char combo_temp_data[32][64];
			char *combo_temp[32];
			for (int i = 0; i < length; ++i) {
				String::Utf8Value str(isolate, jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked());
				strcpy(combo_temp_data[i], *str);
				combo_temp[i] = combo_temp_data[i];
			}
			i = zui_combo(handle, combo_temp, length, *label, show_label, align, search_bar);
		}

		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_zui_slider(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		String::Utf8Value text(isolate, args[1]);
		float from = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float to = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool filled = (bool)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float precision = (float)args[5]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool display_value = (bool)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int align = (int)args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool text_edit = (bool)args[8]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float f = zui_slider(handle, *text, from, to, filled, precision, display_value, align, text_edit);
		args.GetReturnValue().Set(Number::New(isolate, f));
	}

	void krom_zui_image(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> image_object = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int tint = (int)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int h = (int)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int sx = (int)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int sy = (int)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int sw = (int)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int sh = (int)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		void *image;
		bool is_rt;
		Local<Value> tex = image_object->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			image = (void *)texfield->Value();
			is_rt = false;
		}
		else {
			Local<Value> rt = image_object->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();
			Local<External> rtfield = Local<External>::Cast(rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			image = (void *)rtfield->Value();
			is_rt = true;
		}
		int i = zui_sub_image(image, is_rt, tint, h, sx, sy, sw, sh);
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_zui_text(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value text(isolate, args[0]);
		int align = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int bg = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int i = zui_text(*text, align, bg);
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_zui_text_input(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		String::Utf8Value label(isolate, args[1]);
		int align = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int editable = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int live_update = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		char *str = zui_text_input(handle, *label, align, editable, live_update);
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, str).ToLocalChecked());
	}

	void krom_zui_tab(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		String::Utf8Value text(isolate, args[1]);
		int vertical = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool b = zui_tab(handle, *text, vertical, color);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	void krom_zui_panel(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		String::Utf8Value text(isolate, args[1]);
		int is_tree = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int filled = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int pack = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool b = zui_panel(handle, *text, is_tree, filled, pack);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	void krom_zui_handle(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> ops = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		zui_handle_t *handle = (zui_handle_t *)malloc(sizeof(zui_handle_t));
		memset(handle, 0, sizeof(zui_handle_t));
		handle->redraws = 2;
		handle->selected = (bool)ops->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "selected").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		handle->position = (int)ops->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "position").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		handle->value = (float)ops->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "value").ToLocalChecked()).ToLocalChecked()->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<Value> str = ops->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "text").ToLocalChecked()).ToLocalChecked();
		String::Utf8Value text(isolate, str);
		assert(strlen(*text) < 128);
		strcpy(handle->text, *text);
		handle->color = (int)ops->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "color").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		handle->layout = (int)ops->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "layout").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, handle));
		args.GetReturnValue().Set(obj);
	}

	void krom_zui_separator(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int h = (int)args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool fill = (bool)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_separator(h, fill);
	}

	void krom_zui_tooltip(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value text(isolate, args[0]);
		zui_tooltip(*text);
	}

	void krom_zui_tooltip_image(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int max_width = (int)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<Object> image_object = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> tex = image_object->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t *image = (kinc_g4_texture_t *)texfield->Value();
			zui_tooltip_image(image, max_width);
		}
		else {
			Local<Value> rt = image_object->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();
			Local<External> rtfield = Local<External>::Cast(rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_render_target_t *image = (kinc_g4_render_target_t *)rtfield->Value();
			zui_tooltip_render_target(image, max_width);
		}
	}

	void krom_zui_row(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> jsarray = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		assert(length <= 128);
		float ratios[128];
		for (int i = 0; i < length; ++i) {
			ratios[i] = (float)jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		}
		zui_row(ratios, length);
	}

	void krom_zui_fill(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float x = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float w = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float h = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = (int)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_fill(x, y, w, h, color);
	}

	void krom_zui_rect(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float x = (float)args[0]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float w = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float h = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = (int)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float strength = (float)args[5]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_rect(x, y, w, h, color, strength);
	}

	void krom_zui_draw_rect(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		bool fill = (bool)args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float x = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float w = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float h = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		// float strength = (float)args[5]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		// zui_draw_rect(fill, x, y, w, h, strength);
		zui_draw_rect(fill, x, y, w, h);
	}

	void krom_zui_draw_string(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value text(isolate, args[0]);
		float x_offset = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float y_offset = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int align = (int)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool truncation = (bool)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_draw_string(*text, x_offset, y_offset, align, truncation);
	}

	void krom_zui_get_hovered_tab_name(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		char *str = zui_hovered_tab_name();
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, str).ToLocalChecked());
	}

	void krom_zui_set_hovered_tab_name(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[0]);
		zui_set_hovered_tab_name(*name);
	}

	void krom_zui_begin_menu(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		zui_begin_menu();
	}

	void krom_zui_end_menu(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		zui_end_menu();
	}

	void krom_zui_menu_button(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value text(isolate, args[0]);
		bool b = zui_menu_button(*text);
		args.GetReturnValue().Set(Int32::New(isolate, b));
	}

	void krom_zui_float_input(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		String::Utf8Value label(isolate, args[1]);
		int align = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float precision = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float f = zui_float_input(handle, *label, align, precision);
		args.GetReturnValue().Set(Number::New(isolate, f));
	}

	char radio_texts[32][64];
	char *radio_temp[32];
	void krom_zui_inline_radio(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		int align = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		assert(length <= 32);
		for (int i = 0; i < length; ++i) {
			String::Utf8Value str(isolate, jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked());
			strcpy(radio_texts[i], *str);
			radio_temp[i] = radio_texts[i];
		}

		int i = zui_inline_radio(handle, radio_temp, length, align);
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_picker_callback(void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, picker_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void krom_zui_color_wheel(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		bool alpha = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float w = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float h = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool color_preview = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<Value> picker_arg = args[5];
		Local<Function> pickerFunc = Local<Function>::Cast(picker_arg);
		picker_func.Reset(isolate, pickerFunc);
		int i = zui_color_wheel(handle, alpha, w, h, color_preview, &krom_picker_callback, NULL);
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_zui_text_area(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		int align = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool editable = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value label(isolate, args[3]);
		bool word_wrap = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		char *str = zui_text_area(handle, align, editable, *label, word_wrap);
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, str).ToLocalChecked());
	}

	void krom_zui_text_area_coloring(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		if (zui_text_area_coloring != NULL) {
			free(zui_text_area_coloring);
		}
		if (args[0]->IsNullOrUndefined()) {
			zui_text_area_coloring = NULL;
			return;
		}
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		zui_text_area_coloring = (zui_text_coloring_t *)armpack_decode(content->Data(), (int)content->ByteLength());
	}

	void krom_zui_nodes_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		zui_nodes_t *nodes = (zui_nodes_t *)malloc(sizeof(zui_nodes_t));
		zui_nodes_init(nodes);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, nodes));
		args.GetReturnValue().Set(obj);
	}

	void *encoded = NULL;
	uint32_t encoded_size = 0;
	void krom_zui_node_canvas(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();

		// TODO: decode on change
		zui_node_canvas_t *decoded = (zui_node_canvas_t *)armpack_decode(content->Data(), (int)content->ByteLength());
		zui_node_canvas(decoded);

		int byteLength = zui_node_canvas_encoded_size(decoded);
		if (byteLength > encoded_size) {
			if (encoded != NULL) free(encoded);
			encoded_size = byteLength;
			encoded = malloc(byteLength);
		}
		zui_node_canvas_encode(encoded, decoded);

		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)encoded, byteLength, [](void *, size_t, void *) {}, nullptr);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(abuffer);
		free(decoded);
	}

	void krom_zui_nodes_rgba_popup(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
		float *val = (float *)content->Data();
		int x = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		zui_nodes_rgba_popup(handle, val, x, y);
	}

	void krom_zui_nodes_scale(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float f = ZUI_NODES_SCALE();
		args.GetReturnValue().Set(Number::New(isolate, f));
	}

	void krom_zui_nodes_pan_x(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float f = ZUI_NODES_PAN_X();
		args.GetReturnValue().Set(Number::New(isolate, f));
	}

	void krom_zui_nodes_pan_y(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float f = ZUI_NODES_PAN_Y();
		args.GetReturnValue().Set(Number::New(isolate, f));
	}

	#define ZUI_GET_I32_GLOBAL(prop)\
		if (strcmp(*name, #prop) == 0) {\
			args.GetReturnValue().Set(Int32::New(isolate, prop));\
			return;\
		}

	#define ZUI_GET_I32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			args.GetReturnValue().Set(Int32::New(isolate, obj->prop));\
			return;\
		}

	#define ZUI_GET_PTR(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			args.GetReturnValue().Set(Number::New(isolate, (size_t)obj->prop));\
			return;\
		}

	#define ZUI_GET_F32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			args.GetReturnValue().Set(Number::New(isolate, obj->prop));\
			return;\
		}

	#define ZUI_SET_I32_GLOBAL(prop)\
		if (strcmp(*name, #prop) == 0) {\
			prop = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();\
			return;\
		}

	#define ZUI_SET_I32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			obj->prop = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();\
			return;\
		}

	#define ZUI_SET_F32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			obj->prop = args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();\
			return;\
		}

	void krom_zui_get(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[1]);
		if (args[0]->IsNullOrUndefined()) {
			ZUI_GET_I32_GLOBAL(zui_always_redraw_window)
			ZUI_GET_I32_GLOBAL(zui_touch_scroll)
			ZUI_GET_I32_GLOBAL(zui_touch_hold)
			ZUI_GET_I32_GLOBAL(zui_touch_tooltip)
			ZUI_GET_I32_GLOBAL(zui_is_paste)
			return;
		}
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_t *ui = (zui_t *)field->Value();
		ZUI_GET_I32(ui, enabled)
		ZUI_GET_I32(ui, changed)
		ZUI_GET_I32(ui, is_hovered)
		ZUI_GET_I32(ui, is_released)
		ZUI_GET_I32(ui, is_scrolling)
		ZUI_GET_I32(ui, is_typing)
		ZUI_GET_I32(ui, input_enabled)
		ZUI_GET_I32(ui, input_started)
		ZUI_GET_I32(ui, input_started_r)
		ZUI_GET_I32(ui, input_released)
		ZUI_GET_I32(ui, input_released_r)
		ZUI_GET_I32(ui, input_down)
		ZUI_GET_I32(ui, input_down_r)
		ZUI_GET_I32(ui, is_key_pressed)
		ZUI_GET_I32(ui, is_ctrl_down)
		ZUI_GET_I32(ui, is_delete_down)
		ZUI_GET_I32(ui, is_escape_down)
		ZUI_GET_I32(ui, is_return_down)
		ZUI_GET_I32(ui, scissor)
		ZUI_GET_I32(ui, current_ratio)
		ZUI_GET_I32(ui, font_size)
		ZUI_GET_I32(ui, _w)
		ZUI_GET_I32(ui, key_code)
		ZUI_GET_PTR(ui, text_selected_handle)
		ZUI_GET_PTR(ui, submit_text_handle)
		ZUI_GET_PTR(ui, combo_selected_handle)
		ZUI_GET_F32(ui, input_x)
		ZUI_GET_F32(ui, input_y)
		ZUI_GET_F32(ui, input_started_x)
		ZUI_GET_F32(ui, input_started_y)
		ZUI_GET_F32(ui, input_dx)
		ZUI_GET_F32(ui, input_dy)
		ZUI_GET_F32(ui, input_wheel_delta)
		ZUI_GET_F32(ui, font_offset_y)
		ZUI_GET_F32(ui, _x)
		ZUI_GET_F32(ui, _y)
		ZUI_GET_F32(ui, _window_x)
		ZUI_GET_F32(ui, _window_y)
		ZUI_GET_F32(ui, _window_w)
		ZUI_GET_F32(ui, _window_h)
	}

	void krom_zui_set(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[1]);
		if (args[0]->IsNullOrUndefined()) {
			ZUI_SET_I32_GLOBAL(zui_touch_scroll)
			ZUI_SET_I32_GLOBAL(zui_touch_hold)
			ZUI_SET_I32_GLOBAL(zui_touch_tooltip)
			ZUI_SET_I32_GLOBAL(zui_is_cut)
			ZUI_SET_I32_GLOBAL(zui_is_copy)
			ZUI_SET_I32_GLOBAL(zui_is_paste)
			ZUI_SET_I32_GLOBAL(zui_text_area_line_numbers)
			ZUI_SET_I32_GLOBAL(zui_text_area_scroll_past_end)
			return;
		}
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_t *ui = (zui_t *)field->Value();
		ZUI_SET_I32(ui, enabled)
		ZUI_SET_I32(ui, changed)
		ZUI_SET_I32(ui, image_invert_y)
		ZUI_SET_I32(ui, is_hovered)
		ZUI_SET_I32(ui, always_redraw)
		ZUI_SET_I32(ui, scroll_enabled)
		ZUI_SET_I32(ui, input_enabled)
		ZUI_SET_I32(ui, input_started)
		ZUI_SET_I32(ui, is_delete_down)
		ZUI_SET_I32(ui, image_scroll_align)
		ZUI_SET_I32(ui, scissor)
		ZUI_SET_I32(ui, elements_baked)
		ZUI_SET_I32(ui, window_border_top)
		ZUI_SET_I32(ui, window_border_bottom)
		ZUI_SET_I32(ui, window_border_right)
		ZUI_SET_I32(ui, current_ratio)
		ZUI_SET_I32(ui, font_size)
		ZUI_SET_I32(ui, _w)
		ZUI_SET_F32(ui, input_x)
		ZUI_SET_F32(ui, input_y)
		ZUI_SET_F32(ui, font_offset_y)
		ZUI_SET_F32(ui, _x)
		ZUI_SET_F32(ui, _y)
	}

	void krom_zui_handle_get(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[1]);
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		ZUI_GET_I32(handle, selected)
		ZUI_GET_I32(handle, position)
		ZUI_GET_I32(handle, color)
		ZUI_GET_I32(handle, changed)
		ZUI_GET_I32(handle, drag_x)
		ZUI_GET_I32(handle, drag_y)
		ZUI_GET_F32(handle, value)
		ZUI_GET_F32(handle, scroll_offset)
		if (strcmp(*name, "text") == 0) {
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, handle->text).ToLocalChecked());
		}
	}

	void krom_zui_handle_set(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[1]);
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		ZUI_SET_I32(handle, selected)
		ZUI_SET_I32(handle, position)
		ZUI_SET_I32(handle, color)
		ZUI_SET_I32(handle, redraws)
		ZUI_SET_I32(handle, changed)
		ZUI_SET_I32(handle, drag_x)
		ZUI_SET_I32(handle, drag_y)
		ZUI_SET_F32(handle, value)
		if (strcmp(*name, "text") == 0) {
			String::Utf8Value text(isolate, args[2]);
			strcpy(handle->text, *text);
		}
	}

	void krom_zui_handle_ptr(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_handle_t *handle = (zui_handle_t *)field->Value();
		args.GetReturnValue().Set(Number::New(isolate, (size_t)handle));
	}

	void krom_zui_theme_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());

		zui_theme_t *theme = (zui_theme_t *)malloc(sizeof(zui_theme_t));
		zui_theme_default(theme);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, theme));
		args.GetReturnValue().Set(obj);
	}

	void krom_zui_theme_get(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[1]);
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_theme_t *theme = (zui_theme_t *)field->Value();
		ZUI_GET_I32(theme, WINDOW_BG_COL)
		ZUI_GET_I32(theme, WINDOW_TINT_COL)
		ZUI_GET_I32(theme, ACCENT_COL)
		ZUI_GET_I32(theme, ACCENT_HOVER_COL)
		ZUI_GET_I32(theme, ACCENT_SELECT_COL)
		ZUI_GET_I32(theme, BUTTON_COL)
		ZUI_GET_I32(theme, BUTTON_TEXT_COL)
		ZUI_GET_I32(theme, BUTTON_HOVER_COL)
		ZUI_GET_I32(theme, BUTTON_PRESSED_COL)
		ZUI_GET_I32(theme, TEXT_COL)
		ZUI_GET_I32(theme, LABEL_COL)
		ZUI_GET_I32(theme, SEPARATOR_COL)
		ZUI_GET_I32(theme, HIGHLIGHT_COL)
		ZUI_GET_I32(theme, CONTEXT_COL)
		ZUI_GET_I32(theme, PANEL_BG_COL)
		ZUI_GET_I32(theme, FONT_SIZE)
		ZUI_GET_I32(theme, ELEMENT_W)
		ZUI_GET_I32(theme, ELEMENT_H)
		ZUI_GET_I32(theme, ELEMENT_OFFSET)
		ZUI_GET_I32(theme, ARROW_SIZE)
		ZUI_GET_I32(theme, BUTTON_H)
		ZUI_GET_I32(theme, CHECK_SIZE)
		ZUI_GET_I32(theme, CHECK_SELECT_SIZE)
		ZUI_GET_I32(theme, SCROLL_W)
		ZUI_GET_I32(theme, SCROLL_MINI_W)
		ZUI_GET_I32(theme, TEXT_OFFSET)
		ZUI_GET_I32(theme, TAB_W)
		ZUI_GET_I32(theme, FILL_WINDOW_BG)
		ZUI_GET_I32(theme, FILL_BUTTON_BG)
		ZUI_GET_I32(theme, FILL_ACCENT_BG)
		ZUI_GET_I32(theme, LINK_STYLE)
		ZUI_GET_I32(theme, FULL_TABS)
		ZUI_GET_I32(theme, ROUND_CORNERS)
	}

	void krom_zui_theme_set(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value name(isolate, args[1]);
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		zui_theme_t *theme = (zui_theme_t *)field->Value();
		ZUI_SET_I32(theme, WINDOW_BG_COL)
		ZUI_SET_I32(theme, WINDOW_TINT_COL)
		ZUI_SET_I32(theme, ACCENT_COL)
		ZUI_SET_I32(theme, ACCENT_HOVER_COL)
		ZUI_SET_I32(theme, ACCENT_SELECT_COL)
		ZUI_SET_I32(theme, BUTTON_COL)
		ZUI_SET_I32(theme, BUTTON_TEXT_COL)
		ZUI_SET_I32(theme, BUTTON_HOVER_COL)
		ZUI_SET_I32(theme, BUTTON_PRESSED_COL)
		ZUI_SET_I32(theme, TEXT_COL)
		ZUI_SET_I32(theme, LABEL_COL)
		ZUI_SET_I32(theme, SEPARATOR_COL)
		ZUI_SET_I32(theme, HIGHLIGHT_COL)
		ZUI_SET_I32(theme, CONTEXT_COL)
		ZUI_SET_I32(theme, PANEL_BG_COL)
		ZUI_SET_I32(theme, FONT_SIZE)
		ZUI_SET_I32(theme, ELEMENT_W)
		ZUI_SET_I32(theme, ELEMENT_H)
		ZUI_SET_I32(theme, ELEMENT_OFFSET)
		ZUI_SET_I32(theme, ARROW_SIZE)
		ZUI_SET_I32(theme, BUTTON_H)
		ZUI_SET_I32(theme, CHECK_SIZE)
		ZUI_SET_I32(theme, CHECK_SELECT_SIZE)
		ZUI_SET_I32(theme, SCROLL_W)
		ZUI_SET_I32(theme, SCROLL_MINI_W)
		ZUI_SET_I32(theme, TEXT_OFFSET)
		ZUI_SET_I32(theme, TAB_W)
		ZUI_SET_I32(theme, FILL_WINDOW_BG)
		ZUI_SET_I32(theme, FILL_BUTTON_BG)
		ZUI_SET_I32(theme, FILL_ACCENT_BG)
		ZUI_SET_I32(theme, LINK_STYLE)
		ZUI_SET_I32(theme, FULL_TABS)
		ZUI_SET_I32(theme, ROUND_CORNERS)
	}

	void krom_zui_set_on_border_hover(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		on_border_hover_func.Reset(isolate, func);
	}

	void krom_zui_set_on_text_hover(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		on_text_hover_func.Reset(isolate, func);
	}

	void krom_zui_set_on_deselect_text(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		on_deselect_text_func.Reset(isolate, func);
	}

	void krom_zui_set_on_tab_drop(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		on_tab_drop_func.Reset(isolate, func);
	}
	#endif

	#define SET_FUNCTION_FAST(object, name, fn)\
		CFunction fn ## _ = CFunction::Make(fn ## _fast);\
		object->Set(String::NewFromUtf8(isolate, name).ToLocalChecked(),\
		FunctionTemplate::New(isolate, fn, Local<v8::Value>(), Local<v8::Signature>(), 0,\
		v8::ConstructorBehavior::kThrow, v8::SideEffectType::kHasNoSideEffect, &fn ## _))

	#define SET_FUNCTION(object, name, fn)\
		object->Set(String::NewFromUtf8(isolate, name).ToLocalChecked(),\
		FunctionTemplate::New(isolate, fn, Local<v8::Value>(), Local<v8::Signature>(), 0,\
		v8::ConstructorBehavior::kThrow, v8::SideEffectType::kHasNoSideEffect, nullptr))

	void start_v8(char *krom_bin, int krom_bin_size) {
		plat = platform::NewDefaultPlatform();
		V8::InitializePlatform(plat.get());

		std::string flags = "";
		#ifdef KORE_IOS
		flags += "--jitless ";
		#endif
		V8::SetFlagsFromString(flags.c_str(), (int)flags.size());

		V8::Initialize();

		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
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
		SET_FUNCTION(krom, "init", krom_init);
		SET_FUNCTION(krom, "setApplicationName", krom_set_application_name);
		SET_FUNCTION(krom, "log", krom_log);
		SET_FUNCTION_FAST(krom, "clear", krom_clear);
		SET_FUNCTION(krom, "setCallback", krom_set_callback);
		SET_FUNCTION(krom, "setDropFilesCallback", krom_set_drop_files_callback);
		SET_FUNCTION(krom, "setCutCopyPasteCallback", krom_set_cut_copy_paste_callback); ////
		SET_FUNCTION(krom, "setApplicationStateCallback", krom_set_application_state_callback);
		SET_FUNCTION(krom, "setKeyboardDownCallback", krom_set_keyboard_down_callback);
		SET_FUNCTION(krom, "setKeyboardUpCallback", krom_set_keyboard_up_callback);
		SET_FUNCTION(krom, "setKeyboardPressCallback", krom_set_keyboard_press_callback);
		SET_FUNCTION(krom, "setMouseDownCallback", krom_set_mouse_down_callback);
		SET_FUNCTION(krom, "setMouseUpCallback", krom_set_mouse_up_callback);
		SET_FUNCTION(krom, "setMouseMoveCallback", krom_set_mouse_move_callback);
		SET_FUNCTION(krom, "setTouchDownCallback", krom_set_touch_down_callback);
		SET_FUNCTION(krom, "setTouchUpCallback", krom_set_touch_up_callback);
		SET_FUNCTION(krom, "setTouchMoveCallback", krom_set_touch_move_callback);
		SET_FUNCTION(krom, "setMouseWheelCallback", krom_set_mouse_wheel_callback);
		SET_FUNCTION(krom, "setPenDownCallback", krom_set_pen_down_callback);
		SET_FUNCTION(krom, "setPenUpCallback", krom_set_pen_up_callback);
		SET_FUNCTION(krom, "setPenMoveCallback", krom_set_pen_move_callback);
		SET_FUNCTION(krom, "setGamepadAxisCallback", krom_set_gamepad_axis_callback);
		SET_FUNCTION(krom, "setGamepadButtonCallback", krom_set_gamepad_button_callback);
		SET_FUNCTION_FAST(krom, "lockMouse", krom_lock_mouse);
		SET_FUNCTION_FAST(krom, "unlockMouse", krom_unlock_mouse);
		SET_FUNCTION_FAST(krom, "canLockMouse", krom_can_lock_mouse);
		SET_FUNCTION_FAST(krom, "isMouseLocked", krom_is_mouse_locked);
		SET_FUNCTION_FAST(krom, "setMousePosition", krom_set_mouse_position);
		SET_FUNCTION_FAST(krom, "showMouse", krom_show_mouse);
		SET_FUNCTION_FAST(krom, "showKeyboard", krom_show_keyboard);
		SET_FUNCTION(krom, "createIndexBuffer", krom_create_indexbuffer);
		SET_FUNCTION(krom, "deleteIndexBuffer", krom_delete_indexbuffer);
		SET_FUNCTION(krom, "lockIndexBuffer", krom_lock_indexbuffer);
		SET_FUNCTION(krom, "unlockIndexBuffer", krom_unlock_indexbuffer);
		SET_FUNCTION(krom, "setIndexBuffer", krom_set_indexbuffer);
		SET_FUNCTION(krom, "createVertexBuffer", krom_create_vertexbuffer);
		SET_FUNCTION(krom, "deleteVertexBuffer", krom_delete_vertexbuffer);
		SET_FUNCTION(krom, "lockVertexBuffer", krom_lock_vertex_buffer);
		SET_FUNCTION(krom, "unlockVertexBuffer", krom_unlock_vertex_buffer);
		SET_FUNCTION(krom, "setVertexBuffer", krom_set_vertexbuffer);
		SET_FUNCTION(krom, "setVertexBuffers", krom_set_vertexbuffers);
		SET_FUNCTION_FAST(krom, "drawIndexedVertices", krom_draw_indexed_vertices);
		SET_FUNCTION_FAST(krom, "drawIndexedVerticesInstanced", krom_draw_indexed_vertices_instanced);
		SET_FUNCTION(krom, "createVertexShader", krom_create_vertex_shader);
		SET_FUNCTION(krom, "createVertexShaderFromSource", krom_create_vertex_shader_from_source);
		SET_FUNCTION(krom, "createFragmentShader", krom_create_fragment_shader);
		SET_FUNCTION(krom, "createFragmentShaderFromSource", krom_create_fragment_shader_from_source);
		SET_FUNCTION(krom, "createGeometryShader", krom_create_geometry_shader);
		SET_FUNCTION(krom, "createTessellationControlShader", krom_create_tessellation_control_shader);
		SET_FUNCTION(krom, "createTessellationEvaluationShader", krom_create_tessellation_evaluation_shader);
		SET_FUNCTION(krom, "deleteShader", krom_delete_shader);
		SET_FUNCTION(krom, "createPipeline", krom_create_pipeline);
		SET_FUNCTION(krom, "deletePipeline", krom_delete_pipeline);
		SET_FUNCTION(krom, "compilePipeline", krom_compile_pipeline);
		SET_FUNCTION(krom, "setPipeline", krom_set_pipeline);
		SET_FUNCTION(krom, "loadImage", krom_load_image);
		SET_FUNCTION(krom, "unloadImage", krom_unload_image);
		#ifdef WITH_AUDIO
		SET_FUNCTION(krom, "loadSound", krom_load_sound);
		SET_FUNCTION(krom, "unloadSound", krom_unload_sound);
		SET_FUNCTION(krom, "playSound", krom_play_sound);
		SET_FUNCTION(krom, "stopSound", krom_stop_sound);
		#endif
		SET_FUNCTION(krom, "loadBlob", krom_load_blob);
		SET_FUNCTION(krom, "loadUrl", krom_load_url);
		SET_FUNCTION(krom, "copyToClipboard", krom_copy_to_clipboard);
		SET_FUNCTION(krom, "getConstantLocation", krom_get_constant_location);
		SET_FUNCTION(krom, "getTextureUnit", krom_get_texture_unit);
		SET_FUNCTION(krom, "setTexture", krom_set_texture);
		SET_FUNCTION(krom, "setRenderTarget", krom_set_render_target);
		SET_FUNCTION(krom, "setTextureDepth", krom_set_texture_depth);
		SET_FUNCTION(krom, "setImageTexture", krom_set_image_texture);
		SET_FUNCTION(krom, "setTextureParameters", krom_set_texture_parameters);
		SET_FUNCTION(krom, "setTexture3DParameters", krom_set_texture3d_parameters);
		SET_FUNCTION(krom, "setTextureCompareMode", krom_set_texture_compare_mode);
		SET_FUNCTION(krom, "setCubeMapCompareMode", krom_set_cube_map_compare_mode);
		SET_FUNCTION(krom, "setBool", krom_set_bool);
		SET_FUNCTION(krom, "setInt", krom_set_int);
		SET_FUNCTION(krom, "setFloat", krom_set_float);
		SET_FUNCTION(krom, "setFloat2", krom_set_float2);
		SET_FUNCTION(krom, "setFloat3", krom_set_float3);
		SET_FUNCTION(krom, "setFloat4", krom_set_float4);
		SET_FUNCTION(krom, "setFloats", krom_set_floats);
		SET_FUNCTION(krom, "setMatrix", krom_set_matrix);
		SET_FUNCTION(krom, "setMatrix3", krom_set_matrix3);
		SET_FUNCTION_FAST(krom, "getTime", krom_get_time);
		SET_FUNCTION_FAST(krom, "windowWidth", krom_window_width);
		SET_FUNCTION_FAST(krom, "windowHeight", krom_window_height);
		SET_FUNCTION(krom, "setWindowTitle", krom_set_window_title);
		SET_FUNCTION(krom, "getWindowMode", krom_get_window_mode);
		SET_FUNCTION(krom, "setWindowMode", krom_set_window_mode);
		SET_FUNCTION(krom, "resizeWindow", krom_resize_window);
		SET_FUNCTION(krom, "moveWindow", krom_move_window);
		SET_FUNCTION(krom, "screenDpi", krom_screen_dpi);
		SET_FUNCTION(krom, "systemId", krom_system_id);
		SET_FUNCTION(krom, "requestShutdown", krom_request_shutdown);
		SET_FUNCTION(krom, "displayCount", krom_display_count);
		SET_FUNCTION(krom, "displayWidth", krom_display_width);
		SET_FUNCTION(krom, "displayHeight", krom_display_height);
		SET_FUNCTION(krom, "displayX", krom_display_x);
		SET_FUNCTION(krom, "displayY", krom_display_y);
		SET_FUNCTION(krom, "displayFrequency", krom_display_frequency);
		SET_FUNCTION(krom, "displayIsPrimary", krom_display_is_primary);
		SET_FUNCTION(krom, "writeStorage", krom_write_storage);
		SET_FUNCTION(krom, "readStorage", krom_read_storage);
		SET_FUNCTION(krom, "createRenderTarget", krom_create_render_target);
		SET_FUNCTION(krom, "createRenderTargetCubeMap", krom_create_render_target_cube_map);
		SET_FUNCTION(krom, "createTexture", krom_create_texture);
		SET_FUNCTION(krom, "createTexture3D", krom_create_texture3d);
		SET_FUNCTION(krom, "createTextureFromBytes", krom_create_texture_from_bytes);
		SET_FUNCTION(krom, "createTextureFromBytes3D", krom_create_texture_from_bytes3d);
		SET_FUNCTION(krom, "createTextureFromEncodedBytes", krom_create_texture_from_encoded_bytes);
		SET_FUNCTION(krom, "getTexturePixels", krom_get_texture_pixels);
		SET_FUNCTION(krom, "getRenderTargetPixels", krom_get_render_target_pixels);
		SET_FUNCTION(krom, "lockTexture", krom_lock_texture);
		SET_FUNCTION(krom, "unlockTexture", krom_unlock_texture);
		SET_FUNCTION(krom, "clearTexture", krom_clear_texture);
		SET_FUNCTION(krom, "generateTextureMipmaps", krom_generate_texture_mipmaps);
		SET_FUNCTION(krom, "generateRenderTargetMipmaps", krom_generate_render_target_mipmaps);
		SET_FUNCTION(krom, "setMipmaps", krom_set_mipmaps);
		SET_FUNCTION(krom, "setDepthStencilFrom", krom_set_depth_stencil_from);
		SET_FUNCTION_FAST(krom, "viewport", krom_viewport);
		SET_FUNCTION_FAST(krom, "scissor", krom_scissor);
		SET_FUNCTION_FAST(krom, "disableScissor", krom_disable_scissor);
		SET_FUNCTION_FAST(krom, "renderTargetsInvertedY", krom_render_targets_inverted_y);
		SET_FUNCTION(krom, "begin", krom_begin);
		SET_FUNCTION(krom, "beginFace", krom_begin_face);
		SET_FUNCTION(krom, "end", krom_end);
		SET_FUNCTION(krom, "fileSaveBytes", krom_file_save_bytes);
		SET_FUNCTION(krom, "sysCommand", krom_sys_command);
		SET_FUNCTION(krom, "savePath", krom_save_path);
		SET_FUNCTION(krom, "getArgCount", krom_get_arg_count);
		SET_FUNCTION(krom, "getArg", krom_get_arg);
		SET_FUNCTION(krom, "getFilesLocation", krom_get_files_location);
		SET_FUNCTION(krom, "httpRequest", krom_http_request);
		#ifdef WITH_COMPUTE
		SET_FUNCTION(krom, "setBoolCompute", krom_set_bool_compute);
		SET_FUNCTION(krom, "setIntCompute", krom_set_int_compute);
		SET_FUNCTION(krom, "setFloatCompute", krom_set_float_compute);
		SET_FUNCTION(krom, "setFloat2Compute", krom_set_float2_compute);
		SET_FUNCTION(krom, "setFloat3Compute", krom_set_float3_compute);
		SET_FUNCTION(krom, "setFloat4Compute", krom_set_float4_compute);
		SET_FUNCTION(krom, "setFloatsCompute", krom_set_floats_compute);
		SET_FUNCTION(krom, "setMatrixCompute", krom_set_matrix_compute);
		SET_FUNCTION(krom, "setMatrix3Compute", krom_set_matrix3_compute);
		SET_FUNCTION(krom, "setTextureCompute", krom_set_texture_compute);
		SET_FUNCTION(krom, "setRenderTargetCompute", krom_set_render_target_compute);
		SET_FUNCTION(krom, "setSampledTextureCompute", krom_set_sampled_texture_compute);
		SET_FUNCTION(krom, "setSampledRenderTargetCompute", krom_set_sampled_render_target_compute);
		SET_FUNCTION(krom, "setSampledDepthTextureCompute", krom_set_sampled_depth_texture_compute);
		SET_FUNCTION(krom, "setTextureParametersCompute", krom_set_texture_parameters_compute);
		SET_FUNCTION(krom, "setTexture3DParametersCompute", krom_set_texture3d_parameters_compute);
		SET_FUNCTION(krom, "setShaderCompute", krom_set_shader_compute);
		SET_FUNCTION(krom, "deleteShaderCompute", krom_delete_shader_compute);
		SET_FUNCTION(krom, "createShaderCompute", krom_create_shader_compute);
		SET_FUNCTION(krom, "getConstantLocationCompute", krom_get_constant_location_compute);
		SET_FUNCTION(krom, "getTextureUnitCompute", krom_get_texture_unit_compute);
		SET_FUNCTION(krom, "compute", krom_compute);
		#endif
		// Extended
		#ifdef WITH_G2
		SET_FUNCTION(krom, "g2_init", krom_g2_init);
		SET_FUNCTION(krom, "g2_begin", krom_g2_begin);
		SET_FUNCTION(krom, "g2_end", krom_g2_end);
		SET_FUNCTION(krom, "g2_draw_scaled_sub_image", krom_g2_draw_scaled_sub_image);
		SET_FUNCTION(krom, "g2_fill_triangle", krom_g2_fill_triangle);
		SET_FUNCTION(krom, "g2_fill_rect", krom_g2_fill_rect);
		SET_FUNCTION(krom, "g2_draw_rect", krom_g2_draw_rect);
		SET_FUNCTION(krom, "g2_draw_line", krom_g2_draw_line);
		SET_FUNCTION(krom, "g2_draw_string", krom_g2_draw_string);
		SET_FUNCTION(krom, "g2_set_font", krom_g2_set_font);
		SET_FUNCTION(krom, "g2_font_init", krom_g2_font_init);
		SET_FUNCTION(krom, "g2_font_13", krom_g2_font_13);
		SET_FUNCTION(krom, "g2_font_set_glyphs", krom_g2_font_set_glyphs);
		SET_FUNCTION(krom, "g2_font_count", krom_g2_font_count);
		SET_FUNCTION(krom, "g2_font_height", krom_g2_font_height);
		SET_FUNCTION(krom, "g2_string_width", krom_g2_string_width);
		SET_FUNCTION(krom, "g2_set_bilinear_filter", krom_g2_set_bilinear_filter);
		SET_FUNCTION(krom, "g2_restore_render_target", krom_g2_restore_render_target);
		SET_FUNCTION(krom, "g2_set_render_target", krom_g2_set_render_target);
		SET_FUNCTION(krom, "g2_set_color", krom_g2_set_color);
		SET_FUNCTION(krom, "g2_set_pipeline", krom_g2_set_pipeline);
		SET_FUNCTION(krom, "g2_set_transform", krom_g2_set_transform);
		SET_FUNCTION(krom, "g2_fill_circle", krom_g2_fill_circle);
		SET_FUNCTION(krom, "g2_draw_circle", krom_g2_draw_circle);
		SET_FUNCTION(krom, "g2_draw_cubic_bezier", krom_g2_draw_cubic_bezier);
		#endif
		SET_FUNCTION(krom, "setSaveAndQuitCallback", krom_set_save_and_quit_callback);
		SET_FUNCTION(krom, "setMouseCursor", krom_set_mouse_cursor);
		SET_FUNCTION_FAST(krom, "delayIdleSleep", krom_delay_idle_sleep);
		#if defined(WITH_NFD) || defined(KORE_IOS) || defined(KORE_ANDROID)
		SET_FUNCTION(krom, "openDialog", krom_open_dialog);
		SET_FUNCTION(krom, "saveDialog", krom_save_dialog);
		#endif
		#ifdef WITH_TINYDIR
		SET_FUNCTION(krom, "readDirectory", krom_read_directory);
		#endif
		SET_FUNCTION(krom, "fileExists", krom_file_exists);
		SET_FUNCTION(krom, "deleteFile", krom_delete_file);
		#ifdef WITH_ZLIB
		SET_FUNCTION(krom, "inflate", krom_inflate);
		SET_FUNCTION(krom, "deflate", krom_deflate);
		#endif
		#ifdef WITH_STB_IMAGE_WRITE
		SET_FUNCTION(krom, "writeJpg", krom_write_jpg);
		SET_FUNCTION(krom, "writePng", krom_write_png);
		SET_FUNCTION(krom, "encodeJpg", krom_encode_jpg);
		SET_FUNCTION(krom, "encodePng", krom_encode_png);
		#endif
		#ifdef WITH_MPEG_WRITE
		SET_FUNCTION(krom, "writeMpeg", krom_write_mpeg);
		#endif
		#ifdef WITH_ONNX
		SET_FUNCTION(krom, "mlInference", krom_ml_inference);
		SET_FUNCTION(krom, "mlUnload", krom_ml_unload);
		#endif
		#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
		SET_FUNCTION(krom, "raytraceSupported", krom_raytrace_supported);
		SET_FUNCTION(krom, "raytraceInit", krom_raytrace_init);
		SET_FUNCTION(krom, "raytraceSetTextures", krom_raytrace_set_textures);
		SET_FUNCTION(krom, "raytraceDispatchRays", krom_raytrace_dispatch_rays);
		#endif
		#ifdef KORE_VR
		SET_FUNCTION(krom, "vrBegin", krom_vr_begin);
		SET_FUNCTION(krom, "vrBeginRender", krom_vr_begin_render);
		SET_FUNCTION(krom, "vrEndRender", krom_vr_end_render);
		SET_FUNCTION(krom, "vrWarpSwap", krom_vr_warp_swap);
		SET_FUNCTION(krom, "vrGetSensorStateView", krom_vr_get_sensor_state_view);
		SET_FUNCTION(krom, "vrGetSensorStateProjection", krom_vr_get_sensor_state_projection);
		SET_FUNCTION(krom, "vrGetSensorStateHmdMounted", krom_vr_get_sensor_state_hmd_mounted);
		#endif
		SET_FUNCTION(krom, "windowX", krom_window_x);
		SET_FUNCTION(krom, "windowY", krom_window_y);
		SET_FUNCTION(krom, "language", krom_language);
		#ifdef WITH_IRON
		SET_FUNCTION(krom, "io_obj_parse", krom_io_obj_parse);
		#endif
		#ifdef WITH_ZUI
		SET_FUNCTION(krom, "zui_init", krom_zui_init);
		SET_FUNCTION(krom, "zui_get_scale", krom_zui_get_scale);
		SET_FUNCTION(krom, "zui_set_scale", krom_zui_set_scale);
		SET_FUNCTION(krom, "zui_set_font", krom_zui_set_font);
		SET_FUNCTION(krom, "zui_begin", krom_zui_begin);
		SET_FUNCTION(krom, "zui_end", krom_zui_end);
		SET_FUNCTION(krom, "zui_begin_region", krom_zui_begin_region);
		SET_FUNCTION(krom, "zui_end_region", krom_zui_end_region);
		SET_FUNCTION(krom, "zui_begin_sticky", krom_zui_begin_sticky);
		SET_FUNCTION(krom, "zui_end_sticky", krom_zui_end_sticky);
		SET_FUNCTION(krom, "zui_end_input", krom_zui_end_input);
		SET_FUNCTION(krom, "zui_end_window", krom_zui_end_window);
		SET_FUNCTION(krom, "zui_end_element", krom_zui_end_element);
		SET_FUNCTION(krom, "zui_start_text_edit", krom_zui_start_text_edit);
		SET_FUNCTION(krom, "zui_input_in_rect", krom_zui_input_in_rect);
		SET_FUNCTION(krom, "zui_window", krom_zui_window);
		SET_FUNCTION(krom, "zui_button", krom_zui_button);
		SET_FUNCTION(krom, "zui_check", krom_zui_check);
		SET_FUNCTION(krom, "zui_radio", krom_zui_radio);
		SET_FUNCTION(krom, "zui_combo", krom_zui_combo);
		SET_FUNCTION(krom, "zui_slider", krom_zui_slider);
		SET_FUNCTION(krom, "zui_image", krom_zui_image);
		SET_FUNCTION(krom, "zui_text", krom_zui_text);
		SET_FUNCTION(krom, "zui_text_input", krom_zui_text_input);
		SET_FUNCTION(krom, "zui_tab", krom_zui_tab);
		SET_FUNCTION(krom, "zui_panel", krom_zui_panel);
		SET_FUNCTION(krom, "zui_handle", krom_zui_handle);
		SET_FUNCTION(krom, "zui_separator", krom_zui_separator);
		SET_FUNCTION(krom, "zui_tooltip", krom_zui_tooltip);
		SET_FUNCTION(krom, "zui_tooltip_image", krom_zui_tooltip_image);
		SET_FUNCTION(krom, "zui_row", krom_zui_row);
		SET_FUNCTION(krom, "zui_fill", krom_zui_fill);
		SET_FUNCTION(krom, "zui_rect", krom_zui_rect);
		SET_FUNCTION(krom, "zui_draw_rect", krom_zui_draw_rect);
		SET_FUNCTION(krom, "zui_draw_string", krom_zui_draw_string);
		SET_FUNCTION(krom, "zui_get_hovered_tab_name", krom_zui_get_hovered_tab_name);
		SET_FUNCTION(krom, "zui_set_hovered_tab_name", krom_zui_set_hovered_tab_name);
		SET_FUNCTION(krom, "zui_begin_menu", krom_zui_begin_menu);
		SET_FUNCTION(krom, "zui_end_menu", krom_zui_end_menu);
		SET_FUNCTION(krom, "zui_menu_button", krom_zui_menu_button);
		SET_FUNCTION(krom, "zui_float_input", krom_zui_float_input);
		SET_FUNCTION(krom, "zui_inline_radio", krom_zui_inline_radio);
		SET_FUNCTION(krom, "zui_color_wheel", krom_zui_color_wheel);
		SET_FUNCTION(krom, "zui_text_area", krom_zui_text_area);
		SET_FUNCTION(krom, "zui_text_area_coloring", krom_zui_text_area_coloring);
		SET_FUNCTION(krom, "zui_nodes_init", krom_zui_nodes_init);
		SET_FUNCTION(krom, "zui_node_canvas", krom_zui_node_canvas);
		SET_FUNCTION(krom, "zui_nodes_rgba_popup", krom_zui_nodes_rgba_popup);
		SET_FUNCTION(krom, "zui_nodes_scale", krom_zui_nodes_scale);
		SET_FUNCTION(krom, "zui_nodes_pan_x", krom_zui_nodes_pan_x);
		SET_FUNCTION(krom, "zui_nodes_pan_y", krom_zui_nodes_pan_y);
		SET_FUNCTION(krom, "zui_set", krom_zui_set);
		SET_FUNCTION(krom, "zui_get", krom_zui_get);
		SET_FUNCTION(krom, "zui_handle_get", krom_zui_handle_get);
		SET_FUNCTION(krom, "zui_handle_set", krom_zui_handle_set);
		SET_FUNCTION(krom, "zui_handle_ptr", krom_zui_handle_ptr);
		SET_FUNCTION(krom, "zui_theme_init", krom_zui_theme_init);
		SET_FUNCTION(krom, "zui_theme_get", krom_zui_theme_get);
		SET_FUNCTION(krom, "zui_theme_set", krom_zui_theme_set);
		SET_FUNCTION(krom, "zui_set_on_border_hover", krom_zui_set_on_border_hover);
		SET_FUNCTION(krom, "zui_set_on_text_hover", krom_zui_set_on_text_hover);
		SET_FUNCTION(krom, "zui_set_on_deselect_text", krom_zui_set_on_deselect_text);
		SET_FUNCTION(krom, "zui_set_on_tab_drop", krom_zui_set_on_tab_drop);
		#endif

		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		global->Set(String::NewFromUtf8(isolate, "Krom").ToLocalChecked(), krom);

		#ifdef WITH_PLUGIN_EMBED
		plugin_embed(isolate, global);
		#endif

		Local<Context> context = Context::New(isolate, NULL, global);
		global_context.Reset(isolate, context);
	}

	void start_krom(char *scriptfile) {
		Locker locker{isolate};

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
			Local<Value> js_kickstart = context->Global()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "kickstart").ToLocalChecked()).ToLocalChecked();
			if (!js_kickstart->IsNullOrUndefined()) {
				Local<Value> result;
				if (!js_kickstart->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->CallAsFunction(context, context->Global(), 0, nullptr).ToLocal(&result)) {
					handle_exception(&try_catch);
				}
			}
		}
	}

	void run_v8() {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		MicrotasksScope microtasks_scope(isolate, MicrotasksScope::kRunMicrotasks);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, update_func);
		Local<Value> result;

		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void update(void *data) {
		#ifdef KORE_WINDOWS
		if (show_window && enable_window) {
			show_window = false;
			kinc_window_show(0);
		}

		if (in_background && ++paused_frames > 3 && armorcore) {
			Sleep(1);
			return;
		}
		#endif

		#ifdef IDLE_SLEEP
		if (last_window_width != kinc_window_width(0) || last_window_height != kinc_window_height(0)) {
			last_window_width = kinc_window_width(0);
			last_window_height = kinc_window_height(0);
			paused_frames = 0;
		}
		#if defined(KORE_IOS) || defined(KORE_ANDROID)
		int start_sleep = 1200;
		#else
		int start_sleep = 120;
		#endif
		if (++paused_frames > start_sleep && !input_down) {
			#ifdef KORE_WINDOWS
			Sleep(1);
			#else
			usleep(1000);
			#endif
			return;
		}
		#endif

		#ifdef WITH_AUDIO
		if (enable_audio) {
			kinc_a2_update();
		}
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

	void drop_files(wchar_t *file_path, void *data) {
		// Update mouse position
		#ifdef KORE_WINDOWS
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(kinc_windows_window_handle(0), &p);
		mouse_move(0, p.x, p.y, 0, 0, NULL);
		#endif

		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, drop_files_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc];
		if (sizeof(wchar_t) == 2) {
			argv[0] = {String::NewFromTwoByte(isolate, (const uint16_t *)file_path).ToLocalChecked()};
		}
		else {
			size_t len = wcslen(file_path);
			uint16_t *str = new uint16_t[len + 1];
			for (int i = 0; i < len; i++) str[i] = file_path[i];
			str[len] = 0;
			argv[0] = {String::NewFromTwoByte(isolate, str).ToLocalChecked()};
			delete[] str;
		}
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		in_background = false;

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	char *copy(void *data) {
		// Locker locker{isolate};

		// Isolate::Scope isolate_scope(isolate);
		// HandleScope handle_scope(isolate);
		// Local<Context> context = Local<Context>::New(isolate, global_context);
		// Context::Scope context_scope(context);

		// TryCatch try_catch(isolate);
		// Local<Function> func = Local<Function>::New(isolate, copy_func);
		// Local<Value> result;
		// if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
		// 	handle_exception(&try_catch);
		// }
		// String::Utf8Value cutCopyString(isolate, result);
		// strcpy(temp_string, *cutCopyString);

		#ifdef WITH_ZUI
		strcpy(temp_string, zui_copy());
		#endif

		return temp_string;
	}

	char *cut(void *data) {
		// Locker locker{isolate};

		// Isolate::Scope isolate_scope(isolate);
		// HandleScope handle_scope(isolate);
		// Local<Context> context = Local<Context>::New(isolate, global_context);
		// Context::Scope context_scope(context);

		// TryCatch try_catch(isolate);
		// Local<Function> func = Local<Function>::New(isolate, cut_func);
		// Local<Value> result;
		// if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
		// 	handle_exception(&try_catch);
		// }
		// String::Utf8Value cutCopyString(isolate, result);
		// strcpy(temp_string, *cutCopyString);

		#ifdef WITH_ZUI
		strcpy(temp_string, zui_cut());
		#endif

		return temp_string;
	}

	void paste(char *text, void *data) {
		// Locker locker{isolate};

		// Isolate::Scope isolate_scope(isolate);
		// HandleScope handle_scope(isolate);
		// Local<Context> context = Local<Context>::New(isolate, global_context);
		// Context::Scope context_scope(context);

		// TryCatch try_catch(isolate);
		// Local<Function> func = Local<Function>::New(isolate, paste_func);
		// Local<Value> result;
		// const int argc = 1;
		// Local<Value> argv[argc] = {String::NewFromUtf8(isolate, text).ToLocalChecked()};
		// if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
		// 	handle_exception(&try_catch);
		// }

		#ifdef WITH_ZUI
		zui_paste(text);
		#endif
	}

	void foreground(void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, foreground_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		in_background = false;
	}

	void resume(void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, resume_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void pause(void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, pause_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void background(void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, background_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		in_background = true;
		paused_frames = 0;
	}

	void shutdown(void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, shutdown_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			handle_exception(&try_catch);
		}
	}

	void key_down(int code, void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, keyboard_down_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_key_down(zui_instances[i], code);
		#endif

		#ifdef IDLE_SLEEP
		input_down = true;
		paused_frames = 0;
		#endif
	}

	void key_up(int code, void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, keyboard_up_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_key_up(zui_instances[i], code);
		#endif

		#ifdef IDLE_SLEEP
		input_down = false;
		paused_frames = 0;
		#endif
	}

	void key_press(unsigned int character, void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, keyboard_press_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, character)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_key_press(zui_instances[i], character);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void mouse_move(int window, int x, int y, int mx, int my, void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, mouse_move_func);
		Local<Value> result;
		const int argc = 4;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Int32::New(isolate, mx), Int32::New(isolate, my)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_move(zui_instances[i], x, y, mx, my);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void mouse_down(int window, int button, int x, int y, void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, mouse_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_down(zui_instances[i], button, x, y);
		#endif

		#ifdef IDLE_SLEEP
		input_down = true;
		paused_frames = 0;
		#endif
	}

	void mouse_up(int window, int button, int x, int y, void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, mouse_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_up(zui_instances[i], button, x, y);
		#endif

		#ifdef IDLE_SLEEP
		input_down = false;
		paused_frames = 0;
		#endif
	}

	void mouse_wheel(int window, int delta, void *data) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, mouse_wheel_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, delta)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_wheel(zui_instances[i], delta);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void touch_move(int index, int x, int y) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, touch_move_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		#if defined(KORE_ANDROID) || defined(KORE_IOS)
		for (int i = 0; i < zui_instances_count; ++i) zui_touch_move(zui_instances[i], index, x, y);
		#endif
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void touch_down(int index, int x, int y) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, touch_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		#if defined(KORE_ANDROID) || defined(KORE_IOS)
		for (int i = 0; i < zui_instances_count; ++i) zui_touch_down(zui_instances[i], index, x, y);
		#endif
		#endif

		#ifdef IDLE_SLEEP
		input_down = true;
		paused_frames = 0;
		#endif
	}

	void touch_up(int index, int x, int y) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, touch_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		#if defined(KORE_ANDROID) || defined(KORE_IOS)
		for (int i = 0; i < zui_instances_count; ++i) zui_touch_up(zui_instances[i], index, x, y);
		#endif
		#endif

		#ifdef IDLE_SLEEP
		input_down = false;
		paused_frames = 0;
		#endif
	}

	void pen_down(int window, int x, int y, float pressure) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, pen_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_pen_down(zui_instances[i], x, y, pressure);
		#endif

		#ifdef IDLE_SLEEP
		input_down = true;
		paused_frames = 0;
		#endif
	}

	void pen_up(int window, int x, int y, float pressure) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, pen_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_pen_up(zui_instances[i], x, y, pressure);
		#endif

		#ifdef IDLE_SLEEP
		input_down = false;
		paused_frames = 0;
		#endif
	}

	void pen_move(int window, int x, int y, float pressure) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, pen_move_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_pen_move(zui_instances[i], x, y, pressure);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void gamepad_axis(int gamepad, int axis, float value) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, gamepad_axis_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, axis), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void gamepad_button(int gamepad, int button, float value) {
		Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<Function> func = Local<Function>::New(isolate, gamepad_button_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, button), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			handle_exception(&try_catch);
		}

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}
}

int kickstart(int argc, char **argv) {
	_argc = argc;
	_argv = argv;
#ifdef KORE_ANDROID
	std::string bindir("/");
#elif defined(KORE_IOS)
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
		if (strcmp(argv[i], "--nowindow") == 0) {
			enable_window = false;
		}
		#ifdef WITH_AUDIO
		else if (strcmp(argv[i], "--nosound") == 0) {
			enable_audio = false;
		}
		#endif
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

#ifdef KORE_MACOS
	// Handle loading assets located outside of '.app/Contents/Resources/Deployment' folder
	// when assets and shaders dir is passed as an argument
	if (argc > 2) {
		kinc_internal_set_files_location(&assetsdir[0u]);
	}
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
	char *code = (char *)malloc(reader_size + 1);
	kinc_file_reader_read(&reader, code, reader_size);
	code[reader_size] = 0;
	kinc_file_reader_close(&reader);

	if (snapshot) {
		plat = platform::NewDefaultPlatform();
		V8::InitializePlatform(plat.get());

		std::string flags = "--nolazy";
		V8::SetFlagsFromString(flags.c_str(), (int)flags.size());
		V8::Initialize();

		ScriptCompiler::CachedData *cache;
		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
		Isolate *isolate_cache = Isolate::New(create_params);
		{
			HandleScope handle_scope(isolate_cache);
			{
				Local<Context> context = Context::New(isolate_cache);
				Context::Scope context_scope(context);

				ScriptOrigin origin(isolate_cache, String::NewFromUtf8(isolate_cache, "krom_cache").ToLocalChecked());
				ScriptCompiler::Source source(String::NewFromUtf8(isolate_cache, code).ToLocalChecked(), origin);

				Local<Script> compiled_script = ScriptCompiler::Compile(context, &source, ScriptCompiler::kEagerCompile).ToLocalChecked();
				cache = ScriptCompiler::CreateCodeCache(compiled_script->GetUnboundScript());
			}
		}

		SnapshotCreator creator_cold;
		Isolate *isolate_cold = creator_cold.GetIsolate();
		{
			HandleScope handle_scope(isolate_cold);
			{
				Local<Context> context = Context::New(isolate_cold);
				Context::Scope context_scope(context);

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
						std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
						kinc_file_reader_read(&reader, content->Data(), reader_size);
						kinc_file_reader_close(&reader);

						(void) context->Global()->Set(context, String::NewFromUtf8(isolate_cold, line).ToLocalChecked(), buffer);
					}
					fclose (fp);
				}

				ScriptOrigin origin(isolate_cache, String::NewFromUtf8(isolate_cold, "krom_cold").ToLocalChecked());
				ScriptCompiler::Source source(String::NewFromUtf8(isolate_cold, code).ToLocalChecked(), origin, cache);

				Local<Script> compiled_script = ScriptCompiler::Compile(context, &source, ScriptCompiler::kConsumeCodeCache).ToLocalChecked();
				(void) compiled_script->Run(context);

				creator_cold.SetDefaultContext(context);
			}
		}
		StartupData coldData = creator_cold.CreateBlob(SnapshotCreator::FunctionCodeHandling::kKeep);

		// SnapshotCreator creator_warm(nullptr, &coldData);
		// Isolate *isolate_warm = creator_warm.GetIsolate();
		// {
		// 	HandleScope handle_scope(isolate_warm);
		// 	{
		// 		Local<Context> context = Context::New(isolate_warm);
		// 		Context::Scope context_scope(context);

		// 		// std::string code_warm("Main.main();");
		// 		ScriptOrigin origin(String::NewFromUtf8(isolate_warm, "krom_warm").ToLocalChecked());
		// 		ScriptCompiler::Source source(String::NewFromUtf8(isolate_warm, code).ToLocalChecked(), origin);

		// 		Local<Script> compiled_script = ScriptCompiler::Compile(context, &source, ScriptCompiler::kEagerCompile).ToLocalChecked();
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
		FILE *file = fopen(&krombin[0u], "wb");
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

	#ifdef WITH_AUDIO
	if (enable_audio) {
		kinc_a2_shutdown();
	}
	#endif

	#ifdef WITH_ONNX
	if (ort != NULL) {
		ort->ReleaseEnv(ort_env);
		ort->ReleaseSessionOptions(ort_session_options);
	}
	#endif

	free(code);

	return 0;
}
