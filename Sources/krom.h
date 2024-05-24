#pragma once

#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <kinc/log.h>
#include <kinc/window.h>
#include <kinc/system.h>
#include <kinc/display.h>
#include <kinc/threads/thread.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/io/filereader.h>
#include <iron/iron_string.h>
#include <iron/iron_array.h>
#include <iron/iron_map.h>
#include <iron/iron_armpack.h>
#include <iron/iron_json.h>
#include <iron/iron_gc.h>
#include "iron/io_obj.h"
#ifdef KINC_WINDOWS
#include <Windows.h>
#endif
#if defined(IDLE_SLEEP) && !defined(KINC_WINDOWS)
#include <unistd.h>
#endif
#ifdef WITH_G2
#include "g2/g2.h"
#include "g2/g2_ext.h"
#endif
#ifdef WITH_ZUI
#include "zui/zui.h"
#include "zui/zui_ext.h"
#include "zui/zui_nodes.h"
#endif

#define i64 int64_t
#define f64 double
#define f32 float
#define i32 int32_t
#define u32 uint32_t
#define i16 int16_t
#define u16 uint16_t
#define i8 int8_t
#define u8 uint8_t
#define string_t char
#define any void *
#define null NULL

void _kickstart();

int _argc;
char **_argv;
bool enable_window = true;
bool in_background = false;
int paused_frames = 0;
bool save_and_quit_callback_set = false;
#ifdef IDLE_SLEEP
bool input_down = false;
int last_window_width = 0;
int last_window_height = 0;
#endif

char temp_string[4096];
char temp_string_vs[1024 * 1024];
char temp_string_fs[1024 * 1024];
char temp_string_vstruct[4][32][32];
#ifdef KINC_WINDOWS
wchar_t temp_wstring[1024];
bool show_window = false;
#endif

void (*krom_update)(void);
void (*krom_drop_files)(char *);
// void (*krom_cut)(void);
// void (*krom_copy])(void);
// void (*krom_paste)(void);
void (*krom_foreground)(void);
void (*krom_resume)(void);
void (*krom_pause)(void);
void (*krom_background)(void);
void (*krom_shutdown)(void);
void (*krom_pause)(void);
void (*krom_key_down)(int);
void (*krom_key_up)(int);
void (*krom_key_press)(int);
void (*krom_mouse_down)(int, int, int);
void (*krom_mouse_up)(int, int, int);
void (*krom_mouse_move)(int, int, int, int);
void (*krom_mouse_wheel)(int);
void (*krom_touch_down)(int, int, int);
void (*krom_touch_up)(int, int, int);
void (*krom_touch_move)(int, int, int);
void (*krom_pen_down)(int, int, float);
void (*krom_pen_up)(int, int, float);
void (*krom_pen_move)(int, int, float);
void (*krom_gamepad_axis)(int, int, float);
void (*krom_gamepad_button)(int, int, float);
void (*krom_save_and_quit)(bool);

char *_substring(char *s, int32_t start, int32_t end) {
	char *buffer = calloc(1, end - start + 1);
	for (int i = 0; i < end - start; ++i) {
		buffer[i] = s[start + i];
	}
	return buffer;
}

int kickstart(int argc, char **argv) {
	_argc = argc;
	_argv = argv;
#ifdef KINC_ANDROID
	char *bindir = "/";
#elif defined(KINC_IOS)
	char *bindir = "";
#else
	char *bindir = argv[0];
#endif

#ifdef KINC_WINDOWS // Handle non-ascii path
	HMODULE hmodule = GetModuleHandleW(NULL);
	GetModuleFileNameW(hmodule, temp_wstring, 1024);
	WideCharToMultiByte(CP_UTF8, 0, temp_wstring, -1, temp_string, 4096, NULL, NULL);
	bindir = temp_string;
#endif

#ifdef KINC_WINDOWS
	bindir = _substring(bindir, 0, string_last_index_of(bindir, "\\"));
#else
	bindir = _substring(bindir, 0, string_last_index_of(bindir, "/"));
#endif
	char *assetsdir = argc > 1 ? argv[1] : bindir;

	// Opening a file
	int l = strlen(assetsdir);
	if ((l > 6 && assetsdir[l - 6] == '.') ||
		(l > 5 && assetsdir[l - 5] == '.') ||
		(l > 4 && assetsdir[l - 4] == '.')) {
		assetsdir = bindir;
	}

	for (int i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "--nowindow") == 0) {
			enable_window = false;
		}
	}

#if !defined(KINC_MACOS) && !defined(KINC_IOS)
	kinc_internal_set_files_location(assetsdir);
#endif

#ifdef KINC_MACOS
	// Handle loading assets located outside of '.app/Contents/Resources/Deployment' folder
	// when assets and shaders dir is passed as an argument
	if (argc > 2) {
		kinc_internal_set_files_location(&assetsdir[0u]);
	}
#endif

	kinc_threads_init();
	kinc_display_init();

	// #ifdef WITH_PLUGIN_EMBED
	// plugin_embed(isolate, global);
	// #endif

	gc_start(&argc);
	_kickstart();

	#ifdef WITH_AUDIO
	kinc_a2_shutdown();
	#endif
	#ifdef WITH_ONNX
	if (ort != NULL) {
		ort->ReleaseEnv(ort_env);
		ort->ReleaseSessionOptions(ort_session_options);
	}
	#endif
	gc_stop();
	return 0;
}

i32 krom_get_arg_count() {
	return _argc;
}

string_t *krom_get_arg(i32 index) {
	return _argv[index];
}

#ifndef NO_KROM_API

#include <stdio.h>
#include <kinc/io/filewriter.h>
#include <kinc/input/mouse.h>
#include <kinc/input/surface.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/pen.h>
#include <kinc/input/gamepad.h>
#include <kinc/math/random.h>
#include <kinc/math/core.h>
#include <kinc/threads/mutex.h>
#include <kinc/network/http.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#ifdef WITH_AUDIO
#include <kinc/audio1/audio.h>
#include <kinc/audio1/sound.h>
#include <kinc/audio2/audio.h>
#endif
int LZ4_decompress_safe(const char *source, char *dest, int compressed_size, int maxOutputSize);
#define STB_IMAGE_IMPLEMENTATION
#include <kinc/libs/stb_image.h>
#ifdef KINC_DIRECT3D11
#include <d3d11.h>
#endif
#ifdef KINC_DIRECT3D12
#include <d3d12.h>
bool waitAfterNextDraw;
#endif
#if defined(KINC_DIRECT3D12) || defined(KINC_VULKAN) || defined(KINC_METAL)
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/raytrace.h>
#endif

#ifdef KINC_WINDOWS
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
//struct HWND__ *kinc_windows_window_handle(int window_index); // KINC/Windows.h
// Enable visual styles for ui controls
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#ifdef WITH_D3DCOMPILER
#include <d3d11.h>
#include <D3Dcompiler.h>
#endif
#ifdef WITH_NFD
#include <nfd.h>
#elif defined(KINC_ANDROID)
#include "android/android_file_dialog.h"
#include "android/android_http_request.h"
#elif defined(KINC_IOS)
#include "ios/ios_file_dialog.h"
#endif
#ifdef WITH_TINYDIR
#include <tinydir.h>
#endif
#ifdef WITH_STB_IMAGE_WRITE
#ifdef WITH_ZLIB
unsigned char *stbiw_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality);
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
#ifdef KINC_WINDOWS
#include <dml_provider_factory.h>
#elif defined(KINC_MACOS)
#include <coreml_provider_factory.h>
#endif
#endif
#if defined(IDLE_SLEEP) && !defined(KINC_WINDOWS)
#include <unistd.h>
#endif

#ifdef WITH_PLUGIN_EMBED
void plugin_embed();
#endif

#ifdef KINC_MACOS
const char *macgetresourcepath();
#endif
#ifdef KINC_IOS
const char *iphonegetresourcepath();
#endif

#if defined(KINC_IOS) || defined(KINC_ANDROID)
char mobile_title[1024];
#endif

#if defined(KINC_VULKAN) && defined(KRAFIX_LIBRARY)
int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
#endif

#if defined(KINC_DIRECT3D12) || defined(KINC_VULKAN) || defined(KINC_METAL)
kinc_g5_command_list_t commandList;
static kinc_g5_constant_buffer_t constant_buffer;
static kinc_g4_render_target_t *render_target;
static kinc_raytrace_pipeline_t pipeline;
static kinc_raytrace_acceleration_structure_t accel;
static bool accel_created = false;
const int constant_buffer_size = 24;
#endif

void _update(void *data) {
	#ifdef KINC_WINDOWS
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
	#if defined(KINC_IOS) || defined(KINC_ANDROID)
	int start_sleep = 1200;
	#else
	int start_sleep = 120;
	#endif
	if (++paused_frames > start_sleep && !input_down) {
		#ifdef KINC_WINDOWS
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
	krom_update();
	kinc_g4_end(0);
	kinc_g4_swap_buffers();
}

char *_copy(void *data) {
	// result = krom_copy();
	// strcpy(temp_string, result);

	#ifdef WITH_ZUI
	strcpy(temp_string, zui_copy());
	#endif
	return temp_string;
}

char *_cut(void *data) {
	// result = krom_cut();
	// strcpy(temp_string, result);

	#ifdef WITH_ZUI
	strcpy(temp_string, zui_cut());
	#endif
	return temp_string;
}

void _paste(char *text, void *data) {
	// krom_paste(text);

	#ifdef WITH_ZUI
	zui_paste(text);
	#endif
}

void _foreground(void *data) {
	krom_foreground();
	in_background = false;
}

void _resume(void *data) {
	krom_resume();
}

void _pause(void *data) {
	krom_pause();
}

void _background(void *data) {
	krom_background();
	in_background = true;
	paused_frames = 0;
}

void _shutdown(void *data) {
	krom_shutdown();
}

void _key_down(int code, void *data) {
	krom_key_down(code);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_key_down(zui_instances[i], code);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _key_up(int code, void *data) {
	krom_key_up(code);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_key_up(zui_instances[i], code);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _key_press(unsigned int character, void *data) {
	krom_key_press(character);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_key_press(zui_instances[i], character);
	}
	#endif

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _mouse_down(int window, int button, int x, int y, void *data) {
	krom_mouse_down(button, x, y);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_mouse_down(zui_instances[i], button, x, y);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _mouse_up(int window, int button, int x, int y, void *data) {
	krom_mouse_up(button, x, y);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_mouse_up(zui_instances[i], button, x, y);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _mouse_move(int window, int x, int y, int mx, int my, void *data) {
	krom_mouse_move(x, y, mx, my);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_mouse_move(zui_instances[i], x, y, mx, my);
	}
	#endif

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _mouse_wheel(int window, int delta, void *data) {
	krom_mouse_wheel(delta);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_mouse_wheel(zui_instances[i], delta);
	}
	#endif

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _touch_move(int index, int x, int y) {
	krom_touch_move(index, x, y);

	#ifdef WITH_ZUI
	#if defined(KINC_ANDROID) || defined(KINC_IOS)
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_touch_move(zui_instances[i], index, x, y);
	}
	#endif
	#endif

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _touch_down(int index, int x, int y) {
	krom_touch_down(index, x, y);

	#ifdef WITH_ZUI
	#if defined(KINC_ANDROID) || defined(KINC_IOS)
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_touch_down(zui_instances[i], index, x, y);
	}
	#endif
	#endif

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _touch_up(int index, int x, int y) {
	krom_touch_up(index, x, y);

	#ifdef WITH_ZUI
	#if defined(KINC_ANDROID) || defined(KINC_IOS)
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_touch_up(zui_instances[i], index, x, y);
	}
	#endif
	#endif

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _pen_down(int window, int x, int y, float pressure) {
	krom_pen_down(x, y, pressure);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_pen_down(zui_instances[i], x, y, pressure);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _pen_up(int window, int x, int y, float pressure) {
	krom_pen_up(x, y, pressure);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_pen_up(zui_instances[i], x, y, pressure);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _pen_move(int window, int x, int y, float pressure) {
	krom_pen_move(x, y, pressure);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) {
		zui_pen_move(zui_instances[i], x, y, pressure);
	}
	#endif

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _gamepad_axis(int gamepad, int axis, float value, void *data) {
	krom_gamepad_axis(gamepad, axis, value);

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _gamepad_button(int gamepad, int button, float value, void *data) {
	krom_gamepad_button(gamepad, button, value);

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _drop_files(wchar_t *file_path, void *data) {
// Update mouse position
#ifdef KINC_WINDOWS
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(kinc_windows_window_handle(0), &p);
	_mouse_move(0, p.x, p.y, 0, 0, NULL);
#endif

	// if (sizeof(wchar_t) == 2) {
	// 	(const uint16_t *)file_path
	// }
	// else {
	// 	size_t len = wcslen(file_path);
	// 	uint16_t *str = new uint16_t[len + 1];
	// 	for (int i = 0; i < len; i++) str[i] = file_path[i];
	// 	str[len] = 0;
	// }

	char buffer[512];
	wcstombs(buffer, file_path, sizeof(buffer));
	krom_drop_files(buffer);
	in_background = false;

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

f32 js_eval(const char *js, const char *context) {
	return 0.0;
}

char *uri_decode(const char *src) {
	char *dst = gc_alloc(1024);
	char a, b;
	while (*src) {
		if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
			if (a >= 'a') {
				a -= 'a' - 'A';
			}
			if (a >= 'A') {
				a -= ('A' - 10);
			}
			else {
				a -= '0';
			}
			if (b >= 'a') {
				b -= 'a' - 'A';
			}
			if (b >= 'A') {
				b -= ('A' - 10);
			}
			else {
				b -= '0';
			}
			*dst++ = 16 * a + b;
			src += 3;
		}
		else if (*src == '+') {
			*dst++ = ' ';
			src++;
		}
		else {
			*dst++ = *src++;
		}
	}
	*dst++ = '\0';
	return dst;
}

f32 math_floor(f32 x) { return floorf(x); }
f32 math_cos(f32 x) { return cosf(x); }
f32 math_sin(f32 x) { return sinf(x); }
f32 math_tan(f32 x) { return tanf(x); }
f32 math_sqrt(f32 x) { return sqrtf(x); }
f32 math_abs(f32 x) { return fabsf(x); }
f32 math_random() { return rand() / (float)RAND_MAX; }
f32 math_atan2(f32 y, f32 x) { return atan2f(y, x); }
f32 math_asin(f32 x) { return asinf(x); }
f32 math_pi() { return 3.14159265358979323846; }
f32 math_pow(f32 x, f32 y) { return powf(x, y); }
f32 math_round(f32 x) { return roundf(x); }
f32 math_ceil(f32 x) { return ceilf(x); }
f32 math_min(f32 x, f32 y) { return x < y ? x : y; }
f32 math_max(f32 x, f32 y) { return x > y ? x : y; }
f32 math_log(f32 x) { return logf(x); }
f32 math_log2(f32 x) { return log2f(x); }
f32 math_atan(f32 x) { return atanf(x); }
f32 math_acos(f32 x) { return acosf(x); }
f32 math_exp(f32 x) { return expf(x); }
f32 math_fmod(f32 x, f32 y) { return fmod(x, y); }

i32 parse_int(const char *s) { return strtol(s, NULL, 10); }
i32 parse_int_hex(const char *s) { return strtol(s, NULL, 16); }
f32 parse_float(const char *s) { return strtof(s, NULL); }

i32 color_from_floats(f32 r, f32 g, f32 b, f32 a) {
	return ((int)(a * 255) << 24) | ((int)(r * 255) << 16) | ((int)(g * 255) << 8) | (int)(b * 255);
}

u8 color_get_rb(i32 c) {
	return (c & 0x00ff0000) >> 16;
}

u8 color_get_gb(i32 c) {
	return (c & 0x0000ff00) >> 8;
}

u8 color_get_bb(i32 c) {
	return c & 0x000000ff;
}

u8 color_get_ab(i32 c) {
	return c & 0x000000ff;
}

i32 color_set_rb(i32 c, u8 i) {
	return (color_get_ab(c) << 24) | (i << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

i32 color_set_gb(i32 c, u8 i) {
	return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (i << 8) | color_get_bb(c);
}

i32 color_set_bb(i32 c, u8 i) {
	return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | i;
}

i32 color_set_ab(i32 c, u8 i) {
	return (i << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

// ██╗  ██╗    ██████╗      ██████╗     ███╗   ███╗
// ██║ ██╔╝    ██╔══██╗    ██╔═══██╗    ████╗ ████║
// █████╔╝     ██████╔╝    ██║   ██║    ██╔████╔██║
// ██╔═██╗     ██╔══██╗    ██║   ██║    ██║╚██╔╝██║
// ██║  ██╗    ██║  ██║    ╚██████╔╝    ██║ ╚═╝ ██║
// ╚═╝  ╚═╝    ╚═╝  ╚═╝     ╚═════╝     ╚═╝     ╚═╝

#ifdef WITH_ONNX
const OrtApi *ort = NULL;
OrtEnv *ort_env;
OrtSessionOptions *ort_session_options;
OrtSession *session = NULL;
#endif

void krom_init(string_t *title, i32 width, i32 height, bool vsync, i32 window_mode, i32 window_features, i32 x, i32 y, i32 frequency) {
	kinc_window_options_t win;
	kinc_framebuffer_options_t frame;
	win.title = title;
	win.width = width;
	win.height = height;
	frame.vertical_sync = vsync;
	win.mode = (kinc_window_mode_t)window_mode;
	win.window_features = window_features;
	win.x = x;
	win.y = y;
	frame.frequency = frequency;

	win.display_index = -1;
	#ifdef KINC_WINDOWS
	win.visible = false; // Prevent white flicker when opening the window
	#else
	win.visible = enable_window;
	#endif
	frame.color_bits = 32;
	frame.depth_bits = 0;
	frame.stencil_bits = 0;
	kinc_init(title, win.width, win.height, &win, &frame);
	kinc_random_init((int)(kinc_time() * 1000));

	#ifdef KINC_WINDOWS
	// Maximized window has x < -1, prevent window centering done by kinc
	if (win.x < -1 && win.y < -1) {
		kinc_window_move(0, win.x, win.y);
	}

	char vdata[4];
	DWORD cbdata = 4 * sizeof(char);
	RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, vdata, &cbdata);
	BOOL use_dark_mode = (int)(vdata[3] << 24 | vdata[2] << 16 | vdata[1] << 8 | vdata[0]) != 1;
	DwmSetWindowAttribute(kinc_windows_window_handle(0), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));

	show_window = true;
	#endif

	#ifdef WITH_AUDIO
	kinc_a1_init();
	kinc_a2_init();
	#endif

	#ifdef KINC_ANDROID
	android_check_permissions();
	#endif
}

void krom_set_app_name(string_t *name) {
	kinc_set_application_name(name);
}

void krom_log(string_t *value) {
	kinc_log(KINC_LOG_LEVEL_INFO, value);
}

void krom_g4_clear(i32 flags, i32 color, f32 depth) {
	kinc_g4_clear(flags, color, depth, 0);
}

void krom_set_update_callback(void (*callback)(void)) {
	krom_update = callback;
	kinc_set_update_callback(_update, NULL);
}

void krom_set_drop_files_callback(void (*callback)(char *)) {
	krom_drop_files = callback;
	kinc_set_drop_files_callback(_drop_files, NULL);
}

void krom_set_cut_copy_paste_callback(any on_cut, any on_copy, any on_paste) {
	// kinc_set_cut_callback(_cut, NULL);
	// kinc_set_copy_callback(_copy, NULL);
	// kinc_set_paste_callback(_paste, NULL);
	// krom_cut = on_cut;
	// krom_copy = on_copy;
	// krom_paste = on_paste;
}

void krom_set_application_state_callback(void (*on_foreground)(void), void (*on_resume)(void), void (*on_pause)(void), void (*on_background)(void), void (*on_shutdown)(void)) {
	kinc_set_foreground_callback(on_foreground != NULL ? _foreground : NULL, NULL);
	kinc_set_resume_callback(on_resume != NULL ? _resume : NULL, NULL);
	kinc_set_pause_callback(on_pause != NULL ? _pause : NULL, NULL);
	kinc_set_background_callback(on_background != NULL ? _background : NULL, NULL);
	kinc_set_shutdown_callback(on_shutdown != NULL ? _shutdown : NULL, NULL);
	krom_foreground = on_foreground;
	krom_resume = on_resume;
	krom_pause = on_pause;
	krom_background = on_background;
	krom_shutdown = on_shutdown;
}

void krom_set_keyboard_down_callback(void (*callback)(int)) {
	krom_key_down = callback;
	kinc_keyboard_set_key_down_callback(_key_down, NULL);
}

void krom_set_keyboard_up_callback(void (*callback)(int)) {
	krom_key_up = callback;
	kinc_keyboard_set_key_up_callback(_key_up, NULL);
}

void krom_set_keyboard_press_callback(void (*callback)(int)) {
	krom_key_press = callback;
	kinc_keyboard_set_key_press_callback(_key_press, NULL);
}

void krom_set_mouse_down_callback(void (*callback)(int, int, int)) {
	krom_mouse_down = callback;
	kinc_mouse_set_press_callback(_mouse_down, NULL);
}

void krom_set_mouse_up_callback(void (*callback)(int, int, int)) {
	krom_mouse_up = callback;
	kinc_mouse_set_release_callback(_mouse_up, NULL);
}

void krom_set_mouse_move_callback(void (*callback)(int, int, int, int)) {
	krom_mouse_move = callback;
	kinc_mouse_set_move_callback(_mouse_move, NULL);
}

void krom_set_mouse_wheel_callback(void (*callback)(int)) {
	krom_mouse_wheel = callback;
	kinc_mouse_set_scroll_callback(_mouse_wheel, NULL);
}

void krom_set_touch_down_callback(void (*callback)(int, int, int)) {
	krom_touch_down = callback;
	kinc_surface_set_touch_start_callback(_touch_down);
}

void krom_set_touch_up_callback(void (*callback)(int, int, int)) {
	krom_touch_up = callback;
	kinc_surface_set_touch_end_callback(_touch_up);
}

void krom_set_touch_move_callback(void (*callback)(int, int, int)) {
	krom_touch_move = callback;
	kinc_surface_set_move_callback(_touch_move);
}

void krom_set_pen_down_callback(void (*callback)(int, int, float)) {
	krom_pen_down = callback;
	kinc_pen_set_press_callback(_pen_down);
}

void krom_set_pen_up_callback(void (*callback)(int, int, float)) {
	krom_pen_up = callback;
	kinc_pen_set_release_callback(_pen_up);
}

void krom_set_pen_move_callback(void (*callback)(int, int, float)) {
	krom_pen_move = callback;
	kinc_pen_set_move_callback(_pen_move);
}

void krom_set_gamepad_axis_callback(void (*callback)(int, int, float)) {
	krom_gamepad_axis = callback;
	kinc_gamepad_set_axis_callback(_gamepad_axis, NULL);
}

void krom_set_gamepad_button_callback(void (*callback)(int, int, float)) {
	krom_gamepad_button = callback;
	kinc_gamepad_set_button_callback(_gamepad_button, NULL);
}

void krom_lock_mouse() {
	kinc_mouse_lock(0);
}

void krom_unlock_mouse() {
	kinc_mouse_unlock();
}

bool krom_can_lock_mouse() {
	return kinc_mouse_can_lock();
}

bool krom_is_mouse_locked() {
	return kinc_mouse_is_locked();
}

void krom_set_mouse_position(i32 x, i32 y) {
	kinc_mouse_set_position(0, x, y);
}

void krom_show_mouse(bool show) {
	show ? kinc_mouse_show() : kinc_mouse_hide();
}

void krom_show_keyboard(bool show) {
	show ? kinc_keyboard_show() : kinc_keyboard_hide();
}


any krom_g4_create_index_buffer(i32 count) {
	kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)malloc(sizeof(kinc_g4_index_buffer_t));
	kinc_g4_index_buffer_init(buffer, count, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
	return buffer;
}

void krom_g4_delete_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_destroy(buffer);
	free(buffer);
}

u32_array_t *krom_g4_lock_index_buffer(kinc_g4_index_buffer_t *buffer) {
	void *vertices = kinc_g4_index_buffer_lock_all(buffer);
	u32_array_t *ar = (u32_array_t *)malloc(sizeof(u32_array_t));
	ar->buffer = vertices;
	ar->length = kinc_g4_index_buffer_count(buffer);
	return ar;
}

void krom_g4_unlock_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_unlock_all(buffer);
}

void krom_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_set_index_buffer(buffer);
}

typedef struct kinc_vertex_elem {
	char *name;
	int data; // vertex_data_t
} kinc_vertex_elem_t;

any krom_g4_create_vertex_buffer(i32 count, any_array_t *elements, i32 usage, i32 inst_data_step_rate) {
	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	for (int32_t i = 0; i < elements->length; ++i) {
		kinc_vertex_elem_t *element = elements->buffer[i];
		char *str = element->name;
		int32_t data = element->data;
		strcpy(temp_string_vstruct[0][i], str);
		kinc_g4_vertex_structure_add(&structure, temp_string_vstruct[0][i], (kinc_g4_vertex_data_t)data);
	}
	kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)malloc(sizeof(kinc_g4_vertex_buffer_t));
	kinc_g4_vertex_buffer_init(buffer, count, &structure, (kinc_g4_usage_t)usage, inst_data_step_rate);
	return buffer;
}

void krom_g4_delete_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_destroy(buffer);
	free(buffer);
}

buffer_t *krom_g4_lock_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	float *vertices = kinc_g4_vertex_buffer_lock_all(buffer);
	buffer_t *b = (buffer_t *)malloc(sizeof(buffer_t));
	b->buffer = vertices;
	b->length = kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer);
	return b;
}

void krom_g4_unlock_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_unlock_all(buffer);
}

void krom_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_set_vertex_buffer(buffer);
}

void krom_g4_set_vertex_buffers(any_array_t *vertex_buffers) {
	kinc_g4_set_vertex_buffers(vertex_buffers->buffer, vertex_buffers->length);
}

void krom_g4_draw_indexed_vertices(i32 start, i32 count) {
	#ifdef KINC_DIRECT3D12
	// TODO: Prevent heapIndex overflow in texture.c.h/kinc_g5_internal_set_textures
	waitAfterNextDraw = true;
	#endif
	if (count < 0) {
		kinc_g4_draw_indexed_vertices();
	}
	else {
		kinc_g4_draw_indexed_vertices_from_to(start, count);
	}
}

void krom_g4_draw_indexed_vertices_instanced(i32 instance_count, i32 start, i32 count) {
	if (count < 0) {
		kinc_g4_draw_indexed_vertices_instanced(instance_count);
	}
	else {
		kinc_g4_draw_indexed_vertices_instanced_from_to(instance_count, start, count);
	}
}

kinc_g4_shader_t *krom_g4_create_shader(buffer_t *data, i32 shader_type) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, data->buffer, data->length, (kinc_g4_shader_type_t)shader_type);
	return shader;
}

kinc_g4_shader_t *krom_g4_create_vertex_shader_from_source(string_t *source) {

#ifdef WITH_D3DCOMPILER

	strcpy(temp_string_vs, source);

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
	HRESULT hr = D3DCompile(temp_string_vs, strlen(source) + 1, NULL, NULL, NULL, "main", "vs_5_0", flags, 0, &shader_buffer, &error_message);
	if (hr != S_OK) {
		kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char *)error_message->lpVtbl->GetBufferPointer(error_message));
		return NULL;
	}

	ID3D11ShaderReflection *reflector = NULL;
	D3DReflect(shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer), &IID_ID3D11ShaderReflection, (void **)&reflector);

	int size = shader_buffer->lpVtbl->GetBufferSize(shader_buffer);
	char *file = malloc(size * 2);
	int output_len = 0;

	bool hasBone = strstr(temp_string_vs, "bone :") != NULL;
	bool hasCol = strstr(temp_string_vs, "col :") != NULL;
	bool hasNor = strstr(temp_string_vs, "nor :") != NULL;
	bool hasPos = strstr(temp_string_vs, "pos :") != NULL;
	bool hasTex = strstr(temp_string_vs, "tex :") != NULL;

	i32_map_t *attributes = calloc(sizeof(i32_map_t), 1);
	int index = 0;
	if (hasBone) i32_map_set(attributes, "bone", index++);
	if (hasCol) i32_map_set(attributes, "col", index++);
	if (hasNor) i32_map_set(attributes, "nor", index++);
	if (hasPos) i32_map_set(attributes, "pos", index++);
	if (hasTex) i32_map_set(attributes, "tex", index++);
	if (hasBone) i32_map_set(attributes, "weight", index++);

	file[output_len] = (char)index;
	output_len += 1;

	any_array_t *keys = map_keys(attributes);
	for (int i = 0; i < keys->length; ++i) {
		strcpy(file + output_len, keys->buffer[i]);
		output_len += strlen(keys->buffer[i]);
		file[output_len] = 0;
		output_len += 1;
		file[output_len] = i32_map_get(attributes, keys->buffer[i]);
		output_len += 1;
	}

	D3D11_SHADER_DESC desc;
	reflector->lpVtbl->GetDesc(reflector, &desc);

	file[output_len] = desc.BoundResources;
	output_len += 1;
	for (int i = 0; i < desc.BoundResources; ++i) {
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflector->lpVtbl->GetResourceBindingDesc(reflector, i, &bindDesc);
		strcpy(file + output_len, bindDesc.Name);
		output_len += strlen(bindDesc.Name);
		file[output_len] = 0;
		output_len += 1;
		file[output_len] = bindDesc.BindPoint;
		output_len += 1;
	}

	ID3D11ShaderReflectionConstantBuffer *constants = reflector->lpVtbl->GetConstantBufferByName(reflector, "$Globals");
	D3D11_SHADER_BUFFER_DESC bufferDesc;
	hr = constants->lpVtbl->GetDesc(constants, &bufferDesc);
	if (hr == S_OK) {
		file[output_len] = bufferDesc.Variables;
		output_len += 1;
		for (int i = 0; i < bufferDesc.Variables; ++i) {
			ID3D11ShaderReflectionVariable *variable = constants->lpVtbl->GetVariableByIndex(constants, i);
			D3D11_SHADER_VARIABLE_DESC variableDesc;
			hr = variable->lpVtbl->GetDesc(variable, &variableDesc);
			if (hr == S_OK) {
				strcpy(file + output_len, variableDesc.Name);
				output_len += strlen(variableDesc.Name);
				file[output_len] = 0;
				output_len += 1;

				file[output_len] = (char *)(&variableDesc.StartOffset)[0];
				file[output_len + 1] = (char *)(&variableDesc.StartOffset)[1];
				file[output_len + 2] = (char *)(&variableDesc.StartOffset)[2];
				file[output_len + 3] = (char *)(&variableDesc.StartOffset)[3];
				output_len += 4;

				file[output_len] = (char *)(&variableDesc.Size)[0];
				file[output_len + 1] = (char *)(&variableDesc.Size)[1];
				file[output_len + 2] = (char *)(&variableDesc.Size)[2];
				file[output_len + 3] = (char *)(&variableDesc.Size)[3];
				output_len += 4;

				D3D11_SHADER_TYPE_DESC typeDesc;
				ID3D11ShaderReflectionType *type = variable->lpVtbl->GetType(variable);
				hr = type->lpVtbl->GetDesc(type, &typeDesc);
				if (hr == S_OK) {
					file[output_len] = typeDesc.Columns;
					output_len += 1;
					file[output_len] = typeDesc.Rows;
					output_len += 1;
				}
				else {
					file[output_len] = 0;
					output_len += 1;
					file[output_len] = 0;
					output_len += 1;
				}
			}
		}
	}
	else {
		file[output_len] = 0;
		output_len += 1;
	}

	memcpy(file + output_len, (char *)shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer));
	output_len += shader_buffer->lpVtbl->GetBufferSize(shader_buffer);

	shader_buffer->lpVtbl->Release(shader_buffer);
	reflector->lpVtbl->Release(reflector);

	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, file, (int)output_len, KINC_G4_SHADER_TYPE_VERTEX);
	free(file);

	#elif defined(KINC_METAL)

	strcpy(temp_string_vs, "// my_main\n");
	strcat(temp_string_vs, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, temp_string_vs, strlen(temp_string_vs), KINC_G4_SHADER_TYPE_VERTEX);

	#elif defined(KINC_VULKAN) && defined(KRAFIX_LIBRARY)

	char *output = new char[1024 * 1024];
	int length;
	krafix_compile(source, output, &length, "spirv", "windows", "vert", -1);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_VERTEX);

	#else

	char *source_ = malloc(strlen(source) + 1);
	strcpy(source_, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, source_, strlen(source_), KINC_G4_SHADER_TYPE_VERTEX);

	#endif

	return shader;
}

kinc_g4_shader_t *krom_g4_create_fragment_shader_from_source(string_t *source) {

	#ifdef WITH_D3DCOMPILER

	strcpy(temp_string_fs, source);

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
	HRESULT hr = D3DCompile(temp_string_fs, strlen(source) + 1, NULL, NULL, NULL, "main", "ps_5_0", flags, 0, &shader_buffer, &error_message);
	if (hr != S_OK) {
		kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char *)error_message->lpVtbl->GetBufferPointer(error_message));
		return NULL;
	}

	ID3D11ShaderReflection *reflector = NULL;
	D3DReflect(shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer), &IID_ID3D11ShaderReflection, (void **)&reflector);

	int size = shader_buffer->lpVtbl->GetBufferSize(shader_buffer);
	char *file = malloc(size * 2);
	int output_len = 0;
	file[output_len] = 0;
	output_len += 1;

	D3D11_SHADER_DESC desc;
	reflector->lpVtbl->GetDesc(reflector, &desc);

	file[output_len] = desc.BoundResources;
	output_len += 1;
	for (int i = 0; i < desc.BoundResources; ++i) {
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflector->lpVtbl->GetResourceBindingDesc(reflector, i, &bindDesc);
		strcpy(file + output_len, bindDesc.Name);
		output_len += strlen(bindDesc.Name);
		file[output_len] = 0;
		output_len += 1;
		file[output_len] = bindDesc.BindPoint;
		output_len += 1;
	}

	ID3D11ShaderReflectionConstantBuffer *constants = reflector->lpVtbl->GetConstantBufferByName(reflector, "$Globals");
	D3D11_SHADER_BUFFER_DESC bufferDesc;
	hr = constants->lpVtbl->GetDesc(constants, &bufferDesc);
	if (hr == S_OK) {
		file[output_len] = bufferDesc.Variables;
		output_len += 1;
		for (int i = 0; i < bufferDesc.Variables; ++i) {
			ID3D11ShaderReflectionVariable *variable = constants->lpVtbl->GetVariableByIndex(constants, i);
			D3D11_SHADER_VARIABLE_DESC variableDesc;
			hr = variable->lpVtbl->GetDesc(variable, &variableDesc);
			if (hr == S_OK) {
				strcpy(file + output_len, variableDesc.Name);
				output_len += strlen(variableDesc.Name);
				file[output_len] = 0;
				output_len += 1;

				file[output_len] = (char *)(&variableDesc.StartOffset)[0];
				file[output_len + 1] = (char *)(&variableDesc.StartOffset)[1];
				file[output_len + 2] = (char *)(&variableDesc.StartOffset)[2];
				file[output_len + 3] = (char *)(&variableDesc.StartOffset)[3];
				output_len += 4;

				file[output_len] = (char *)(&variableDesc.Size)[0];
				file[output_len + 1] = (char *)(&variableDesc.Size)[1];
				file[output_len + 2] = (char *)(&variableDesc.Size)[2];
				file[output_len + 3] = (char *)(&variableDesc.Size)[3];
				output_len += 4;

				D3D11_SHADER_TYPE_DESC typeDesc;
				ID3D11ShaderReflectionType *type = variable->lpVtbl->GetType(variable);
				hr = type->lpVtbl->GetDesc(type, &typeDesc);
				if (hr == S_OK) {
					file[output_len] = typeDesc.Columns;
					output_len += 1;
					file[output_len] = typeDesc.Rows;
					output_len += 1;
				}
				else {
					file[output_len] = 0;
					output_len += 1;
					file[output_len] = 0;
					output_len += 1;
				}
			}
		}
	}
	else {
		file[output_len] = 0;
		output_len += 1;
	}

	memcpy(file + output_len, (char *)shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer));
	output_len += shader_buffer->lpVtbl->GetBufferSize(shader_buffer);

	shader_buffer->lpVtbl->Release(shader_buffer);
	reflector->lpVtbl->Release(reflector);

	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, file, (int)output_len, KINC_G4_SHADER_TYPE_FRAGMENT);
	free(file);

	#elif defined(KINC_METAL)

	strcpy(temp_string_fs, "// my_main\n");
	strcat(temp_string_fs, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, temp_string_fs, strlen(temp_string_fs), KINC_G4_SHADER_TYPE_FRAGMENT);

	#elif defined(KINC_VULKAN) && defined(KRAFIX_LIBRARY)

	char *output = new char[1024 * 1024];
	int length;
	krafix_compile(source, output, &length, "spirv", "windows", "frag", -1);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_FRAGMENT);

	#else

	char *source_ = malloc(strlen(source) + 1);
	strcpy(source_, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, source_, strlen(source_), KINC_G4_SHADER_TYPE_FRAGMENT);

	#endif

	return shader;
}

void krom_g4_delete_shader(kinc_g4_shader_t *shader) {
	kinc_g4_shader_destroy(shader);
}

kinc_g4_pipeline_t *krom_g4_create_pipeline() {
	kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)malloc(sizeof(kinc_g4_pipeline_t));
	kinc_g4_pipeline_init(pipeline);
	return pipeline;
}

void krom_g4_delete_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_pipeline_destroy(pipeline);
	free(pipeline);
}

typedef struct vertex_struct {
	any_array_t *elements; // kinc_vertex_elem_t
	bool instanced;
} vertex_struct_t;

typedef struct krom_pipeline_state {
	int cull_mode; // cull_mode_t;
	bool depth_write;
	int depth_mode; // compare_mode_t;
	int blend_source; // blend_factor_t;
	int blend_dest; // blend_factor_t;
	int alpha_blend_source; // blend_factor_t;
	int alpha_blend_dest; // blend_factor_t;
	u8_array_t *color_write_masks_red;
	u8_array_t *color_write_masks_green;
	u8_array_t *color_write_masks_blue;
	u8_array_t *color_write_masks_alpha;
	int color_attachment_count;
	i32_array_t *color_attachments; // tex_format_t
	int depth_attachment_bits;
} krom_pipeline_state_t;

void krom_g4_compile_pipeline(kinc_g4_pipeline_t *pipeline, vertex_struct_t *structure0, vertex_struct_t *structure1, vertex_struct_t *structure2, vertex_struct_t *structure3, i32 length, kinc_g4_shader_t *vertex_shader, kinc_g4_shader_t *fragment_shader, kinc_g4_shader_t *geometry_shader, krom_pipeline_state_t *state) {
	kinc_g4_vertex_structure_t s0, s1, s2, s3;
	kinc_g4_vertex_structure_init(&s0);
	kinc_g4_vertex_structure_init(&s1);
	kinc_g4_vertex_structure_init(&s2);
	kinc_g4_vertex_structure_init(&s3);
	kinc_g4_vertex_structure_t *structures[4] = { &s0, &s1, &s2, &s3 };

	for (int32_t i1 = 0; i1 < length; ++i1) {
		vertex_struct_t *structure = i1 == 0 ? structure0 : i1 == 1 ? structure1 : i1 == 2 ? structure2 : structure3;
		structures[i1]->instanced = structure->instanced;
		any_array_t *elements = structure->elements;
		for (int32_t i2 = 0; i2 < elements->length; ++i2) {
			kinc_vertex_elem_t *element = elements->buffer[i2];
			char *str = element->name;
			int32_t data = element->data;
			strcpy(temp_string_vstruct[i1][i2], str);
			kinc_g4_vertex_structure_add(structures[i1], temp_string_vstruct[i1][i2], (kinc_g4_vertex_data_t)data);
		}
	}

	pipeline->vertex_shader = vertex_shader;
	pipeline->fragment_shader = fragment_shader;
	if (geometry_shader != null) {
		pipeline->geometry_shader = geometry_shader;
	}
	for (int i = 0; i < length; ++i) {
		pipeline->input_layout[i] = structures[i];
	}
	pipeline->input_layout[length] = NULL;

	pipeline->cull_mode = (kinc_g4_cull_mode_t)state->cull_mode;
	pipeline->depth_write = state->depth_write;
	pipeline->depth_mode = (kinc_g4_compare_mode_t)state->depth_mode;
	pipeline->blend_source = (kinc_g4_blending_factor_t)state->blend_source;
	pipeline->blend_destination = (kinc_g4_blending_factor_t)state->blend_dest;
	pipeline->alpha_blend_source = (kinc_g4_blending_factor_t)state->alpha_blend_source;
	pipeline->alpha_blend_destination = (kinc_g4_blending_factor_t)state->alpha_blend_dest;

	u8_array_t *mask_red_array = state->color_write_masks_red;
	u8_array_t *mask_green_array = state->color_write_masks_green;
	u8_array_t *mask_blue_array = state->color_write_masks_blue;
	u8_array_t *mask_alpha_array = state->color_write_masks_alpha;

	for (int i = 0; i < 8; ++i) {
		pipeline->color_write_mask_red[i] = mask_red_array->buffer[i];
		pipeline->color_write_mask_green[i] = mask_green_array->buffer[i];
		pipeline->color_write_mask_blue[i] = mask_blue_array->buffer[i];
		pipeline->color_write_mask_alpha[i] = mask_alpha_array->buffer[i];
	}

	pipeline->color_attachment_count = state->color_attachment_count;
	i32_array_t *color_attachment_array = state->color_attachments;
	for (int i = 0; i < 8; ++i) {
		pipeline->color_attachment[i] = (kinc_g4_render_target_format_t)color_attachment_array->buffer[i];
	}
	pipeline->depth_attachment_bits = state->depth_attachment_bits;
	pipeline->stencil_attachment_bits = 0;

	kinc_g4_pipeline_compile(pipeline);
}

void krom_g4_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_set_pipeline(pipeline);
}

bool _load_image(kinc_file_reader_t *reader, const char *filename, unsigned char **output, int *width, int *height, kinc_image_format_t *format) {
	*format = KINC_IMAGE_FORMAT_RGBA32;
	int size = (int)kinc_file_reader_size(reader);
	bool success = true;
	unsigned char *data = (unsigned char *)malloc(size);
	kinc_file_reader_read(reader, data, size);
	kinc_file_reader_close(reader);

	if (ends_with(filename, "k")) {
		*width = kinc_read_s32le(data);
		*height = kinc_read_s32le(data + 4);
		char fourcc[5];
		fourcc[0] = data[8];
		fourcc[1] = data[9];
		fourcc[2] = data[10];
		fourcc[3] = data[11];
		fourcc[4] = 0;
		int compressed_size = size - 12;
		if (strcmp(fourcc, "LZ4 ") == 0) {
			int output_size = *width * *height * 4;
			*output = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)(data + 12), (char *)*output, compressed_size, output_size);
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			int output_size = *width * *height * 16;
			*output = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)(data + 12), (char *)*output, compressed_size, output_size);
			*format = KINC_IMAGE_FORMAT_RGBA128;

			#ifdef KINC_IOS // No RGBA128 filtering, convert to RGBA64
			uint32_t *_output32 = (uint32_t *)*output;
			unsigned char *_output = (unsigned char *)malloc(output_size / 2);
			uint16_t *_output16 = (uint16_t *)_output;
			for (int i = 0; i < output_size / 4; ++i) {
				uint32_t x = *((uint32_t *)&_output32[i]);
				_output16[i] = ((x >> 16) & 0x8000) | ((((x & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((x >> 13) & 0x03ff);
			}
			*format = KINC_IMAGE_FORMAT_RGBA64;
			free(*output);
			*output = _output;
			#endif
		}
		else {
			success = false;
		}
	}
	else if (ends_with(filename, "hdr")) {
		int comp;
		*output = (unsigned char *)stbi_loadf_from_memory(data, size, width, height, &comp, 4);
		if (*output == NULL) {
			kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			success = false;
		}
		*format = KINC_IMAGE_FORMAT_RGBA128;
	}
	else { // jpg, png, ..
		int comp;
		*output = stbi_load_from_memory(data, size, width, height, &comp, 4);
		if (*output == NULL) {
			kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			success = false;
		}
	}
	free(data);
	return success;
}

kinc_g4_texture_t *krom_load_image(string_t *file, bool readable) {
	kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));
	kinc_file_reader_t reader;
	if (kinc_file_reader_open(&reader, file, KINC_FILE_TYPE_ASSET)) {
		unsigned char *image_data;
		int image_width;
		int image_height;
		kinc_image_format_t image_format;
		if (!_load_image(&reader, file, &image_data, &image_width, &image_height, &image_format)) {
			free(image);
			return NULL;
		}
		kinc_image_init(image, image_data, image_width, image_height, image_format);
	}
	else {
		free(image);
		return NULL;
	}

	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init_from_image(texture, image);
	if (!readable) {
		free(image->data);
		kinc_image_destroy(image);
		free(image);
	}

	return texture;
}

typedef struct image {
	any texture_;
	any render_target_;
	int format; // tex_format_t;
	bool readable;
	buffer_t *pixels;
	int width;
	int height;
	int depth;
} image_t;

void krom_unload_image(image_t *image) {
	if (image == NULL) {
		return;
	}
	if (image->texture_ != NULL) {
		kinc_g4_texture_destroy(image->texture_);
		free(image->texture_);
	}
	else if (image->render_target_ != NULL) {
		kinc_g4_render_target_destroy(image->render_target_);
		free(image->render_target_);
	}
}

#ifdef WITH_AUDIO
any krom_load_sound(string_t *file) {
	kinc_a1_sound_t *sound = kinc_a1_sound_create(file);
	return sound;
}

void krom_unload_sound(kinc_a1_sound_t *sound) {
	kinc_a1_sound_destroy(sound);
}

void krom_play_sound(kinc_a1_sound_t *sound, bool loop) {
	kinc_a1_play_sound(sound, loop, 1.0, false);
}

void krom_stop_sound(kinc_a1_sound_t *sound) {
	kinc_a1_stop_sound(sound);
}
#endif

buffer_t *krom_load_blob(string_t *file) {
	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, file, KINC_FILE_TYPE_ASSET)) {
		return NULL;
	}
	uint32_t reader_size = (uint32_t)kinc_file_reader_size(&reader);
	buffer_t *buffer = buffer_create(reader_size);
	kinc_file_reader_read(&reader, buffer->buffer, reader_size);
	kinc_file_reader_close(&reader);
	return buffer;
}

void krom_load_url(string_t *url) {
	kinc_load_url(url);
}

void krom_copy_to_clipboard(string_t *text) {
	kinc_copy_to_clipboard(text);
}


kinc_g4_constant_location_t *krom_g4_get_constant_location(kinc_g4_pipeline_t *pipeline, string_t *name) {
	kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, name);
	kinc_g4_constant_location_t *location_copy = (kinc_g4_constant_location_t *)malloc(sizeof(kinc_g4_constant_location_t));
	memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
	return location_copy;
}

kinc_g4_texture_unit_t *krom_g4_get_texture_unit(kinc_g4_pipeline_t *pipeline, string_t *name) {
	kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, name);
	kinc_g4_texture_unit_t *unit_copy = (kinc_g4_texture_unit_t *)malloc(sizeof(kinc_g4_texture_unit_t));
	memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
	return unit_copy;
}

void krom_g4_set_texture(kinc_g4_texture_unit_t *unit, kinc_g4_texture_t *texture) {
	kinc_g4_set_texture(*unit, texture);
}

void krom_g4_set_render_target(kinc_g4_texture_unit_t *unit, kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_use_color_as_texture(render_target, *unit);
}

void krom_g4_set_texture_depth(kinc_g4_texture_unit_t *unit, kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_use_depth_as_texture(render_target, *unit);
}

void krom_g4_set_image_texture(kinc_g4_texture_unit_t *unit, kinc_g4_texture_t *texture) {
	kinc_g4_set_image_texture(*unit, texture);
}

void krom_g4_set_texture_parameters(kinc_g4_texture_unit_t *unit, i32 u_addr, i32 v_addr, i32 min_filter, i32 mag_filter, i32 mip_filter) {
	kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)u_addr);
	kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)v_addr);
	kinc_g4_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)min_filter);
	kinc_g4_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)mag_filter);
	kinc_g4_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)mip_filter);
}

void krom_g4_set_texture3d_parameters(kinc_g4_texture_unit_t *unit, i32 u_addr, i32 v_addr, i32 w_addr, i32 min_filter, i32 mag_filter, i32 mip_filter) {
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)u_addr);
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)v_addr);
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)w_addr);
	kinc_g4_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)min_filter);
	kinc_g4_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)mag_filter);
	kinc_g4_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)mip_filter);
}

void krom_g4_set_bool(kinc_g4_constant_location_t *location, bool value) {
	kinc_g4_set_bool(*location, value != 0);
}

void krom_g4_set_int(kinc_g4_constant_location_t *location, i32 value) {
	kinc_g4_set_int(*location, value);
}

void krom_g4_set_float(kinc_g4_constant_location_t *location, f32 value) {
	kinc_g4_set_float(*location, value);
}

void krom_g4_set_float2(kinc_g4_constant_location_t *location, f32 value1, f32 value2) {
	kinc_g4_set_float2(*location, value1, value2);
}

void krom_g4_set_float3(kinc_g4_constant_location_t *location, f32 value1, f32 value2, f32 value3) {
	kinc_g4_set_float3(*location, value1, value2, value3);
}

void krom_g4_set_float4(kinc_g4_constant_location_t *location, f32 value1, f32 value2, f32 value3, f32 value4) {
	kinc_g4_set_float4(*location, value1, value2, value3, value4);
}

void krom_g4_set_floats(kinc_g4_constant_location_t *location, buffer_t *values) {
	kinc_g4_set_floats(*location, (float *)values->buffer, (int)(values->length / 4));
}

void krom_g4_set_matrix4(kinc_g4_constant_location_t *location, /*buffer_t*/ float *matrix) {
	kinc_g4_set_matrix4(*location, (kinc_matrix4x4_t *)matrix);
}

void krom_g4_set_matrix3(kinc_g4_constant_location_t *location, /*buffer_t*/ float *matrix) {
	kinc_g4_set_matrix3(*location, (kinc_matrix3x3_t *)matrix);
}


f32 krom_get_time() {
	return kinc_time();
}

i32 krom_window_width() {
	return kinc_window_width(0);
}

i32 krom_window_height() {
	return kinc_window_height(0);
}

void krom_set_window_title(string_t *title) {
	kinc_window_set_title(0, title);
	#if defined(KINC_IOS) || defined(KINC_ANDROID)
	strcpy(mobile_title, *title);
	#endif
}

i32 krom_get_window_mode() {
	return kinc_window_get_mode(0);
}

void krom_set_window_mode(i32 mode) {
	kinc_window_change_mode(0, (kinc_window_mode_t)mode);
}

void krom_resize_window(i32 width, i32 height) {
	kinc_window_resize(0, width, height);
}

void krom_move_window(i32 x, i32 y) {
	kinc_window_move(0, x, y);
}

i32 krom_screen_dpi() {
	return kinc_display_current_mode(kinc_primary_display()).pixels_per_inch;
}

string_t *krom_system_id() {
	return kinc_system_id();
}

void krom_request_shutdown() {
	kinc_stop();

	#ifdef KINC_LINUX
	exit(1);
	#endif
}

i32 krom_display_count() {
	return kinc_count_displays();
}

i32 krom_display_width(i32 index) {
	return kinc_display_current_mode(index).width;
}

i32 krom_display_height(i32 index) {
	return kinc_display_current_mode(index).height;
}

i32 krom_display_x(i32 index) {
	return kinc_display_current_mode(index).x;
}

i32 krom_display_y(i32 index) {
	return kinc_display_current_mode(index).y;
}

i32 krom_display_frequency(i32 index) {
	return kinc_display_current_mode(index).frequency;
}

bool krom_display_is_primary(i32 index) {
	#ifdef KINC_LINUX // TODO: Primary display detection broken in Kinc
	return true;
	#else
	return index == kinc_primary_display();
	#endif
}

void krom_write_storage(string_t *name, buffer_t *data) {
	kinc_file_writer_t writer;
	kinc_file_writer_open(&writer, name);
	kinc_file_writer_write(&writer, data->buffer, data->length);
	kinc_file_writer_close(&writer);
}

buffer_t *krom_read_storage(string_t *name) {
	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, name, KINC_FILE_TYPE_SAVE)) {
		return NULL;
	}
	int reader_size = (int)kinc_file_reader_size(&reader);
	buffer_t *buffer = buffer_create(reader_size);
	kinc_file_reader_read(&reader, buffer->buffer, reader_size);
	kinc_file_reader_close(&reader);
	return buffer;
}


kinc_g4_render_target_t *krom_g4_create_render_target(i32 width, i32 height, i32 format, i32 depth_buffer_bits, i32 stencil_buffer_bits) {
	kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
	kinc_g4_render_target_init(render_target, width, height, (kinc_g4_render_target_format_t)format, depth_buffer_bits, stencil_buffer_bits);
	return render_target;
}

kinc_g4_texture_t *krom_g4_create_texture(i32 width, i32 height, i32 format) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init(texture, width, height, (kinc_image_format_t)format);
	return texture;
}

kinc_g4_texture_t *krom_g4_create_texture3d(i32 width, i32 height, i32 depth, i32 format) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init3d(texture, width, height, depth, (kinc_image_format_t)format);
	return texture;
}

kinc_g4_texture_t *krom_g4_create_texture_from_bytes(buffer_t *data, i32 width, i32 height, i32 format, bool readable) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));
	void *image_data;
	if (readable) {
		image_data = malloc(data->length);
		memcpy(image_data, data->buffer, data->length);
	}
	else {
		image_data = data->buffer;
	}

	kinc_image_init(image, image_data, width, height, (kinc_image_format_t)format);
	kinc_g4_texture_init_from_image(texture, image);
	if (!readable) {
		kinc_image_destroy(image);
		free(image);
	}
	// else {
	// 	obj->SetInternalField(1, External::New(isolate, image));
	// }
	return texture;
}

kinc_g4_texture_t *krom_g4_create_texture_from_bytes3d(buffer_t *data, i32 width, i32 height, i32 depth, i32 format, bool readable) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));
	void *image_data;
	if (readable) {
		image_data = malloc(data->length);
		memcpy(image_data, data->buffer, data->length);
	}
	else {
		image_data = data->buffer;
	}

	kinc_image_init3d(image, image_data, width, height, depth, (kinc_image_format_t)format);
	kinc_g4_texture_init_from_image3d(texture, image);

	if (!readable) {
		kinc_image_destroy(image);
		free(image);
	}
	// else {
	// 	obj->SetInternalField(1, External::New(isolate, image));
	// }
	return texture;
}

kinc_g4_texture_t *krom_g4_create_texture_from_encoded_bytes(buffer_t *data, string_t *format, bool readable) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));

	unsigned char *content_data = (unsigned char *)data->buffer;
	int content_length = (int)data->length;
	unsigned char *image_data;
	kinc_image_format_t image_format;
	int image_width;
	int image_height;

	if (ends_with(format, "k")) {
		image_width = kinc_read_s32le(content_data);
		image_height = kinc_read_s32le(content_data + 4);
		char fourcc[5];
		fourcc[0] = content_data[8];
		fourcc[1] = content_data[9];
		fourcc[2] = content_data[10];
		fourcc[3] = content_data[11];
		fourcc[4] = 0;
		int compressed_size = content_length - 12;
		if (strcmp(fourcc, "LZ4 ") == 0) {
			int output_size = image_width * image_height * 4;
			image_data = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)content_data + 12, (char *)image_data, compressed_size, output_size);
			image_format = KINC_IMAGE_FORMAT_RGBA32;
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			int output_size = image_width * image_height * 16;
			image_data = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)content_data + 12, (char *)image_data, compressed_size, output_size);
			image_format = KINC_IMAGE_FORMAT_RGBA128;
		}
	}
	else if (ends_with(format, "hdr")) {
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
	// else {
	// 	obj->SetInternalField(1, External::New(isolate, image));
	// }

	return texture;
}

int _format_byte_size(kinc_image_format_t format) {
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

buffer_t *krom_g4_get_texture_pixels(kinc_image_t *image) {
	uint8_t *data = kinc_image_get_pixels(image);
	int byte_length = _format_byte_size(image->format) * image->width * image->height * image->depth;
	buffer_t *buffer = malloc(sizeof(buffer_t));
	buffer->buffer = data;
	buffer->length = byte_length;
	return buffer;
}

void krom_g4_get_render_target_pixels(kinc_g4_render_target_t *rt, buffer_t *data) {
	uint8_t *b = (uint8_t *)data->buffer;
	kinc_g4_render_target_get_pixels(rt, b);

	// Release staging texture immediately to save memory
	#ifdef KINC_DIRECT3D11
	rt->impl.textureStaging->lpVtbl->Release(rt->impl.textureStaging);
	rt->impl.textureStaging = NULL;
	#elif defined(KINC_DIRECT3D12)
	rt->impl._renderTarget.impl.renderTargetReadback->lpVtbl->Release(rt->impl._renderTarget.impl.renderTargetReadback);
	rt->impl._renderTarget.impl.renderTargetReadback = NULL;
	#elif defined(KINC_METAL)
	// id<MTLTexture> texReadback = (__bridge_transfer id<MTLTexture>)rt->impl._renderTarget.impl._texReadback;
	// texReadback = nil;
	// rt->impl._renderTarget.impl._texReadback = NULL;
	#endif
}

buffer_t *krom_g4_lock_texture(kinc_g4_texture_t *texture, i32 level) {
	uint8_t *tex = kinc_g4_texture_lock(texture);
	int stride = kinc_g4_texture_stride(texture);
	int byte_length = stride * texture->tex_height * texture->tex_depth;
	buffer_t *buffer = malloc(sizeof(buffer_t));
	buffer->buffer = tex;
	buffer->length = byte_length;
	return buffer;
}

void krom_g4_unlock_texture(kinc_g4_texture_t *texture) {
	kinc_g4_texture_unlock(texture);
}

void krom_g4_clear_texture(kinc_g4_texture_t *texture, i32 x, i32 y, i32 z, i32 width, i32 height, i32 depth, i32 color) {
	kinc_g4_texture_clear(texture, x, y, z, width, height, depth, color);
}

void krom_g4_generate_texture_mipmaps(kinc_g4_texture_t *texture, i32 levels) {
	kinc_g4_texture_generate_mipmaps(texture, levels);
}

void krom_g4_generate_render_target_mipmaps(kinc_g4_render_target_t *render_target, i32 levels) {
	kinc_g4_render_target_generate_mipmaps(render_target, levels);
}

void krom_g4_set_mipmaps(kinc_g4_texture_t *texture, any_array_t *mipmaps) {
	for (int32_t i = 0; i < mipmaps->length; ++i) {
		image_t *img = mipmaps->buffer[i];
		kinc_g4_texture_t *mipmapobj = img->texture_;
		// kinc_image_t *mipmap = (kinc_image_t *)mipmapobj internal
		// kinc_g4_texture_set_mipmap(texture, mipmap, i + 1);
	}
}

void krom_g4_set_depth_from(kinc_g4_render_target_t *target, kinc_g4_render_target_t *source) {
	kinc_g4_render_target_set_depth_stencil_from(target, source);
}

void krom_g4_viewport(i32 x, i32 y, i32 width, i32 height) {
	kinc_g4_viewport(x, y, width, height);
}

void krom_g4_scissor(i32 x, i32 y, i32 width, i32 height) {
	kinc_g4_scissor(x, y, width, height);
}

void krom_g4_disable_scissor() {
	kinc_g4_disable_scissor();
}

bool krom_g4_render_targets_inverted_y() {
	return kinc_g4_render_targets_inverted_y();
}

void krom_g4_begin(image_t *render_target, any_array_t *additional) {
	if (render_target == NULL) {
		kinc_g4_restore_render_target();
	}
	else {
		kinc_g4_render_target_t *rt = (kinc_g4_render_target_t *)render_target->render_target_;
		int32_t length = 1;
		kinc_g4_render_target_t *render_targets[8] = { rt, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		if (additional != NULL) {
			length = additional->length + 1;
			if (length > 8) {
				length = 8;
			}
			for (int32_t i = 1; i < length; ++i) {
				image_t *img = additional->buffer[i - 1];
				kinc_g4_render_target_t *art = (kinc_g4_render_target_t *)img->render_target_;
				render_targets[i] = art;
			}
		}
		kinc_g4_set_render_targets(render_targets, length);
	}
}

void krom_g4_end() {

}

void krom_g4_swap_buffers() {
	kinc_g4_end(0);
	kinc_g4_swap_buffers();
	kinc_g4_begin(0);
}

void krom_file_save_bytes(string_t *path, buffer_t *bytes, i32 length) {
	int byte_length = length > 0 ? length : (int)bytes->length;
	if (byte_length > (int)bytes->length) {
		byte_length = (int)bytes->length;
	}

	#ifdef KINC_WINDOWS
	MultiByteToWideChar(CP_UTF8, 0, path, -1, temp_wstring, 1024);
	FILE *file = _wfopen(temp_wstring, L"wb");
	#else
	FILE *file = fopen(path, "wb");
	#endif
	if (file == NULL) {
		return;
	}
	fwrite(bytes->buffer, 1, byte_length, file);
	fclose(file);
}

i32 krom_sys_command(string_t *cmd) {
	#ifdef KINC_WINDOWS
	int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd, -1, NULL, 0);
	wchar_t *wstr = malloc(sizeof(wchar_t) * wlen);
	MultiByteToWideChar(CP_UTF8, 0, cmd, -1, wstr, wlen);
	int result = _wsystem(wstr);
	free(wstr);
	#elif defined(KINC_IOS)
	int result = 0;
	#else
	int result = system(cmd);
	#endif
	return result;
}

string_t *krom_save_path() {
	return kinc_internal_save_path();
}

string_t *krom_get_files_location() {
	#ifdef KINC_MACOS
	char path[1024];
	strcpy(path, macgetresourcepath());
	strcat(path, "/");
	strcat(path, KINC_DEBUGDIR);
	strcat(path, "/");
	return path;
	#elif defined(KINC_IOS)
	char path[1024];
	strcpy(path, iphonegetresourcepath());
	strcat(path, "/");
	strcat(path, KINC_DEBUGDIR);
	strcat(path, "/");
	return path;
	#else
	return kinc_internal_get_files_location();
	#endif
}

typedef struct _callback_data {
	int32_t size;
	char url[512];
	void (*func)(char *, buffer_t *);
} _callback_data_t;

void _http_callback(int error, int response, const char *body, void *callback_data) {
	_callback_data_t *cbd = (_callback_data_t *)callback_data;
	buffer_t *buffer = NULL;
	if (body != NULL) {
		buffer = malloc(sizeof(buffer_t));
		buffer->length = cbd->size > 0 ? cbd->size : strlen(body);
		buffer->buffer = body;
	}
	cbd->func(cbd->url, buffer);
	free(cbd);
}

void krom_http_request(string_t *url, i32 size, void (*callback)(char *, buffer_t *)) {
	_callback_data_t *cbd = malloc(sizeof(_callback_data_t));
	cbd->size = size;
	strcpy(cbd->url, url);
	cbd->func = callback;

	char url_base[512];
	char url_path[512];
	const char *curl = url;
	int i = 0;
	for (; i < strlen(curl) - 8; ++i) {
		if (curl[i + 8] == '/') {
			break;
		}
		url_base[i] = curl[i + 8]; // Strip https://
	}
	url_base[i] = 0;
	int j = 0;
	if (strlen(url_base) < strlen(curl) - 8) {
		++i; // Skip /
	}
	for (; j < strlen(curl) - 8 - i; ++j) {
		if (curl[i + 8 + j] == 0) {
			break;
		}
		url_path[j] = curl[i + 8 + j];
	}
	url_path[j] = 0;
	#ifdef KINC_ANDROID // TODO: move to Kinc
	android_http_request(curl, url_path, NULL, 443, true, 0, NULL, &_http_callback, cbd);
	#else
	kinc_http_request(url_base, url_path, NULL, 443, true, 0, NULL, &_http_callback, cbd);
	#endif
}

#ifdef WITH_G2
void krom_g2_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *colored_vert, buffer_t *colored_frag, buffer_t *text_vert, buffer_t *text_frag) {
	arm_g2_init(image_vert->buffer, image_vert->length, image_frag->buffer, image_frag->length, colored_vert->buffer, colored_vert->length, colored_frag->buffer, colored_frag->length, text_vert->buffer, text_vert->length, text_frag->buffer, text_frag->length);
}

void krom_g2_begin() {
	arm_g2_begin();
}

void krom_g2_end() {
	arm_g2_end();
}

void krom_g2_draw_scaled_sub_image(image_t *image, f32 sx, f32 sy, f32 sw, f32 sh, f32 dx, f32 dy, f32 dw, f32 dh) {
	#ifdef KINC_DIRECT3D12
	waitAfterNextDraw = true;
	#endif
	if (image->texture_ != NULL) {
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)image->texture_;
		arm_g2_draw_scaled_sub_image(texture, sx, sy, sw, sh, dx, dy, dw, dh);
	}
	else {
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)image->render_target_;
		arm_g2_draw_scaled_sub_render_target(render_target, sx, sy, sw, sh, dx, dy, dw, dh);
	}
}

void krom_g2_fill_triangle(f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2) {
	arm_g2_fill_triangle(x0, y0, x1, y1, x2, y2);
}

void krom_g2_fill_rect(f32 x, f32 y, f32 width, f32 height) {
	arm_g2_fill_rect(x, y, width, height);
}

void krom_g2_draw_rect(f32 x, f32 y, f32 width, f32 height, f32 strength) {
	arm_g2_draw_rect(x, y, width, height, strength);
}

void krom_g2_draw_line(f32 x0, f32 y0, f32 x1, f32 y1, f32 strength) {
	arm_g2_draw_line(x0, y0, x1, y1, strength);
}

void krom_g2_draw_string(string_t *text, f32 x, f32 y) {
	arm_g2_draw_string(text, x, y);
}

void krom_g2_set_font(arm_g2_font_t *font, i32 size) {
	arm_g2_set_font(font, size);
}

arm_g2_font_t *krom_g2_font_init(buffer_t *blob, i32 font_index) {
	arm_g2_font_t *font = (arm_g2_font_t *)malloc(sizeof(arm_g2_font_t));
	arm_g2_font_init(font, blob->buffer, font_index);
	return font;
}

arm_g2_font_t *krom_g2_font_13(buffer_t *blob) {
	arm_g2_font_t *font = (arm_g2_font_t *)malloc(sizeof(arm_g2_font_t));
	arm_g2_font_13(font, blob->buffer);
	return font;
}

void krom_g2_font_set_glyphs(i32_array_t *glyphs) {
	arm_g2_font_set_glyphs(glyphs->buffer, glyphs->length);
}

i32 krom_g2_font_count(arm_g2_font_t *font) {
	return arm_g2_font_count(font);
}

i32 krom_g2_font_height(arm_g2_font_t *font, i32 size) {
	return (int)arm_g2_font_height(font, size);
}

i32 krom_g2_string_width(arm_g2_font_t *font, i32 size, string_t *text) {
	return (int)arm_g2_string_width(font, size, text);
}

void krom_g2_set_bilinear_filter(bool bilinear) {
	arm_g2_set_bilinear_filter(bilinear);
}

void krom_g2_restore_render_target() {
	arm_g2_restore_render_target();
}

void krom_g2_set_render_target(kinc_g4_render_target_t *render_target) {
	arm_g2_set_render_target(render_target);
}

void krom_g2_set_color(i32 color) {
	arm_g2_set_color(color);
}

void krom_g2_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	arm_g2_set_pipeline(pipeline);
}

void krom_g2_set_transform(buffer_t *matrix) {
	arm_g2_set_transform(matrix != NULL ? (kinc_matrix3x3_t *)matrix->buffer : NULL);
}

void krom_g2_fill_circle(f32 cx, f32 cy, f32 radius, i32 segments) {
	arm_g2_fill_circle(cx, cy, radius, segments);
}

void krom_g2_draw_circle(f32 cx, f32 cy, f32 radius, i32 segments, f32 strength) {
	arm_g2_draw_circle(cx, cy, radius, segments, strength);
}

void krom_g2_draw_cubic_bezier(f32_array_t *x, f32_array_t *y, i32 segments, f32 strength) {
	arm_g2_draw_cubic_bezier(x->buffer, y->buffer, segments, strength);
}

#endif

bool _window_close_callback(void *data) {
	#ifdef KINC_WINDOWS
	bool save = false;
	wchar_t title[1024];
	GetWindowTextW(kinc_windows_window_handle(0), title, sizeof(title));
	bool dirty = wcsstr(title, L"* - ArmorPaint") != NULL;
	if (dirty) {
		int res = MessageBox(kinc_windows_window_handle(0), L"Project has been modified, save changes?", L"Save Changes?", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
		if (res == IDYES) {
			save = true;
		}
		else if (res == IDNO) {
			save = false;
		}
		else { // Cancel
			return false;
		}
	}
	//if (save_and_quit_func_set) {
	//	krom_save_and_quit(save);
	//	return false;
	//}
	#endif
	return true;
}

void krom_set_save_and_quit_callback(void (*callback)(bool)) {
	krom_save_and_quit = callback;
	save_and_quit_callback_set = true;
	kinc_window_set_close_callback(0, _window_close_callback, NULL);
}

void krom_set_mouse_cursor(i32 id) {
	kinc_mouse_set_cursor(id);
	#ifdef KINC_WINDOWS
	// Set hand icon for drag even when mouse button is pressed
	if (id == 1) {
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}
	#endif
}

void krom_delay_idle_sleep() {
	paused_frames = 0;
}

#ifdef WITH_NFD
char_ptr_array_t *krom_open_dialog(char *filter_list, char *default_path, bool open_multiple) {
	nfdpathset_t out_paths;
	nfdchar_t* out_path;
	nfdresult_t result = open_multiple ? NFD_OpenDialogMultiple(filter_list, default_path, &out_paths) : NFD_OpenDialog(filter_list, default_path, &out_path);

	if (result == NFD_OKAY) {
		int path_count = open_multiple ? (int)NFD_PathSet_GetCount(&out_paths) : 1;
		char_ptr_array_t *result = any_array_create(path_count);

		if (open_multiple) {
			for (int i = 0; i < path_count; ++i) {
				nfdchar_t* out_path = NFD_PathSet_GetPath(&out_paths, i);
				result->buffer[i] = out_path;
			}
			// NFD_PathSet_Free(&out_paths);
		}
		else {
			result->buffer[0] = out_path;
			// free(out_path);
		}
		return result;
	}
	else if (result == NFD_CANCEL) {
	}
	else {
		kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
	}
	return NULL;
}

char *krom_save_dialog(char *filter_list, char *default_path) {
	nfdchar_t *out_path = NULL;
	nfdresult_t result = NFD_SaveDialog(filter_list, default_path, &out_path);
	if (result == NFD_OKAY) {
		// free(out_path);
		return out_path;
	}
	else if (result == NFD_CANCEL) {
	}
	else {
		kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
	}
	return NULL;
}

#elif defined(KINC_ANDROID)

char_ptr_array_t *krom_open_dialog(char *filter_list, char *default_path, bool open_multiple) {
	AndroidFileDialogOpen();
	return NULL;
}

char *krom_save_dialog(char *filter_list, char *default_path) {
	wchar_t *out_path = AndroidFileDialogSave();
	size_t len = wcslen(out_path);
	uint16_t *str = malloc(sizeof(uint16_t) * (len + 1));
	for (int i = 0; i < len; i++) {
		str[i] = out_path[i];
	}
	str[len] = 0;
	// free(str);
	return str;
}

#elif defined(KINC_IOS)

char_ptr_array_t *krom_open_dialog(char *filter_list, char *default_path, bool open_multiple) {
	// Once finished drop_files callback is called
	IOSFileDialogOpen();
	return NULL;
}

char *krom_save_dialog(char *filter_list, char *default_path) {
	// Path to app document directory
	wchar_t *out_path = IOSFileDialogSave();
	size_t len = wcslen(out_path);
	uint16_t *str = malloc(sizeof(uint16_t) * (len + 1));
	for (int i = 0; i < len; i++) {
		str[i] = out_path[i];
	}
	str[len] = 0;
	// free(str);
	return str;;
}
#endif

#ifdef WITH_TINYDIR
char *krom_read_directory(char *path, bool folders_only) {
	tinydir_dir dir;
	#ifdef KINC_WINDOWS
	MultiByteToWideChar(CP_UTF8, 0, path, -1, temp_wstring, 1023);
	tinydir_open_sorted(&dir, temp_wstring);
	#else
	tinydir_open_sorted(&dir, path);
	#endif

	#ifdef KINC_WINDOWS
	wchar_t *files = malloc(1024 * sizeof(wchar_t));
	#else
	char *files = malloc(1024);
	#endif
	files[0] = 0;

	for (int i = 0; i < dir.n_files; i++) {
		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);

		if (!file.is_dir || !folders_only) {
			#ifdef KINC_WINDOWS
			if (FILE_ATTRIBUTE_HIDDEN & GetFileAttributesW(file.path)) {
				continue; // Skip hidden files
			}
			if (wcscmp(file.name, L".") == 0 || wcscmp(file.name, L"..") == 0) {
				continue;
			}
			wcscat(files, file.name);

			if (i < dir.n_files - 1) {
				wcscat(files, L"\n"); // Separator
			}
			#else
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) {
				continue;
			}
			strcat(files, file.name);
			if (i < dir.n_files - 1) {
				strcat(files, "\n");
			}
			#endif
		}
	}

	tinydir_close(&dir);
	return files;
}
#endif

bool krom_file_exists(char *path) {
	kinc_file_reader_t reader;
	if (kinc_file_reader_open(&reader, path, KINC_FILE_TYPE_ASSET)) {
		kinc_file_reader_close(&reader);
		return true;
	}
	return false;
}

void krom_delete_file(char *path) {
	#ifdef KINC_IOS
	IOSDeleteFile(path);
	#elif defined(KINC_WINDOWS)
	char cmd[1024];
	strcpy(cmd, "del /f \"");
	strcat(cmd, path);
	strcat(cmd, "\"");
	krom_sys_command(cmd);
	#else
	char cmd[1024];
	strcpy(cmd, "rm \"");
	strcat(cmd, path);
	strcat(cmd, "\"");
	krom_sys_command(cmd);
	#endif
}

#ifdef WITH_ZLIB
buffer_t *krom_inflate(buffer_t *bytes, bool raw) {
	unsigned char *inflated = (unsigned char *)malloc(bytes->length);
	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;
	infstream.avail_in = (uInt)bytes->length;
	infstream.next_in = (Bytef *)bytes->buffer;
	infstream.avail_out = (uInt)bytes->length;
	infstream.next_out = (Bytef *)inflated;
	inflateInit2(&infstream, raw ? -15 : 15 + 32);

	int i = 2;
	while (true) {
		int res = inflate(&infstream, Z_NO_FLUSH);
		if (res == Z_STREAM_END) {
			break;
		}
		if (infstream.avail_out == 0) {
			inflated = (unsigned char *)realloc(inflated, bytes->length * i);
			infstream.avail_out = (uInt)bytes->length;
			infstream.next_out = (Bytef *)(inflated + bytes->length * (i - 1));
			i++;
		}
	}
	inflateEnd(&infstream);

	buffer_t *output = buffer_create(infstream.total_out);
	memcpy(output->buffer, inflated, infstream.total_out);
	free(inflated);
	return output;
}

buffer_t *krom_deflate(buffer_t *bytes, bool raw) {
	int deflated_size = compressBound((uInt)bytes->length);
	void *deflated = malloc(deflated_size);
	z_stream defstream;
	defstream.zalloc = Z_NULL;
	defstream.zfree = Z_NULL;
	defstream.opaque = Z_NULL;
	defstream.avail_in = (uInt)bytes->length;
	defstream.next_in = (Bytef *)bytes->buffer;
	defstream.avail_out = deflated_size;
	defstream.next_out = (Bytef *)deflated;
	deflateInit2(&defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, raw ? -15 : 15, 5, Z_DEFAULT_STRATEGY);
	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);

	buffer_t *output = buffer_create(defstream.total_out);
	memcpy(output->buffer, deflated, defstream.total_out);
	free(deflated);
	return output;
}
#endif

#ifdef WITH_STB_IMAGE_WRITE
void _write_image(char *path, buffer_t *bytes, i32 w, i32 h, i32 format, int image_format, int quality) {
	int comp = 0;
	unsigned char *pixels = NULL;
	unsigned char *rgba = (unsigned char *)bytes->buffer;
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
			#if defined(KINC_METAL) || defined(KINC_VULKAN)
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
		#if defined(KINC_METAL) || defined(KINC_VULKAN)
		off = 2 - off;
		#endif
		for (int i = 0; i < w * h; ++i) {
			pixels[i] = rgba[i * 4 + off];
		}
	}

	image_format == 0 ?
		stbi_write_jpg(path, w, h, comp, pixels, quality) :
		stbi_write_png(path, w, h, comp, pixels, w * comp);

	if (pixels != rgba) {
		free(pixels);
	}
}

void krom_write_jpg(char *path, buffer_t *bytes, i32 w, i32 h, i32 format, i32 quality) {
	// RGBA, R, RGB1, RRR1, GGG1, BBB1, AAA1
	_write_image(path, bytes, w, h, format, 0, quality);
}

void krom_write_png(char *path, buffer_t *bytes, i32 w, i32 h, i32 format) {
	_write_image(path, bytes, w, h, format, 1, 100);
}

unsigned char *_encode_data;
int _encode_size;
void _encode_image_func(void *context, void *data, int size) {
	memcpy(_encode_data + _encode_size, data, size);
	_encode_size += size;
}

buffer_t *_encode_image(buffer_t *bytes, i32 w, i32 h, i32 format, i32 quality) {
	_encode_data = (unsigned char *)malloc(w * h * 4);
	_encode_size = 0;
	format == 0 ?
		stbi_write_jpg_to_func(&_encode_image_func, NULL, w, h, 4, bytes->buffer, quality) :
		stbi_write_png_to_func(&_encode_image_func, NULL, w, h, 4, bytes->buffer, w * 4);
	buffer_t *buffer = malloc(sizeof(buffer_t));
	buffer->buffer = _encode_data;
	buffer->length = _encode_size;
	return buffer;
}

buffer_t *krom_encode_jpg(buffer_t *bytes, i32 w, i32 h, i32 format, i32 quality) {
	return _encode_image(bytes, w, h, 0, quality);
}

buffer_t *krom_encode_png(buffer_t *bytes, i32 w, i32 h, i32 format) {
	return _encode_image(bytes, w, h, 1, 100);
}

#ifdef WITH_ZLIB
unsigned char *stbiw_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality) {
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
buffer_t *krom_write_mpeg() {
}
#endif

#ifdef WITH_ONNX
buffer_t *krom_ml_inference(buffer_t *model, buffer_t_array_t tensors, /*i32[][]*/ any_array_t *input_shape, i32_array_t output_shape, bool use_gpu) {
	OrtStatus *onnx_status = NULL;
	static bool use_gpu_last = false;
	if (ort == NULL || use_gpu_last != use_gpu) {
		use_gpu_last = use_gpu;
		ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
		ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "armorcore", &ort_env);

		ort->CreateSessionOptions(&ort_session_options);
		ort->SetIntraOpNumThreads(ort_session_options, 8);
		ort->SetInterOpNumThreads(ort_session_options, 8);

		if (use_gpu) {
			#ifdef KINC_WINDOWS
			ort->SetSessionExecutionMode(ort_session_options, ORT_SEQUENTIAL);
			ort->DisableMemPattern(ort_session_options);
			onnx_status = OrtSessionOptionsAppendExecutionProvider_DML(ort_session_options, 0);
			#elif defined(KINC_LINUX)
			// onnx_status = OrtSessionOptionsAppendExecutionProvider_CUDA(ort_session_options, 0);
			#elif defined(KINC_MACOS)
			onnx_status = OrtSessionOptionsAppendExecutionProvider_CoreML(ort_session_options, 0);
			#endif
			if (onnx_status != NULL) {
				const char *msg = ort->GetErrorMessage(onnx_status);
				kinc_log(KINC_LOG_LEVEL_ERROR, "%s", msg);
				ort->ReleaseStatus(onnx_status);
			}
		}
	}

	static void *content_last = 0;
	if (content_last != model->data || session == NULL) {
		if (session != NULL) {
			ort->ReleaseSession(session);
			session = NULL;
		}
		onnx_status = ort->CreateSessionFromArray(ort_env, model->data, (int)model->length, ort_session_options, &session);
		if (onnx_status != NULL) {
			const char* msg = ort->GetErrorMessage(onnx_status);
			kinc_log(KINC_LOG_LEVEL_ERROR, "%s", msg);
			ort->ReleaseStatus(onnx_status);
		}
	}
	content_last = model->data;

	OrtAllocator *allocator;
	ort->GetAllocatorWithDefaultOptions(&allocator);
	OrtMemoryInfo *memory_info;
	ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);

	int32_t length = tensors->length;
	if (length > 4) {
		length = 4;
	}
	char *input_node_names[4];
	OrtValue *input_tensors[4];
	for (int32_t i = 0; i < length; ++i) {
		ort->SessionGetInputName(session, i, allocator, &input_node_names[i]);

		OrtTypeInfo *input_type_info;
		ort->SessionGetInputTypeInfo(session, i, &input_type_info);
		const OrtTensorTypeAndShapeInfo *input_tensor_info;
		ort->CastTypeInfoToTensorInfo(input_type_info, &input_tensor_info);
		size_t num_input_dims;
		ort->GetDimensionsCount(input_tensor_info, &num_input_dims);
		std::vector<int64_t> input_node_dims(num_input_dims);

		if (input_shape != NULL) {
			for (int32_t j = 0; j < num_input_dims; ++j) {
				input_node_dims[j] = input_shape->buffer[i]->data[j];
			}
		}
		else {
			ort->GetDimensions(input_tensor_info, (int64_t *)input_node_dims.data(), num_input_dims);
		}
		ONNXTensorElementDataType tensor_element_type;
		ort->GetTensorElementType(input_tensor_info, &tensor_element_type);

		ort->CreateTensorWithDataAsOrtValue(memory_info, tensors->buffer[i]->data, (int)tensors->buffer[i]->length, input_node_dims.data(), num_input_dims,  tensor_element_type, &input_tensors[i]);
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
	if (output_shape != NULL) {
		int32_t length = output_shape->length;
		for (int i = 0; i < length; ++i) {
			output_byte_length *= output_shape->data[i];
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
		for (int i = 0; i < num_output_dims; ++i) {
			if (output_node_dims[i] > 1) {
				output_byte_length *= output_node_dims[i];
			}
		}
	}

	buffer_t *output = buffer_create(output_byte_length);
	memcpy(output->data, float_array, output_byte_length);

	ort->ReleaseMemoryInfo(memory_info);
	ort->ReleaseValue(output_tensor);
	for (int i = 0; i < length; ++i) {
		ort->ReleaseValue(input_tensors[i]);
	}
	return output;
}

void krom_ml_unload() {
	if (session != NULL) {
		ort->ReleaseSession(session);
		session = NULL;
	}
}
#endif

#if defined(KINC_DIRECT3D12) || defined(KINC_VULKAN) || defined(KINC_METAL)
bool krom_raytrace_supported() {
	#ifdef KINC_METAL
	return kinc_raytrace_supported();
	#else
	return true;
	#endif
}

void krom_raytrace_init(buffer_t *shader, kinc_g4_vertex_buffer_t *vb, kinc_g4_index_buffer_t *ib, f32 scale) {
	if (accel_created) {
		kinc_g5_constant_buffer_destroy(&constant_buffer);
		kinc_raytrace_acceleration_structure_destroy(&accel);
		kinc_raytrace_pipeline_destroy(&pipeline);
	}

	kinc_g5_vertex_buffer_t *vertex_buffer = &vb->impl._buffer;
	kinc_g5_index_buffer_t *index_buffer = &ib->impl._buffer;

	float scale = TO_F32(args[3]);
	kinc_g5_constant_buffer_init(&constant_buffer, constant_buffer_size * 4);
	kinc_raytrace_pipeline_init(&pipeline, &commandList, shader->data, (int)shader->length, &constant_buffer);
	kinc_raytrace_acceleration_structure_init(&accel, &commandList, vertex_buffer, index_buffer, scale);
	accel_created = true;
}

void krom_raytrace_set_textures(image_t *tex0, image_t *tex1, image_t *tex2, kinc_g4_texture_t *texenv, kinc_g4_texture_t *tex_sobol, kinc_g4_texture_t *tex_scramble, kinc_g4_texture_t *tex_rank) {
	kinc_g4_render_target_t *texpaint0;
	kinc_g4_render_target_t *texpaint1;
	kinc_g4_render_target_t *texpaint2;

	image_t *texpaint0_image = tex0;
	kinc_g4_texture_t *texpaint0_tex = texpaint0_image->texture_;
	kinc_g4_render_target_t *texpaint0_rt = texpaint0_image->render_target_;

	if (texpaint0_tex != NULL) {
		#ifdef KINC_DIRECT3D12
		kinc_g4_texture_t *texture = texpaint0_tex;
		if (!texture->impl._uploaded) {
			kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
			texture->impl._uploaded = true;
		}
		texpaint0 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
		texpaint0->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
		#endif
	}
	else {
		texpaint0 = texpaint0_rt;
	}

	image_t *texpaint1_image = tex1;
	kinc_g4_texture_t *texpaint1_tex = texpaint1_image->texture_;
	kinc_g4_render_target_t *texpaint1_rt = texpaint1_image->render_target_;

	if (texpaint1_tex != NULL) {
		#ifdef KINC_DIRECT3D12
		kinc_g4_texture_t *texture = texpaint1_tex;
		if (!texture->impl._uploaded) {
			kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
			texture->impl._uploaded = true;
		}
		texpaint1 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
		texpaint1->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
		#endif
	}
	else {
		texpaint1 = texpaint1_rt;
	}

	image_t *texpaint2_image = tex2;
	kinc_g4_texture_t *texpaint2_tex = texpaint2_image->texture_;
	kinc_g4_render_target_t *texpaint2_rt = texpaint2_image->render_target_;

	if (texpaint2_tex != NULL) {
		#ifdef KINC_DIRECT3D12
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)texpaint2_tex;
		if (!texture->impl._uploaded) {
			kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
			texture->impl._uploaded = true;
		}
		texpaint2 = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
		texpaint2->impl._renderTarget.impl.srvDescriptorHeap = texture->impl._texture.impl.srvDescriptorHeap;
		#endif
	}
	else {
		texpaint2 = texpaint2_rt;
	}

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

	if (texpaint0_tex != NULL) {
		free(texpaint0);
	}
	if (texpaint1_tex != NULL) {
		free(texpaint1);
	}
	if (texpaint2_tex != NULL) {
		free(texpaint2);
	}
}

void krom_raytrace_dispatch_rays(kinc_g4_render_target_t *render_target, buffer_t *buffer) {
	float *cb = (float *)buffer->data;
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

i32 krom_window_x() {
	return kinc_window_x(0);
}

i32 krom_window_y() {
	return kinc_window_y(0);
}

char *krom_language() {
	return kinc_language();
}

obj_part_t *krom_io_obj_parse(buffer_t *file_bytes, i32 split_code, i32 start_pos, bool udim) {
	obj_part_t *part = io_obj_parse(file_bytes->buffer, split_code, start_pos, udim);
	return part;

	// if (udim) {
	// 	obj->udims_u = part->udims_u;
	// 	any_array_t *udims = any_array_create(part->udims_u * part->udims_v);
	// 	for (int i = 0; i < part->udims_u * part->udims_v; ++i) {
	// 		u32_array_t *data = malloc(sizeof(u32_array_t));
	// 		data->buffer = part->udims[i];
	// 		data->length = part->udims_count[i];
	// 		data->capacity = part->udims_count[i];
	// 		udims->buffer[i] = data;
	// 	}
	// 	obj->udims = udims;
	// }
}

#ifdef WITH_ZUI
float krom_js_eval(char *str) {
	return 0.0;
}
#endif

#endif
