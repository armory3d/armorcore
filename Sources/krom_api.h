#pragma once

#include "krom.h"
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
int LZ4_decompress_safe(const char *source, char *dest, int compressed_size, int maxOutputSize);
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

// ██╗  ██╗    ██████╗      ██████╗     ███╗   ███╗
// ██║ ██╔╝    ██╔══██╗    ██╔═══██╗    ████╗ ████║
// █████╔╝     ██████╔╝    ██║   ██║    ██╔████╔██║
// ██╔═██╗     ██╔══██╗    ██║   ██║    ██║╚██╔╝██║
// ██║  ██╗    ██║  ██║    ╚██████╔╝    ██║ ╚═╝ ██║
// ╚═╝  ╚═╝    ╚═╝  ╚═╝     ╚═════╝     ╚═╝     ╚═╝

bool stderr_created = false;
int paused_frames = 0;
#ifdef IDLE_SLEEP
bool input_down = false;
int last_window_width = 0;
int last_window_height = 0;
#endif

char temp_string[4096];
char temp_string_vs[1024 * 1024];
char temp_string_fs[1024 * 1024];
char temp_string_vstruct[4][32][32];
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
	RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, vdata, &cbdata);
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
	kinc_set_application_name(name);
}

void krom_log(string_t *value) {
	size_t len = strlen(value);
	if (len < 2048) {
		kinc_log(KINC_LOG_LEVEL_INFO, value);
	}
	else {
		int pos = 0;
		while (pos < len) {
			strncpy(temp_string, value + pos, 2047);
			temp_string[2047] = 0;
			kinc_log(KINC_LOG_LEVEL_INFO, temp_string);
			pos += 2047;
		}
	}
}

void krom_g4_clear(i32 flags, i32 color, f32 depth) {
	kinc_g4_clear(flags, color, depth, 0);
}

void krom_set_update_callback(void (*callback)(void)) {
	krom_update = callback;
	kinc_set_update_callback(update, NULL);
}

void krom_set_drop_files_callback(void (*callback)(wchar_t *)) {
	krom_drop_files = callback;
	kinc_set_drop_files_callback(drop_files, NULL);
}

void krom_set_cut_copy_paste_callback(any on_cut, any on_copy, any on_paste) {
	// kinc_set_cut_callback(cut, NULL);
	// kinc_set_copy_callback(copy, NULL);
	// kinc_set_paste_callback(paste, NULL);
	// SET_FUNC(cut_func, args[0]);
	// SET_FUNC(copy_func, args[1]);
	// SET_FUNC(paste_func, args[2]);
}

void krom_set_application_state_callback(any on_foreground, any on_resume, any on_pause, any on_background, any on_shutdown) {
	// kinc_set_foreground_callback(foreground, NULL);
	// kinc_set_resume_callback(resume, NULL);
	// kinc_set_pause_callback(pause, NULL);
	// kinc_set_background_callback(background, NULL);
	// kinc_set_shutdown_callback(shutdown, NULL);
	// SET_FUNC(foreground_func, args[0]);
	// SET_FUNC(resume_func, args[1]);
	// SET_FUNC(pause_func, args[2]);
	// SET_FUNC(background_func, args[3]);
	// SET_FUNC(shutdown_func, args[4]);
}

void krom_set_keyboard_down_callback(void (*callback)(int)) {
	krom_key_down = callback;
	kinc_keyboard_set_key_down_callback(key_down, NULL);
}

void krom_set_keyboard_up_callback(void (*callback)(int)) {
	krom_key_up = callback;
	kinc_keyboard_set_key_up_callback(key_up, NULL);
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
	kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)malloc(sizeof(kinc_g4_index_buffer_t));
	kinc_g4_index_buffer_init(buffer, count, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
	return buffer;
}

void krom_g4_delete_index_buffer(any buffer) {

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

any krom_g4_create_vertex_buffer(i32 count, kinc_vertex_elem_t_array_t *elements, i32 usage, i32 inst_data_step_rate) {
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

void krom_g4_delete_vertex_buffer(any buffer) {

}

buffer_t *krom_g4_lock_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	float *vertices = kinc_g4_vertex_buffer_lock_all(buffer);
	buffer_t *b = (buffer_t *)malloc(sizeof(buffer_t));
	b->data = vertices;
	b->length = kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer);
	return b;
}

void krom_g4_unlock_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_unlock_all(buffer);
}

void krom_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_set_vertex_buffer(buffer);
}

void krom_g4_set_vertex_buffers(vertex_buffer_t **vertex_buffers) {

}

void krom_g4_draw_indexed_vertices(i32 start, i32 count) {
	#ifdef KORE_DIRECT3D12
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

void krom_g4_draw_indexed_vertices_instanced(i32 inst_count, i32 start, i32 count) {

}

kinc_g4_shader_t *krom_g4_create_shader(buffer_t *data, i32 shader_type) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, data->data, data->length, (kinc_g4_shader_type_t)shader_type);
	return shader;
}

any krom_g4_create_vertex_shader_from_source(string_t *source) {
	return NULL;
}

any krom_g4_create_fragment_shader_from_source(string_t *source) {
	return NULL;
}

void krom_g4_delete_shader(any shader) {

}

kinc_g4_pipeline_t *krom_g4_create_pipeline() {
	kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)malloc(sizeof(kinc_g4_pipeline_t));
	kinc_g4_pipeline_init(pipeline);
	return pipeline;
}

void krom_g4_delete_pipeline(kinc_g4_pipeline_t *pipeline) {

}

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
		kinc_vertex_elem_t_array_t *elements = structure->elements;
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

	pipeline->cull_mode = state->cull_mode;
	pipeline->depth_write = state->depth_write;
	pipeline->depth_mode = state->depth_mode;
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

			#ifdef KORE_IOS // No RGBA128 filtering, convert to RGBA64
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

void krom_unload_image(image_t *image) {

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


any krom_g4_get_constant_location(kinc_g4_pipeline_t *pipeline, string_t *name) {
	kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, name);
	kinc_g4_constant_location_t *location_copy = (kinc_g4_constant_location_t *)malloc(sizeof(kinc_g4_constant_location_t));
	memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
	return location_copy;
}

any krom_g4_get_texture_unit(kinc_g4_pipeline_t *pipeline, string_t *name) {
	kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, name);
	kinc_g4_texture_unit_t *unit_copy = (kinc_g4_texture_unit_t *)malloc(sizeof(kinc_g4_texture_unit_t));
	memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
	return unit_copy;
}

void krom_g4_set_texture(kinc_g4_texture_unit_t *unit, kinc_g4_texture_t *texture) {
	kinc_g4_set_texture(*unit, texture);
}

void krom_g4_set_render_target(kinc_g4_texture_unit_t *unit, any render_target) {

}

void krom_g4_set_texture_depth(any unit, any texture) {

}

void krom_g4_set_image_texture(kinc_g4_texture_unit_t *unit, any texture) {

}

void krom_g4_set_texture_parameters(any tex_unit, i32 u_addr, i32 v_addr, i32 min_filter, i32 mag_filter, i32 mip_filter) {

}

void krom_g4_set_texture3d_parameters(any tex_unit, i32 u_addr, i32 v_addr, i32 w_addr, i32 min_filter, i32 mag_filter, i32 mip_filter) {

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
	kinc_g4_set_floats(*location, (float *)values->data, (int)(values->length / 4));
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
	return 1280;
}

i32 krom_window_height() {
	return 720;
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
	return 1;
}

i32 krom_display_width(i32 index) {
	return 1280;
}

i32 krom_display_height(i32 index) {
	return 720;
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
	return true;
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

void krom_g4_set_mipmaps(any texture, image_t **mipmaps) {

}

void krom_g4_set_depth_from(any target, any source) {

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
	return false;
}

void krom_g4_begin(image_t *render_target, image_t **additional) {
	// if (args[0]->IsNullOrUndefined()) {
		kinc_g4_restore_render_target();
	// }
	// else {
	// 	Local<Object> obj = TO_OBJ(OBJ_GET(TO_OBJ(args[0]), "render_target_"));
	// 	Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
	// 	kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)rtfield->Value();

	// 	int32_t length = 1;
	// 	kinc_g4_render_target_t *render_targets[8] = { render_target, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	// 	if (!args[1]->IsNullOrUndefined()) {
	// 		Local<Object> js_array = TO_OBJ(args[1]);
	// 		length = TO_I32(OBJ_GET(js_array, "length")) + 1;
	// 		if (length > 8) length = 8;
	// 		for (int32_t i = 1; i < length; ++i) {
	// 			Local<Object> artobj = TO_OBJ(OBJ_GET(TO_OBJ(ARRAY_GET(js_array, i - 1)), "render_target_"));
	// 			Local<External> artfield = Local<External>::Cast(artobj->GetInternalField(0));
	// 			kinc_g4_render_target_t *art = (kinc_g4_render_target_t *)artfield->Value();
	// 			render_targets[i] = art;
	// 		}
	// 	}
	// 	kinc_g4_set_render_targets(render_targets, length);
	// }
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

void krom_g2_draw_scaled_sub_image(image_t *image, f32 sx, f32 sy, f32 sw, f32 sh, f32 dx, f32 dy, f32 dw, f32 dh) {

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
// declare function krom_inflate(bytes: buffer_t, raw: bool): buffer_t;
// declare function krom_deflate(bytes: buffer_t, raw: bool): buffer_t;
// declare function krom_write_jpg(path: string, bytes: buffer_t, w: i32, h: i32, format: i32, quality: i32): void; // RGBA, R, RGB1, RRR1, GGG1, BBB1, AAA1
// declare function krom_write_png(path: string, bytes: buffer_t, w: i32, h: i32, format: i32): void;
// declare function krom_encode_jpg(bytes: buffer_t, w: i32, h: i32, format: i32, quality: i32): buffer_t;
// declare function krom_encode_png(bytes: buffer_t, w: i32, h: i32, format: i32): buffer_t;
// declare function krom_write_mpeg(): buffer_t;
// declare function krom_ml_inference(model: buffer_t, tensors: buffer_t[], input_shape?: i32[][], output_shape?: i32[], use_gpu?: bool): buffer_t;
// declare function krom_ml_unload(): void;

// declare function krom_raytrace_supported(): bool;
// declare function krom_raytrace_init(shader: buffer_t, vb: any, ib: any, scale: f32): void;
// declare function krom_raytrace_set_textures(tex0: image_t, tex1: image_t, tex2: image_t, texenv: any, tex_sobol: any, tex_scramble: any, tex_rank: any): void;
// declare function krom_raytrace_dispatch_rays(target: any, cb: buffer_t): void;

i32 krom_window_x() {
	return 0;
}

i32 krom_window_y() {
	return 0;
}

char *krom_language() {
	return NULL;
}
// declare function krom_io_obj_parse(file_bytes: buffer_t, split_code: i32, start_pos: i32, udim: bool): any;
