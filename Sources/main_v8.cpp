#include <map>
#include <string>
#include <vector>
#include <kinc/log.h>
#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>
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

#ifdef WITH_AUDIO
#include <kinc/audio1/audio.h>
#include <kinc/audio1/sound.h>
#include <kinc/audio2/audio.h>
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
extern "C" bool waitAfterNextDraw;
#endif

#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/raytrace.h>
#endif

#include <libplatform/libplatform.h>
#include <v8.h>
#include <v8-fast-api-calls.h>

#ifdef KORE_WINDOWS
#include <Windows.h> // AttachConsole
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
extern "C" struct HWND__ *kinc_windows_window_handle(int window_index); // Kore/Windows.h
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

#if defined(KORE_IOS) || defined(KORE_ANDROID)
char mobile_title[1024];
#endif

#if defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)
extern "C" int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
#endif

#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
extern "C" kinc_g5_command_list_t commandList;
static kinc_g5_constant_buffer_t constant_buffer;
static kinc_g4_render_target_t *render_target;
static kinc_raytrace_pipeline_t pipeline;
static kinc_raytrace_acceleration_structure_t accel;
static bool accel_created = false;
const int constant_buffer_size = 24;
#endif

#define TO_I32(arg) arg->Int32Value(isolate->GetCurrentContext()).FromJust()
#define TO_F32(arg) (float)arg->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value()
#define TO_OBJ(arg) arg->ToObject(isolate->GetCurrentContext()).ToLocalChecked()
#define TO_STR(arg) String::NewFromUtf8(isolate, arg).ToLocalChecked()
#define GET_INTERNALI(arg, i) TO_OBJ(arg)->GetInternalField(i)
#define GET_INTERNAL(arg) GET_INTERNALI(arg, 0)
#define TO_EXTERNAL(arg) Local<External>::Cast(arg)->Value()
#define MAKE_OBJI(arg, i)\
	Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);\
	templ->SetInternalFieldCount(i);\
	Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();\
	obj->SetInternalField(0, External::New(isolate, arg));
#define MAKE_OBJ(arg) MAKE_OBJI(arg, 1)
#define MAKE_EXTERNAL(arg1, arg2) Local<External> arg1 = Local<External>::Cast(arg2)
#define MAKE_CONTENT(arg)\
	Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(arg);\
	std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
#define OBJ_GET(arg, str) arg->Get(isolate->GetCurrentContext(), TO_STR(str)).ToLocalChecked()
#define ARRAY_GET(arg, i) arg->Get(isolate->GetCurrentContext(), i).ToLocalChecked()
#define SET_FUNC(func, arg) func.Reset(isolate, Local<Function>::Cast(arg))
#define RETURN_I32(i) args.GetReturnValue().Set(Int32::New(isolate, i))
#define RETURN_F64(f) args.GetReturnValue().Set(Number::New(isolate, f))
#define RETURN_STR(str) args.GetReturnValue().Set(TO_STR(str))
#define RETURN_OBJI(arg, i)\
	MAKE_OBJI(arg, i);\
	args.GetReturnValue().Set(obj);
#define RETURN_OBJ(arg) RETURN_OBJI(arg, 1)
#define RETURN_BUFFER(data, size)\
	std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore((void *)data, size, [](void *, size_t, void *) {}, nullptr);\
	Local<ArrayBuffer> out = ArrayBuffer::New(isolate, std::move(backing));\
	args.GetReturnValue().Set(out);
#define SCOPE() HandleScope scope(args.GetIsolate())
#define ARGS const FunctionCallbackInfo<Value> &args
#define LOCKER()\
	Locker locker{isolate};\
	Isolate::Scope isolate_scope(isolate);\
	HandleScope handle_scope(isolate);\
	Local<Context> context = Local<Context>::New(isolate, global_context);\
	Context::Scope context_scope(context);
#define CALL_FUNCI(fn, argc, argv)\
	TryCatch try_catch(isolate);\
	Local<Function> func = Local<Function>::New(isolate, fn);\
	Local<Value> result;\
	if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result))\
		handle_exception(&try_catch);
#define CALL_FUNC(fn) CALL_FUNCI(fn, 0, NULL)

namespace {
	int _argc;
	char **_argv;
	bool enable_window = true;
	bool snapshot = false;
	bool stderr_created = false;
	bool in_background = false;
	int paused_frames = 0;
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
	#ifdef WITH_ZUI
	Global<Function> picker_func;
	Global<Function> on_border_hover_func;
	Global<Function> on_text_hover_func;
	Global<Function> on_deselect_text_func;
	Global<Function> on_tab_drop_func;
	Global<Function> enum_texts_func;
	Global<Function> on_custom_button_func;
	Global<Function> on_canvas_control_func;
	Global<Function> on_canvas_released_func;
	Global<Function> on_socket_released_func;
	Global<Function> on_link_drag_func;
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

	void krom_init(ARGS) {
		SCOPE();
		kinc_window_options_t win;
		kinc_framebuffer_options_t frame;
		String::Utf8Value title(isolate, args[0]);
		win.title = *title;
		win.width = TO_I32(args[1]);
		win.height = TO_I32(args[2]);
		frame.vertical_sync = TO_I32(args[3]);
		win.mode = (kinc_window_mode_t)TO_I32(args[4]);
		win.window_features = TO_I32(args[5]);
		win.x = TO_I32(args[6]);
		win.y = TO_I32(args[7]);
		frame.frequency = TO_I32(args[8]);

		win.display_index = -1;
		#ifdef KORE_WINDOWS
		win.visible = false; // Prevent white flicker when opening the window
		#else
		win.visible = enable_window;
		#endif
		frame.color_bits = 32;
		frame.depth_bits = 0;
		frame.stencil_bits = 0;
		kinc_init(*title, win.width, win.height, &win, &frame);
		kinc_random_init((int)(kinc_time() * 1000));

		#ifdef KORE_WINDOWS
		// Maximized window has x < -1, prevent window centering done by kinc
		if (win.x < -1 && win.y < -1) {
			kinc_window_move(0, win.x, win.y);
		}

		char vdata[4];
		DWORD cbdata = 4 * sizeof(char);
		RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, vdata, &cbdata);
		BOOL use_dark_mode = int(vdata[3] << 24 | vdata[2] << 16 | vdata[1] << 8 | vdata[0]) != 1;
		DwmSetWindowAttribute(kinc_windows_window_handle(0), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));

		show_window = true;
		#endif

		#ifdef WITH_AUDIO
		kinc_a1_init();
		kinc_a2_init();
		#endif

		#ifdef KORE_ANDROID
		android_check_permissions();
		#endif
	}

	void krom_set_app_name(ARGS) {
		// Name used by kinc_internal_save_path()
		SCOPE();
		String::Utf8Value name(isolate, args[0]);
		kinc_set_application_name(*name);
	}

	void krom_log(ARGS) {
		SCOPE();
		String::Utf8Value value(isolate, args[0]);
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

	void krom_g4_clear_fast(Local<Object> receiver, int flags, int color, float depth) {
		kinc_g4_clear(flags, color, depth, 0);
	}

	void krom_g4_clear(ARGS) {
		SCOPE();
		int flags = TO_I32(args[0]);
		int color = TO_I32(args[1]);
		float depth = TO_F32(args[2]);
		krom_g4_clear_fast(args.This(), flags, color, depth);
	}

	void krom_set_update_callback(ARGS) {
		kinc_set_update_callback(update, NULL);
		SCOPE();
		SET_FUNC(update_func, args[0]);
	}

	void krom_set_drop_files_callback(ARGS) {
		kinc_set_drop_files_callback(drop_files, NULL);
		SCOPE();
		SET_FUNC(drop_files_func, args[0]);
	}

	void krom_set_cut_copy_paste_callback(ARGS) {
		// SCOPE();
		// kinc_set_cut_callback(cut, NULL);
		// kinc_set_copy_callback(copy, NULL);
		// kinc_set_paste_callback(paste, NULL);
		// SET_FUNC(cut_func, args[0]);
		// SET_FUNC(copy_func, args[1]);
		// SET_FUNC(paste_func, args[2]);
	}

	void krom_set_application_state_callback(ARGS) {
		SCOPE();
		kinc_set_foreground_callback(foreground, NULL);
		kinc_set_resume_callback(resume, NULL);
		kinc_set_pause_callback(pause, NULL);
		kinc_set_background_callback(background, NULL);
		kinc_set_shutdown_callback(shutdown, NULL);
		SET_FUNC(foreground_func, args[0]);
		SET_FUNC(resume_func, args[1]);
		SET_FUNC(pause_func, args[2]);
		SET_FUNC(background_func, args[3]);
		SET_FUNC(shutdown_func, args[4]);
	}

	void krom_set_keyboard_down_callback(ARGS) {
		SCOPE();
		kinc_keyboard_set_key_down_callback(key_down, NULL);
		SET_FUNC(keyboard_down_func, args[0]);
	}

	void krom_set_keyboard_up_callback(ARGS) {
		SCOPE();
		kinc_keyboard_set_key_up_callback(key_up, NULL);
		SET_FUNC(keyboard_up_func, args[0]);
	}

	void krom_set_keyboard_press_callback(ARGS) {
		SCOPE();
		kinc_keyboard_set_key_press_callback(key_press, NULL);
		SET_FUNC(keyboard_press_func, args[0]);
	}

	void krom_set_mouse_down_callback(ARGS) {
		SCOPE();
		kinc_mouse_set_press_callback(mouse_down, NULL);
		SET_FUNC(mouse_down_func, args[0]);
	}

	void krom_set_mouse_up_callback(ARGS) {
		SCOPE();
		kinc_mouse_set_release_callback(mouse_up, NULL);
		SET_FUNC(mouse_up_func, args[0]);
	}

	void krom_set_mouse_move_callback(ARGS) {
		SCOPE();
		kinc_mouse_set_move_callback(mouse_move, NULL);
		SET_FUNC(mouse_move_func, args[0]);
	}

	void krom_set_mouse_wheel_callback(ARGS) {
		SCOPE();
		kinc_mouse_set_scroll_callback(mouse_wheel, NULL);
		SET_FUNC(mouse_wheel_func, args[0]);
	}

	void krom_set_touch_down_callback(ARGS) {
		SCOPE();
		kinc_surface_set_touch_start_callback(touch_down);
		SET_FUNC(touch_down_func, args[0]);
	}

	void krom_set_touch_up_callback(ARGS) {
		SCOPE();
		kinc_surface_set_touch_end_callback(touch_up);
		SET_FUNC(touch_up_func, args[0]);
	}

	void krom_set_touch_move_callback(ARGS) {
		SCOPE();
		kinc_surface_set_move_callback(touch_move);
		SET_FUNC(touch_move_func, args[0]);
	}

	void krom_set_pen_down_callback(ARGS) {
		SCOPE();
		kinc_pen_set_press_callback(pen_down);
		SET_FUNC(pen_down_func, args[0]);
	}

	void krom_set_pen_up_callback(ARGS) {
		SCOPE();
		kinc_pen_set_release_callback(pen_up);
		SET_FUNC(pen_up_func, args[0]);
	}

	void krom_set_pen_move_callback(ARGS) {
		SCOPE();
		kinc_pen_set_move_callback(pen_move);
		SET_FUNC(pen_move_func, args[0]);
	}

	void krom_set_gamepad_axis_callback(ARGS) {
		SCOPE();
		kinc_gamepad_set_axis_callback(gamepad_axis);
		SET_FUNC(gamepad_axis_func, args[0]);
	}

	void krom_set_gamepad_button_callback(ARGS) {
		SCOPE();
		kinc_gamepad_set_button_callback(gamepad_button);
		SET_FUNC(gamepad_button_func, args[0]);
	}

	void krom_lock_mouse_fast(Local<Object> receiver) {
		kinc_mouse_lock(0);
	}

	void krom_lock_mouse(ARGS) {
		SCOPE();
		krom_lock_mouse_fast(args.This());
	}

	void krom_unlock_mouse_fast(Local<Object> receiver) {
		kinc_mouse_unlock();
	}

	void krom_unlock_mouse(ARGS) {
		SCOPE();
		krom_unlock_mouse_fast(args.This());
	}

	int krom_can_lock_mouse_fast(Local<Object> receiver) {
		return kinc_mouse_can_lock();
	}

	void krom_can_lock_mouse(ARGS) {
		SCOPE();
		RETURN_I32(krom_can_lock_mouse_fast(args.This()));
	}

	int krom_is_mouse_locked_fast(Local<Object> receiver) {
		return kinc_mouse_is_locked();
	}

	void krom_is_mouse_locked(ARGS) {
		SCOPE();
		RETURN_I32(krom_is_mouse_locked_fast(args.This()));
	}

	void krom_set_mouse_position_fast(Local<Object> receiver, int x, int y) {
		kinc_mouse_set_position(0, x, y);
	}

	void krom_set_mouse_position(ARGS) {
		SCOPE();
		int x = TO_I32(args[0]);
		int y = TO_I32(args[1]);
		krom_set_mouse_position_fast(args.This(), x, y);
	}

	void krom_show_mouse_fast(Local<Object> receiver, int show) {
		show ? kinc_mouse_show() : kinc_mouse_hide();
	}

	void krom_show_mouse(ARGS) {
		SCOPE();
		int show = TO_I32(args[0]);
		krom_show_mouse_fast(args.This(), show);
	}

	void krom_show_keyboard_fast(Local<Object> receiver, int show) {
		show ? kinc_keyboard_show() : kinc_keyboard_hide();
	}

	void krom_show_keyboard(ARGS) {
		SCOPE();
		int show = TO_I32(args[0]);
		krom_show_keyboard_fast(args.This(), show);
	}

	void krom_g4_create_index_buffer(ARGS) {
		SCOPE();
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)malloc(sizeof(kinc_g4_index_buffer_t));
		kinc_g4_index_buffer_init(buffer, TO_I32(args[0]), KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
		RETURN_OBJ(buffer)
	}

	void krom_g4_delete_index_buffer(ARGS) {
		SCOPE();
		Local<External> field = Local<External>::Cast(GET_INTERNAL(args[0]));
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)field->Value();
		kinc_g4_index_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_g4_lock_index_buffer(ARGS) {
		SCOPE();
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		void *vertices = kinc_g4_index_buffer_lock_all(buffer);
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore(
			vertices, kinc_g4_index_buffer_count(buffer) * sizeof(int), [](void *, size_t, void *) {}, nullptr);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(Uint32Array::New(abuffer, 0, kinc_g4_index_buffer_count(buffer)));
	}

	void krom_g4_unlock_index_buffer(ARGS) {
		SCOPE();
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_index_buffer_unlock_all(buffer);
	}

	void krom_g4_set_index_buffer(ARGS) {
		SCOPE();
		kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_set_index_buffer(buffer);
	}

	void krom_g4_create_vertex_buffer(ARGS) {
		SCOPE();
		Local<Object> js_structure = TO_OBJ(args[1]);
		int32_t length = TO_I32(OBJ_GET(js_structure, "length"));
		kinc_g4_vertex_structure_t structure;
		kinc_g4_vertex_structure_init(&structure);
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> element = TO_OBJ(ARRAY_GET(js_structure, i));
			Local<Value> str = OBJ_GET(element, "name");
			String::Utf8Value utf8_value(isolate, str);
			int32_t data = TO_I32(OBJ_GET(element, "data"));
			strcpy(temp_string_vstruct[0][i], *utf8_value);
			kinc_g4_vertex_structure_add(&structure, temp_string_vstruct[0][i], (kinc_g4_vertex_data_t)data);
		}
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)malloc(sizeof(kinc_g4_vertex_buffer_t));
		kinc_g4_vertex_buffer_init(buffer, TO_I32(args[0]), &structure, (kinc_g4_usage_t)TO_I32(args[2]), TO_I32(args[3]));
		RETURN_OBJ(buffer);
	}

	void krom_g4_delete_vertex_buffer(ARGS) {
		SCOPE();
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_vertex_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_g4_lock_vertex_buffer(ARGS) {
		SCOPE();
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		float *vertices = kinc_g4_vertex_buffer_lock_all(buffer);
		RETURN_BUFFER(vertices, (kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer)));
	}

	void krom_g4_unlock_vertex_buffer(ARGS) {
		SCOPE();
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_vertex_buffer_unlock_all(buffer);
	}

	void krom_g4_set_vertex_buffer(ARGS) {
		SCOPE();
		kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_set_vertex_buffer(buffer);
	}

	void krom_g4_set_vertex_buffers(ARGS) {
		SCOPE();
		kinc_g4_vertex_buffer_t *vertex_buffers[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		Local<Object> js_array = TO_OBJ(args[0]);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> bufferobj = TO_OBJ(OBJ_GET(TO_OBJ(ARRAY_GET(js_array, i)), "buffer_"));
			kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)TO_EXTERNAL(GET_INTERNAL(bufferobj));
			vertex_buffers[i] = buffer;
		}
		kinc_g4_set_vertex_buffers(vertex_buffers, length);
	}

	void krom_g4_draw_indexed_vertices_fast(Local<Object> receiver, int start, int count) {
		#ifdef KORE_DIRECT3D12
		// TODO: Prevent heapIndex overflow in texture.c.h/kinc_g5_internal_set_textures
		waitAfterNextDraw = true;
		#endif
		if (count < 0) kinc_g4_draw_indexed_vertices();
		else kinc_g4_draw_indexed_vertices_from_to(start, count);
	}

	void krom_g4_draw_indexed_vertices(ARGS) {
		SCOPE();
		int start = TO_I32(args[0]);
		int count = TO_I32(args[1]);
		krom_g4_draw_indexed_vertices_fast(args.This(), start, count);
	}

	void krom_g4_draw_indexed_vertices_instanced_fast(Local<Object> receiver, int instance_count, int start, int count) {
		if (count < 0) kinc_g4_draw_indexed_vertices_instanced(instance_count);
		else kinc_g4_draw_indexed_vertices_instanced_from_to(instance_count, start, count);
	}

	void krom_g4_draw_indexed_vertices_instanced(ARGS) {
		SCOPE();
		int instance_count = TO_I32(args[0]);
		int start = TO_I32(args[1]);
		int count = TO_I32(args[2]);
		krom_g4_draw_indexed_vertices_instanced_fast(args.This(), instance_count, start, count);
	}

	void krom_g4_create_shader(ARGS) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		int shader_type = TO_I32(args[1]);
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content->Data(), (int)content->ByteLength(), (kinc_g4_shader_type_t)shader_type);
		RETURN_OBJ(shader);
	}

	void krom_g4_create_vertex_shader_from_source(ARGS) {
		SCOPE();
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

		RETURN_OBJ(shader);
	}

	void krom_g4_create_fragment_shader_from_source(ARGS) {
		SCOPE();
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

		RETURN_OBJ(shader);
	}

	void krom_g4_delete_shader(ARGS) {
		SCOPE();
		kinc_g4_shader_t *shader = (kinc_g4_shader_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_shader_destroy(shader);
		free(shader);
	}

	void krom_g4_create_pipeline(ARGS) {
		SCOPE();
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)malloc(sizeof(kinc_g4_pipeline_t));
		kinc_g4_pipeline_init(pipeline);
		RETURN_OBJ(pipeline);
	}

	void krom_g4_delete_pipeline(ARGS) {
		SCOPE();
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_pipeline_destroy(pipeline);
		free(pipeline);
	}

	void krom_g4_compile_pipeline(ARGS) {
		SCOPE();
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_vertex_structure_t s0, s1, s2, s3;
		kinc_g4_vertex_structure_init(&s0);
		kinc_g4_vertex_structure_init(&s1);
		kinc_g4_vertex_structure_init(&s2);
		kinc_g4_vertex_structure_init(&s3);
		kinc_g4_vertex_structure_t *structures[4] = { &s0, &s1, &s2, &s3 };

		int32_t size = TO_I32(TO_OBJ(args[5]));
		for (int32_t i1 = 0; i1 < size; ++i1) {
			Local<Object> js_structure = TO_OBJ(args[i1 + 1]);
			structures[i1]->instanced = TO_I32(OBJ_GET(js_structure, "instanced"));
			Local<Object> elements = TO_OBJ(OBJ_GET(js_structure, "elements"));
			int32_t length = TO_I32(OBJ_GET(elements, "length"));
			for (int32_t i2 = 0; i2 < length; ++i2) {
				Local<Object> element = TO_OBJ(ARRAY_GET(elements, i2));
				Local<Value> str = OBJ_GET(element, "name");
				String::Utf8Value utf8_value(isolate, str);
				int32_t data = TO_I32(OBJ_GET(element, "data"));

				strcpy(temp_string_vstruct[i1][i2], *utf8_value);
				kinc_g4_vertex_structure_add(structures[i1], temp_string_vstruct[i1][i2], (kinc_g4_vertex_data_t)data);
			}
		}

		pipeline->vertex_shader = (kinc_g4_shader_t *)TO_EXTERNAL(GET_INTERNAL(args[6]));
		pipeline->fragment_shader = (kinc_g4_shader_t *)TO_EXTERNAL(GET_INTERNAL(args[7]));;
		if (!args[8]->IsNullOrUndefined()) {
			pipeline->geometry_shader = (kinc_g4_shader_t *)TO_EXTERNAL(GET_INTERNAL(args[8]));
		}
		for (int i = 0; i < size; ++i) {
			pipeline->input_layout[i] = structures[i];
		}
		pipeline->input_layout[size] = nullptr;

		Local<Object> state = TO_OBJ(args[9]);
		pipeline->cull_mode = (kinc_g4_cull_mode_t)TO_I32(OBJ_GET(state, "cull_mode"));
		pipeline->depth_write = OBJ_GET(state, "depth_write")->BooleanValue(isolate);
		pipeline->depth_mode = (kinc_g4_compare_mode_t)TO_I32(OBJ_GET(state, "depth_mode"));
		pipeline->blend_source = (kinc_g4_blending_factor_t)TO_I32(OBJ_GET(state, "blend_source"));
		pipeline->blend_destination = (kinc_g4_blending_factor_t)TO_I32(OBJ_GET(state, "blend_dest"));
		pipeline->alpha_blend_source = (kinc_g4_blending_factor_t)TO_I32(OBJ_GET(state, "alpha_blend_dource"));
		pipeline->alpha_blend_destination = (kinc_g4_blending_factor_t)TO_I32(OBJ_GET(state, "alpha_blend_dest"));

		Local<Object> mask_red_array = TO_OBJ(OBJ_GET(state, "color_write_masks_red"));
		Local<Object> mask_green_array = TO_OBJ(OBJ_GET(state, "color_write_masks_green"));
		Local<Object> mask_blue_array = TO_OBJ(OBJ_GET(state, "color_write_masks_blue"));
		Local<Object> mask_alpha_array = TO_OBJ(OBJ_GET(state, "color_write_masks_alpha"));

		for (int i = 0; i < 8; ++i) {
			pipeline->color_write_mask_red[i] = ARRAY_GET(mask_red_array, i)->BooleanValue(isolate);
			pipeline->color_write_mask_green[i] = ARRAY_GET(mask_green_array, i)->BooleanValue(isolate);
			pipeline->color_write_mask_blue[i] = ARRAY_GET(mask_blue_array, i)->BooleanValue(isolate);
			pipeline->color_write_mask_alpha[i] = ARRAY_GET(mask_alpha_array, i)->BooleanValue(isolate);
		}

		pipeline->color_attachment_count = TO_I32(OBJ_GET(state, "color_attachment_count"));
		Local<Object> color_attachment_array = TO_OBJ(OBJ_GET(state, "color_attachments"));
		for (int i = 0; i < 8; ++i) {
			pipeline->color_attachment[i] = (kinc_g4_render_target_format_t)TO_I32(ARRAY_GET(color_attachment_array, i));
		}
		pipeline->depth_attachment_bits = TO_I32(OBJ_GET(state, "depth_attachment_bits"));
		pipeline->stencil_attachment_bits = 0;

		kinc_g4_pipeline_compile(pipeline);
	}

	void krom_g4_set_pipeline(ARGS) {
		SCOPE();
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
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

	void krom_load_image(ARGS) {
		SCOPE();
		String::Utf8Value utf8_value(isolate, args[0]);
		bool readable = TO_I32(args[1]);

		kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));
		kinc_file_reader_t reader;
		if (kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) {
			unsigned char *image_data;
			int image_width;
			int image_height;
			kinc_image_format_t image_format;
			if (!load_image(reader, *utf8_value, image_data, image_width, image_height, image_format)) {
				free(image);
				return;
			}
			kinc_image_init(image, image_data, image_width, image_height, image_format);
		}
		else {
			free(image);
			return;
		}

		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init_from_image(texture, image);
		if (!readable) {
			free(image->data);
			kinc_image_destroy(image);
			free(image);
		}

		RETURN_OBJI(texture, readable ? 2 : 1);
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, texture->tex_height));
	}

	void krom_unload_image(ARGS) {
		SCOPE();
		if (args[0]->IsNullOrUndefined()) return;
		Local<Object> image = TO_OBJ(args[0]);
		Local<Value> tex = OBJ_GET(image, "texture_");
		Local<Value> rt = OBJ_GET(image, "render_target_");

		if (tex->IsObject()) {
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(tex));
			kinc_g4_texture_destroy(texture);
			free(texture);
		}
		else if (rt->IsObject()) {
			kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(rt));
			kinc_g4_render_target_destroy(render_target);
			free(render_target);
		}
	}

	#ifdef WITH_AUDIO
	void krom_load_sound(ARGS) {
		SCOPE();
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_a1_sound_t *sound = kinc_a1_sound_create(*utf8_value);
		RETURN_OBJ(sound);
	}

	void krom_unload_sound(ARGS) {
		SCOPE();
		kinc_a1_sound_t *sound = (kinc_a1_sound_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_a1_sound_destroy(sound);
	}

	void krom_play_sound(ARGS) {
		SCOPE();
		kinc_a1_sound_t *sound = (kinc_a1_sound_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		bool loop = TO_I32(args[1]);
		kinc_a1_play_sound(sound, loop, 1.0, false);
	}

	void krom_stop_sound(ARGS) {
		SCOPE();
		kinc_a1_sound_t *sound = (kinc_a1_sound_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_a1_stop_sound(sound);
	}
	#endif

	void krom_load_blob(ARGS) {
		SCOPE();
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

	void krom_load_url(ARGS) {
		SCOPE();
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_load_url(*utf8_value);
	}

	void krom_copy_to_clipboard(const FunctionCallbackInfo<Value>& args) {
		SCOPE();
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_copy_to_clipboard(*utf8_value);
	}

	void krom_g4_get_constant_location(ARGS) {
		SCOPE();
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, *utf8_value);
		kinc_g4_constant_location_t *location_copy = (kinc_g4_constant_location_t *)malloc(sizeof(kinc_g4_constant_location_t));
		memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
		RETURN_OBJ(location_copy);
	}

	void krom_g4_get_texture_unit(ARGS) {
		SCOPE();
		kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, *utf8_value);
		kinc_g4_texture_unit_t *unit_copy = (kinc_g4_texture_unit_t *)malloc(sizeof(kinc_g4_texture_unit_t));
		memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
		RETURN_OBJ(unit_copy)
	}

	void krom_g4_set_texture(ARGS) {
		SCOPE();
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[1]));
		kinc_g4_set_texture(*unit, texture);
	}

	void krom_g4_set_render_target(ARGS) {
		SCOPE();
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[1]));
		kinc_g4_render_target_use_color_as_texture(render_target, *unit);
	}

	void krom_g4_set_texture_depth(ARGS) {
		SCOPE();
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[1]));
		kinc_g4_render_target_use_depth_as_texture(render_target, *unit);
	}

	void krom_g4_set_image_texture(ARGS) {
		SCOPE();
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[1]));
		kinc_g4_set_image_texture(*unit, texture);
	}

	void krom_g4_set_texture_parameters(ARGS) {
		SCOPE();
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)TO_I32(args[1]));
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)TO_I32(args[2]));
		kinc_g4_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)TO_I32(args[3]));
		kinc_g4_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)TO_I32(args[4]));
		kinc_g4_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)TO_I32(args[5]));
	}

	void krom_g4_set_texture3d_parameters(ARGS) {
		SCOPE();
		kinc_g4_texture_unit_t *unit = (kinc_g4_texture_unit_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)TO_I32(args[1]));
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)TO_I32(args[2]));
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)TO_I32(args[3]));
		kinc_g4_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)TO_I32(args[4]));
		kinc_g4_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)TO_I32(args[5]));
		kinc_g4_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)TO_I32(args[6]));
	}

	void krom_g4_set_bool(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int32_t value = TO_I32(args[1]);
		kinc_g4_set_bool(*location, value != 0);
	}

	void krom_g4_set_int(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int32_t value = TO_I32(args[1]);
		kinc_g4_set_int(*location, value);
	}

	void krom_g4_set_float(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		float value = TO_F32(args[1]);
		kinc_g4_set_float(*location, value);
	}

	void krom_g4_set_float2(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		float value1 = TO_F32(args[1]);
		float value2 = TO_F32(args[2]);
		kinc_g4_set_float2(*location, value1, value2);
	}

	void krom_g4_set_float3(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		float value1 = TO_F32(args[1]);
		float value2 = TO_F32(args[2]);
		float value3 = TO_F32(args[3]);
		kinc_g4_set_float3(*location, value1, value2, value3);
	}

	void krom_g4_set_float4(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		float value1 = TO_F32(args[1]);
		float value2 = TO_F32(args[2]);
		float value3 = TO_F32(args[3]);
		float value4 = TO_F32(args[4]);
		kinc_g4_set_float4(*location, value1, value2, value3, value4);
	}

	void krom_g4_set_floats(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		MAKE_CONTENT(args[1]);
		kinc_g4_set_floats(*location, (float *)content->Data(), int(content->ByteLength() / 4));
	}

	void krom_g4_set_matrix4(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		MAKE_CONTENT(args[1]);
		kinc_g4_set_matrix4(*location, (kinc_matrix4x4_t *)content->Data());
	}

	void krom_g4_set_matrix3(ARGS) {
		SCOPE();
		kinc_g4_constant_location_t *location = (kinc_g4_constant_location_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		MAKE_CONTENT(args[1]);
		kinc_g4_set_matrix3(*location, (kinc_matrix3x3_t *)content->Data());
	}

	double krom_get_time_fast(Local<Object> receiver) {
		return kinc_time();
	}

	void krom_get_time(ARGS) {
		SCOPE();
		RETURN_F64(krom_get_time_fast(args.This()));
	}

	int krom_window_width_fast(Local<Object> receiver) {
		return kinc_window_width(0);
	}

	void krom_window_width(ARGS) {
		SCOPE();
		RETURN_I32(krom_window_width_fast(args.This()));
	}

	int krom_window_height_fast(Local<Object> receiver) {
		return kinc_window_height(0);
	}

	void krom_window_height(ARGS) {
		SCOPE();
		RETURN_I32(krom_window_height_fast(args.This()));
	}

	void krom_set_window_title(ARGS) {
		SCOPE();
		String::Utf8Value title(isolate, args[0]);
		kinc_window_set_title(0, *title);
		#if defined(KORE_IOS) || defined(KORE_ANDROID)
		strcpy(mobile_title, *title);
		#endif
	}

	void krom_get_window_mode(ARGS) {
		SCOPE();
		RETURN_I32(kinc_window_get_mode(0));
	}

	void krom_set_window_mode(ARGS) {
		SCOPE();
		kinc_window_mode_t windowMode = (kinc_window_mode_t)TO_I32(args[0]);
		kinc_window_change_mode(0, windowMode);
	}

	void krom_resize_window(ARGS) {
		SCOPE();
		int width = TO_I32(args[0]);
		int height = TO_I32(args[1]);
		kinc_window_resize(0, width, height);
	}

	void krom_move_window(ARGS) {
		SCOPE();
		int x = TO_I32(args[0]);
		int y = TO_I32(args[1]);
		kinc_window_move(0, x, y);
	}

	void krom_screen_dpi(ARGS) {
		SCOPE();
		int ppi = kinc_display_current_mode(kinc_primary_display()).pixels_per_inch;
		RETURN_I32(ppi);
	}

	void krom_system_id(ARGS) {
		SCOPE();
		RETURN_STR(kinc_system_id());
	}

	void krom_request_shutdown(ARGS) {
		SCOPE();
		kinc_stop();
	}

	void krom_display_count(ARGS) {
		SCOPE();
		RETURN_I32(kinc_count_displays());
	}

	void krom_display_width(ARGS) {
		SCOPE();
		int index = TO_I32(args[0]);
		RETURN_I32(kinc_display_current_mode(index).width);
	}

	void krom_display_height(ARGS) {
		SCOPE();
		int index = TO_I32(args[0]);
		RETURN_I32(kinc_display_current_mode(index).height);
	}

	void krom_display_x(ARGS) {
		SCOPE();
		int index = TO_I32(args[0]);
		RETURN_I32(kinc_display_current_mode(index).x);
	}

	void krom_display_y(ARGS) {
		SCOPE();
		int index = TO_I32(args[0]);
		RETURN_I32(kinc_display_current_mode(index).y);
	}

	void krom_display_frequency(ARGS) {
		SCOPE();
		int index = TO_I32(args[0]);
		RETURN_I32(kinc_display_current_mode(index).frequency);
	}

	void krom_display_is_primary(ARGS) {
		SCOPE();
		int index = TO_I32(args[0]);
		#ifdef KORE_LINUX // TODO: Primary display detection broken in Kinc
		RETURN_I32(true);
		#else
		RETURN_I32(index == kinc_primary_display());
		#endif
	}

	void krom_write_storage(ARGS) {
		SCOPE();
		String::Utf8Value utf8_name(isolate, args[0]);
		MAKE_CONTENT(args[1]);

		kinc_file_writer_t writer;
		kinc_file_writer_open(&writer, *utf8_name);
		kinc_file_writer_write(&writer, content->Data(), (int)content->ByteLength());
		kinc_file_writer_close(&writer);
	}

	void krom_read_storage(ARGS) {
		SCOPE();
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

	void krom_g4_create_render_target(ARGS) {
		SCOPE();
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
		kinc_g4_render_target_init(render_target, TO_I32(args[0]), TO_I32(args[1]), (kinc_g4_render_target_format_t)TO_I32(args[2]), TO_I32(args[3]), TO_I32(args[4]));

		RETURN_OBJ(render_target);
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, render_target->width));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, render_target->height));
	}

	void krom_g4_create_texture(ARGS) {
		SCOPE();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init(texture, TO_I32(args[0]), TO_I32(args[1]), (kinc_image_format_t)TO_I32(args[2]));

		RETURN_OBJ(texture);
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, texture->tex_height));
	}

	void krom_g4_create_texture3d(ARGS) {
		SCOPE();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init3d(texture, TO_I32(args[0]), TO_I32(args[1]), TO_I32(args[2]), (kinc_image_format_t)TO_I32(args[3]));

		RETURN_OBJ(texture);
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, texture->tex_height));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("depth"), Int32::New(isolate, texture->tex_depth));
	}

	void krom_g4_create_texture_from_bytes(ARGS) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));

		bool readable = TO_I32(args[4]);
		void *image_data;
		if (readable) {
			image_data = malloc(content->ByteLength());
			memcpy(image_data, content->Data(), content->ByteLength());
		}
		else {
			image_data = content->Data();
		}

		kinc_image_init(image, image_data, TO_I32(args[1]), TO_I32(args[2]), (kinc_image_format_t)TO_I32(args[3]));
		kinc_g4_texture_init_from_image(texture, image);

		if (!readable) {
			kinc_image_destroy(image);
			free(image);
		}

		RETURN_OBJI(texture, readable ? 2 : 1);
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, texture->tex_height));
	}

	void krom_g4_create_texture_from_bytes3d(ARGS) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t *image = (kinc_image_t*)malloc(sizeof(kinc_image_t));

		bool readable = TO_I32(args[5]);
		void *image_data;
		if (readable) {
			image_data = malloc(content->ByteLength());
			memcpy(image_data, content->Data(), content->ByteLength());
		}
		else {
			image_data = content->Data();
		}

		kinc_image_init3d(image, image_data, TO_I32(args[1]), TO_I32(args[2]), TO_I32(args[3]), (kinc_image_format_t)TO_I32(args[4]));
		kinc_g4_texture_init_from_image3d(texture, image);

		if (!readable) {
			kinc_image_destroy(image);
			free(image);
		}

		RETURN_OBJI(texture, readable ? 2 : 1);
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, texture->tex_height));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("depth"), Int32::New(isolate, texture->tex_depth));
	}

	void krom_g4_create_texture_from_encoded_bytes(ARGS) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		String::Utf8Value format(isolate, args[1]);
		bool readable = TO_I32(args[2]);

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

		RETURN_OBJI(texture, readable ? 2 : 1);
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, texture->tex_width));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, texture->tex_height));
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

	void krom_g4_get_texture_pixels(ARGS) {
		SCOPE();
		kinc_image_t *image = (kinc_image_t *)TO_EXTERNAL(GET_INTERNALI(args[0], 1));
		uint8_t *data = kinc_image_get_pixels(image);
		int byteLength = format_byte_size(image->format) * image->width * image->height * image->depth;
		RETURN_BUFFER(data, byteLength);
	}

	void krom_g4_get_render_target_pixels(ARGS) {
		SCOPE();
		kinc_g4_render_target_t *rt = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));

		MAKE_CONTENT(args[1]);
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

	void krom_g4_lock_texture(ARGS) {
		SCOPE();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		uint8_t *tex = kinc_g4_texture_lock(texture);
		int stride = kinc_g4_texture_stride(texture);
		int byteLength = stride * texture->tex_height * texture->tex_depth;
		RETURN_BUFFER(tex, byteLength);
	}

	void krom_g4_unlock_texture(ARGS) {
		SCOPE();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_texture_unlock(texture);
	}

	void krom_g4_clear_texture(ARGS) {
		SCOPE();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int x = TO_I32(args[1]);
		int y = TO_I32(args[2]);
		int z = TO_I32(args[3]);
		int width = TO_I32(args[4]);
		int height = TO_I32(args[5]);
		int depth = TO_I32(args[6]);
		int color = TO_I32(args[7]);
		kinc_g4_texture_clear(texture, x, y, z, width, height, depth, color);
	}

	void krom_g4_generate_texture_mipmaps(ARGS) {
		SCOPE();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_texture_generate_mipmaps(texture, TO_I32(args[1]));
	}

	void krom_g4_generate_render_target_mipmaps(ARGS) {
		SCOPE();
		kinc_g4_render_target_t *rt = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_render_target_generate_mipmaps(rt, TO_I32(args[1]));
	}

	void krom_g4_set_mipmaps(ARGS) {
		SCOPE();
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));

		Local<Object> js_array = TO_OBJ(args[1]);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> mipmapobj = TO_OBJ(OBJ_GET(TO_OBJ(ARRAY_GET(js_array, i)), "texture_"));
			Local<External> mipmapfield = Local<External>::Cast(mipmapobj->GetInternalField(1));
			kinc_image_t *mipmap = (kinc_image_t *)mipmapfield->Value();
			kinc_g4_texture_set_mipmap(texture, mipmap, i + 1);
		}
	}

	void krom_g4_set_depth_from(ARGS) {
		SCOPE();
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		kinc_g4_render_target_t *source_target = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[1]));
		kinc_g4_render_target_set_depth_stencil_from(render_target, source_target);
	}

	void krom_g4_viewport_fast(Local<Object> receiver, int x, int y, int w, int h) {
		kinc_g4_viewport(x, y, w, h);
	}

	void krom_g4_viewport(ARGS) {
		SCOPE();
		int x = TO_I32(args[0]);
		int y = TO_I32(args[1]);
		int w = TO_I32(args[2]);
		int h = TO_I32(args[3]);
		krom_g4_viewport_fast(args.This(), x, y, w, h);
	}

	void krom_g4_scissor_fast(Local<Object> receiver, int x, int y, int w, int h) {
		kinc_g4_scissor(x, y, w, h);
	}

	void krom_g4_scissor(ARGS) {
		SCOPE();
		int x = TO_I32(args[0]);
		int y = TO_I32(args[1]);
		int w = TO_I32(args[2]);
		int h = TO_I32(args[3]);
		krom_g4_scissor_fast(args.This(), x, y, w, h);
	}

	void krom_g4_disable_scissor_fast(Local<Object> receiver) {
		kinc_g4_disable_scissor();
	}

	void krom_g4_disable_scissor(ARGS) {
		krom_g4_disable_scissor_fast(args.This());
	}

	int krom_g4_render_targets_inverted_y_fast(Local<Object> receiver) {
		return kinc_g4_render_targets_inverted_y();
	}

	void krom_g4_render_targets_inverted_y(ARGS) {
		SCOPE();
		RETURN_I32(krom_g4_render_targets_inverted_y_fast(args.This()));
	}

	void krom_g4_begin(ARGS) {
		SCOPE();
		if (args[0]->IsNullOrUndefined()) {
			kinc_g4_restore_render_target();
		}
		else {
			Local<Object> obj = TO_OBJ(OBJ_GET(TO_OBJ(args[0]), "render_target_"));
			Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
			kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();

			int32_t length = 1;
			kinc_g4_render_target_t *render_targets[8] = { render_target, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
			if (!args[1]->IsNullOrUndefined()) {
				Local<Object> js_array = TO_OBJ(args[1]);
				length = TO_I32(OBJ_GET(js_array, "length")) + 1;
				if (length > 8) length = 8;
				for (int32_t i = 1; i < length; ++i) {
					Local<Object> artobj = TO_OBJ(OBJ_GET(TO_OBJ(ARRAY_GET(js_array, i - 1)), "render_target_"));
					Local<External> artfield = Local<External>::Cast(artobj->GetInternalField(0));
					kinc_g4_render_target_t *art = (kinc_g4_render_target_t *)artfield->Value();
					render_targets[i] = art;
				}
			}
			kinc_g4_set_render_targets(render_targets, length);
		}
	}

	void krom_g4_end(ARGS) {
		SCOPE();
	}

	void krom_file_save_bytes(ARGS) {
		SCOPE();
		String::Utf8Value utf8_path(isolate, args[0]);

		MAKE_CONTENT(args[1]);
		bool hasLengthArg = args.Length() > 2 && !args[2]->IsNullOrUndefined();
		int byteLength = hasLengthArg ? TO_I32(args[2]) : (int)content->ByteLength();
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

	void krom_sys_command(ARGS) {
		SCOPE();
		String::Utf8Value utf8_cmd(isolate, args[0]);
		int result = sys_command(*utf8_cmd);
		RETURN_I32(result);
	}

	void krom_save_path(ARGS) {
		SCOPE();
		RETURN_STR(kinc_internal_save_path());
	}

	void krom_get_arg_count(ARGS) {
		SCOPE();
		RETURN_I32(_argc);
	}

	void krom_get_arg(ARGS) {
		SCOPE();
		int index = TO_I32(args[0]);
		RETURN_STR(_argv[index]);
	}

	void krom_get_files_location(ARGS) {
		SCOPE();
		#ifdef KORE_MACOS
		char path[1024];
		strcpy(path, macgetresourcepath());
		strcat(path, "/");
		strcat(path, KORE_DEBUGDIR);
		strcat(path, "/");
		RETURN_STR(path);
		#elif defined(KORE_IOS)
		char path[1024];
		strcpy(path, iphonegetresourcepath());
		strcat(path, "/");
		strcat(path, KORE_DEBUGDIR);
		strcat(path, "/");
		RETURN_STR(path);
		#else
		RETURN_STR(kinc_internal_get_files_location());
		#endif
	}

	void krom_http_callback(int error, int response, const char *body, void *callbackdata) {
		LOCKER();
		Local<Value> argv[1];
		KromCallbackdata *cbd = (KromCallbackdata *)callbackdata;
		if (body != NULL) {
			std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore(
				(void *)body, cbd->size > 0 ? cbd->size : strlen(body), [](void *, size_t, void *) {}, nullptr);
			argv[0] = ArrayBuffer::New(isolate, std::move(backing));
		}
		CALL_FUNCI(cbd->func, body != NULL ? 1 : 0, argv);
		delete cbd;
	}

	void krom_http_request(ARGS) {
		SCOPE();
		String::Utf8Value url(isolate, args[0]);

		KromCallbackdata *cbd = new KromCallbackdata();
		cbd->size = TO_I32(args[1]);
		SET_FUNC(cbd->func, args[2]);

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

	#ifdef WITH_G2
	void krom_g2_init(ARGS) {
		SCOPE();
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

	void krom_g2_begin(ARGS) {
		SCOPE();
		g2_begin();
	}

	void krom_g2_end(ARGS) {
		SCOPE();
		g2_end();
	}

	void krom_g2_draw_scaled_sub_image(ARGS) {
		#ifdef KORE_DIRECT3D12
		waitAfterNextDraw = true;
		#endif
		SCOPE();
		Local<Object> image = TO_OBJ(args[0]);
		float sx = TO_F32(args[1]);
		float sy = TO_F32(args[2]);
		float sw = TO_F32(args[3]);
		float sh = TO_F32(args[4]);
		float dx = TO_F32(args[5]);
		float dy = TO_F32(args[6]);
		float dw = TO_F32(args[7]);
		float dh = TO_F32(args[8]);
		Local<Value> tex = OBJ_GET(image, "texture_");
		if (tex->IsObject()) {
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(tex));
			g2_draw_scaled_sub_image(texture, sx, sy, sw, sh, dx, dy, dw, dh);
		}
		else {
			Local<Value> rt = OBJ_GET(image, "render_target_");
			kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(rt));
			g2_draw_scaled_sub_render_target(render_target, sx, sy, sw, sh, dx, dy, dw, dh);
		}
	}

	void krom_g2_fill_triangle(ARGS) {
		SCOPE();
		float x0 = TO_F32(args[0]);
		float y0 = TO_F32(args[1]);
		float x1 = TO_F32(args[2]);
		float y1 = TO_F32(args[3]);
		float x2 = TO_F32(args[4]);
		float y2 = TO_F32(args[5]);
		g2_fill_triangle(x0, y0, x1, y1, x2, y2);
	}

	void krom_g2_fill_rect(ARGS) {
		SCOPE();
		float x = TO_F32(args[0]);
		float y = TO_F32(args[1]);
		float width = TO_F32(args[2]);
		float height = TO_F32(args[3]);
		g2_fill_rect(x, y, width, height);
	}

	void krom_g2_draw_rect(ARGS) {
		SCOPE();
		float x = TO_F32(args[0]);
		float y = TO_F32(args[1]);
		float width = TO_F32(args[2]);
		float height = TO_F32(args[3]);
		float strength = TO_F32(args[4]);
		g2_draw_rect(x, y, width, height, strength);
	}

	void krom_g2_draw_line(ARGS) {
		SCOPE();
		float x0 = TO_F32(args[0]);
		float y0 = TO_F32(args[1]);
		float x1 = TO_F32(args[2]);
		float y1 = TO_F32(args[3]);
		float strength = TO_F32(args[4]);
		g2_draw_line(x0, y0, x1, y1, strength);
	}

	void krom_g2_draw_string(ARGS) {
		SCOPE();
		String::Utf8Value text(isolate, args[0]);
		float x = TO_F32(args[1]);
		float y = TO_F32(args[2]);
		g2_draw_string(*text, x, y);
	}

	void krom_g2_set_font(ARGS) {
		SCOPE();
		g2_font_t *font = (g2_font_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int size = TO_I32(args[1]);
		g2_set_font(font, size);
	}

	void krom_g2_font_init(ARGS) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		g2_font_t *font = (g2_font_t *)malloc(sizeof(g2_font_t));
		g2_font_init(font, content->Data(), TO_I32(args[1]));
		MAKE_OBJ(font);
		args.GetReturnValue().Set(obj);
	}

	void krom_g2_font_13(ARGS) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		g2_font_t *font = (g2_font_t *)malloc(sizeof(g2_font_t));
		g2_font_13(font, content->Data());
		MAKE_OBJ(font);
		args.GetReturnValue().Set(obj);
	}

	void krom_g2_font_set_glyphs(ARGS) {
		SCOPE();
		Local<Object> js_array = TO_OBJ(args[0]);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		int *ar = (int *)malloc(sizeof(int) * length);
		for (int i = 0; i < length; ++i) {
			int32_t j = TO_I32(ARRAY_GET(js_array, i));
			ar[i] = j;
		}
		g2_font_set_glyphs(ar, length);
		free(ar);
	}

	void krom_g2_font_count(ARGS) {
		SCOPE();
		g2_font_t *font = (g2_font_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int i = g2_font_count(font);
		RETURN_I32(i);
	}

	void krom_g2_font_height(ARGS) {
		SCOPE();
		g2_font_t *font = (g2_font_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int size = TO_I32(args[1]);
		int i = (int)g2_font_height(font, size);
		RETURN_I32(i);
	}

	void krom_g2_string_width(ARGS) {
		SCOPE();
		g2_font_t *font = (g2_font_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int size = TO_I32(args[1]);
		String::Utf8Value text(isolate, args[2]);
		int i = (int)g2_string_width(font, size, *text);
		RETURN_I32(i);
	}

	void krom_g2_set_bilinear_filter(ARGS) {
		SCOPE();
		g2_set_bilinear_filter(TO_I32(args[0]));
	}

	void krom_g2_restore_render_target(ARGS) {
		SCOPE();
		g2_restore_render_target();
	}

	void krom_g2_set_render_target(ARGS) {
		SCOPE();
		kinc_g4_render_target_t *rt = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		g2_set_render_target(rt);
	}

	void krom_g2_set_color(ARGS) {
		SCOPE();
		int color = TO_I32(args[0]);
		g2_set_color(color);
	}

	void krom_g2_set_pipeline(ARGS) {
		SCOPE();
		kinc_g4_pipeline_t *pipeline = NULL;
		if (!args[0]->IsNullOrUndefined()) {
			pipeline = (kinc_g4_pipeline_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		}
		g2_set_pipeline(pipeline);
	}

	void krom_g2_set_transform(ARGS) {
		SCOPE();
		if (args[0]->IsNullOrUndefined()) {
			g2_set_transform(NULL);
		}
		else {
			MAKE_CONTENT(args[0]);
			g2_set_transform((kinc_matrix3x3_t *)content->Data());
		}
	}

	void krom_g2_fill_circle(ARGS) {
		SCOPE();
		float cx = TO_F32(args[0]);
		float cy = TO_F32(args[1]);
		float radius = TO_F32(args[2]);
		int segments = TO_I32(args[3]);
		g2_fill_circle(cx, cy, radius, segments);
	}

	void krom_g2_draw_circle(ARGS) {
		SCOPE();
		float cx = TO_F32(args[0]);
		float cy = TO_F32(args[1]);
		float radius = TO_F32(args[2]);
		int segments = TO_I32(args[3]);
		float strength = TO_F32(args[4]);
		g2_draw_circle(cx, cy, radius, segments, strength);
	}

	void krom_g2_draw_cubic_bezier(ARGS) {
		SCOPE();

		Local<Object> js_array_x = TO_OBJ(args[0]);
		int32_t length = TO_I32(OBJ_GET(js_array_x, "length"));
		float *x = (float *)malloc(sizeof(float) * length);
		for (int i = 0; i < length; ++i) {
			float j = TO_F32(ARRAY_GET(js_array_x, i));
			x[i] = j;
		}

		Local<Object> js_array_y = TO_OBJ(args[1]);
		length = TO_I32(OBJ_GET(js_array_y, "length"));
		float *y = (float *)malloc(sizeof(float) * length);
		for (int i = 0; i < length; ++i) {
			float j = TO_F32(ARRAY_GET(js_array_y, i));
			y[i] = j;
		}

		int segments = TO_I32(args[2]);
		float strength = TO_F32(args[3]);
		g2_draw_cubic_bezier(x, y, segments, strength);
		free(x);
		free(y);
	}
	#endif

	bool window_close_callback(void *data) {
		#ifdef KORE_WINDOWS
		LOCKER();
		bool save = false;

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
			Local<Value> argv[1] = { Int32::New(isolate, save) };
			CALL_FUNCI(save_and_quit_func, 1, argv);
			return false;
		}
		#endif

		return true;
	}

	void krom_set_save_and_quit_callback(ARGS) {
		SCOPE();
		SET_FUNC(save_and_quit_func, args[0]);
		save_and_quit_func_set = true;
		kinc_window_set_close_callback(0, window_close_callback, NULL);
	}

	void krom_set_mouse_cursor(ARGS) {
		SCOPE();
		int id = TO_I32(args[0]);
		kinc_mouse_set_cursor(id);
		#ifdef KORE_WINDOWS
		// Set hand icon for drag even when mouse button is pressed
		if (id == 1) SetCursor(LoadCursor(NULL, IDC_HAND));
		#endif
	}

	void krom_delay_idle_sleep_fast(Local<Object> receiver) {
		paused_frames = 0;
	}

	void krom_delay_idle_sleep(ARGS) {
		krom_delay_idle_sleep_fast(args.This());
	}

	#ifdef WITH_NFD
	void krom_open_dialog(ARGS) {
		SCOPE();
		String::Utf8Value filterList(isolate, args[0]);
		String::Utf8Value defaultPath(isolate, args[1]);
		bool openMultiple = TO_I32(args[2]);

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

	void krom_save_dialog(ARGS) {
		SCOPE();
		String::Utf8Value filterList(isolate, args[0]);
		String::Utf8Value defaultPath(isolate, args[1]);
		nfdchar_t *outPath = NULL;
		nfdresult_t result = NFD_SaveDialog(*filterList, *defaultPath, &outPath);
		if (result == NFD_OKAY) {
			RETURN_STR(outPath);
			free(outPath);
		}
		else if (result == NFD_CANCEL) {}
		else {
			kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
		}
	}
	#elif defined(KORE_ANDROID)
	void krom_open_dialog(ARGS) {
		SCOPE();
		AndroidFileDialogOpen();
	}

	void krom_save_dialog(ARGS) {
		SCOPE();
		wchar_t *outPath = AndroidFileDialogSave();
		size_t len = wcslen(outPath);
		uint16_t *str = new uint16_t[len + 1];
		for (int i = 0; i < len; i++) str[i] = outPath[i];
		str[len] = 0;
		args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t *)str).ToLocalChecked());
		delete[] str;
	}
	#elif defined(KORE_IOS)
	void krom_open_dialog(ARGS) {
		SCOPE();
		// Once finished drop_files callback is called
		IOSFileDialogOpen();
	}

	void krom_save_dialog(ARGS) {
		SCOPE();
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
	void krom_read_directory(ARGS) {
		SCOPE();
		String::Utf8Value path(isolate, args[0]);
		bool foldersOnly = TO_I32(args[1]);

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
		RETURN_STR(files.c_str());
		#endif
	}
	#endif

	void krom_file_exists(ARGS) {
		SCOPE();
		bool exists = false;
		String::Utf8Value utf8_value(isolate, args[0]);
		kinc_file_reader_t reader;
		if (kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) {
			exists = true;
			kinc_file_reader_close(&reader);
		}
		RETURN_I32(exists);
	}

	void krom_delete_file(ARGS) {
		SCOPE();
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
	void krom_inflate(ARGS) {
		SCOPE();

		MAKE_CONTENT(args[0]);
		bool raw = TO_I32(args[1]);
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

	void krom_deflate(ARGS) {
		SCOPE();

		MAKE_CONTENT(args[0]);
		bool raw = TO_I32(args[1]);
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
		SCOPE();
		String::Utf8Value utf8_path(isolate, args[0]);
		MAKE_CONTENT(args[1]);
		int w = TO_I32(args[2]);
		int h = TO_I32(args[3]);
		int format = TO_I32(args[4]);

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

	void krom_write_jpg(ARGS) {
		SCOPE();
		int quality = TO_I32(args[5]);
		write_image(args, 0, quality);
	}

	void krom_write_png(ARGS) {
		SCOPE();
		write_image(args, 1, 100);
	}

	unsigned char *encode_data;
	int encode_size;
	void encode_image_func(void *context, void *data, int size) {
		memcpy(encode_data + encode_size, data, size);
		encode_size += size;
	}

	void encode_image(const FunctionCallbackInfo<Value> &args, int imageFormat, int quality) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		int w = TO_I32(args[1]);
		int h = TO_I32(args[2]);
		int format = TO_I32(args[3]);
		encode_data = (unsigned char *)malloc(w * h * 4);
		encode_size = 0;
		imageFormat == 0 ?
			stbi_write_jpg_to_func(&encode_image_func, NULL, w, h, 4, content->Data(), quality) :
			stbi_write_png_to_func(&encode_image_func, NULL, w, h, 4, content->Data(), w * 4);
		RETURN_BUFFER(encode_data, encode_size);
	}

	void krom_encode_jpg(ARGS) {
		SCOPE();
		int quality = TO_I32(args[4]);
		encode_image(args, 0, quality);
	}

	void krom_encode_png(ARGS) {
		SCOPE();
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
	void krom_write_mpeg(ARGS) {
		SCOPE();
	}
	#endif

	#ifdef WITH_ONNX
	void krom_ml_inference(ARGS) {
		SCOPE();
		OrtStatus *onnx_status = NULL;

		static bool use_gpu_last = false;
		bool use_gpu = !(args.Length() > 4 && !TO_I32(args[4]));
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

		MAKE_CONTENT(args[0]);
		static void *content_last = 0;
		if (content_last != content->Data() || session == NULL) {
			if (session != NULL) {
				ort->ReleaseSession(session);
				session = NULL;
			}
			onnx_status = ort->CreateSessionFromArray(ort_env, content->Data(), (int)content->ByteLength(), ort_session_options, &session);
			if (onnx_status != NULL) {
				const char* msg = ort->GetErrorMessage(onnx_status);
				kinc_log(KINC_LOG_LEVEL_ERROR, "%s", msg);
				ort->ReleaseStatus(onnx_status);
			}
		}
		content_last = content->Data();

		OrtAllocator *allocator;
		ort->GetAllocatorWithDefaultOptions(&allocator);
		OrtMemoryInfo *memory_info;
		ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);

		Local<Object> js_array = TO_OBJ(args[1]);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		if (length > 4) length = 4;
		char *input_node_names[4];
		OrtValue *input_tensors[4];
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> tensorobj = TO_OBJ(ARRAY_GET(js_array, i));
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
				Local<Object> js_array = TO_OBJ(args[2]);
				Local<Object> js_array2 = TO_OBJ(ARRAY_GET(js_array, i));
				for (int32_t i = 0; i < num_input_dims; ++i) {
					int32_t j = TO_I32(ARRAY_GET(js_array2, i));
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
			Local<Object> js_array = TO_OBJ(args[3]);
			int32_t length = TO_I32(OBJ_GET(js_array, "length"));
			for (int i = 0; i < length; ++i) {
				int32_t j = TO_I32(ARRAY_GET(js_array, i));
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

	void krom_ml_unload(ARGS) {
		if (session != NULL) {
			ort->ReleaseSession(session);
			session = NULL;
		}
	}
	#endif

	#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
	void krom_raytrace_supported(ARGS) {
		SCOPE();
		#ifdef KORE_METAL
		bool supported = kinc_raytrace_supported();
		#else
		bool supported = true;
		#endif
		RETURN_I32(supported);
	}

	void krom_raytrace_init(ARGS) {
		SCOPE();
		if (accel_created) {
			kinc_g5_constant_buffer_destroy(&constant_buffer);
			kinc_raytrace_acceleration_structure_destroy(&accel);
			kinc_raytrace_pipeline_destroy(&pipeline);
		}

		MAKE_CONTENT(args[0]);
		kinc_g4_vertex_buffer_t *vertex_buffer4 = (kinc_g4_vertex_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[1]));
		kinc_g5_vertex_buffer_t *vertex_buffer = &vertex_buffer4->impl._buffer;
		kinc_g4_index_buffer_t *index_buffer4 = (kinc_g4_index_buffer_t *)TO_EXTERNAL(GET_INTERNAL(args[2]));
		kinc_g5_index_buffer_t *index_buffer = &index_buffer4->impl._buffer;

		float scale = TO_F32(args[3]);
		kinc_g5_constant_buffer_init(&constant_buffer, constant_buffer_size * 4);
		kinc_raytrace_pipeline_init(&pipeline, &commandList, content->Data(), (int)content->ByteLength(), &constant_buffer);
		kinc_raytrace_acceleration_structure_init(&accel, &commandList, vertex_buffer, index_buffer, scale);
		accel_created = true;
	}

	void krom_raytrace_set_textures(ARGS) {
		SCOPE();

		kinc_g4_render_target_t *texpaint0;
		kinc_g4_render_target_t *texpaint1;
		kinc_g4_render_target_t *texpaint2;
		Local<Object> texpaint0_image = TO_OBJ(args[0]);
		Local<Value> texpaint0_tex = OBJ_GET(texpaint0_image, "texture_");
		Local<Value> texpaint0_rt = OBJ_GET(texpaint0_image, "render_target_");

		if (texpaint0_tex->IsObject()) {
			#ifdef KORE_DIRECT3D12
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(texpaint0_tex));
			if (!texture->impl._uploaded) {
				kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
				texture->impl._uploaded = true;
			}
			texpaint0 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
			texpaint0->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
			#endif
		}
		else {
			texpaint0 = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(texpaint0_rt));
		}

		Local<Object> texpaint1_image = TO_OBJ(args[1]);
		Local<Value> texpaint1_tex = OBJ_GET(texpaint1_image, "texture_");
		Local<Value> texpaint1_rt = OBJ_GET(texpaint1_image, "render_target_");

		if (texpaint1_tex->IsObject()) {
			#ifdef KORE_DIRECT3D12
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(texpaint1_tex));
			if (!texture->impl._uploaded) {
				kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
				texture->impl._uploaded = true;
			}
			texpaint1 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
			texpaint1->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
			#endif
		}
		else {
			texpaint1 = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(texpaint1_rt));
		}

		Local<Object> texpaint2_image = TO_OBJ(args[2]);
		Local<Value> texpaint2_tex = OBJ_GET(texpaint2_image, "texture_");
		Local<Value> texpaint2_rt = OBJ_GET(texpaint2_image, "render_target_");

		if (texpaint2_tex->IsObject()) {
			#ifdef KORE_DIRECT3D12
			kinc_g4_texture_t *texture = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(texpaint2_tex));
			if (!texture->impl._uploaded) {
				kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
				texture->impl._uploaded = true;
			}
			texpaint2 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
			texpaint2->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
			#endif
		}
		else {
			texpaint2 = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(texpaint2_rt));
		}

		kinc_g4_texture_t *texenv = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[3]));
		kinc_g4_texture_t *texsobol = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[4]));
		kinc_g4_texture_t *texscramble = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[5]));
		kinc_g4_texture_t *texrank = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(args[6]));

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

	void krom_raytrace_dispatch_rays(ARGS) {
		SCOPE();
		render_target = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));

		MAKE_CONTENT(args[1]);
		float *cb = (float *)content->Data();
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

	void krom_window_x(ARGS) {
		SCOPE();
		RETURN_I32(kinc_window_x(0));
	}

	void krom_window_y(ARGS) {
		SCOPE();
		RETURN_I32(kinc_window_y(0));
	}

	void krom_language(ARGS) {
		SCOPE();
		RETURN_STR(kinc_language());
	}

	#ifdef WITH_IRON
	void krom_io_obj_parse(ARGS) {
		SCOPE();
		MAKE_CONTENT(args[0]);
		int split_code = TO_I32(args[1]);
		int start_pos = TO_I32(args[2]);
		int udim = TO_I32(args[3]);

		obj_part_t *part = io_obj_parse((uint8_t *)content->Data(), split_code, start_pos, udim);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();

		std::unique_ptr<v8::BackingStore> backing_posa = v8::ArrayBuffer::NewBackingStore((void *)part->posa, part->vertex_count * 4 * 2, [](void *data, size_t, void *) { free(data); }, nullptr);
		Local<ArrayBuffer> buffer_posa = ArrayBuffer::New(isolate, std::move(backing_posa));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("posa"), Int16Array::New(buffer_posa, 0, buffer_posa->ByteLength() / 2));

		std::unique_ptr<v8::BackingStore> backing_nora = v8::ArrayBuffer::NewBackingStore((void *)part->nora, part->vertex_count * 2 * 2, [](void *data, size_t, void *) { free(data); }, nullptr);
		Local<ArrayBuffer> buffer_nora = ArrayBuffer::New(isolate, std::move(backing_nora));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("nora"), Int16Array::New(buffer_nora, 0, buffer_nora->ByteLength() / 2));

		if (part->texa != NULL) {
			std::unique_ptr<v8::BackingStore> backing_texa = v8::ArrayBuffer::NewBackingStore((void *)part->texa, part->vertex_count * 2 * 2, [](void *data, size_t, void *) { free(data); }, nullptr);
			Local<ArrayBuffer> buffer_texa = ArrayBuffer::New(isolate, std::move(backing_texa));
			(void) obj->Set(isolate->GetCurrentContext(), TO_STR("texa"), Int16Array::New(buffer_texa, 0, buffer_texa->ByteLength() / 2));
		}

		std::unique_ptr<v8::BackingStore> backing_inda = v8::ArrayBuffer::NewBackingStore((void *)part->inda, part->index_count * 4, [](void *data, size_t, void *) { free(data); }, nullptr);
		Local<ArrayBuffer> buffer_inda = ArrayBuffer::New(isolate, std::move(backing_inda));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("inda"), Uint32Array::New(buffer_inda, 0, buffer_inda->ByteLength() / 4));

		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("name"), String::NewFromUtf8(isolate, part->name).ToLocalChecked());
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("scalePos"), Number::New(isolate, part->scale_pos));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("has_next"), Number::New(isolate, part->has_next));
		(void) obj->Set(isolate->GetCurrentContext(), TO_STR("pos"), Number::New(isolate, (int)part->pos));

		if (udim) {
			(void) obj->Set(isolate->GetCurrentContext(), TO_STR("udims_u"), Number::New(isolate, part->udims_u));

			Local<Array> udims = Array::New(isolate, part->udims_u * part->udims_v);
			for (int i = 0; i < part->udims_u * part->udims_v; ++i) {
				std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore(
					(void *)part->udims[i], part->udims_count[i] * 4, [](void *data, size_t, void *) { free(data); }, nullptr);
				Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing));
				(void) udims->Set(isolate->GetCurrentContext(), i, Uint32Array::New(buffer, 0, buffer->ByteLength() / 4));
			}
			(void) obj->Set(isolate->GetCurrentContext(), TO_STR("udims"), udims);
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
		return TO_F32(result);
	}

	void krom_zui_on_border_hover(zui_handle_t *handle, int side) {
		LOCKER();
		Local<Value> argv[2] = {Number::New(isolate, (size_t)handle), Int32::New(isolate, side)};
		CALL_FUNCI(on_border_hover_func, 2, argv);
	}

	void krom_zui_on_text_hover() {
		LOCKER();
		CALL_FUNC(on_text_hover_func);
	}

	void krom_zui_on_deselect_text() {
		LOCKER();
		CALL_FUNC(on_deselect_text_func);
	}

	void krom_zui_on_tab_drop(zui_handle_t *to, int to_pos, zui_handle_t *from, int from_pos) {
		LOCKER();
		Local<Value> argv[4] = {Number::New(isolate, (size_t)to), Int32::New(isolate, to_pos), Number::New(isolate, (size_t)from), Int32::New(isolate, from_pos)};
		CALL_FUNCI(on_tab_drop_func, 4, argv);
	}

	char enum_texts_data[32][64];
	char *enum_texts[32];
	char **krom_zui_nodes_enum_texts(char *type) {
		LOCKER();
		Local<Value> argv[1] = {TO_STR(type)};
		CALL_FUNCI(enum_texts_func, 1, argv);

		Local<Object> js_array = TO_OBJ(result);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		assert(length <= 32);
		for (int i = 0; i < length; ++i) {
			String::Utf8Value str(isolate, ARRAY_GET(js_array, i));
			strcpy(enum_texts_data[i], *str);
			enum_texts[i] = enum_texts_data[i];
		}
		enum_texts[length] = NULL;
		return enum_texts;
	}

	void krom_zui_nodes_on_custom_button(int node_id, char *button_name) {
		LOCKER();
		Local<Value> argv[2] = {Int32::New(isolate, node_id), TO_STR(button_name)};
		CALL_FUNCI(on_custom_button_func, 2, argv);
	}

	zui_canvas_control_t krom_zui_nodes_on_canvas_control() {
		LOCKER();
		CALL_FUNC(on_canvas_control_func);

		Local<Object> jso = TO_OBJ(result);
		zui_canvas_control_t c;
		c.pan_x = TO_F32(OBJ_GET(jso, "pan_x"));
		c.pan_y = TO_F32(OBJ_GET(jso, "pan_y"));
		c.zoom = TO_F32(OBJ_GET(jso, "zoom"));
		return c;
	}

	void krom_zui_nodes_on_canvas_released() {
		LOCKER();
		CALL_FUNC(on_canvas_released_func);
	}

	void krom_zui_nodes_on_socket_released(int socket_id) {
		LOCKER();
		Local<Value> argv[1] = {Int32::New(isolate, socket_id)};
		CALL_FUNCI(on_socket_released_func, 1, argv);
	}

	void krom_zui_nodes_on_link_drag(int link_drag_id, bool is_new_link) {
		LOCKER();
		Local<Value> argv[2] = {Int32::New(isolate, link_drag_id), Int32::New(isolate, is_new_link)};
		CALL_FUNCI(on_link_drag_func, 2, argv)
	}

	void krom_zui_init(ARGS) {
		SCOPE();
		Local<Object> arg0 = TO_OBJ(args[0]);

		zui_options_t ops;
		ops.scale_factor = TO_F32(OBJ_GET(arg0, "scale_factor"));

		Local<Value> theme = OBJ_GET(arg0, "theme");
		ops.theme = (zui_theme_t *)TO_EXTERNAL(GET_INTERNAL(theme));

		Local<Value> font = OBJ_GET(arg0, "font");
		ops.font = (g2_font_t *)TO_EXTERNAL(GET_INTERNAL(font));

		Local<Value> colorwheel = OBJ_GET(arg0, "color_wheel");
		if (!colorwheel->IsNullOrUndefined()) {
			ops.color_wheel = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(colorwheel));
		}

		Local<Value> blackwhitegradient = OBJ_GET(arg0, "black_white_gradient");
		if (!blackwhitegradient->IsNullOrUndefined()) {
			ops.black_white_gradient = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(blackwhitegradient));
		}

		zui_t *ui = (zui_t *)malloc(sizeof(zui_t));
		zui_init(ui, ops);

		RETURN_OBJ(ui);
		zui_on_border_hover = &krom_zui_on_border_hover;
		zui_on_text_hover = &krom_zui_on_text_hover;
		zui_on_deselect_text = &krom_zui_on_deselect_text;
		zui_on_tab_drop = &krom_zui_on_tab_drop;
		zui_nodes_enum_texts = &krom_zui_nodes_enum_texts;
		zui_nodes_on_custom_button = &krom_zui_nodes_on_custom_button;
		zui_nodes_on_canvas_control = &krom_zui_nodes_on_canvas_control;
		zui_nodes_on_canvas_released = &krom_zui_nodes_on_canvas_released;
		zui_nodes_on_socket_released = &krom_zui_nodes_on_socket_released;
		zui_nodes_on_link_drag = &krom_zui_nodes_on_link_drag;
	}

	void krom_zui_get_scale(ARGS) {
		SCOPE();
		zui_t *ui = (zui_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		zui_t *current = zui_get_current();
		zui_set_current(ui);
		RETURN_F64(ZUI_SCALE());
		zui_set_current(current);
	}

	void krom_zui_set_scale(ARGS) {
		SCOPE();
		zui_t *ui = (zui_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		zui_t *current = zui_get_current();
		zui_set_current(ui);
		float factor = TO_F32(args[1]);
		zui_set_scale(factor);
		zui_set_current(current);
	}

	void krom_zui_set_font(ARGS) {
		SCOPE();
		zui_t *ui = (zui_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		g2_font_t *font = (g2_font_t *)TO_EXTERNAL(GET_INTERNAL(args[1]));
		ui->ops.font = font;
	}

	void krom_zui_begin(ARGS) {
		SCOPE();
		zui_t *ui = (zui_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		zui_begin(ui);
	}

	void krom_zui_end(ARGS) {
		SCOPE();
		bool last = TO_I32(args[0]);
		zui_end(last);
	}

	void krom_zui_begin_region(ARGS) {
		SCOPE();
		zui_t *ui = (zui_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int x = (int)TO_I32(args[1]);
		int y = (int)TO_I32(args[2]);
		int w = (int)TO_I32(args[3]);
		zui_begin_region(ui, x, y, w);
	}

	void krom_zui_end_region(ARGS) {
		SCOPE();
		bool last = (bool)TO_I32(args[0]);
		zui_end_region(last);
	}

	void krom_zui_begin_sticky(ARGS) {
		SCOPE();
		zui_begin_sticky();
	}

	void krom_zui_end_sticky(ARGS) {
		SCOPE();
		zui_end_sticky();
	}

	void krom_zui_end_input(ARGS) {
		SCOPE();
		zui_end_input();
	}

	void krom_zui_end_window(ARGS) {
		SCOPE();
		bool bing_global_g = TO_I32(args[0]);
		zui_end_window(bing_global_g);
	}

	void krom_zui_end_element(ARGS) {
		SCOPE();
		float element_size = TO_F32(args[0]);
		if (element_size < 0) zui_end_element();
		else zui_end_element_of_size(element_size);
	}

	void krom_zui_start_text_edit(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int align = TO_I32(args[1]);
		zui_start_text_edit(handle, align);
	}

	void krom_zui_input_in_rect(ARGS) {
		SCOPE();
		float x = TO_F32(args[0]);
		float y = TO_F32(args[1]);
		float w = TO_F32(args[2]);
		float h = TO_F32(args[3]);
		bool b = zui_input_in_rect(x, y, w, h);
		RETURN_I32(b);
	}

	void krom_zui_window(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int x = TO_I32(args[1]);
		int y = TO_I32(args[2]);
		int w = TO_I32(args[3]);
		int h = TO_I32(args[4]);
		int drag = TO_I32(args[5]);
		bool b = zui_window(handle, x, y, w, h, drag);
		RETURN_I32(b);
	}

	void krom_zui_button(ARGS) {
		SCOPE();
		String::Utf8Value text(isolate, args[0]);
		int align = TO_I32(args[1]);
		String::Utf8Value label(isolate, args[2]);
		bool b = zui_button(*text, align, *label);
		RETURN_I32(b);
	}

	void krom_zui_check(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value text(isolate, args[1]);
		String::Utf8Value label(isolate, args[2]);
		bool b = zui_check(handle, *text, *label);
		RETURN_I32(b);
	}

	void krom_zui_radio(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int position = TO_I32(args[1]);
		String::Utf8Value text(isolate, args[2]);
		String::Utf8Value label(isolate, args[3]);
		bool b = zui_radio(handle, position, *text, *label);
		RETURN_I32(b);
	}

	char combo_label[32];
	char combo_texts_data[32][64];
	char *combo_texts[32];
	void krom_zui_combo(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value utf8label(isolate, args[2]);
		int show_label = TO_I32(args[3]);
		int align = TO_I32(args[4]);
		int search_bar = TO_I32(args[5]);
		Local<Object> js_array = TO_OBJ(args[1]);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		assert(length <= 32);

		char temp_label[32];
		char temp_texts_data[32][64];
		char *temp_texts[32];
		bool combo_selected = zui_get_current()->combo_selected_handle != NULL;
		char *label = combo_selected ? temp_label : combo_label;
		char (*texts_data)[64] = combo_selected ? temp_texts_data : combo_texts_data;
		char **texts = combo_selected ? temp_texts : combo_texts;
		for (int i = 0; i < length; ++i) {
			String::Utf8Value str(isolate, ARRAY_GET(js_array, i));
			strcpy(texts_data[i], *str);
			texts[i] = texts_data[i];
		}
		strcpy(label, *utf8label);
		int i = zui_combo(handle, texts, length, label, show_label, align, search_bar);
		RETURN_I32(i);
	}

	void krom_zui_slider(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value text(isolate, args[1]);
		float from = TO_F32(args[2]);
		float to = TO_F32(args[3]);
		bool filled = (bool)TO_I32(args[4]);
		float precision = TO_F32(args[5]);
		bool display_value = (bool)TO_I32(args[6]);
		int align = (int)TO_I32(args[7]);
		bool text_edit = (bool)TO_I32(args[8]);
		float f = zui_slider(handle, *text, from, to, filled, precision, display_value, align, text_edit);
		RETURN_F64(f);
	}

	void krom_zui_image(ARGS) {
		SCOPE();
		Local<Object> image_object = TO_OBJ(args[0]);
		int tint = (int)TO_I32(args[1]);
		int h = (int)TO_I32(args[2]);
		int sx = (int)TO_I32(args[3]);
		int sy = (int)TO_I32(args[4]);
		int sw = (int)TO_I32(args[5]);
		int sh = (int)TO_I32(args[6]);
		void *image;
		bool is_rt;
		Local<Value> tex = OBJ_GET(image_object, "texture_");
		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(GET_INTERNAL(tex));
			image = (void *)texfield->Value();
			is_rt = false;
		}
		else {
			Local<Value> rt = OBJ_GET(image_object, "render_target_");
			Local<External> rtfield = Local<External>::Cast(GET_INTERNAL(rt));
			image = (void *)rtfield->Value();
			is_rt = true;
		}
		int i = zui_sub_image(image, is_rt, tint, h, sx, sy, sw, sh);
		RETURN_I32(i);
	}

	void krom_zui_text(ARGS) {
		SCOPE();
		String::Utf8Value text(isolate, args[0]);
		int align = TO_I32(args[1]);
		int bg = TO_I32(args[2]);
		int i = zui_text(*text, align, bg);
		RETURN_I32(i);
	}

	void krom_zui_text_input(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value label(isolate, args[1]);
		int align = TO_I32(args[2]);
		int editable = TO_I32(args[3]);
		int live_update = TO_I32(args[4]);
		char *str = zui_text_input(handle, *label, align, editable, live_update);
		RETURN_STR(str);
	}

	void krom_zui_tab(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value text(isolate, args[1]);
		int vertical = TO_I32(args[2]);
		int color = TO_I32(args[3]);
		bool b = zui_tab(handle, *text, vertical, color);
		RETURN_I32(b);
	}

	void krom_zui_panel(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value text(isolate, args[1]);
		int is_tree = TO_I32(args[2]);
		int filled = TO_I32(args[3]);
		int pack = TO_I32(args[4]);
		bool b = zui_panel(handle, *text, is_tree, filled, pack);
		RETURN_I32(b);
	}

	void krom_zui_handle(ARGS) {
		SCOPE();
		Local<Object> ops = TO_OBJ(args[0]);
		zui_handle_t *handle = (zui_handle_t *)malloc(sizeof(zui_handle_t));
		memset(handle, 0, sizeof(zui_handle_t));
		handle->redraws = 2;
		handle->selected = (bool)TO_I32(OBJ_GET(ops, "selected"));
		handle->position = TO_I32(OBJ_GET(ops, "position"));
		handle->value = TO_F32(OBJ_GET(ops, "value"));
		Local<Value> str = OBJ_GET(ops, "text");
		String::Utf8Value text(isolate, str);
		assert(strlen(*text) < 128);
		strcpy(handle->text, *text);
		handle->color = TO_I32(OBJ_GET(ops, "color"));
		handle->layout = TO_I32(OBJ_GET(ops, "layout"));
		RETURN_OBJ(handle);
	}

	void krom_zui_separator(ARGS) {
		SCOPE();
		int h = (int)TO_I32(args[0]);
		bool fill = (bool)TO_I32(args[1]);
		zui_separator(h, fill);
	}

	void krom_zui_tooltip(ARGS) {
		SCOPE();
		String::Utf8Value text(isolate, args[0]);
		zui_tooltip(*text);
	}

	void krom_zui_tooltip_image(ARGS) {
		SCOPE();
		int max_width = TO_I32(args[1]);
		Local<Object> image_object = TO_OBJ(args[0]);
		Local<Value> tex = OBJ_GET(image_object, "texture_");
		if (tex->IsObject()) {
			kinc_g4_texture_t *image = (kinc_g4_texture_t *)TO_EXTERNAL(GET_INTERNAL(tex));
			zui_tooltip_image(image, max_width);
		}
		else {
			Local<Value> rt = OBJ_GET(image_object, "render_target_");
			kinc_g4_render_target_t *image = (kinc_g4_render_target_t *)TO_EXTERNAL(GET_INTERNAL(rt));
			zui_tooltip_render_target(image, max_width);
		}
	}

	void krom_zui_row(ARGS) {
		SCOPE();
		Local<Object> js_array = TO_OBJ(args[0]);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		assert(length <= 128);
		float ratios[128];
		for (int i = 0; i < length; ++i) {
			ratios[i] = TO_F32(ARRAY_GET(js_array, i));
		}
		zui_row(ratios, length);
	}

	void krom_zui_fill(ARGS) {
		SCOPE();
		float x = TO_F32(args[0]);
		float y = TO_F32(args[1]);
		float w = TO_F32(args[2]);
		float h = TO_F32(args[3]);
		int color = (int)TO_I32(args[4]);
		zui_fill(x, y, w, h, color);
	}

	void krom_zui_rect(ARGS) {
		SCOPE();
		float x = TO_F32(args[0]);
		float y = TO_F32(args[1]);
		float w = TO_F32(args[2]);
		float h = TO_F32(args[3]);
		int color = (int)TO_I32(args[4]);
		float strength = TO_F32(args[5]);
		zui_rect(x, y, w, h, color, strength);
	}

	void krom_zui_draw_rect(ARGS) {
		SCOPE();
		bool fill = (bool)TO_I32(args[0]);
		float x = TO_F32(args[1]);
		float y = TO_F32(args[2]);
		float w = TO_F32(args[3]);
		float h = TO_F32(args[4]);
		// float strength = TO_F32(args[5]);
		// zui_draw_rect(fill, x, y, w, h, strength);
		zui_draw_rect(fill, x, y, w, h);
	}

	void krom_zui_draw_string(ARGS) {
		SCOPE();
		String::Utf8Value text(isolate, args[0]);
		float x_offset = TO_F32(args[1]);
		float y_offset = TO_F32(args[2]);
		int align = TO_I32(args[3]);
		bool truncation = (bool)TO_I32(args[4]);
		zui_draw_string(*text, x_offset, y_offset, align, truncation);
	}

	void krom_zui_get_hovered_tab_name(ARGS) {
		SCOPE();
		char *str = zui_hovered_tab_name();
		RETURN_STR(str);
	}

	void krom_zui_set_hovered_tab_name(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[0]);
		zui_set_hovered_tab_name(*name);
	}

	void krom_zui_begin_menu(ARGS) {
		SCOPE();
		zui_begin_menu();
	}

	void krom_zui_end_menu(ARGS) {
		SCOPE();
		zui_end_menu();
	}

	void krom_zui_menu_button(ARGS) {
		SCOPE();
		String::Utf8Value text(isolate, args[0]);
		bool b = zui_menu_button(*text);
		RETURN_I32(b);
	}

	void krom_zui_float_input(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		String::Utf8Value label(isolate, args[1]);
		int align = TO_I32(args[2]);
		float precision = TO_F32(args[3]);
		float f = zui_float_input(handle, *label, align, precision);
		RETURN_F64(f);
	}

	char radio_texts[32][64];
	char *radio_temp[32];
	void krom_zui_inline_radio(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int align = TO_I32(args[2]);

		Local<Object> js_array = TO_OBJ(args[1]);
		int32_t length = TO_I32(OBJ_GET(js_array, "length"));
		assert(length <= 32);
		for (int i = 0; i < length; ++i) {
			String::Utf8Value str(isolate, ARRAY_GET(js_array, i));
			strcpy(radio_texts[i], *str);
			radio_temp[i] = radio_texts[i];
		}

		int i = zui_inline_radio(handle, radio_temp, length, align);
		RETURN_I32(i);
	}

	void krom_picker_callback(void *data) {
		LOCKER();
		CALL_FUNC(picker_func);
	}

	void krom_zui_color_wheel(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		bool alpha = TO_I32(args[1]);
		float w = TO_F32(args[2]);
		float h = TO_F32(args[3]);
		bool color_preview = TO_I32(args[4]);
		SET_FUNC(picker_func, args[5]);
		int i = zui_color_wheel(handle, alpha, w, h, color_preview, &krom_picker_callback, NULL);
		RETURN_I32(i);
	}

	void krom_zui_text_area(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		int align = TO_I32(args[1]);
		bool editable = TO_I32(args[2]);
		String::Utf8Value label(isolate, args[3]);
		bool word_wrap = TO_I32(args[4]);
		char *str = zui_text_area(handle, align, editable, *label, word_wrap);
		RETURN_STR(str);
	}

	void krom_zui_text_area_coloring(ARGS) {
		SCOPE();
		if (zui_text_area_coloring != NULL) {
			free(zui_text_area_coloring);
		}
		if (args[0]->IsNullOrUndefined()) {
			zui_text_area_coloring = NULL;
			return;
		}
		MAKE_CONTENT(args[0]);
		zui_text_area_coloring = (zui_text_coloring_t *)armpack_decode(content->Data(), (int)content->ByteLength());
	}

	void krom_zui_nodes_init(ARGS) {
		SCOPE();
		zui_nodes_t *nodes = (zui_nodes_t *)malloc(sizeof(zui_nodes_t));
		zui_nodes_init(nodes);

		RETURN_OBJ(nodes);
		zui_nodes_exclude_remove[0] = (char *)"OUTPUT_MATERIAL_PBR";
		zui_nodes_exclude_remove[1] = (char *)"GROUP_OUTPUT";
		zui_nodes_exclude_remove[2] = (char *)"GROUP_INPUT";
		zui_nodes_exclude_remove[3] = (char *)"BrushOutputNode";
	}

	void *encoded = NULL;
	uint32_t encoded_size = 0;
	void krom_zui_node_canvas(ARGS) {
		SCOPE();
		zui_nodes_t *nodes = (zui_nodes_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		MAKE_CONTENT(args[1]);

		// TODO: decode on change
		zui_node_canvas_t *decoded = (zui_node_canvas_t *)armpack_decode(content->Data(), (int)content->ByteLength());
		zui_node_canvas(nodes, decoded);

		int byteLength = zui_node_canvas_encoded_size(decoded);
		if (byteLength > encoded_size) {
			if (encoded != NULL) free(encoded);
			encoded_size = byteLength;
			encoded = malloc(byteLength);
		}
		zui_node_canvas_encode(encoded, decoded);

		RETURN_BUFFER(encoded, byteLength);
		free(decoded);
	}

	void krom_zui_nodes_rgba_popup(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		MAKE_CONTENT(args[1]);
		float *val = (float *)content->Data();
		int x = TO_I32(args[2]);
		int y = TO_I32(args[3]);
		zui_nodes_rgba_popup(handle, val, x, y);
	}

	void krom_zui_nodes_scale(ARGS) {
		SCOPE();
		float f = ZUI_NODES_SCALE();
		RETURN_F64(f);
	}

	void krom_zui_nodes_pan_x(ARGS) {
		SCOPE();
		float f = ZUI_NODES_PAN_X();
		RETURN_F64(f);
	}

	void krom_zui_nodes_pan_y(ARGS) {
		SCOPE();
		float f = ZUI_NODES_PAN_Y();
		RETURN_F64(f);
	}

	#define ZUI_GET_I32_GLOBAL(prop)\
		if (strcmp(*name, #prop) == 0) {\
			RETURN_I32(prop);\
			return;\
		}

	#define ZUI_GET_I32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			RETURN_I32(obj->prop);\
			return;\
		}

	#define ZUI_GET_PTR(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			RETURN_F64((size_t)obj->prop);\
			return;\
		}

	#define ZUI_GET_F32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			RETURN_F64(obj->prop);\
			return;\
		}

	#define ZUI_SET_I32_GLOBAL(prop)\
		if (strcmp(*name, #prop) == 0) {\
			prop = TO_I32(args[2]);\
			return;\
		}

	#define ZUI_SET_I32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			obj->prop = TO_I32(args[2]);\
			return;\
		}

	#define ZUI_SET_F32(obj, prop)\
		if (strcmp(*name, #prop) == 0) {\
			obj->prop = TO_F32(args[2]);\
			return;\
		}

	void krom_zui_get(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		if (args[0]->IsNullOrUndefined()) {
			ZUI_GET_I32_GLOBAL(zui_always_redraw_window)
			ZUI_GET_I32_GLOBAL(zui_touch_scroll)
			ZUI_GET_I32_GLOBAL(zui_touch_hold)
			ZUI_GET_I32_GLOBAL(zui_touch_tooltip)
			ZUI_GET_I32_GLOBAL(zui_is_paste)
			return;
		}
		Local<External> field = Local<External>::Cast(GET_INTERNAL(args[0]));
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

	void krom_zui_set(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		if (args[0]->IsNullOrUndefined()) {
			ZUI_SET_I32_GLOBAL(zui_always_redraw_window)
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
		Local<External> field = Local<External>::Cast(GET_INTERNAL(args[0]));
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

	void krom_zui_handle_get(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		ZUI_GET_I32(handle, selected)
		ZUI_GET_I32(handle, position)
		ZUI_GET_I32(handle, color)
		ZUI_GET_I32(handle, changed)
		ZUI_GET_I32(handle, drag_x)
		ZUI_GET_I32(handle, drag_y)
		ZUI_GET_F32(handle, value)
		ZUI_GET_F32(handle, scroll_offset)
		if (strcmp(*name, "text") == 0) {
			RETURN_STR(handle->text);
		}
		else if (strcmp(*name, "texture") == 0) {
			MAKE_OBJ(&handle->texture);
			(void) obj->Set(isolate->GetCurrentContext(), TO_STR("width"), Int32::New(isolate, handle->texture.width));
			(void) obj->Set(isolate->GetCurrentContext(), TO_STR("height"), Int32::New(isolate, handle->texture.height));
			args.GetReturnValue().Set(obj);
		}
	}

	void krom_zui_handle_set(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
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

	void krom_zui_handle_ptr(ARGS) {
		SCOPE();
		zui_handle_t *handle = (zui_handle_t *)TO_EXTERNAL(GET_INTERNAL(args[0]));
		RETURN_F64((size_t)handle);
	}

	void krom_zui_theme_init(ARGS) {
		SCOPE();
		zui_theme_t *theme = (zui_theme_t *)malloc(sizeof(zui_theme_t));
		zui_theme_default(theme);
		RETURN_OBJ(theme);
	}

	void krom_zui_theme_get(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		Local<External> field = Local<External>::Cast(GET_INTERNAL(args[0]));
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

	void krom_zui_theme_set(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		Local<External> field = Local<External>::Cast(GET_INTERNAL(args[0]));
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

	void krom_zui_nodes_get(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		if (args[0]->IsNullOrUndefined()) {
			ZUI_GET_I32_GLOBAL(zui_nodes_socket_released)
			return;
		}
		Local<External> field = Local<External>::Cast(GET_INTERNAL(args[0]));
		zui_nodes_t *nodes = (zui_nodes_t *)field->Value();
		ZUI_GET_F32(nodes, pan_x)
		ZUI_GET_F32(nodes, pan_y)
		ZUI_GET_F32(nodes, link_drag_id)
		if (strcmp(*name, "nodes_selected_id") == 0) {
			Local<Array> result = Array::New(isolate, nodes->nodes_selected_count);
			for (int i = 0; i < nodes->nodes_selected_count; ++i) {
				(void)result->Set(isolate->GetCurrentContext(), i, Int32::New(isolate, nodes->nodes_selected_id[i]));
			}
			args.GetReturnValue().Set(result);
		}
	}

	void krom_zui_nodes_set(ARGS) {
		SCOPE();
		String::Utf8Value name(isolate, args[1]);
		Local<External> field = Local<External>::Cast(GET_INTERNAL(args[0]));
		zui_nodes_t *nodes = (zui_nodes_t *)field->Value();
		ZUI_SET_F32(nodes, pan_x)
		ZUI_SET_F32(nodes, pan_y)
		ZUI_SET_F32(nodes, zoom)
		ZUI_SET_I32(nodes, _input_started)
		ZUI_SET_I32(nodes, link_drag_id)
		ZUI_SET_I32(nodes, nodes_drag)
		if (strcmp(*name, "nodes_selected_id") == 0) {
			Local<Object> js_array = TO_OBJ(args[2]);
			int32_t length = TO_I32(OBJ_GET(js_array, "length"));
			for (int i = 0; i < length; ++i) {
				int32_t j = TO_I32(ARRAY_GET(js_array, i));
				nodes->nodes_selected_id[i] = j;
			}
			nodes->nodes_selected_count = length;
		}
	}

	void krom_zui_set_on_border_hover(ARGS) {
		SCOPE();
		SET_FUNC(on_border_hover_func, args[0]);
	}

	void krom_zui_set_on_text_hover(ARGS) {
		SCOPE();
		SET_FUNC(on_text_hover_func, args[0]);
	}

	void krom_zui_set_on_deselect_text(ARGS) {
		SCOPE();
		SET_FUNC(on_deselect_text_func, args[0]);
	}

	void krom_zui_set_on_tab_drop(ARGS) {
		SCOPE();
		SET_FUNC(on_tab_drop_func, args[0]);
	}

	void krom_zui_nodes_set_enum_texts(ARGS) {
		SCOPE();
		SET_FUNC(enum_texts_func, args[0]);
	}

	void krom_zui_nodes_set_on_custom_button(ARGS) {
		SCOPE();
		SET_FUNC(on_custom_button_func, args[0]);
	}

	void krom_zui_nodes_set_on_canvas_control(ARGS) {
		SCOPE();
		SET_FUNC(on_canvas_control_func, args[0]);
	}

	void krom_zui_nodes_set_on_canvas_released(ARGS) {
		SCOPE();
		SET_FUNC(on_canvas_released_func, args[0]);
	}

	void krom_zui_nodes_set_on_socket_released(ARGS) {
		SCOPE();
		SET_FUNC(on_socket_released_func, args[0]);
	}

	void krom_zui_nodes_set_on_link_drag(ARGS) {
		SCOPE();
		SET_FUNC(on_link_drag_func, args[0]);
	}
	#endif

	#define BIND_FUNCTION_FAST(object, name, fn)\
		CFunction fn ## _ = CFunction::Make(fn ## _fast);\
		object->Set(String::NewFromUtf8(isolate, name).ToLocalChecked(),\
		FunctionTemplate::New(isolate, fn, Local<v8::Value>(), Local<v8::Signature>(), 0,\
		v8::ConstructorBehavior::kThrow, v8::SideEffectType::kHasNoSideEffect, &fn ## _))

	#define BIND_FUNCTION(object, name, fn)\
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

		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		BIND_FUNCTION(global, "krom_init", krom_init);
		BIND_FUNCTION(global, "krom_set_app_name", krom_set_app_name);
		BIND_FUNCTION(global, "krom_log", krom_log);
		BIND_FUNCTION_FAST(global, "krom_g4_clear", krom_g4_clear);
		BIND_FUNCTION(global, "krom_set_update_callback", krom_set_update_callback);
		BIND_FUNCTION(global, "krom_set_drop_files_callback", krom_set_drop_files_callback);
		BIND_FUNCTION(global, "krom_set_cut_copy_paste_callback", krom_set_cut_copy_paste_callback);
		BIND_FUNCTION(global, "krom_set_application_state_callback", krom_set_application_state_callback);
		BIND_FUNCTION(global, "krom_set_keyboard_down_callback", krom_set_keyboard_down_callback);
		BIND_FUNCTION(global, "krom_set_keyboard_up_callback", krom_set_keyboard_up_callback);
		BIND_FUNCTION(global, "krom_set_keyboard_press_callback", krom_set_keyboard_press_callback);
		BIND_FUNCTION(global, "krom_set_mouse_down_callback", krom_set_mouse_down_callback);
		BIND_FUNCTION(global, "krom_set_mouse_up_callback", krom_set_mouse_up_callback);
		BIND_FUNCTION(global, "krom_set_mouse_move_callback", krom_set_mouse_move_callback);
		BIND_FUNCTION(global, "krom_set_touch_down_callback", krom_set_touch_down_callback);
		BIND_FUNCTION(global, "krom_set_touch_up_callback", krom_set_touch_up_callback);
		BIND_FUNCTION(global, "krom_set_touch_move_callback", krom_set_touch_move_callback);
		BIND_FUNCTION(global, "krom_set_mouse_wheel_callback", krom_set_mouse_wheel_callback);
		BIND_FUNCTION(global, "krom_set_pen_down_callback", krom_set_pen_down_callback);
		BIND_FUNCTION(global, "krom_set_pen_up_callback", krom_set_pen_up_callback);
		BIND_FUNCTION(global, "krom_set_pen_move_callback", krom_set_pen_move_callback);
		BIND_FUNCTION(global, "krom_set_gamepad_axis_callback", krom_set_gamepad_axis_callback);
		BIND_FUNCTION(global, "krom_set_gamepad_button_callback", krom_set_gamepad_button_callback);
		BIND_FUNCTION_FAST(global, "krom_lock_mouse", krom_lock_mouse);
		BIND_FUNCTION_FAST(global, "krom_unlock_mouse", krom_unlock_mouse);
		BIND_FUNCTION_FAST(global, "krom_can_lock_mouse", krom_can_lock_mouse);
		BIND_FUNCTION_FAST(global, "krom_is_mouse_locked", krom_is_mouse_locked);
		BIND_FUNCTION_FAST(global, "krom_set_mouse_position", krom_set_mouse_position);
		BIND_FUNCTION_FAST(global, "krom_show_mouse", krom_show_mouse);
		BIND_FUNCTION_FAST(global, "krom_show_keyboard", krom_show_keyboard);

		BIND_FUNCTION(global, "krom_g4_create_index_buffer", krom_g4_create_index_buffer);
		BIND_FUNCTION(global, "krom_g4_delete_index_buffer", krom_g4_delete_index_buffer);
		BIND_FUNCTION(global, "krom_g4_lock_index_buffer", krom_g4_lock_index_buffer);
		BIND_FUNCTION(global, "krom_g4_unlock_index_buffer", krom_g4_unlock_index_buffer);
		BIND_FUNCTION(global, "krom_g4_set_index_buffer", krom_g4_set_index_buffer);
		BIND_FUNCTION(global, "krom_g4_create_vertex_buffer", krom_g4_create_vertex_buffer);
		BIND_FUNCTION(global, "krom_g4_delete_vertex_buffer", krom_g4_delete_vertex_buffer);
		BIND_FUNCTION(global, "krom_g4_lock_vertex_buffer", krom_g4_lock_vertex_buffer);
		BIND_FUNCTION(global, "krom_g4_unlock_vertex_buffer", krom_g4_unlock_vertex_buffer);
		BIND_FUNCTION(global, "krom_g4_set_vertex_buffer", krom_g4_set_vertex_buffer);
		BIND_FUNCTION(global, "krom_g4_set_vertex_buffers", krom_g4_set_vertex_buffers);
		BIND_FUNCTION_FAST(global, "krom_g4_draw_indexed_vertices", krom_g4_draw_indexed_vertices);
		BIND_FUNCTION_FAST(global, "krom_g4_draw_indexed_vertices_instanced", krom_g4_draw_indexed_vertices_instanced);
		BIND_FUNCTION(global, "krom_g4_create_shader", krom_g4_create_shader);
		BIND_FUNCTION(global, "krom_g4_create_vertex_shader_from_source", krom_g4_create_vertex_shader_from_source);
		BIND_FUNCTION(global, "krom_g4_create_fragment_shader_from_source", krom_g4_create_fragment_shader_from_source);
		BIND_FUNCTION(global, "krom_g4_delete_shader", krom_g4_delete_shader);
		BIND_FUNCTION(global, "krom_g4_create_pipeline", krom_g4_create_pipeline);
		BIND_FUNCTION(global, "krom_g4_delete_pipeline", krom_g4_delete_pipeline);
		BIND_FUNCTION(global, "krom_g4_compile_pipeline", krom_g4_compile_pipeline);
		BIND_FUNCTION(global, "krom_g4_set_pipeline", krom_g4_set_pipeline);
		BIND_FUNCTION(global, "krom_load_image", krom_load_image);
		BIND_FUNCTION(global, "krom_unload_image", krom_unload_image);
		#ifdef WITH_AUDIO
		BIND_FUNCTION(global, "krom_load_sound", krom_load_sound);
		BIND_FUNCTION(global, "krom_unload_sound", krom_unload_sound);
		BIND_FUNCTION(global, "krom_play_sound", krom_play_sound);
		BIND_FUNCTION(global, "krom_stop_sound", krom_stop_sound);
		#endif
		BIND_FUNCTION(global, "krom_load_blob", krom_load_blob);
		BIND_FUNCTION(global, "krom_load_url", krom_load_url);
		BIND_FUNCTION(global, "krom_copy_to_clipboard", krom_copy_to_clipboard);

		BIND_FUNCTION(global, "krom_g4_get_constant_location", krom_g4_get_constant_location);
		BIND_FUNCTION(global, "krom_g4_get_texture_unit", krom_g4_get_texture_unit);
		BIND_FUNCTION(global, "krom_g4_set_texture", krom_g4_set_texture);
		BIND_FUNCTION(global, "krom_g4_set_render_target", krom_g4_set_render_target);
		BIND_FUNCTION(global, "krom_g4_set_texture_depth", krom_g4_set_texture_depth);
		BIND_FUNCTION(global, "krom_g4_set_image_texture", krom_g4_set_image_texture);
		BIND_FUNCTION(global, "krom_g4_set_texture_parameters", krom_g4_set_texture_parameters);
		BIND_FUNCTION(global, "krom_g4_set_texture3d_parameters", krom_g4_set_texture3d_parameters);
		BIND_FUNCTION(global, "krom_g4_set_bool", krom_g4_set_bool);
		BIND_FUNCTION(global, "krom_g4_set_int", krom_g4_set_int);
		BIND_FUNCTION(global, "krom_g4_set_float", krom_g4_set_float);
		BIND_FUNCTION(global, "krom_g4_set_float2", krom_g4_set_float2);
		BIND_FUNCTION(global, "krom_g4_set_float3", krom_g4_set_float3);
		BIND_FUNCTION(global, "krom_g4_set_float4", krom_g4_set_float4);
		BIND_FUNCTION(global, "krom_g4_set_floats", krom_g4_set_floats);
		BIND_FUNCTION(global, "krom_g4_set_matrix4", krom_g4_set_matrix4);
		BIND_FUNCTION(global, "krom_g4_set_matrix3", krom_g4_set_matrix3);

		BIND_FUNCTION_FAST(global, "krom_get_time", krom_get_time);
		BIND_FUNCTION_FAST(global, "krom_window_width", krom_window_width);
		BIND_FUNCTION_FAST(global, "krom_window_height", krom_window_height);
		BIND_FUNCTION(global, "krom_set_window_title", krom_set_window_title);
		BIND_FUNCTION(global, "krom_get_window_mode", krom_get_window_mode);
		BIND_FUNCTION(global, "krom_set_window_mode", krom_set_window_mode);
		BIND_FUNCTION(global, "krom_resize_window", krom_resize_window);
		BIND_FUNCTION(global, "krom_move_window", krom_move_window);
		BIND_FUNCTION(global, "krom_screen_dpi", krom_screen_dpi);
		BIND_FUNCTION(global, "krom_system_id", krom_system_id);
		BIND_FUNCTION(global, "krom_request_shutdown", krom_request_shutdown);
		BIND_FUNCTION(global, "krom_display_count", krom_display_count);
		BIND_FUNCTION(global, "krom_display_width", krom_display_width);
		BIND_FUNCTION(global, "krom_display_height", krom_display_height);
		BIND_FUNCTION(global, "krom_display_x", krom_display_x);
		BIND_FUNCTION(global, "krom_display_y", krom_display_y);
		BIND_FUNCTION(global, "krom_display_frequency", krom_display_frequency);
		BIND_FUNCTION(global, "krom_display_is_primary", krom_display_is_primary);
		BIND_FUNCTION(global, "krom_write_storage", krom_write_storage);
		BIND_FUNCTION(global, "krom_read_storage", krom_read_storage);

		BIND_FUNCTION(global, "krom_g4_create_render_target", krom_g4_create_render_target);
		BIND_FUNCTION(global, "krom_g4_create_texture", krom_g4_create_texture);
		BIND_FUNCTION(global, "krom_g4_create_texture3d", krom_g4_create_texture3d);
		BIND_FUNCTION(global, "krom_g4_create_texture_from_bytes", krom_g4_create_texture_from_bytes);
		BIND_FUNCTION(global, "krom_g4_create_texture_from_bytes3d", krom_g4_create_texture_from_bytes3d);
		BIND_FUNCTION(global, "krom_g4_create_texture_from_encoded_bytes", krom_g4_create_texture_from_encoded_bytes);
		BIND_FUNCTION(global, "krom_g4_get_texture_pixels", krom_g4_get_texture_pixels);
		BIND_FUNCTION(global, "krom_g4_get_render_target_pixels", krom_g4_get_render_target_pixels);
		BIND_FUNCTION(global, "krom_g4_lock_texture", krom_g4_lock_texture);
		BIND_FUNCTION(global, "krom_g4_unlock_texture", krom_g4_unlock_texture);
		BIND_FUNCTION(global, "krom_g4_clear_texture", krom_g4_clear_texture);
		BIND_FUNCTION(global, "krom_g4_generate_texture_mipmaps", krom_g4_generate_texture_mipmaps);
		BIND_FUNCTION(global, "krom_g4_generate_render_target_mipmaps", krom_g4_generate_render_target_mipmaps);
		BIND_FUNCTION(global, "krom_g4_set_mipmaps", krom_g4_set_mipmaps);
		BIND_FUNCTION(global, "krom_g4_set_depth_from", krom_g4_set_depth_from);
		BIND_FUNCTION_FAST(global, "krom_g4_viewport", krom_g4_viewport);
		BIND_FUNCTION_FAST(global, "krom_g4_scissor", krom_g4_scissor);
		BIND_FUNCTION_FAST(global, "krom_g4_disable_scissor", krom_g4_disable_scissor);
		BIND_FUNCTION_FAST(global, "krom_g4_render_targets_inverted_y", krom_g4_render_targets_inverted_y);
		BIND_FUNCTION(global, "krom_g4_begin", krom_g4_begin);
		BIND_FUNCTION(global, "krom_g4_end", krom_g4_end);
		BIND_FUNCTION(global, "krom_file_save_bytes", krom_file_save_bytes);
		BIND_FUNCTION(global, "krom_sys_command", krom_sys_command);
		BIND_FUNCTION(global, "krom_save_path", krom_save_path);
		BIND_FUNCTION(global, "krom_get_arg_count", krom_get_arg_count);
		BIND_FUNCTION(global, "krom_get_arg", krom_get_arg);
		BIND_FUNCTION(global, "krom_get_files_location", krom_get_files_location);
		BIND_FUNCTION(global, "krom_http_request", krom_http_request);
		#ifdef WITH_G2
		BIND_FUNCTION(global, "krom_g2_init", krom_g2_init);
		BIND_FUNCTION(global, "krom_g2_begin", krom_g2_begin);
		BIND_FUNCTION(global, "krom_g2_end", krom_g2_end);
		BIND_FUNCTION(global, "krom_g2_draw_scaled_sub_image", krom_g2_draw_scaled_sub_image);
		BIND_FUNCTION(global, "krom_g2_fill_triangle", krom_g2_fill_triangle);
		BIND_FUNCTION(global, "krom_g2_fill_rect", krom_g2_fill_rect);
		BIND_FUNCTION(global, "krom_g2_draw_rect", krom_g2_draw_rect);
		BIND_FUNCTION(global, "krom_g2_draw_line", krom_g2_draw_line);
		BIND_FUNCTION(global, "krom_g2_draw_string", krom_g2_draw_string);
		BIND_FUNCTION(global, "krom_g2_set_font", krom_g2_set_font);
		BIND_FUNCTION(global, "krom_g2_font_init", krom_g2_font_init);
		BIND_FUNCTION(global, "krom_g2_font_13", krom_g2_font_13);
		BIND_FUNCTION(global, "krom_g2_font_set_glyphs", krom_g2_font_set_glyphs);
		BIND_FUNCTION(global, "krom_g2_font_count", krom_g2_font_count);
		BIND_FUNCTION(global, "krom_g2_font_height", krom_g2_font_height);
		BIND_FUNCTION(global, "krom_g2_string_width", krom_g2_string_width);
		BIND_FUNCTION(global, "krom_g2_set_bilinear_filter", krom_g2_set_bilinear_filter);
		BIND_FUNCTION(global, "krom_g2_restore_render_target", krom_g2_restore_render_target);
		BIND_FUNCTION(global, "krom_g2_set_render_target", krom_g2_set_render_target);
		BIND_FUNCTION(global, "krom_g2_set_color", krom_g2_set_color);
		BIND_FUNCTION(global, "krom_g2_set_pipeline", krom_g2_set_pipeline);
		BIND_FUNCTION(global, "krom_g2_set_transform", krom_g2_set_transform);
		BIND_FUNCTION(global, "krom_g2_fill_circle", krom_g2_fill_circle);
		BIND_FUNCTION(global, "krom_g2_draw_circle", krom_g2_draw_circle);
		BIND_FUNCTION(global, "krom_g2_draw_cubic_bezier", krom_g2_draw_cubic_bezier);
		#endif
		BIND_FUNCTION(global, "krom_set_save_and_quit_callback", krom_set_save_and_quit_callback);
		BIND_FUNCTION(global, "krom_set_mouse_cursor", krom_set_mouse_cursor);
		BIND_FUNCTION_FAST(global, "krom_delay_idle_sleep", krom_delay_idle_sleep);
		#if defined(WITH_NFD) || defined(KORE_IOS) || defined(KORE_ANDROID)
		BIND_FUNCTION(global, "krom_open_dialog", krom_open_dialog);
		BIND_FUNCTION(global, "krom_save_dialog", krom_save_dialog);
		#endif
		#ifdef WITH_TINYDIR
		BIND_FUNCTION(global, "krom_read_directory", krom_read_directory);
		#endif
		BIND_FUNCTION(global, "krom_file_exists", krom_file_exists);
		BIND_FUNCTION(global, "krom_delete_file", krom_delete_file);
		#ifdef WITH_ZLIB
		BIND_FUNCTION(global, "krom_inflate", krom_inflate);
		BIND_FUNCTION(global, "krom_deflate", krom_deflate);
		#endif
		#ifdef WITH_STB_IMAGE_WRITE
		BIND_FUNCTION(global, "krom_write_jpg", krom_write_jpg);
		BIND_FUNCTION(global, "krom_write_png", krom_write_png);
		BIND_FUNCTION(global, "krom_encode_jpg", krom_encode_jpg);
		BIND_FUNCTION(global, "krom_encode_png", krom_encode_png);
		#endif
		#ifdef WITH_MPEG_WRITE
		BIND_FUNCTION(global, "krom_write_mpeg", krom_write_mpeg);
		#endif
		#ifdef WITH_ONNX
		BIND_FUNCTION(global, "krom_ml_inference", krom_ml_inference);
		BIND_FUNCTION(global, "krom_ml_unload", krom_ml_unload);
		#endif

		#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
		BIND_FUNCTION(global, "krom_raytrace_supported", krom_raytrace_supported);
		BIND_FUNCTION(global, "krom_raytrace_init", krom_raytrace_init);
		BIND_FUNCTION(global, "krom_raytrace_set_textures", krom_raytrace_set_textures);
		BIND_FUNCTION(global, "krom_raytrace_dispatch_rays", krom_raytrace_dispatch_rays);
		#endif

		BIND_FUNCTION(global, "krom_window_x", krom_window_x);
		BIND_FUNCTION(global, "krom_window_y", krom_window_y);
		BIND_FUNCTION(global, "krom_language", krom_language);
		#ifdef WITH_IRON
		BIND_FUNCTION(global, "krom_io_obj_parse", krom_io_obj_parse);
		#endif

		#ifdef WITH_ZUI
		BIND_FUNCTION(global, "krom_zui_init", krom_zui_init);
		BIND_FUNCTION(global, "krom_zui_get_scale", krom_zui_get_scale);
		BIND_FUNCTION(global, "krom_zui_set_scale", krom_zui_set_scale);
		BIND_FUNCTION(global, "krom_zui_set_font", krom_zui_set_font);
		BIND_FUNCTION(global, "krom_zui_begin", krom_zui_begin);
		BIND_FUNCTION(global, "krom_zui_end", krom_zui_end);
		BIND_FUNCTION(global, "krom_zui_begin_region", krom_zui_begin_region);
		BIND_FUNCTION(global, "krom_zui_end_region", krom_zui_end_region);
		BIND_FUNCTION(global, "krom_zui_begin_sticky", krom_zui_begin_sticky);
		BIND_FUNCTION(global, "krom_zui_end_sticky", krom_zui_end_sticky);
		BIND_FUNCTION(global, "krom_zui_end_input", krom_zui_end_input);
		BIND_FUNCTION(global, "krom_zui_end_window", krom_zui_end_window);
		BIND_FUNCTION(global, "krom_zui_end_element", krom_zui_end_element);
		BIND_FUNCTION(global, "krom_zui_start_text_edit", krom_zui_start_text_edit);
		BIND_FUNCTION(global, "krom_zui_input_in_rect", krom_zui_input_in_rect);
		BIND_FUNCTION(global, "krom_zui_window", krom_zui_window);
		BIND_FUNCTION(global, "krom_zui_button", krom_zui_button);
		BIND_FUNCTION(global, "krom_zui_check", krom_zui_check);
		BIND_FUNCTION(global, "krom_zui_radio", krom_zui_radio);
		BIND_FUNCTION(global, "krom_zui_combo", krom_zui_combo);
		BIND_FUNCTION(global, "krom_zui_slider", krom_zui_slider);
		BIND_FUNCTION(global, "krom_zui_image", krom_zui_image);
		BIND_FUNCTION(global, "krom_zui_text", krom_zui_text);
		BIND_FUNCTION(global, "krom_zui_text_input", krom_zui_text_input);
		BIND_FUNCTION(global, "krom_zui_tab", krom_zui_tab);
		BIND_FUNCTION(global, "krom_zui_panel", krom_zui_panel);
		BIND_FUNCTION(global, "krom_zui_handle", krom_zui_handle);
		BIND_FUNCTION(global, "krom_zui_separator", krom_zui_separator);
		BIND_FUNCTION(global, "krom_zui_tooltip", krom_zui_tooltip);
		BIND_FUNCTION(global, "krom_zui_tooltip_image", krom_zui_tooltip_image);
		BIND_FUNCTION(global, "krom_zui_row", krom_zui_row);
		BIND_FUNCTION(global, "krom_zui_fill", krom_zui_fill);
		BIND_FUNCTION(global, "krom_zui_rect", krom_zui_rect);
		BIND_FUNCTION(global, "krom_zui_draw_rect", krom_zui_draw_rect);
		BIND_FUNCTION(global, "krom_zui_draw_string", krom_zui_draw_string);
		BIND_FUNCTION(global, "krom_zui_get_hovered_tab_name", krom_zui_get_hovered_tab_name);
		BIND_FUNCTION(global, "krom_zui_set_hovered_tab_name", krom_zui_set_hovered_tab_name);
		BIND_FUNCTION(global, "krom_zui_begin_menu", krom_zui_begin_menu);
		BIND_FUNCTION(global, "krom_zui_end_menu", krom_zui_end_menu);
		BIND_FUNCTION(global, "krom_zui_menu_button", krom_zui_menu_button);
		BIND_FUNCTION(global, "krom_zui_float_input", krom_zui_float_input);
		BIND_FUNCTION(global, "krom_zui_inline_radio", krom_zui_inline_radio);
		BIND_FUNCTION(global, "krom_zui_color_wheel", krom_zui_color_wheel);
		BIND_FUNCTION(global, "krom_zui_text_area", krom_zui_text_area);
		BIND_FUNCTION(global, "krom_zui_text_area_coloring", krom_zui_text_area_coloring);
		BIND_FUNCTION(global, "krom_zui_nodes_init", krom_zui_nodes_init);
		BIND_FUNCTION(global, "krom_zui_node_canvas", krom_zui_node_canvas);
		BIND_FUNCTION(global, "krom_zui_nodes_rgba_popup", krom_zui_nodes_rgba_popup);
		BIND_FUNCTION(global, "krom_zui_nodes_scale", krom_zui_nodes_scale);
		BIND_FUNCTION(global, "krom_zui_nodes_pan_x", krom_zui_nodes_pan_x);
		BIND_FUNCTION(global, "krom_zui_nodes_pan_y", krom_zui_nodes_pan_y);
		BIND_FUNCTION(global, "krom_zui_set", krom_zui_set);
		BIND_FUNCTION(global, "krom_zui_get", krom_zui_get);
		BIND_FUNCTION(global, "krom_zui_handle_get", krom_zui_handle_get);
		BIND_FUNCTION(global, "krom_zui_handle_set", krom_zui_handle_set);
		BIND_FUNCTION(global, "krom_zui_handle_ptr", krom_zui_handle_ptr);
		BIND_FUNCTION(global, "krom_zui_theme_init", krom_zui_theme_init);
		BIND_FUNCTION(global, "krom_zui_theme_get", krom_zui_theme_get);
		BIND_FUNCTION(global, "krom_zui_theme_set", krom_zui_theme_set);
		BIND_FUNCTION(global, "krom_zui_nodes_get", krom_zui_nodes_get);
		BIND_FUNCTION(global, "krom_zui_nodes_set", krom_zui_nodes_set);
		BIND_FUNCTION(global, "krom_zui_set_on_border_hover", krom_zui_set_on_border_hover);
		BIND_FUNCTION(global, "krom_zui_set_on_text_hover", krom_zui_set_on_text_hover);
		BIND_FUNCTION(global, "krom_zui_set_on_deselect_text", krom_zui_set_on_deselect_text);
		BIND_FUNCTION(global, "krom_zui_set_on_tab_drop", krom_zui_set_on_tab_drop);
		BIND_FUNCTION(global, "krom_zui_nodes_set_enum_texts", krom_zui_nodes_set_enum_texts);
		BIND_FUNCTION(global, "krom_zui_nodes_set_on_custom_button", krom_zui_nodes_set_on_custom_button);
		BIND_FUNCTION(global, "krom_zui_nodes_set_on_canvas_control", krom_zui_nodes_set_on_canvas_control);
		BIND_FUNCTION(global, "krom_zui_nodes_set_on_canvas_released", krom_zui_nodes_set_on_canvas_released);
		BIND_FUNCTION(global, "krom_zui_nodes_set_on_socket_released", krom_zui_nodes_set_on_socket_released);
		BIND_FUNCTION(global, "krom_zui_nodes_set_on_link_drag", krom_zui_nodes_set_on_link_drag);
		#endif

		#ifdef WITH_PLUGIN_EMBED
		plugin_embed(isolate, global);
		#endif

		Local<Context> context = Context::New(isolate, NULL, global);
		global_context.Reset(isolate, context);
	}

	void start_krom(char *scriptfile) {
		LOCKER();
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
			Local<Value> js_kickstart = context->Global()->Get(isolate->GetCurrentContext(), TO_STR("kickstart")).ToLocalChecked();
			if (!js_kickstart->IsNullOrUndefined()) {
				Local<Value> result;
				if (!js_kickstart->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->CallAsFunction(context, context->Global(), 0, nullptr).ToLocal(&result)) {
					handle_exception(&try_catch);
				}
			}
		}
	}

	void run_v8() {
		LOCKER();
		MicrotasksScope microtasks_scope(isolate, MicrotasksScope::kRunMicrotasks);
		CALL_FUNC(update_func);
	}

	void update(void *data) {
		#ifdef KORE_WINDOWS
		if (show_window && enable_window) {
			show_window = false;
			kinc_window_show(0);
		}

		if (in_background && ++paused_frames > 3) {
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
		kinc_a2_update();
		#endif

		kinc_g4_begin(0);
		run_v8();
		kinc_g4_end(0);
		kinc_g4_swap_buffers();
	}

	void drop_files(wchar_t *file_path, void *data) {
		// Update mouse position
		#ifdef KORE_WINDOWS
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(kinc_windows_window_handle(0), &p);
		mouse_move(0, p.x, p.y, 0, 0, NULL);
		#endif

		LOCKER();
		Local<Value> argv[1];
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
		CALL_FUNCI(drop_files_func, 1, argv);
		in_background = false;

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	char *copy(void *data) {
		// LOCKER();
		// CALL_FUNC(copy_func);
		// String::Utf8Value cutCopyString(isolate, result);
		// strcpy(temp_string, *cutCopyString);

		#ifdef WITH_ZUI
		strcpy(temp_string, zui_copy());
		#endif
		return temp_string;
	}

	char *cut(void *data) {
		// LOCKER();
		// CALL_FUNCTION(cut_func);
		// String::Utf8Value cutCopyString(isolate, result);
		// strcpy(temp_string, *cutCopyString);

		#ifdef WITH_ZUI
		strcpy(temp_string, zui_cut());
		#endif
		return temp_string;
	}

	void paste(char *text, void *data) {
		// LOCKER();
		// Local<Value> argv[1] = {String::NewFromUtf8(isolate, text).ToLocalChecked()};
		// CALL_FUNCI(paste_func, 1, argv);

		#ifdef WITH_ZUI
		zui_paste(text);
		#endif
	}

	void foreground(void *data) {
		LOCKER();
		CALL_FUNC(foreground_func);
		in_background = false;
	}

	void resume(void *data) {
		LOCKER();
		CALL_FUNC(resume_func);
	}

	void pause(void *data) {
		LOCKER();
		CALL_FUNC(pause_func);
	}

	void background(void *data) {
		LOCKER();
		CALL_FUNC(background_func);
		in_background = true;
		paused_frames = 0;
	}

	void shutdown(void *data) {
		LOCKER();
		CALL_FUNC(shutdown_func);
	}

	void key_down(int code, void *data) {
		LOCKER();
		Local<Value> argv[1] = {Int32::New(isolate, code)};
		CALL_FUNCI(keyboard_down_func, 1, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_key_down(zui_instances[i], code);
		#endif

		#ifdef IDLE_SLEEP
		input_down = true;
		paused_frames = 0;
		#endif
	}

	void key_up(int code, void *data) {
		LOCKER();
		Local<Value> argv[1] = {Int32::New(isolate, code)};
		CALL_FUNCI(keyboard_up_func, 1, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_key_up(zui_instances[i], code);
		#endif

		#ifdef IDLE_SLEEP
		input_down = false;
		paused_frames = 0;
		#endif
	}

	void key_press(unsigned int character, void *data) {
		LOCKER();
		Local<Value> argv[1] = {Int32::New(isolate, character)};
		CALL_FUNCI(keyboard_press_func, 1, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_key_press(zui_instances[i], character);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void mouse_move(int window, int x, int y, int mx, int my, void *data) {
		LOCKER();
		Local<Value> argv[4] = {Int32::New(isolate, x), Int32::New(isolate, y), Int32::New(isolate, mx), Int32::New(isolate, my)};
		CALL_FUNCI(mouse_move_func, 4, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_move(zui_instances[i], x, y, mx, my);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void mouse_down(int window, int button, int x, int y, void *data) {
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		CALL_FUNCI(mouse_down_func, 3, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_down(zui_instances[i], button, x, y);
		#endif

		#ifdef IDLE_SLEEP
		input_down = true;
		paused_frames = 0;
		#endif
	}

	void mouse_up(int window, int button, int x, int y, void *data) {
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		CALL_FUNCI(mouse_up_func, 3, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_up(zui_instances[i], button, x, y);
		#endif

		#ifdef IDLE_SLEEP
		input_down = false;
		paused_frames = 0;
		#endif
	}

	void mouse_wheel(int window, int delta, void *data) {
		LOCKER();
		Local<Value> argv[1] = {Int32::New(isolate, delta)};
		CALL_FUNCI(mouse_wheel_func, 1, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_mouse_wheel(zui_instances[i], delta);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void touch_move(int index, int x, int y) {
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		CALL_FUNCI(touch_move_func, 3, argv);

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
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		CALL_FUNCI(touch_down_func, 3, argv);

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
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, index), Int32::New(isolate, x), Int32::New(isolate, y)};
		CALL_FUNCI(touch_up_func, 3, argv);

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
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		CALL_FUNCI(pen_down_func, 3, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_pen_down(zui_instances[i], x, y, pressure);
		#endif

		#ifdef IDLE_SLEEP
		input_down = true;
		paused_frames = 0;
		#endif
	}

	void pen_up(int window, int x, int y, float pressure) {
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		CALL_FUNCI(pen_up_func, 3, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_pen_up(zui_instances[i], x, y, pressure);
		#endif

		#ifdef IDLE_SLEEP
		input_down = false;
		paused_frames = 0;
		#endif
	}

	void pen_move(int window, int x, int y, float pressure) {
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		CALL_FUNCI(pen_move_func, 3, argv);

		#ifdef WITH_ZUI
		for (int i = 0; i < zui_instances_count; ++i) zui_pen_move(zui_instances[i], x, y, pressure);
		#endif

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void gamepad_axis(int gamepad, int axis, float value) {
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, gamepad), Int32::New(isolate, axis), Number::New(isolate, value)};
		CALL_FUNCI(gamepad_axis_func, 3, argv);

		#ifdef IDLE_SLEEP
		paused_frames = 0;
		#endif
	}

	void gamepad_button(int gamepad, int button, float value) {
		LOCKER();
		Local<Value> argv[3] = {Int32::New(isolate, gamepad), Int32::New(isolate, button), Number::New(isolate, value)};
		CALL_FUNCI(gamepad_button_func, 3, argv);

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

		SnapshotCreator creator;
		Isolate *isolate_creator = creator.GetIsolate();
		{
			HandleScope handle_scope(isolate_creator);
			{
				Local<Context> context = Context::New(isolate_creator);
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

						Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate_creator, reader_size);
						std::shared_ptr<BackingStore> content = buffer->GetBackingStore();
						kinc_file_reader_read(&reader, content->Data(), reader_size);
						kinc_file_reader_close(&reader);

						(void) context->Global()->Set(context, String::NewFromUtf8(isolate_creator, line).ToLocalChecked(), buffer);
					}
					fclose (fp);
				}

				ScriptOrigin origin(isolate_cache, String::NewFromUtf8(isolate_creator, "krom_snapshot").ToLocalChecked());
				ScriptCompiler::Source source(String::NewFromUtf8(isolate_creator, code).ToLocalChecked(), origin, cache);

				Local<Script> compiled_script = ScriptCompiler::Compile(context, &source, ScriptCompiler::kConsumeCodeCache).ToLocalChecked();
				(void) compiled_script->Run(context);

				creator.SetDefaultContext(context);
			}
		}
		StartupData snapshotData = creator.CreateBlob(SnapshotCreator::FunctionCodeHandling::kKeep);

		std::string krombin = assetsdir + "/krom.bin";
		FILE *file = fopen(&krombin[0u], "wb");
		if (file != nullptr) {
			fwrite(snapshotData.data, 1, snapshotData.raw_size, file);
			fclose(file);
		}
		exit(0);
	}

	kinc_threads_init();
	kinc_display_init();

	start_v8(snapshot_found ? code : NULL, snapshot_found ? reader_size : 0);
	start_krom(snapshot_found ? NULL : code);

	kinc_start();

	#ifdef WITH_AUDIO
	kinc_a2_shutdown();
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
