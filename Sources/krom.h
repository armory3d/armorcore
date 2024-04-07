#pragma once

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/display.h>
#include <kinc/threads/thread.h>
#include <kinc/graphics4/graphics.h>
#include <iron/iron_string.h>
#include <iron/iron_array.h>
#include <iron/iron_map.h>
#include <iron/iron_armpack.h>
#include <gc.h>
// #include <quickjs.h>

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
bool in_background = false;
bool gc_static_alloc = false;

void (*krom_update)(void);
void (*krom_drop_files)(wchar_t *);
void (*krom_key_down)(int);
void (*krom_key_up)(int);

void *gc_alloc(size_t size) {
	return gc_static_alloc ? _gc_calloc_static(size, sizeof(uint8_t)) : _gc_calloc(size, sizeof(uint8_t));
}

void *gc_realloc(void *ptr, size_t size) {
	return _gc_realloc(ptr, size);
}

void gc_free(void *ptr) {
	_gc_free(ptr);
}

// point_t *p = GC_ALLOC_INIT(point_t, {x: 1.5, y: 3.5});
#define GC_ALLOC_INIT(type, ...) (type *)memcpy(gc_alloc(sizeof(type)), (type[]){ __VA_ARGS__ }, sizeof(type))

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

	gc_static_alloc = true;
	_gc_start(&argc);

	void (*volatile __kickstart)(void) = _kickstart; // No inline, for gc
	__kickstart();

	#ifdef WITH_AUDIO
	kinc_a2_shutdown();
	#endif
	#ifdef WITH_ONNX
	if (ort != NULL) {
		ort->ReleaseEnv(ort_env);
		ort->ReleaseSessionOptions(ort_session_options);
	}
	#endif
	_gc_stop();
	return 0;
}

void update(void *data) {
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
	gc_static_alloc = false;
}

void drop_files(wchar_t *file_path, void *data) {
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
	krom_drop_files(file_path);
	in_background = false;

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void key_down(int code, void *data) {
	krom_key_down(code);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) zui_key_down(zui_instances[i], code);
	#endif

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void key_up(int code, void *data) {
	krom_key_up(code);

	#ifdef WITH_ZUI
	for (int i = 0; i < zui_instances_count; ++i) zui_key_up(zui_instances[i], code);
	#endif

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

f32 js_eval(const char *js, const char *context) {
	return 0.0;
}

i32_map_t *i32_map_create() {
	return gc_alloc(sizeof(i32_map_t));
}

any_map_t *any_map_create() {
	return gc_alloc(sizeof(any_map_t));
}

buffer_t *buffer_create(i32 length) {
	buffer_t * b = gc_alloc(sizeof(buffer_t));
	buffer_resize(b, length);
	return b;
}

buffer_view_t *buffer_view_create(buffer_t *b) {
	buffer_view_t *view = gc_alloc(sizeof(buffer_view_t));
	view->buffer = b;
	return view;
}

f32_array_t *f32_array_create(i32 length) {
	f32_array_t *a = gc_alloc(sizeof(f32_array_t));
	if (length > 0) {
		f32_array_resize(a, length);
		a->length = length;
	}
	return a;
}

f32_array_t *f32_array_create_from_buffer(buffer_t *b) {
	f32_array_t *a = gc_alloc(sizeof(f32_array_t));
	a->buffer = b->data;
	a->length = b->length / 4;
	a->capacity = b->length / 4;
	return a;
}

f32_array_t *f32_array_create_from_array(f32_array_t *from) {
	f32_array_t *a = f32_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

f32_array_t *f32_array_create_xy(f32 x, f32 y) {
	f32_array_t *a = f32_array_create(2);
	a->buffer[0] = x;
	a->buffer[1] = y;
	return a;
}

f32_array_t *f32_array_create_xyz(f32 x, f32 y, f32 z) {
	f32_array_t *a = f32_array_create(3);
	a->buffer[0] = x;
	a->buffer[1] = y;
	a->buffer[2] = z;
	return a;
}

f32_array_t *f32_array_create_xyzw(f32 x, f32 y, f32 z, f32 w) {
	f32_array_t *a = f32_array_create(4);
	a->buffer[0] = x;
	a->buffer[1] = y;
	a->buffer[2] = z;
	a->buffer[3] = w;
	return a;
}

f32_array_t *f32_array_create_xyzwv(f32 x, f32 y, f32 z, f32 w, f32 v) {
	f32_array_t *a = f32_array_create(5);
	a->buffer[0] = x;
	a->buffer[1] = y;
	a->buffer[2] = z;
	a->buffer[3] = w;
	a->buffer[4] = v;
	return a;
}

u32_array_t *u32_array_create(i32 length) {
	u32_array_t *a = gc_alloc(sizeof(u32_array_t));
	if (length > 0) {
		u32_array_resize(a, length);
		a->length = length;
	}
	return a;
}

u32_array_t *u32_array_create_from_array(u32_array_t *from) {
	u32_array_t *a = u32_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

i32_array_t *i32_array_create(i32 length) {
	i32_array_t *a = gc_alloc(sizeof(i32_array_t));
	if (length > 0) {
		i32_array_resize(a, length);
		a->length = length;
	}
	return a;
}

i32_array_t *i32_array_create_from_array(i32_array_t *from) {
	i32_array_t *a = i32_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

u16_array_t *u16_array_create(i32 length) {
	u16_array_t *a = gc_alloc(sizeof(u16_array_t));
	if (length > 0) {
		u16_array_resize(a, length);
		a->length = length;
	}
	return a;
}

i16_array_t *i16_array_create(i32 length) {
	i16_array_t *a = gc_alloc(sizeof(i16_array_t));
	if (length > 0) {
		i16_array_resize(a, length);
		a->length = length;
	}
	return a;
}

i16_array_t *i16_array_create_from_array(i16_array_t *from) {
	i16_array_t *a = i16_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

u8_array_t *u8_array_create(i32 length) {
	u8_array_t *a = gc_alloc(sizeof(u8_array_t));
	if (length > 0) {
		u8_array_resize(a, length);
		a->length = length;
	}
	return a;
}

u8_array_t *u8_array_create_from_buffer(buffer_t *b) {
	u8_array_t *a = gc_alloc(sizeof(u8_array_t));
	a->buffer = b->data;
	a->length = b->length;
	a->capacity = b->length;
	return a;
}

u8_array_t *u8_array_create_from_array(u8_array_t *from) {
	u8_array_t *a = u8_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

i8_array_t *i8_array_create(i32 length) {
	i8_array_t *a = gc_alloc(sizeof(i8_array_t));
	if (length > 0) {
		i8_array_resize(a, length);
		a->length = length;
	}
	return a;
}

any_array_t *any_array_create(i32 length) {
	any_array_t *a = gc_alloc(sizeof(any_array_t));
	if (length > 0) {
		any_array_resize(a, length);
		a->length = length;
	}
	return a;
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
f32 math_random() { return rand() / RAND_MAX; }
f32 math_atan2(f32 y, f32 x) { return atan2f(y, x); }
f32 math_asin(f32 x) { return asinf(x); }
f32 math_pi() { return 3.14159265358979323846; }
f32 math_pow(f32 x, f32 y) { return powf(x, y); }

i32 parse_int(const char *s) { return 0; } // strtol
i32 parse_int_hex(const char *s) { return 0; } // strtol
f32 parse_float(const char *s) { return 0.0; } // strtof

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
