#pragma once

#pragma clang diagnostic ignored "-Wgnu-designator"
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
#include <iron/iron_string.h>
#include <iron/iron_array.h>
#include <iron/iron_map.h>
#include <iron/iron_armpack.h>
#include <iron/iron_json.h>
#include <iron/iron_gc.h>
// #include <quickjs.h>
#ifdef IDLE_SLEEP
#include <unistd.h>
#endif
#ifdef WITH_ZUI
#include "zui/zui.h"
#endif

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
char assetsdir[256];
bool enable_window = true;
bool stderr_created = false;
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
wchar_t temp_wstring1[1024];
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
	WideCharToMultiByte(CP_UTF8, 0, temp_wstring, -1, temp_string, 4096, nullptr, nullptr);
	bindir = temp_string;
#endif

#ifdef KINC_WINDOWS
	// bindir = bindir.substr(0, bindir.find_last_of("\\"));
#else
	// bindir = bindir.substr(0, bindir.find_last_of("/"));
#endif
	// assetsdir = argc > 1 ? argv[1] : bindir;

	// Opening a file
	// int l = (int)assetsdir.length();
	// if ((l > 6 && assetsdir[l - 6] == '.') ||
	// 	(l > 5 && assetsdir[l - 5] == '.') ||
	// 	(l > 4 && assetsdir[l - 4] == '.')) {
	// 	assetsdir = bindir;
	// }

	for (int i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "--nowindow") == 0) {
			enable_window = false;
		}
	}

#if !defined(KINC_MACOS) && !defined(KINC_IOS)
	// kinc_internal_set_files_location(&assetsdir[0u]);
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

	// JSRuntime *runtime = JS_NewRuntime();
	// JSContext *ctx = JS_NewContext(runtime);
	// JSValue result = JS_Eval(ctx, "5+2", 3, "mini.js", JS_EVAL_TYPE_GLOBAL);
	// printf("%d\n", JS_VALUE_GET_INT(result));
	// JS_FreeValue(ctx, result);
	// JS_RunGC(runtime);

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

void _drop_files(wchar_t *file_path, void *data) {
	// Update mouse position
	#ifdef KINC_WINDOWS
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(kinc_windows_window_handle(0), &p);
	mouse_move(0, p.x, p.y, 0, 0, NULL);
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

f32 js_eval(const char *js, const char *context) {
	return 0.0;
}

void uri_decode(char *dst, const char *src) {
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
