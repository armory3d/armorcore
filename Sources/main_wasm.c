#include <string.h>
#include <stdlib.h>
#include <kinc/log.h>
#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>
#include <kinc/audio2/audio.h>
#include <kinc/audio1/sound.h>
#include <kinc/input/mouse.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/pen.h>
#include <kinc/input/gamepad.h>
#include <kinc/math/random.h>
#include <kinc/math/core.h>
#include <kinc/threads/thread.h>
#include <kinc/threads/mutex.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <kinc/display.h>
#define STB_IMAGE_IMPLEMENTATION
#include <kinc/libs/stb_image.h>
#ifdef KORE_LZ4X
int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxOutputSize);
#else
#include <kinc/io/lz4/lz4.h>
#endif

const int KROM_API = 6;

static char temp_buffer[4096];
static kinc_image_t *last_image = NULL;
static int last_width = 0;
static int last_height = 0;
static int last_depth = 0;
static int reader_size = 0;

__attribute__((import_module("imports"), import_name("js_eval"))) void js_eval(const char *str);

static void (*update_func)() = NULL;
static void (*keyboard_down_func)(int) = NULL;
static void (*keyboard_up_func)(int) = NULL;
static void (*keyboard_press_func)(int) = NULL;
static void (*mouse_move_func)(int, int, int, int) = NULL;
static void (*mouse_down_func)(int, int, int) = NULL;
static void (*mouse_up_func)(int, int, int) = NULL;
static void (*mouse_wheel_func)(int) = NULL;

void update(void *data) {
	// #ifdef WITH_AUDIO
	// if (enable_sound) kinc_a2_update();
	// #endif

	kinc_g4_begin(0);

	// update_func();
	js_eval("js_update_func();");

	kinc_g4_end(0);
	kinc_g4_swap_buffers();
}

void key_down(int code) {
	keyboard_down_func(code);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void key_up(int code) {
	keyboard_up_func(code);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void key_press(unsigned int code) {
	keyboard_press_func(code);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void mouse_move(int window, int x, int y, int mx, int my, void *data) {
	mouse_move_func(x, y, mx, my);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void mouse_down(int window, int button, int x, int y, void *data) {
	mouse_down_func(button, x, y);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void mouse_up(int window, int button, int x, int y, void *data) {
	mouse_up_func(button, x, y);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void mouse_wheel(int window, int delta, void *data) {
	mouse_wheel_func(delta);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
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

bool ends_with(const char *str, const char *suffix) {
	if (!str || !suffix) return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr) return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

__attribute__((export_name("_malloc"))) char *_malloc(int size) {
	return malloc(size);
}

__attribute__((export_name("_get_temp_buffer"))) char *_get_temp_buffer() {
	return temp_buffer;
}

__attribute__((export_name("_get_last_image"))) kinc_image_t *_get_last_image() {
	return last_image;
}

__attribute__((export_name("_get_last_width"))) int _get_last_width() {
	return last_width;
}

__attribute__((export_name("_get_last_height"))) int _get_last_height() {
	return last_height;
}

__attribute__((export_name("_get_last_depth"))) int _get_last_depth() {
	return last_depth;
}

__attribute__((export_name("_init"))) void _init(char *title, int width, int height, int samples_per_pixel, bool vertical_sync, int window_mode, int window_features, int api_version, int x, int y) {
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
	win.title = title;
	win.x = x;
	win.y = y;
	win.width = width;
	win.height = height;
	win.display_index = -1;
	win.visible = true;
	win.window_features = window_features;
	win.mode = (kinc_window_mode_t)window_mode;
	kinc_framebuffer_options_t frame;
	frame.frequency = 60;
	frame.vertical_sync = vertical_sync;
	frame.color_bits = 32;
	frame.depth_bits = 0;
	frame.stencil_bits = 0;
	frame.samples_per_pixel = samples_per_pixel;
	kinc_init(title, width, height, &win, &frame);
	kinc_random_init((int)(kinc_time() * 1000));

	// #ifdef WITH_AUDIO
	// if (enable_sound) {
	// 	kinc_a2_init();
	// 	kinc_a2_set_callback(update_audio);
	// 	audio_buffer.read_location = 0;
	// 	audio_buffer.write_location = 0;
	// 	audio_buffer.data_size = 128 * 1024;
	// 	audio_buffer.data = new uint8_t[audio_buffer.data_size];
	// }
	// #endif

	kinc_set_update_callback(update, NULL);
	// kinc_set_drop_files_callback(drop_files);
	// kinc_set_copy_callback(copy);
	// kinc_set_cut_callback(cut);
	// kinc_set_paste_callback(paste);
	// kinc_set_foreground_callback(foreground);
	// kinc_set_resume_callback(resume);
	// kinc_set_pause_callback(pause);
	// kinc_set_background_callback(background);
	// kinc_set_shutdown_callback(shutdown);

	kinc_keyboard_set_key_down_callback(key_down);
	kinc_keyboard_set_key_up_callback(key_up);
	kinc_keyboard_set_key_press_callback(key_press);
	kinc_mouse_set_move_callback(mouse_move, NULL);
	kinc_mouse_set_press_callback(mouse_down, NULL);
	kinc_mouse_set_release_callback(mouse_up, NULL);
	kinc_mouse_set_scroll_callback(mouse_wheel, NULL);
	// kinc_surface_set_move_callback(touch_move);
	// kinc_surface_set_touch_start_callback(touch_down);
	// kinc_surface_set_touch_end_callback(touch_up);
	// kinc_pen_press_callback = pen_down;
	// kinc_pen_move_callback = pen_move;
	// kinc_pen_release_callback = pen_up;
	// kinc_gamepad_axis_callback = gamepad_axis;
	// kinc_gamepad_button_callback = gamepad_button;
}

__attribute__((export_name("_setApplicationName"))) void _setApplicationName(char *name) {

}

__attribute__((export_name("_log"))) void _log(char *message) {
	kinc_log(KINC_LOG_LEVEL_INFO, "%s", message);
}

__attribute__((export_name("_clear"))) void _clear(int flags, int color, float depth, int stencil) {
	kinc_g4_clear(flags, color, depth, stencil);
}

__attribute__((export_name("_setCallback"))) void _setCallback(void (*f)()) {
	update_func = f;
}

__attribute__((export_name("_setDropFilesCallback"))) void _setDropFilesCallback() {

}

__attribute__((export_name("_setCutCopyPasteCallback"))) void _setCutCopyPasteCallback() {

}

__attribute__((export_name("_setApplicationStateCallback"))) void _setApplicationStateCallback() {

}

__attribute__((export_name("_setKeyboardDownCallback"))) void _setKeyboardDownCallback(void (*f)(int)) {
	keyboard_down_func = f;
}

__attribute__((export_name("_setKeyboardUpCallback"))) void _setKeyboardUpCallback(void (*f)(int)) {
	keyboard_up_func = f;
}

__attribute__((export_name("_setKeyboardPressCallback"))) void _setKeyboardPressCallback(void (*f)(int)) {
	keyboard_press_func = f;
}

__attribute__((export_name("_setMouseMoveCallback"))) void _setMouseMoveCallback(void (*f)(int, int, int, int)) {
	mouse_move_func = f;
}

__attribute__((export_name("_setMouseDownCallback"))) void _setMouseDownCallback(void (*f)(int, int, int)) {
	mouse_down_func = f;
}

__attribute__((export_name("_setMouseUpCallback"))) void _setMouseUpCallback(void (*f)(int, int, int)) {
	mouse_up_func = f;
}

__attribute__((export_name("_setMouseWheelCallback"))) void _setMouseWheelCallback(void (*f)(int)) {
	mouse_wheel_func = f;
}

__attribute__((export_name("_setTouchDownCallback"))) void _setTouchDownCallback() {

}

__attribute__((export_name("_setTouchUpCallback"))) void _setTouchUpCallback() {

}

__attribute__((export_name("_setTouchMoveCallback"))) void _setTouchMoveCallback() {

}

__attribute__((export_name("_setPenDownCallback"))) void _setPenDownCallback() {

}

__attribute__((export_name("_setPenUpCallback"))) void _setPenUpCallback() {

}

__attribute__((export_name("_setPenMoveCallback"))) void _setPenMoveCallback() {

}

__attribute__((export_name("_setGamepadAxisCallback"))) void _setGamepadAxisCallback() {

}

__attribute__((export_name("_setGamepadButtonCallback"))) void _setGamepadButtonCallback() {

}

__attribute__((export_name("_lockMouse"))) void _lockMouse() {
	kinc_mouse_lock(0);
}

__attribute__((export_name("_unlockMouse"))) void _unlockMouse() {
	kinc_mouse_unlock();
}

__attribute__((export_name("_canLockMouse"))) bool _canLockMouse() {
	return kinc_mouse_can_lock();
}

__attribute__((export_name("_isMouseLocked"))) bool _isMouseLocked() {
	return kinc_mouse_is_locked();
}

__attribute__((export_name("_setMousePosition"))) void _setMousePosition(int windowId, int x, int y) {
	kinc_mouse_set_position(windowId, x, y);
}

__attribute__((export_name("_showMouse"))) void _showMouse(bool show) {
	show ? kinc_mouse_show() : kinc_mouse_hide();
}

__attribute__((export_name("_showKeyboard"))) void _showKeyboard(bool show) {
	// show ? kinc_keyboard_show() : kinc_keyboard_hide();
}

__attribute__((export_name("_createIndexBuffer"))) kinc_g4_index_buffer_t *_createIndexBuffer(int count) {
	kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)malloc(sizeof(kinc_g4_index_buffer_t));
	kinc_g4_index_buffer_init(buffer, count, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
	return buffer;
}

__attribute__((export_name("_deleteIndexBuffer"))) void _deleteIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_destroy(buffer);
	free(buffer);
}

__attribute__((export_name("_index_buffer_size"))) int _index_buffer_size(kinc_g4_index_buffer_t *buffer) {
	return kinc_g4_index_buffer_count(buffer);
}

__attribute__((export_name("_lockIndexBuffer"))) int *_lockIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	int *vertices = kinc_g4_index_buffer_lock(buffer);
	return vertices;
}

__attribute__((export_name("_unlockIndexBuffer"))) void _unlockIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_unlock(buffer);
}

__attribute__((export_name("_setIndexBuffer"))) void _setIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_set_index_buffer(buffer);
}

__attribute__((export_name("_createVertexBuffer"))) kinc_g4_vertex_buffer_t *_createVertexBuffer(int count, char *name0, int data0, char *name1, int data1, char *name2, int data2, char *name3, int data3, char *name4, int data4, char *name5, int data5, char *name6, int data6, char *name7, int data7, int usage, int instanceDataStepRate) {
	kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)malloc(sizeof(kinc_g4_vertex_buffer_t));

	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	if (name0 != NULL) kinc_g4_vertex_structure_add(&structure, name0, convert_vertex_data(data0));
	if (name1 != NULL) kinc_g4_vertex_structure_add(&structure, name1, convert_vertex_data(data1));
	if (name2 != NULL) kinc_g4_vertex_structure_add(&structure, name2, convert_vertex_data(data2));
	if (name3 != NULL) kinc_g4_vertex_structure_add(&structure, name3, convert_vertex_data(data3));
	if (name4 != NULL) kinc_g4_vertex_structure_add(&structure, name4, convert_vertex_data(data4));
	if (name5 != NULL) kinc_g4_vertex_structure_add(&structure, name5, convert_vertex_data(data5));
	if (name6 != NULL) kinc_g4_vertex_structure_add(&structure, name6, convert_vertex_data(data6));
	if (name7 != NULL) kinc_g4_vertex_structure_add(&structure, name7, convert_vertex_data(data7));

	kinc_g4_vertex_buffer_init(buffer, count, &structure, (kinc_g4_usage_t)usage, instanceDataStepRate);
	return buffer;
}

__attribute__((export_name("_deleteVertexBuffer"))) void _deleteVertexBuffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_destroy(buffer);
	free(buffer);
}

__attribute__((export_name("_vertex_buffer_size"))) int _vertex_buffer_size(kinc_g4_vertex_buffer_t *buffer) {
	return (kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer));
}

__attribute__((export_name("_lockVertexBuffer"))) float *_lockVertexBuffer(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	float *vertices = kinc_g4_vertex_buffer_lock(buffer, start, count);
	return vertices;
}

__attribute__((export_name("_unlockVertexBuffer"))) void _unlockVertexBuffer(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_g4_vertex_buffer_unlock(buffer, count);
}

__attribute__((export_name("_setVertexBuffer"))) void _setVertexBuffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_set_vertex_buffer(buffer);
}

__attribute__((export_name("_setVertexBuffers"))) void _setVertexBuffers(kinc_g4_vertex_buffer_t **buffers, int length) {
	kinc_g4_set_vertex_buffers(buffers, length);
}

__attribute__((export_name("_drawIndexedVertices"))) void _drawIndexedVertices(int start, int count) {
	if (count < 0) kinc_g4_draw_indexed_vertices();
	else kinc_g4_draw_indexed_vertices_from_to(start, count);
}

__attribute__((export_name("_drawIndexedVerticesInstanced"))) void _drawIndexedVerticesInstanced(int instance_count, int start, int count) {
	if (count < 0) kinc_g4_draw_indexed_vertices_instanced(instance_count);
	else kinc_g4_draw_indexed_vertices_instanced_from_to(instance_count, start, count);
}

__attribute__((export_name("_createVertexShader"))) kinc_g4_shader_t *_createVertexShader(char *data, int length, char *name) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, data, length, KINC_G4_SHADER_TYPE_VERTEX);
	return shader;
}

__attribute__((export_name("_createVertexShaderFromSource"))) kinc_g4_shader_t *_createVertexShaderFromSource(char *source) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	// kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_VERTEX);
	kinc_g4_shader_init(shader, source, strlen(source), KINC_G4_SHADER_TYPE_VERTEX);
	return shader;
}

__attribute__((export_name("_createFragmentShader"))) kinc_g4_shader_t *_createFragmentShader(char *data, int length, char *name) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, data, length, KINC_G4_SHADER_TYPE_FRAGMENT);
	return shader;
}

__attribute__((export_name("_createFragmentShaderFromSource"))) kinc_g4_shader_t *_createFragmentShaderFromSource(char *source) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	// kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_FRAGMENT);
	kinc_g4_shader_init(shader, source, strlen(source), KINC_G4_SHADER_TYPE_FRAGMENT);
	return shader;
}

__attribute__((export_name("_createGeometryShader"))) void _createGeometryShader() {

}

__attribute__((export_name("_createTessellationControlShader"))) void _createTessellationControlShader() {

}

__attribute__((export_name("_createTessellationEvaluationShader"))) void _createTessellationEvaluationShader() {

}

__attribute__((export_name("_deleteShader"))) void _deleteShader(kinc_g4_shader_t *shader) {
	kinc_g4_shader_destroy(shader);
	free(shader);
}

__attribute__((export_name("_createPipeline"))) kinc_g4_pipeline_t *_createPipeline() {
	kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)malloc(sizeof(kinc_g4_pipeline_t));
	kinc_g4_pipeline_init(pipeline);
	return pipeline;
}

__attribute__((export_name("_deletePipeline"))) void _deletePipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_pipeline_destroy(pipeline);
	free(pipeline);
}

__attribute__((export_name("_compilePipeline"))) void _compilePipeline(kinc_g4_pipeline_t *pipeline, char *name0, int data0, char *name1, int data1, char *name2, int data2, char *name3, int data3, char *name4, int data4, char *name5, int data5, char *name6, int data6, char *name7, int data7, kinc_g4_cull_mode_t cull_mode, bool depth_write, kinc_g4_compare_mode_t depth_mode, int blend_source, int blend_destination, int alpha_blend_source, int alpha_blend_destination, bool color_write_mask_red, bool color_write_mask_green, bool color_write_mask_blue, bool color_write_mask_alpha, int color_attachment_count, int depth_attachment_bits, int stencil_attachment_bits, bool conservative_rasterization, kinc_g4_shader_t *vertex_shader, kinc_g4_shader_t *fragment_shader) {
	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	if (name0 != NULL) kinc_g4_vertex_structure_add(&structure, name0, convert_vertex_data(data0));
	if (name1 != NULL) kinc_g4_vertex_structure_add(&structure, name1, convert_vertex_data(data1));
	if (name2 != NULL) kinc_g4_vertex_structure_add(&structure, name2, convert_vertex_data(data2));
	if (name3 != NULL) kinc_g4_vertex_structure_add(&structure, name3, convert_vertex_data(data3));
	if (name4 != NULL) kinc_g4_vertex_structure_add(&structure, name4, convert_vertex_data(data4));
	if (name5 != NULL) kinc_g4_vertex_structure_add(&structure, name5, convert_vertex_data(data5));
	if (name6 != NULL) kinc_g4_vertex_structure_add(&structure, name6, convert_vertex_data(data6));
	if (name7 != NULL) kinc_g4_vertex_structure_add(&structure, name7, convert_vertex_data(data7));

	pipeline->vertex_shader = vertex_shader;
	pipeline->fragment_shader = fragment_shader;
	pipeline->input_layout[0] = &structure;
	pipeline->input_layout[1] = NULL;
	pipeline->cull_mode = cull_mode;
	pipeline->depth_write = depth_write;
	pipeline->depth_mode = depth_mode;
	pipeline->blend_source = blend_source;
	pipeline->blend_destination = blend_destination;
	pipeline->alpha_blend_source = alpha_blend_source;
	pipeline->alpha_blend_destination = alpha_blend_destination;
	for (int i = 0; i < 8; ++i) {
		pipeline->color_write_mask_red[i] = color_write_mask_red;
		pipeline->color_write_mask_green[i] = color_write_mask_green;
		pipeline->color_write_mask_blue[i] = color_write_mask_blue;
		pipeline->color_write_mask_alpha[i] = color_write_mask_alpha;
	}
	pipeline->color_attachment_count = color_attachment_count;
	pipeline->depth_attachment_bits = depth_attachment_bits;
	pipeline->stencil_attachment_bits = stencil_attachment_bits;
	pipeline->conservative_rasterization = conservative_rasterization;

	kinc_g4_pipeline_compile(pipeline);
}

__attribute__((export_name("_setPipeline"))) void _setPipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_set_pipeline(pipeline);
}

__attribute__((export_name("_loadImage"))) kinc_g4_texture_t *_loadImage(char *file, bool readable) {
	bool success = true;
	kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));

	kinc_file_reader_t reader;
	if (kinc_file_reader_open(&reader, file, KINC_FILE_TYPE_ASSET)) {
		unsigned char *image_data;
		int image_width;
		int image_height;
		kinc_image_format_t image_format = KINC_IMAGE_FORMAT_RGBA32;
		int size = (int)kinc_file_reader_size(&reader);
		int comp;
		unsigned char *data = (unsigned char *)malloc(size);
		kinc_file_reader_read(&reader, data, size);
		kinc_file_reader_close(&reader);

		if (ends_with(file, "k")) {
			image_width = kinc_read_s32le(data);
			image_height = kinc_read_s32le(data + 4);
			char fourcc[5];
			fourcc[0] = data[8];
			fourcc[1] = data[9];
			fourcc[2] = data[10];
			fourcc[3] = data[11];
			fourcc[4] = 0;
			int compressedSize = size - 12;
			if (strcmp(fourcc, "LZ4 ") == 0) {
				int outputSize = image_width * image_height * 4;
				image_data = (unsigned char *)malloc(outputSize);
				LZ4_decompress_safe((char *)(data + 12), (char *)image_data, compressedSize, outputSize);
			}
			else if (strcmp(fourcc, "LZ4F") == 0) {
				int outputSize = image_width * image_height * 16;
				image_data = (unsigned char *)malloc(outputSize);
				LZ4_decompress_safe((char *)(data + 12), (char *)image_data, compressedSize, outputSize);
				image_format = KINC_IMAGE_FORMAT_RGBA128;
			}
			else {
				success = false;
			}
		}
		else if (ends_with(file, "hdr")) {
			image_data = (unsigned char *)stbi_loadf_from_memory(data, size, &image_width, &image_height, &comp, 4);
			if (image_data == NULL) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
				success = false;
			}
			image_format = KINC_IMAGE_FORMAT_RGBA128;
		}
		else { // jpg, png, ..
			image_data = stbi_load_from_memory(data, size, &image_width, &image_height, &comp, 4);
			if (image_data == NULL) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
				success = false;
			}
		}
		free(data);

		if (success) {
			kinc_image_init(image, image_data, image_width, image_height, image_format);
		}
	}
	else {
		success = false;
	}

	if (!success) {
		free(image);
		return NULL;
	}

	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init_from_image(texture, image);
	if (!readable) {
		free(image->data);
		kinc_image_destroy(image);
		free(image);
		// free(memory);
	}

	last_image = readable ? image : NULL;
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	return texture;
}

__attribute__((export_name("_unloadTexture"))) void _unloadTexture(kinc_g4_texture_t *texture) {
	kinc_g4_texture_destroy(texture);
	free(texture);
}

__attribute__((export_name("_unloadRenderTarget"))) void _unloadRenderTarget(kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_destroy(render_target);
	free(render_target);
}

__attribute__((export_name("_loadSound"))) void _loadSound() {

}

__attribute__((export_name("_setAudioCallback"))) void _setAudioCallback() {

}

__attribute__((export_name("_audioThread"))) void _audioThread() {

}

__attribute__((export_name("_writeAudioBuffer"))) void _writeAudioBuffer() {

}

__attribute__((export_name("_readerSize"))) int _readerSize() {
	return reader_size;
}

__attribute__((export_name("_loadBlob"))) char *_loadBlob(char *path) {
	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, path, KINC_FILE_TYPE_ASSET)) return NULL;
	reader_size = (int)kinc_file_reader_size(&reader);

	char *buffer = (char *)malloc(reader_size);
	kinc_file_reader_read(&reader, buffer, reader_size);
	kinc_file_reader_close(&reader);

	return buffer;
}

__attribute__((export_name("_loadUrl"))) void _loadUrl(char *url) {
	// kinc_load_url(url);
}

__attribute__((export_name("_copyToClipboard"))) void _copyToClipboard(char *str) {
	kinc_copy_to_clipboard(str);
}

__attribute__((export_name("_getConstantLocation"))) kinc_g4_constant_location_t *_getConstantLocation(kinc_g4_pipeline_t *pipeline, char *name) {
	kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, name);

	kinc_g4_constant_location_t *location_copy = (kinc_g4_constant_location_t *)malloc(sizeof(kinc_g4_constant_location_t));
	memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
	return location_copy;
}

__attribute__((export_name("_getTextureUnit"))) kinc_g4_texture_unit_t *_getTextureUnit(kinc_g4_pipeline_t *pipeline, char *name) {
	kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, name);

	kinc_g4_texture_unit_t *unit_copy = (kinc_g4_texture_unit_t *)malloc(sizeof(kinc_g4_texture_unit_t));
	memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
	return unit_copy;
}

__attribute__((export_name("_setTexture"))) void _setTexture(kinc_g4_texture_unit_t *unit, kinc_g4_texture_t *texture) {
	kinc_g4_set_texture(*unit, texture);
}

__attribute__((export_name("_setRenderTarget"))) void _setRenderTarget(kinc_g4_texture_unit_t *unit, kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_use_color_as_texture(render_target, *unit);
}

__attribute__((export_name("_setTextureDepth"))) void _setTextureDepth(kinc_g4_texture_unit_t *unit, kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_use_depth_as_texture(render_target, *unit);
}

__attribute__((export_name("_setImageTexture"))) void _setImageTexture(kinc_g4_texture_unit_t *unit, kinc_g4_texture_t *texture) {
	kinc_g4_set_image_texture(*unit, texture);
}

__attribute__((export_name("_setTextureParameters"))) void _setTextureParameters(kinc_g4_texture_unit_t *unit, int uAddressing, int vAddressing, int minificationFilter, int magnificationFilter, int mipmapFilter) {
	kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)uAddressing);
	kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)vAddressing);
	kinc_g4_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)minificationFilter);
	kinc_g4_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)magnificationFilter);
	kinc_g4_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)mipmapFilter);
}

__attribute__((export_name("_setTexture3DParameters"))) void _setTexture3DParameters(kinc_g4_texture_unit_t *unit, int uAddressing, int vAddressing, int wAddressing, int minificationFilter, int magnificationFilter, int mipmapFilter) {
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)uAddressing);
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)vAddressing);
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)wAddressing);
	kinc_g4_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)minificationFilter);
	kinc_g4_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)magnificationFilter);
	kinc_g4_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)mipmapFilter);
}

__attribute__((export_name("_setTextureCompareMode"))) void _setTextureCompareMode(kinc_g4_texture_unit_t *unit, bool enabled) {
	kinc_g4_set_texture_compare_mode(*unit, enabled);
}

__attribute__((export_name("_setCubeMapCompareMode"))) void _setCubeMapCompareMode(kinc_g4_texture_unit_t *unit, bool enabled) {
	kinc_g4_set_cubemap_compare_mode(*unit, enabled);
}

__attribute__((export_name("_setBool"))) void _setBool(kinc_g4_constant_location_t *location, bool value) {
	kinc_g4_set_bool(*location, value);
}

__attribute__((export_name("_setInt"))) void _setInt(kinc_g4_constant_location_t *location, int value) {
	kinc_g4_set_int(*location, value);
}

__attribute__((export_name("_setFloat"))) void _setFloat(kinc_g4_constant_location_t *location, float value) {
	kinc_g4_set_float(*location, value);
}

__attribute__((export_name("_setFloat2"))) void _setFloat2(kinc_g4_constant_location_t *location, float value1, float value2) {
	kinc_g4_set_float2(*location, value1, value2);
}

__attribute__((export_name("_setFloat3"))) void _setFloat3(kinc_g4_constant_location_t *location, float value1, float value2, float value3) {
	kinc_g4_set_float3(*location, value1, value2, value3);
}

__attribute__((export_name("_setFloat4"))) void _setFloat4(kinc_g4_constant_location_t *location, float value1, float value2, float value3, float value4) {
	kinc_g4_set_float4(*location, value1, value2, value3, value4);
}

__attribute__((export_name("_setFloats"))) void _setFloats(kinc_g4_constant_location_t *location, float *from, int count) {
	kinc_g4_set_floats(*location, from, count);
}

__attribute__((export_name("_setMatrix"))) void _setMatrix(kinc_g4_constant_location_t *location, float *from) {
	kinc_g4_set_matrix4(*location, (kinc_matrix4x4_t *)from);
}

__attribute__((export_name("_setMatrix3"))) void _setMatrix3(kinc_g4_constant_location_t *location, float *from) {
	kinc_g4_set_matrix3(*location, (kinc_matrix3x3_t *)from);
}

__attribute__((export_name("_getTime"))) double _getTime() {
	return kinc_time();
}

__attribute__((export_name("_windowWidth"))) int _windowWidth(int id) {
	return kinc_window_width(id);
}

__attribute__((export_name("_windowHeight"))) int _windowHeight(int id) {
	return kinc_window_height(id);
}

__attribute__((export_name("_setWindowTitle"))) void _setWindowTitle(int id, char *title) {
	kinc_window_set_title(id, title);
}

__attribute__((export_name("_screenDpi"))) int _screenDpi() {
	return kinc_display_current_mode(kinc_primary_display()).pixels_per_inch;
}

__attribute__((export_name("_systemId"))) const char *_systemId() {
	// return kinc_system_id();
	return NULL;
}

__attribute__((export_name("_requestShutdown"))) void _requestShutdown() {
	kinc_stop();
}

__attribute__((export_name("_displayCount"))) int _displayCount() {
	return kinc_count_displays();
}

__attribute__((export_name("_displayWidth"))) int _displayWidth(int index) {
	return kinc_display_current_mode(index).width;
}

__attribute__((export_name("_displayHeight"))) int _displayHeight(int index) {
	return kinc_display_current_mode(index).height;
}

__attribute__((export_name("_displayX"))) int _displayX(int index) {
	return kinc_display_current_mode(index).x;
}

__attribute__((export_name("_displayY"))) int _displayY(int index) {
	return kinc_display_current_mode(index).y;
}

__attribute__((export_name("_displayFrequency"))) int _displayFrequency(int index) {
	return kinc_display_current_mode(index).frequency;
}

__attribute__((export_name("_displayIsPrimary"))) bool _displayIsPrimary(int index) {
	return index == kinc_primary_display();
}

__attribute__((export_name("_writeStorage"))) void _writeStorage() {

}

__attribute__((export_name("_readStorage"))) void _readStorage() {

}

__attribute__((export_name("_createRenderTarget"))) kinc_g4_render_target_t *_createRenderTarget(int width, int height, int format, int depthBufferBits, int stencilBufferBits) {
	kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
	kinc_g4_render_target_init(render_target, width, height, (kinc_g4_render_target_format_t)format, depthBufferBits, stencilBufferBits);
	last_width = render_target->width;
	last_height = render_target->height;
	return render_target;
}

__attribute__((export_name("_createRenderTargetCubeMap"))) kinc_g4_render_target_t *_createRenderTargetCubeMap(int size, int format, int depthBufferBits, int stencilBufferBits) {
	kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
	kinc_g4_render_target_init_cube(render_target, size, (kinc_g4_render_target_format_t)format, depthBufferBits, stencilBufferBits);
	last_width = render_target->width;
	last_height = render_target->height;
	return render_target;
}

__attribute__((export_name("_createTexture"))) kinc_g4_texture_t *_createTexture(int width, int height, int format) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init(texture, width, height, (kinc_image_format_t)format);
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	return texture;
}

__attribute__((export_name("_createTexture3D"))) kinc_g4_texture_t *_createTexture3D(int width, int height, int depth, int format) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init3d(texture, width, height, depth, (kinc_image_format_t)format);
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	last_depth = texture->tex_depth;
	return texture;
}

__attribute__((export_name("_createTextureFromBytes"))) kinc_g4_texture_t *_createTextureFromBytes(char *data, int width, int height, int format, bool readable) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_image_t image;
	kinc_image_init(&image, data, width, height, (kinc_image_format_t)format);
	kinc_g4_texture_init_from_image(texture, &image);
	kinc_image_destroy(&image);
	if (!readable) {
		free(data);
	}
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	return texture;
}

__attribute__((export_name("_createTextureFromBytes3D"))) kinc_g4_texture_t *_createTextureFromBytes3D(char *data, int width, int height, int depth, int format, bool readable) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_image_t image;
	kinc_image_init3d(&image, data, width, height, depth, (kinc_image_format_t)format);
	kinc_g4_texture_init_from_image3d(texture, &image);
	kinc_image_destroy(&image);
	if (!readable) {
		free(data);
	}
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	last_depth = texture->tex_depth;
	return texture;
}

__attribute__((export_name("_createTextureFromEncodedBytes"))) kinc_g4_texture_t *_createTextureFromEncodedBytes(unsigned char *content_data, int content_length, char *format, bool readable) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_image_t *image = (kinc_image_t *)malloc(sizeof(kinc_image_t));

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
		int compressedSize = content_length - 12;
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

	last_image = readable ? image : NULL;
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	return texture;
}

__attribute__((export_name("_getTexturePixels"))) uint8_t *_getTexturePixels(kinc_image_t *image) {
	uint8_t *data = kinc_image_get_pixels(image);
	int byteLength = format_byte_size(image->format) * image->width * image->height * image->depth;
	return data;
}

__attribute__((export_name("_getRenderTargetPixels"))) void _getRenderTargetPixels(kinc_g4_render_target_t *render_target, uint8_t *out) {
	kinc_g4_render_target_get_pixels(render_target, out);
}

__attribute__((export_name("_lockTexture"))) uint8_t *_lockTexture(kinc_g4_texture_t *texture, int level) {
	uint8_t *tex = kinc_g4_texture_lock(texture);
	// int byteLength = kinc_g4_texture_stride(texture) * texture->tex_height * texture->tex_depth;
	return tex;
}

__attribute__((export_name("_unlockTexture"))) void _unlockTexture(kinc_g4_texture_t *texture) {
	kinc_g4_texture_unlock(texture);
}

__attribute__((export_name("_clearTexture"))) void _clearTexture(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, int color) {
	// kinc_g4_texture_clear(texture, x, y, z, width, height, depth, color);
}

__attribute__((export_name("_generateTextureMipmaps"))) void _generateTextureMipmaps(kinc_g4_texture_t *texture, int levels) {
	kinc_g4_texture_generate_mipmaps(texture, levels);
}

__attribute__((export_name("_generateRenderTargetMipmaps"))) void _generateRenderTargetMipmaps(kinc_g4_render_target_t *render_target, int levels) {
	kinc_g4_render_target_generate_mipmaps(render_target, levels);
}

__attribute__((export_name("_setMipmap"))) void _setMipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level) {
	kinc_g4_texture_set_mipmap(texture, mipmap, level);
}

__attribute__((export_name("_setDepthStencilFrom"))) void _setDepthStencilFrom(kinc_g4_render_target_t *render_target, kinc_g4_render_target_t *source_target) {
	kinc_g4_render_target_set_depth_stencil_from(render_target, source_target);
}

__attribute__((export_name("_viewport"))) void _viewport(int x, int y, int w, int h) {
	kinc_g4_viewport(x, y, w, h);
}

__attribute__((export_name("_scissor"))) void _scissor(int x, int y, int w, int h) {
	kinc_g4_scissor(x, y, w, h);
}

__attribute__((export_name("_disableScissor"))) void _disableScissor() {
	kinc_g4_disable_scissor();
}

__attribute__((export_name("_renderTargetsInvertedY"))) bool _renderTargetsInvertedY() {
	return kinc_g4_render_targets_inverted_y();
}

__attribute__((export_name("_begin"))) void _begin(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *renderTarget1, kinc_g4_render_target_t *renderTarget2, kinc_g4_render_target_t *renderTarget3, kinc_g4_render_target_t *renderTarget4, kinc_g4_render_target_t *renderTarget5, kinc_g4_render_target_t *renderTarget6, kinc_g4_render_target_t *renderTarget7) {
	if (renderTarget == NULL) {
		kinc_g4_restore_render_target();
	}
	else {
		int32_t length = renderTarget7 != NULL ? 8 :
						 renderTarget6 != NULL ? 7 :
						 renderTarget5 != NULL ? 6 :
						 renderTarget4 != NULL ? 5 :
						 renderTarget3 != NULL ? 4 :
						 renderTarget2 != NULL ? 3 :
						 renderTarget1 != NULL ? 2 : 1;
		kinc_g4_render_target_t *render_targets[8] = { renderTarget, renderTarget1, renderTarget2, renderTarget3, renderTarget4, renderTarget5, renderTarget6, renderTarget7 };
		kinc_g4_set_render_targets(render_targets, length);
	}
}

__attribute__((export_name("_beginFace"))) void _beginFace(kinc_g4_render_target_t *render_target, int face) {
	kinc_g4_set_render_target_face(render_target, face);
}

__attribute__((export_name("_end"))) void _end() {

}

__attribute__((export_name("_fileSaveBytes"))) void _fileSaveBytes() {

}

__attribute__((export_name("_sysCommand"))) int _sysCommand(char *cmd) {
	// return system(cmd);
	return 0;
}

__attribute__((export_name("_savePath"))) const char *_savePath() {
	// return kinc_internal_save_path();
	return NULL;
}

__attribute__((export_name("_getArgCount"))) int _getArgCount() {
	return 0;
}

__attribute__((export_name("_getArg"))) const char *_getArg(int index) {
	return NULL;
}

__attribute__((export_name("_getFilesLocation"))) const char *_getFilesLocation() {
	return kinc_internal_get_files_location();
}

__attribute__((export_name("_setBoolCompute"))) void _setBoolCompute() {

}

__attribute__((export_name("_setIntCompute"))) void _setIntCompute() {

}

__attribute__((export_name("_setFloatCompute"))) void _setFloatCompute() {

}

__attribute__((export_name("_setFloat2Compute"))) void _setFloat2Compute() {

}

__attribute__((export_name("_setFloat3Compute"))) void _setFloat3Compute() {

}

__attribute__((export_name("_setFloat4Compute"))) void _setFloat4Compute() {

}

__attribute__((export_name("_setFloatsCompute"))) void _setFloatsCompute() {

}

__attribute__((export_name("_setMatrixCompute"))) void _setMatrixCompute() {

}

__attribute__((export_name("_setMatrix3Compute"))) void _setMatrix3Compute() {

}

__attribute__((export_name("_setTextureCompute"))) void _setTextureCompute() {

}

__attribute__((export_name("_setRenderTargetCompute"))) void _setRenderTargetCompute() {

}

__attribute__((export_name("_setSampledTextureCompute"))) void _setSampledTextureCompute() {

}

__attribute__((export_name("_setSampledRenderTargetCompute"))) void _setSampledRenderTargetCompute() {

}

__attribute__((export_name("_setSampledDepthTextureCompute"))) void _setSampledDepthTextureCompute() {

}

__attribute__((export_name("_setTextureParametersCompute"))) void _setTextureParametersCompute() {

}

__attribute__((export_name("_setTexture3DParametersCompute"))) void _setTexture3DParametersCompute() {

}

__attribute__((export_name("_setShaderCompute"))) void _setShaderCompute() {

}

__attribute__((export_name("_deleteShaderCompute"))) void _deleteShaderCompute() {

}

__attribute__((export_name("_createShaderCompute"))) void _createShaderCompute() {

}

__attribute__((export_name("_getConstantLocationCompute"))) void _getConstantLocationCompute() {

}

__attribute__((export_name("_getTextureUnitCompute"))) void _getTextureUnitCompute() {

}

__attribute__((export_name("_compute"))) void _compute() {

}

__attribute__((export_name("_setSaveAndQuitCallback"))) void _setSaveAndQuitCallback() {

}

__attribute__((export_name("_setMouseCursor"))) void _setMouseCursor() {

}

__attribute__((export_name("_windowX"))) int _windowX(int id) {
	return kinc_window_x(id);
}

__attribute__((export_name("_windowY"))) int _windowY(int id) {
	return kinc_window_y(id);
}

int kickstart(int argc, char **argv) {
	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, "krom.js", KINC_FILE_TYPE_ASSET)) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not load krom.js, aborting.");
		exit(1);
	}

	int reader_size = (int)kinc_file_reader_size(&reader);
	char *code_krom = (char *)malloc(reader_size + 1);
	kinc_file_reader_read(&reader, code_krom, reader_size);
	code_krom[reader_size] = 0;
	kinc_file_reader_close(&reader);

	char *code_header = "\
	let string = function(str, off = 0) { if (str == null) return null; let ptr = exports._get_temp_buffer() + off; write_string(ptr, str); return ptr; };\
	let update_func = null;\
	function js_update_func() { update_func(); }\
	let KromBuffers = new Map();\
	let Krom = {};\
	let exports = instance.exports;\
	Krom.init = function(title, width, height, samplesPerPixel, vSync, frequency, windowMode, windowFeatures, kromApi, x, y) { exports._init(string(title), width, height, samplesPerPixel, vSync, frequency, windowMode, windowFeatures, kromApi, x, y); };\
	Krom.setApplicationName = function(str) { exports._setApplicationName(string(str)); };\
	Krom.log = function(str) { exports._log(string(str)); };\
	Krom.clear = exports._clear;\
	Krom.setCallback = function(f) { update_func = f; /*exports._setCallback(f);*/ };\
	Krom.setDropFilesCallback = function(f) { /*exports._setDropFilesCallback(f);*/ };\
	Krom.setCutCopyPasteCallback = function(f) { /*exports._setCutCopyPasteCallback(f);*/ };\
	Krom.setApplicationStateCallback = function(f) { /*exports._setApplicationStateCallback(f);*/ };\
	Krom.setKeyboardDownCallback = function(f) { /*exports._setKeyboardDownCallback(f);*/ };\
	Krom.setKeyboardUpCallback = function(f) { /*exports._setKeyboardUpCallback(f);*/ };\
	Krom.setKeyboardPressCallback = function(f) { /*exports._setKeyboardPressCallback(f);*/ };\
	Krom.setMouseMoveCallback = function(f) { /*exports._setMouseMoveCallback(f);*/ };\
	Krom.setMouseDownCallback = function(f) { /*exports._setMouseDownCallback(f);*/ };\
	Krom.setMouseUpCallback = function(f) { /*exports._setMouseUpCallback(f);*/ };\
	Krom.setMouseWheelCallback = function(f) { /*exports._setMouseWheelCallback(f);*/ };\
	Krom.setPenDownCallback = function(f) { /*exports._setPenDownCallback(f);*/ };\
	Krom.setPenUpCallback = function(f) { /*exports._setPenUpCallback(f);*/ };\
	Krom.setPenMoveCallback = function(f) { /*exports._setPenMoveCallback(f);*/ };\
	Krom.setGamepadAxisCallback = function(f) { /*exports._setGamepadAxisCallback(f);*/ };\
	Krom.setGamepadButtonCallback = function(f) { /*exports._setGamepadButtonCallback(f);*/ };\
	Krom.setTouchDownCallback = function(f) { /*exports._setTouchDownCallback(f);*/ };\
	Krom.setTouchUpCallback = function(f) { /*exports._setTouchUpCallback(f);*/ };\
	Krom.setTouchMoveCallback = function(f) { /*exports._setTouchMoveCallback(f);*/ };\
	Krom.lockMouse = exports._lockMouse;\
	Krom.unlockMouse = exports._unlockMouse;\
	Krom.canLockMouse = exports._canLockMouse;\
	Krom.isMouseLocked = exports._isMouseLocked;\
	Krom.setMousePosition = exports._setMousePosition;\
	Krom.showMouse = exports._showMouse;\
	Krom.showKeyboard = exports._showKeyboard;\
	Krom.createIndexBuffer = exports._createIndexBuffer;\
	Krom.deleteIndexBuffer = exports._deleteIndexBuffer;\
	Krom.lockIndexBuffer = function(buffer) {\
		return new Uint32Array(heapu8.buffer, exports._lockIndexBuffer(buffer), exports._index_buffer_size(buffer));\
	};\
	Krom.unlockIndexBuffer = exports._unlockIndexBuffer;\
	Krom.setIndexBuffer = exports._setIndexBuffer;\
	Krom.createVertexBuffer = function(count, structure, usage, instanceDataStepRate) {\
		let name0 = structure.length > 0 ? structure[0].name : null;\
		let data0 = structure.length > 0 ? structure[0].data : null;\
		let name1 = structure.length > 1 ? structure[1].name : null;\
		let data1 = structure.length > 1 ? structure[1].data : null;\
		let name2 = structure.length > 2 ? structure[2].name : null;\
		let data2 = structure.length > 2 ? structure[2].data : null;\
		let name3 = structure.length > 3 ? structure[3].name : null;\
		let data3 = structure.length > 3 ? structure[3].data : null;\
		let name4 = structure.length > 4 ? structure[4].name : null;\
		let data4 = structure.length > 4 ? structure[4].data : null;\
		let name5 = structure.length > 5 ? structure[5].name : null;\
		let data5 = structure.length > 5 ? structure[5].data : null;\
		let name6 = structure.length > 6 ? structure[6].name : null;\
		let data6 = structure.length > 6 ? structure[6].data : null;\
		let name7 = structure.length > 7 ? structure[7].name : null;\
		let data7 = structure.length > 7 ? structure[7].data : null;\
		return exports._createVertexBuffer(count, string(name0), data0, string(name1, 64 * 1), data1, string(name2, 64 * 2), data2, string(name3, 64 * 3), data3, string(name4, 64 * 4), data4, string(name5, 64 * 5), data5, string(name6, 64 * 6), data6, string(name7, 64 * 7), data7, usage, instanceDataStepRate);\
	};\
	Krom.deleteVertexBuffer = exports._deleteVertexBuffer;\
	Krom.lockVertexBuffer = function(vbuffer, vstart, vcount) {\
		let start = exports._lockVertexBuffer(vbuffer, vstart, vcount);\
		let byteLength = exports._vertex_buffer_size(vbuffer);\
		let b = heapu8.buffer.slice(start, start + byteLength);\
		KromBuffers.set(vbuffer, { buffer: b, start: start, byteLength: byteLength });\
		return b;\
	};\
	Krom.unlockVertexBuffer = function(vbuffer, count) {\
		let b = new Uint8Array(KromBuffers.get(vbuffer).buffer);\
		let start = KromBuffers.get(vbuffer).start;\
		let byteLength = KromBuffers.get(vbuffer).byteLength;\
		let u8 = new Uint8Array(heapu8.buffer, start, byteLength);\
		for (let i = 0; i < byteLength; ++i) u8[i] = b[i];\
		exports._unlockVertexBuffer(vbuffer, count);\
	};\
	Krom.setVertexBuffer = exports._setVertexBuffer;\
	Krom.setVertexBuffers = exports._setVertexBuffers;\
	Krom.drawIndexedVertices = exports._drawIndexedVertices;\
	Krom.drawIndexedVerticesInstanced = exports._drawIndexedVerticesInstanced;\
	Krom.createVertexShader = function(buffer, name) {\
		return exports._createVertexShader(KromBuffers.get(buffer).start, KromBuffers.get(buffer).byteLength, string(name));\
	};\
	Krom.createVertexShaderFromSource = function(str) { return exports._createVertexShaderFromSource(string(str)); };\
	Krom.createFragmentShader = function(buffer, name) {\
		return exports._createFragmentShader(KromBuffers.get(buffer).start, KromBuffers.get(buffer).byteLength, string(name));\
	};\
	Krom.createFragmentShaderFromSource = function(str) { return exports._createFragmentShaderFromSource(string(str)); };\
	Krom.createGeometryShader = exports._createGeometryShader;\
	Krom.createTessellationControlShader = exports._createTessellationControlShader;\
	Krom.createTessellationEvaluationShader = exports._createTessellationEvaluationShader;\
	Krom.deleteShader = exports._deleteShader;\
	Krom.createPipeline = exports._createPipeline;\
	Krom.deletePipeline = exports._deletePipeline;\
	Krom.compilePipeline = function(pipeline, structure0, structure1, structure2, structure3, length, vertexShader, fragmentShader, geometryShader, tessellationControlShader, tessellationEvaluationShader, state) {\
		let name0 = structure0.elements.length > 0 ? structure0.elements[0].name : null;\
		let data0 = structure0.elements.length > 0 ? structure0.elements[0].data : null;\
		let name1 = structure0.elements.length > 1 ? structure0.elements[1].name : null;\
		let data1 = structure0.elements.length > 1 ? structure0.elements[1].data : null;\
		let name2 = structure0.elements.length > 2 ? structure0.elements[2].name : null;\
		let data2 = structure0.elements.length > 2 ? structure0.elements[2].data : null;\
		let name3 = structure0.elements.length > 3 ? structure0.elements[3].name : null;\
		let data3 = structure0.elements.length > 3 ? structure0.elements[3].data : null;\
		let name4 = structure0.elements.length > 4 ? structure0.elements[4].name : null;\
		let data4 = structure0.elements.length > 4 ? structure0.elements[4].data : null;\
		let name5 = structure0.elements.length > 5 ? structure0.elements[5].name : null;\
		let data5 = structure0.elements.length > 5 ? structure0.elements[5].data : null;\
		let name6 = structure0.elements.length > 6 ? structure0.elements[6].name : null;\
		let data6 = structure0.elements.length > 6 ? structure0.elements[6].data : null;\
		let name7 = structure0.elements.length > 7 ? structure0.elements[7].name : null;\
		let data7 = structure0.elements.length > 7 ? structure0.elements[7].data : null;\
		exports._compilePipeline(pipeline, string(name0, 0), data0, string(name1, 64 * 1), data1, string(name2, 64 * 2), data2, string(name3, 64 * 3), data3, string(name4, 64 * 4), data4, string(name5, 64 * 5), data5, string(name6, 64 * 6), data6, string(name7, 64 * 7), data7, state.cullMode, state.depthWrite, state.depthMode, state.blendSource, state.blendDestination, state.alphaBlendSource, state.alphaBlendDestination, state.colorWriteMaskRed[0], state.colorWriteMaskGreen[0], state.colorWriteMaskBlue[0], state.colorWriteMaskAlpha[0], state.colorAttachmentCount, state.depthAttachmentBits, state.stencilAttachmentBits, state.conservativeRasterization, vertexShader, fragmentShader);\
	};\
	Krom.setPipeline = exports._setPipeline;\
	Krom.loadImage = function (file, readable) {\
		return { self: exports._loadImage(string(file), readable),\
				 image: exports._get_last_image(),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height()\
		};\
	};\
	Krom.unloadImage = function(image) { image.texture_ != null ? exports._unloadTexture(image.texture_.self) : exports._unloadRenderTarget(image.renderTarget_.self); };\
	Krom.loadSound = exports._loadSound;\
	Krom.setAudioCallback = exports._setAudioCallback;\
	Krom.audioThread = exports._audioThread;\
	Krom.writeAudioBuffer = exports._writeAudioBuffer;\
	Krom.loadBlob = function(path) {\
		let start = exports._loadBlob(string(path));\
		let b = heapu8.buffer.slice(start, start + exports._readerSize());\
		KromBuffers.set(b, { start: start, byteLength: exports._readerSize() });\
		return b;\
	};\
	Krom.loadUrl = exports._loadUrl;\
	Krom.copyToClipboard = exports._copyToClipboard;\
	Krom.getConstantLocation = exports._getConstantLocation;\
	Krom.getTextureUnit = exports._getTextureUnit;\
	Krom.setTexture = function(unit, texture) { exports._setTexture(unit, texture.self); };\
	Krom.setRenderTarget = function(unit, renderTarget) { exports._setRenderTarget(unit, renderTarget.self); };\
	Krom.setTextureDepth = function(unit, renderTarget) { exports._setTextureDepth(unit, renderTarget.self); };\
	Krom.setImageTexture = function(unit, texture) { exports._setImageTexture(unit, texture.self); };\
	Krom.setTextureParameters = exports._setTextureParameters;\
	Krom.setTexture3DParameters = exports._setTexture3DParameters;\
	Krom.setTextureCompareMode = exports._setTextureCompareMode;\
	Krom.setCubeMapCompareMode = exports._setCubeMapCompareMode;\
	Krom.setBool = exports._setBool;\
	Krom.setInt = exports._setInt;\
	Krom.setFloat = exports._setFloat;\
	Krom.setFloat2 = exports._setFloat2;\
	Krom.setFloat3 = exports._setFloat3;\
	Krom.setFloat4 = exports._setFloat4;\
	Krom.setFloats = function(location, floats) {\
		let to = new Uint8Array(heapu8.buffer, exports._get_temp_buffer(), 4096);\
		let from = new Uint8Array(floats);\
		for (let i = 0; i < floats.byteLength; ++i) to[i] = from[i];\
		exports._setFloats(location, exports._get_temp_buffer(), floats.byteLength / 4);\
	};\
	Krom.setMatrix = function(location, matrix) {\
		let to = new Uint8Array(heapu8.buffer, exports._get_temp_buffer(), 4096);\
		let from = new Uint8Array(matrix);\
		for (let i = 0; i < matrix.byteLength; ++i) to[i] = from[i];\
		exports._setMatrix(location, exports._get_temp_buffer());\
	};\
	Krom.setMatrix3 = function(location, matrix) {\
		let to = new Uint8Array(heapu8.buffer, exports._get_temp_buffer(), 4096);\
		let from = new Uint8Array(matrix);\
		for (let i = 0; i < matrix.byteLength; ++i) to[i] = from[i];\
		exports._setMatrix3(location, exports._get_temp_buffer());\
	};\
	Krom.getTime = exports._getTime;\
	Krom.windowWidth = exports._windowWidth;\
	Krom.windowHeight = exports._windowHeight;\
	Krom.setWindowTitle = exports._setWindowTitle;\
	Krom.screenDpi = exports._screenDpi;\
	Krom.systemId = exports._systemId;\
	Krom.requestShutdown = exports._requestShutdown;\
	Krom.displayCount = exports._displayCount;\
	Krom.displayWidth = exports._displayWidth;\
	Krom.displayHeight = exports._displayHeight;\
	Krom.displayX = exports._displayX;\
	Krom.displayY = exports._displayY;\
	Krom.displayFrequency = exports._displayFrequency;\
	Krom.displayIsPrimary = exports._displayIsPrimary;\
	Krom.writeStorage = exports._writeStorage;\
	Krom.readStorage = exports._readStorage;\
	Krom.createRenderTarget = function(width, height, format, depthBufferBits, stencilBufferBits) {\
		return { self: exports._createRenderTarget(width, height, format, depthBufferBits, stencilBufferBits),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height()\
		};\
	};\
	Krom.createRenderTargetCubeMap = function(size, format, depthBufferBits, stencilBufferBits) {\
		return { self: exports._createRenderTargetCubeMap(size, format, depthBufferBits, stencilBufferBits),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height()\
		};\
	};\
	Krom.createTexture = function(width, height, format) {\
		return { self: exports._createTexture(width, height, format),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height()\
		};\
	};\
	Krom.createTexture3D = function(width, height, depth, format) {\
		return { self: exports._createTexture3D(width, height, depth, format),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height(),\
				 depth: exports._get_last_depth()\
		};\
	};\
	Krom.createTextureFromBytes = function(data, width, height, format, readable) {\
		let from = new Uint8Array(data);\
		let start = exports._malloc(data.byteLength);\
		let to = new Uint8Array(heapu8.buffer, start, data.byteLength);\
		for (let i = 0; i < data.byteLength; ++i) to[i] = from[i];\
		return { self: exports._createTextureFromBytes(start, width, height, format, readable),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height()\
		};\
	};\
	Krom.createTextureFromBytes3D = function(data, width, height, depth, format, readable) {\
		let from = new Uint8Array(data);\
		let start = exports._malloc(data.byteLength);\
		let to = new Uint8Array(heapu8.buffer, start, data.byteLength);\
		for (let i = 0; i < data.byteLength; ++i) to[i] = from[i];\
		return { self: exports._createTextureFromBytes3D(start, width, height, depth, format, readable),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height(),\
				 depth: exports._get_last_depth()\
		};\
	};\
	Krom.createTextureFromEncodedBytes = function(data, format, readable) {\
		let from = new Uint8Array(data);\
		let start = exports._malloc(data.byteLength);\
		let to = new Uint8Array(heapu8.buffer, start, data.byteLength);\
		for (let i = 0; i < data.byteLength; ++i) to[i] = from[i];\
		return { self: exports._createTextureFromEncodedBytes(start, data.byteLength, format, readable),\
				 image: exports._get_last_image(),\
				 width: exports._get_last_width(),\
				 height: exports._get_last_height()\
		};\
	};\
	Krom.getTexturePixels = function(texture) { return exports._getTexturePixels(texture.self); };\
	Krom.getRenderTargetPixels = function(renderTarget, out) { return exports._getRenderTargetPixels(renderTarget.self, out); };\
	Krom.lockTexture = function(texture, level) { return exports._lockTexture(texture.self, level); };\
	Krom.unlockTexture = function(texture) { exports._unlockTexture(texture.self); };\
	Krom.clearTexture = function(texture) { exports._clearTexture(texture.self); };\
	Krom.generateTextureMipmaps = function(texture) { exports._generateTextureMipmaps(texture.self); };\
	Krom.generateRenderTargetMipmaps = function(renderTarget) { exports._generateRenderTargetMipmaps(renderTarget.self); };\
	Krom.setMipmaps = function(texture, mipmaps) { for (let i = 0; i < mipmaps.length; ++i) exports._setMipmap(texture.self, mipmaps[i].texture_.image, i + 1); };\
	Krom.setDepthStencilFrom = function(renderTarget, sourceTarget) { exports._setDepthStencilFrom(renderTarget.self, sourceTarget.self); };\
	Krom.viewport = exports._viewport;\
	Krom.scissor = exports._scissor;\
	Krom.disableScissor = exports._disableScissor;\
	Krom.renderTargetsInvertedY = exports._renderTargetsInvertedY;\
	Krom.begin = function(renderTarget, art) {\
		exports._begin(renderTarget != null ? renderTarget.renderTarget_.self : null,\
			   (art != null && art.length > 0) ? art[0].renderTarget_.self : null,\
			   (art != null && art.length > 1) ? art[1].renderTarget_.self : null,\
			   (art != null && art.length > 2) ? art[2].renderTarget_.self : null,\
			   (art != null && art.length > 3) ? art[3].renderTarget_.self : null,\
			   (art != null && art.length > 4) ? art[4].renderTarget_.self : null,\
			   (art != null && art.length > 5) ? art[5].renderTarget_.self : null,\
			   (art != null && art.length > 6) ? art[6].renderTarget_.self : null\
		);\
	};\
	Krom.beginFace = function(renderTarget, face) { exports._beginFace(renderTarget.renderTarget_.self, face); };\
	Krom.end = exports._end;\
	Krom.fileSaveBytes = exports._fileSaveBytes;\
	Krom.sysCommand = exports._sysCommand;\
	Krom.savePath = exports._savePath;\
	Krom.getArgCount = exports._getArgCount;\
	Krom.getArg = exports._getArg;\
	Krom.getFilesLocation = exports._getFilesLocation;\
	Krom.setBoolCompute = exports._setBoolCompute;\
	Krom.setIntCompute = exports._setIntCompute;\
	Krom.setFloatCompute = exports._setFloatCompute;\
	Krom.setFloat2Compute = exports._setFloat2Compute;\
	Krom.setFloat3Compute = exports._setFloat3Compute;\
	Krom.setFloat4Compute = exports._setFloat4Compute;\
	Krom.setFloatsCompute = exports._setFloatsCompute;\
	Krom.setMatrixCompute = exports._setMatrixCompute;\
	Krom.setMatrix3Compute = exports._setMatrix3Compute;\
	Krom.setTextureCompute = exports._setTextureCompute;\
	Krom.setRenderTargetCompute = exports._setRenderTargetCompute;\
	Krom.setSampledTextureCompute = exports._setSampledTextureCompute;\
	Krom.setSampledRenderTargetCompute = exports._setSampledRenderTargetCompute;\
	Krom.setSampledDepthTextureCompute = exports._setSampledDepthTextureCompute;\
	Krom.setTextureParametersCompute = exports._setTextureParametersCompute;\
	Krom.setTexture3DParametersCompute = exports._setTexture3DParametersCompute;\
	Krom.setShaderCompute = exports._setShaderCompute;\
	Krom.deleteShaderCompute = exports._deleteShaderCompute;\
	Krom.createShaderCompute = exports._createShaderCompute;\
	Krom.getConstantLocationCompute = exports._getConstantLocationCompute;\
	Krom.getTextureUnitCompute = exports._getTextureUnitCompute;\
	Krom.compute = exports._compute;\
	Krom.setSaveAndQuitCallback = exports._setSaveAndQuitCallback;\
	Krom.setMouseCursor = exports._setMouseCursor;\
	Krom.windowX = exports._windowX;\
	Krom.windowY = exports._windowY;";

	char *code = (char *)malloc(strlen(code_header) + strlen(code_krom) + 1);
	strcpy(code, code_header);
	strcat(code, code_krom);
	free(code_krom);

	js_eval(code);

	kinc_start();

	free(code);

	return 0;
}
