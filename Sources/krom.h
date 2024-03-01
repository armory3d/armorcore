#pragma once

#include <math.h>
#include <stdlib.h>

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
int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxOutputSize);
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
bool waitAfterNextDraw;
#endif
#if defined(KORE_DIRECT3D12) || defined(KORE_VULKAN) || defined(KORE_METAL)
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/raytrace.h>
#endif

#ifdef KORE_WINDOWS
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
struct HWND__ *kinc_windows_window_handle(int window_index); // Kore/Windows.h
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
#include "g2/g2.h"
#include "g2/g2_ext.h"
#endif
#ifdef WITH_IRON
#include "iron/io_obj.h"
#endif
#ifdef WITH_ZUI
#include "zui/zui.h"
#include "zui/zui_ext.h"
#include "zui/zui_nodes.h"
#endif
#include <iron/iron_string.h>
#include <iron/iron_array.h>
#include <iron/iron_map.h>
#include <iron/iron_armpack.h>
// #include <quickjs.h>

#ifdef WITH_PLUGIN_EMBED
void plugin_embed();
#endif

#ifdef KORE_MACOS
const char *macgetresourcepath();
#endif
#ifdef KORE_IOS
const char *iphonegetresourcepath();
#endif

#if defined(KORE_IOS) || defined(KORE_ANDROID)
char mobile_title[1024];
#endif

#if defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)
int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
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

#ifdef __GNUC__
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

void _globals_init();
void start();

int kickstart(int argc, char **argv) {
	// JSRuntime *runtime = JS_NewRuntime();
	// JSContext *ctx = JS_NewContext(runtime);
	// JSValue result = JS_Eval(ctx, "5+2", 3, "mini.js", JS_EVAL_TYPE_GLOBAL);
	// printf("%d\n", JS_VALUE_GET_INT(result));
	// JS_FreeValue(ctx, result);
	// JS_RunGC(runtime);

	// kinc_log(KINC_LOG_LEVEL_INFO, "123");

	_globals_init();
	start();
	return 0;
}

void *gc_alloc(size_t size) {
	return calloc(size, sizeof(uint8_t));
}

void gc_free() {

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
	}
	return a;
}

u32_array_t *u32_array_create(i32 length) {
	u32_array_t *a = gc_alloc(sizeof(u32_array_t));
	if (length > 0) {
		u32_array_resize(a, length);
	}
	return a;
}

i32_array_t *i32_array_create(i32 length) {
	i32_array_t *a = gc_alloc(sizeof(i32_array_t));
	if (length > 0) {
		i32_array_resize(a, length);
	}
	return a;
}

u16_array_t *u16_array_create(i32 length) {
	u16_array_t *a = gc_alloc(sizeof(u16_array_t));
	if (length > 0) {
		u16_array_resize(a, length);
	}
	return a;
}

i16_array_t *i16_array_create(i32 length) {
	i16_array_t *a = gc_alloc(sizeof(i16_array_t));
	if (length > 0) {
		i16_array_resize(a, length);
	}
	return a;
}

u8_array_t *u8_array_create(i32 length) {
	u8_array_t *a = gc_alloc(sizeof(u8_array_t));
	if (length > 0) {
		u8_array_resize(a, length);
	}
	return a;
}

u8_array_t *u8_array_create_from_buffer(buffer_t *b) {
	u8_array_t *a = gc_alloc(sizeof(u8_array_t));
	a->buffer = b->data;
	return a;
}

i8_array_t *i8_array_create(i32 length) {
	i8_array_t *a = gc_alloc(sizeof(i8_array_t));
	if (length > 0) {
		i8_array_resize(a, length);
	}
	return a;
}

any_array_t *any_array_create(i32 length) {
	any_array_t *a = gc_alloc(sizeof(any_array_t));
	if (length > 0) {
		any_array_resize(a, length);
	}
	return a;
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
bool is_integer(void *a) { return false; } // armpack
bool is_view(void *a) { return false; } // armpack
bool is_array(void *a) { return false; } // armpack
string_t *any_to_string(void *a) { return NULL; } // armpack

string_t *trim_end(string_t *str) {
   return NULL;
}

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

bool enable_window = true;

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
	#ifdef KORE_WINDOWS
	win.visible = false; // Prevent white flicker when opening the window
	#else
	win.visible = enable_window;
	#endif
	frame.color_bits = 32;
	frame.depth_bits = 0;
	frame.stencil_bits = 0;
	kinc_init(title, win.width, win.height, &win, &frame);
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

void krom_set_app_name(string_t *name) {

}

void krom_log(any v) {

}

void krom_g4_clear(i32 flags, i32 color, f32 depth) {

}

void krom_set_update_callback(any callback) {

}

void krom_set_drop_files_callback(any callback) {

}

void krom_set_cut_copy_paste_callback(any on_cut, any on_copy, any on_paste) {

}

void krom_set_application_state_callback(any on_foreground, any on_resume, any on_pause, any on_background, any on_shutdown) {

}

void krom_set_keyboard_down_callback(any callback) {

}

void krom_set_keyboard_up_callback(any callback) {

}

void krom_set_keyboard_press_callback(any callback) {

}

void krom_set_mouse_down_callback(any callback) {

}

void krom_set_mouse_up_callback(any callback) {

}

void krom_set_mouse_move_callback(any callback) {

}

void krom_set_mouse_wheel_callback(any callback) {

}

void krom_set_touch_down_callback(any callback) {

}

void krom_set_touch_up_callback(any callback) {

}

void krom_set_touch_move_callback(any callback) {

}

void krom_set_pen_down_callback(any callback) {

}

void krom_set_pen_up_callback(any callback) {

}

void krom_set_pen_move_callback(any callback) {

}

void krom_set_gamepad_axis_callback(any callback) {

}

void krom_set_gamepad_button_callback(any callback) {

}

void krom_lock_mouse() {

}

void krom_unlock_mouse() {

}

bool krom_can_lock_mouse() {
	return false;
}

bool krom_is_mouse_locked() {
	return false;
}

void krom_set_mouse_position(i32 x, i32 y) {

}

void krom_show_mouse(bool show) {

}

void krom_show_keyboard(bool show) {

}


any krom_g4_create_index_buffer(i32 count) {
	return NULL;
}

void krom_g4_delete_index_buffer(any buffer) {

}

u32_array_t *krom_g4_lock_index_buffer(any buffer) {
	return NULL;
}

void krom_g4_unlock_index_buffer(any buffer) {

}

void krom_g4_set_index_buffer(any buffer) {

}

struct kinc_vertex_elem;

any krom_g4_create_vertex_buffer(i32 count, struct kinc_vertex_elem **structure, i32 usage, i32 inst_data_step_rate) {
	return NULL;
}

void krom_g4_delete_vertex_buffer(any buffer) {

}

buffer_t *krom_g4_lock_vertex_buffer(any buffer) {
	return NULL;
}

void krom_g4_unlock_vertex_buffer(any buffer) {

}

void krom_g4_set_vertex_buffer(any buffer) {

}

struct vertex_buffer;

void krom_g4_set_vertex_buffers(struct vertex_buffer **vertex_buffers) {

}

void krom_g4_draw_indexed_vertices(i32 start, i32 count) {

}

void krom_g4_draw_indexed_vertices_instanced(i32 inst_count, i32 start, i32 count) {

}

any krom_g4_create_shader(buffer_t *data, i32 type) {
	return NULL;
}

any krom_g4_create_vertex_shader_from_source(string_t *source) {
	return NULL;
}

any krom_g4_create_fragment_shader_from_source(string_t *source) {
	return NULL;
}

void krom_g4_delete_shader(any shader) {

}

any krom_g4_create_pipeline() {
	return NULL;
}

void krom_g4_delete_pipeline(any pipeline) {

}

void krom_g4_compile_pipeline(any pipeline, any structure0, any structure1, any structure2, any structure3, i32 length, any vertex_shader, any fragment_shader, any geometry_shader, any state) {

}

void krom_g4_set_pipeline(any pipeline) {

}

any krom_load_image(string_t *file, bool readable) {
	return NULL;
}

struct image;

void krom_unload_image(struct image *image) {

}

any krom_load_sound(string_t *file) {
	return NULL;
}

void krom_unload_sound(any sound) {

}

void krom_play_sound(any sound, bool loop) {

}

void krom_stop_sound(any sound) {

}

buffer_t *krom_load_blob(string_t *file) {
	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, file, KINC_FILE_TYPE_ASSET)) {
		return NULL;
	}
	uint32_t reader_size = (uint32_t)kinc_file_reader_size(&reader);
	buffer_t *buffer = buffer_create(reader_size);
	kinc_file_reader_read(&reader, buffer->data, reader_size);
	kinc_file_reader_close(&reader);
	return buffer;
}

void krom_load_url(string_t *url) {

}

void krom_copy_to_clipboard(string_t *text) {

}


any krom_g4_get_constant_location(any pipeline, string_t *name) {
	return NULL;
}

any krom_g4_get_texture_unit(any pipeline, string_t *name) {
	return NULL;
}

void krom_g4_set_texture(any stage, any texture) {

}

void krom_g4_set_render_target(any stage, any render_target) {

}

void krom_g4_set_texture_depth(any unit, any texture) {

}

void krom_g4_set_image_texture(any stage, any texture) {

}

void krom_g4_set_texture_parameters(any tex_unit, i32 u_addr, i32 v_addr, i32 min_filter, i32 mag_filter, i32 mip_filter) {

}

void krom_g4_set_texture3d_parameters(any tex_unit, i32 u_addr, i32 v_addr, i32 w_addr, i32 min_filter, i32 mag_filter, i32 mip_filter) {

}

void krom_g4_set_bool(any location, bool value) {

}

void krom_g4_set_int(any location, i32 value) {

}

void krom_g4_set_float(any location, f32 value) {

}

void krom_g4_set_float2(any location, f32 value1, f32 value2) {

}

void krom_g4_set_float3(any location, f32 value1, f32 value2, f32 value3) {

}

void krom_g4_set_float4(any location, f32 value1, f32 value2, f32 value3, f32 value4) {

}

void krom_g4_set_floats(any location, buffer_t *values) {

}

void krom_g4_set_matrix4(any location, buffer_t *matrix) {

}

void krom_g4_set_matrix3(any location, buffer_t *matrix) {

}


f32 krom_get_time() {
	return 0.0;
}

i32 krom_window_width() {
	return 640;
}

i32 krom_window_height() {
	return 480;
}

void krom_set_window_title(string_t *title) {

}

i32 krom_get_window_mode() {
	return 0;
}

void krom_set_window_mode(i32 mode) {

}

void krom_resize_window(i32 width, i32 height) {

}

void krom_move_window(i32 x, i32 y) {

}

i32 krom_screen_dpi() {
	return 0;
}

string_t *krom_system_id() {
	return NULL;
}

void krom_request_shutdown() {

}

i32 krom_display_count() {
	return 0;
}

i32 krom_display_width(i32 index) {
	return 0;
}

i32 krom_display_height(i32 index) {
	return 0;
}

i32 krom_display_x(i32 index) {
	return 0;
}

i32 krom_display_y(i32 index) {
	return 0;
}

i32 krom_display_frequency(i32 index) {
	return 0;
}

bool krom_display_is_primary(i32 index) {
	return false;
}

void krom_write_storage(string_t *name, buffer_t *data) {

}

buffer_t *krom_read_storage(string_t *name) {
	return NULL;
}


any krom_g4_create_render_target(i32 width, i32 height, i32 format, i32 depth_buffer_bits, i32 stencil_buffer_bits) {
	return NULL;
}

any krom_g4_create_texture(i32 width, i32 height, i32 format) {
	return NULL;
}

any krom_g4_create_texture3d(i32 width, i32 height, i32 depth, i32 format) {
	return NULL;
}

any krom_g4_create_texture_from_bytes(buffer_t *data, i32 width, i32 height, i32 format, bool readable) {
	return NULL;
}

any krom_g4_create_texture_from_bytes3d(buffer_t *data, i32 width, i32 height, i32 depth, i32 format, bool readable) {
	return NULL;
}

any krom_g4_create_texture_from_encoded_bytes(buffer_t *data, string_t *format, bool readable) {
	return NULL;
}

buffer_t *krom_g4_get_texture_pixels(any texture) {
	return NULL;
}

void krom_g4_get_render_target_pixels(any render_target, buffer_t *data) {

}

buffer_t *krom_g4_lock_texture(any texture, i32 level) {
	return NULL;
}

void krom_g4_unlock_texture(any texture) {

}

void krom_g4_clear_texture(any target, i32 x, i32 y, i32 z, i32 width, i32 height, i32 depth, i32 color) {

}

void krom_g4_generate_texture_mipmaps(any texture, i32 levels) {

}

void krom_g4_generate_render_target_mipmaps(any render_target, i32 levels) {

}

void krom_g4_set_mipmaps(any texture, struct image **mipmaps) {

}

void krom_g4_set_depth_from(any target, any source) {

}

void krom_g4_viewport(i32 x, i32 y, i32 width, i32 height) {

}

void krom_g4_scissor(i32 x, i32 y, i32 width, i32 height) {

}

void krom_g4_disable_scissor() {

}

bool krom_g4_render_targets_inverted_y() {
	return false;
}

void krom_g4_begin(struct image *render_target, struct image **additional) {

}

void krom_g4_end() {

}

void krom_file_save_bytes(string_t *path, buffer_t *bytes, i32 length) {

}

i32 krom_sys_command(string_t *cmd, string_t **args) {
	return 0;
}

string_t *krom_save_path() {
	return NULL;
}

i32 krom_get_arg_count() {
	return 0;
}

string_t *krom_get_arg(i32 index) {
	return NULL;
}

string_t *krom_get_files_location() {
	return NULL;
}

void krom_http_request(string_t *url, i32 size, any callback) {

}

void krom_g2_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *colored_vert, buffer_t *colored_frag, buffer_t *text_vert, buffer_t *text_frag) {

}

void krom_g2_begin() {

}

void krom_g2_end() {

}

void krom_g2_draw_scaled_sub_image(struct image *image, f32 sx, f32 sy, f32 sw, f32 sh, f32 dx, f32 dy, f32 dw, f32 dh) {

}

void krom_g2_fill_triangle(f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2) {

}

void krom_g2_fill_rect(f32 x, f32 y, f32 width, f32 height) {

}

void krom_g2_draw_rect(f32 x, f32 y, f32 width, f32 height, f32 strength) {

}

void krom_g2_draw_line(f32 x0, f32 y0, f32 x1, f32 y1, f32 strength) {

}

void krom_g2_draw_string(string_t *text, f32 x, f32 y) {

}

void krom_g2_set_font(any font, i32 size) {

}

any krom_g2_font_init(buffer_t *blob, i32 font_index) {
	return NULL;
}

any krom_g2_font_13(buffer_t *blob) {
	return NULL;
}

void krom_g2_font_set_glyphs(i32 *glyphs) {

}

i32 krom_g2_font_count(any font) {
	return 0;
}

i32 krom_g2_font_height(any font, i32 size) {
	return 0;
}

i32 krom_g2_string_width(any font, i32 size, string_t *text) {
	return 0;
}

void krom_g2_set_bilinear_filter(bool bilinear) {

}

void krom_g2_restore_render_target() {

}

void krom_g2_set_render_target(any render_target) {

}

void krom_g2_set_color(i32 color) {

}

void krom_g2_set_pipeline(any pipeline) {

}

void krom_g2_set_transform(buffer_t *matrix) {

}

void krom_g2_fill_circle(f32 cx, f32 cy, f32 radius, i32 segments) {

}

void krom_g2_draw_circle(f32 cx, f32 cy, f32 radius, i32 segments, f32 strength) {

}

void krom_g2_draw_cubic_bezier(f32 *x, f32 *y, i32 segments, f32 strength) {

}

// declare function krom_set_save_and_quit_callback(callback: (save: bool)=>void): void;
// declare function krom_set_mouse_cursor(id: i32): void;
// declare function krom_delay_idle_sleep(): void;
// declare function krom_open_dialog(filter_list: string, default_path: string, open_multiple: bool): string[];
// declare function krom_save_dialog(filter_list: string, default_path: string): string;
// declare function krom_read_directory(path: string, foldersOnly: bool): string;
// declare function krom_file_exists(path: string): bool;
// declare function krom_delete_file(path: string): void;
// declare function krom_inflate(bytes: ArrayBuffer, raw: bool): ArrayBuffer;
// declare function krom_deflate(bytes: ArrayBuffer, raw: bool): ArrayBuffer;
// declare function krom_write_jpg(path: string, bytes: ArrayBuffer, w: i32, h: i32, format: i32, quality: i32): void; // RGBA, R, RGB1, RRR1, GGG1, BBB1, AAA1
// declare function krom_write_png(path: string, bytes: ArrayBuffer, w: i32, h: i32, format: i32): void;
// declare function krom_encode_jpg(bytes: ArrayBuffer, w: i32, h: i32, format: i32, quality: i32): ArrayBuffer;
// declare function krom_encode_png(bytes: ArrayBuffer, w: i32, h: i32, format: i32): ArrayBuffer;
// declare function krom_write_mpeg(): ArrayBuffer;
// declare function krom_ml_inference(model: ArrayBuffer, tensors: ArrayBuffer[], input_shape?: i32[][], output_shape?: i32[], use_gpu?: bool): ArrayBuffer;
// declare function krom_ml_unload(): void;

// declare function krom_raytrace_supported(): bool;
// declare function krom_raytrace_init(shader: ArrayBuffer, vb: any, ib: any, scale: f32): void;
// declare function krom_raytrace_set_textures(tex0: image_t, tex1: image_t, tex2: image_t, texenv: any, tex_sobol: any, tex_scramble: any, tex_rank: any): void;
// declare function krom_raytrace_dispatch_rays(target: any, cb: ArrayBuffer): void;

i32 krom_window_x() {
	return 0;
}

i32 krom_window_y() {
	return 0;
}

char *krom_language() {
	return NULL;
}
// declare function krom_io_obj_parse(file_bytes: ArrayBuffer, split_code: i32, start_pos: i32, udim: bool): any;
