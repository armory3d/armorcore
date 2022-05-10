#include <emscripten.h>
#include <string.h>
#include <kinc/memory.h>
#include <kinc/log.h>
#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>
#include <kinc/audio2/audio.h>
#include <kinc/audio1/sound.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <kinc/display.h>
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
#define STB_IMAGE_IMPLEMENTATION
#include <kinc/libs/stb_image.h>
#ifdef KORE_LZ4X
int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxOutputSize);
#else
#include <kinc/io/lz4/lz4.h>
#endif

const int KROM_API = 6;

static char temp_buffer[4096];

static void (*update_func)() = NULL;
static void (*keyboard_down_func)(int) = NULL;
static void (*keyboard_up_func)(int) = NULL;
static void (*keyboard_press_func)(int) = NULL;
static void (*mouse_move_func)(int, int, int, int) = NULL;
static void (*mouse_down_func)(int, int, int) = NULL;
static void (*mouse_up_func)(int, int, int) = NULL;
static void (*mouse_wheel_func)(int) = NULL;

void update() {

	// #ifdef WITH_AUDIO
	// if (enable_sound) kinc_a2_update();
	// #endif

	kinc_g4_begin(0);

	update_func();

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

void mouse_move(int window, int x, int y, int mx, int my) {
	mouse_move_func(x, y, mx, my);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void mouse_down(int window, int button, int x, int y) {
	mouse_down_func(button, x, y);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void mouse_up(int window, int button, int x, int y) {
	mouse_up_func(button, x, y);

	#ifdef IDLE_SLEEP
	pausedFrames = 0;
	#endif
}

void mouse_wheel(int window, int delta) {
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

EMSCRIPTEN_KEEPALIVE char *getTempBuffer() {
	return temp_buffer;
}

EMSCRIPTEN_KEEPALIVE char *c_alloc(int size) {
	return malloc(size);
}

kinc_image_t *last_image = NULL;
int last_width = 0;
int last_height = 0;
int last_depth = 0;

EMSCRIPTEN_KEEPALIVE kinc_image_t *getLastImage() {
	return last_image;
}

EMSCRIPTEN_KEEPALIVE int getLastWidth() {
	return last_width;
}

EMSCRIPTEN_KEEPALIVE int getLastHeight() {
	return last_height;
}

EMSCRIPTEN_KEEPALIVE int getLastDepth() {
	return last_depth;
}

EMSCRIPTEN_KEEPALIVE void init(char *title, int width, int height, int samples_per_pixel, bool vertical_sync, int window_mode, int window_features, int api_version, int x, int y) {

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
	frame.depth_bits = 16;
	frame.stencil_bits = 8;
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

	kinc_set_update_callback(update);
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
	kinc_mouse_set_move_callback(mouse_move);
	kinc_mouse_set_press_callback(mouse_down);
	kinc_mouse_set_release_callback(mouse_up);
	kinc_mouse_set_scroll_callback(mouse_wheel);
	// kinc_surface_set_move_callback(touch_move);
	// kinc_surface_set_touch_start_callback(touch_down);
	// kinc_surface_set_touch_end_callback(touch_up);
	// kinc_pen_press_callback = pen_down;
	// kinc_pen_move_callback = pen_move;
	// kinc_pen_release_callback = pen_up;
	// kinc_gamepad_axis_callback = gamepad_axis;
	// kinc_gamepad_button_callback = gamepad_button;
}

EMSCRIPTEN_KEEPALIVE void setApplicationName(char *name) {

}

EMSCRIPTEN_KEEPALIVE void krom_log(char *message) {
	kinc_log(KINC_LOG_LEVEL_INFO, "%s", message);
}

EMSCRIPTEN_KEEPALIVE void clear(int flags, int color, float depth, int stencil) {
	kinc_g4_clear(flags, color, depth, stencil);
}

EMSCRIPTEN_KEEPALIVE void setCallback(void (*f)()) {
	update_func = f;
}

EMSCRIPTEN_KEEPALIVE void setDropFilesCallback() {

}

EMSCRIPTEN_KEEPALIVE void setCutCopyPasteCallback() {

}

EMSCRIPTEN_KEEPALIVE void setApplicationStateCallback() {

}

EMSCRIPTEN_KEEPALIVE void setKeyboardDownCallback(void (*f)(int)) {
	keyboard_down_func = f;
}

EMSCRIPTEN_KEEPALIVE void setKeyboardUpCallback(void (*f)(int)) {
	keyboard_up_func = f;
}

EMSCRIPTEN_KEEPALIVE void setKeyboardPressCallback(void (*f)(int)) {
	keyboard_press_func = f;
}

EMSCRIPTEN_KEEPALIVE void setMouseMoveCallback(void (*f)(int, int, int, int)) {
	mouse_move_func = f;
}

EMSCRIPTEN_KEEPALIVE void setMouseDownCallback(void (*f)(int, int, int)) {
	mouse_down_func = f;
}

EMSCRIPTEN_KEEPALIVE void setMouseUpCallback(void (*f)(int, int, int)) {
	mouse_up_func = f;
}

EMSCRIPTEN_KEEPALIVE void setMouseWheelCallback(void (*f)(int)) {
	mouse_wheel_func = f;
}

EMSCRIPTEN_KEEPALIVE void setTouchDownCallback() {

}

EMSCRIPTEN_KEEPALIVE void setTouchUpCallback() {

}

EMSCRIPTEN_KEEPALIVE void setTouchMoveCallback() {

}

EMSCRIPTEN_KEEPALIVE void setPenDownCallback() {

}

EMSCRIPTEN_KEEPALIVE void setPenUpCallback() {

}

EMSCRIPTEN_KEEPALIVE void setPenMoveCallback() {

}

EMSCRIPTEN_KEEPALIVE void setGamepadAxisCallback() {

}

EMSCRIPTEN_KEEPALIVE void setGamepadButtonCallback() {

}

EMSCRIPTEN_KEEPALIVE void lockMouse() {
	kinc_mouse_lock(0);
}

EMSCRIPTEN_KEEPALIVE void unlockMouse() {
	kinc_mouse_unlock();
}

EMSCRIPTEN_KEEPALIVE bool canLockMouse() {
	return kinc_mouse_can_lock();
}

EMSCRIPTEN_KEEPALIVE bool isMouseLocked() {
	return kinc_mouse_is_locked();
}

EMSCRIPTEN_KEEPALIVE void setMousePosition(int windowId, int x, int y) {
	kinc_mouse_set_position(windowId, x, y);
}

EMSCRIPTEN_KEEPALIVE void showMouse(bool show) {
	show ? kinc_mouse_show() : kinc_mouse_hide();
}

EMSCRIPTEN_KEEPALIVE void showKeyboard(bool show) {
	// show ? kinc_keyboard_show() : kinc_keyboard_hide();
}

EMSCRIPTEN_KEEPALIVE kinc_g4_index_buffer_t *createIndexBuffer(int count) {
	kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)malloc(sizeof(kinc_g4_index_buffer_t));
	kinc_g4_index_buffer_init(buffer, count, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
	return buffer;
}

EMSCRIPTEN_KEEPALIVE void deleteIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_destroy(buffer);
	free(buffer);
}

EMSCRIPTEN_KEEPALIVE int index_buffer_size(kinc_g4_index_buffer_t *buffer) {
	return kinc_g4_index_buffer_count(buffer);
}

EMSCRIPTEN_KEEPALIVE int *lockIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	int *vertices = kinc_g4_index_buffer_lock(buffer);
	return vertices;
}

EMSCRIPTEN_KEEPALIVE void unlockIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_unlock(buffer);
}

EMSCRIPTEN_KEEPALIVE void setIndexBuffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_set_index_buffer(buffer);
}

EMSCRIPTEN_KEEPALIVE kinc_g4_vertex_buffer_t *createVertexBuffer(int count, char *name0, int data0, char *name1, int data1, char *name2, int data2, char *name3, int data3, char *name4, int data4, char *name5, int data5, char *name6, int data6, char *name7, int data7, int usage, int instanceDataStepRate) {
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

EMSCRIPTEN_KEEPALIVE void deleteVertexBuffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_destroy(buffer);
	free(buffer);
}

EMSCRIPTEN_KEEPALIVE int vertex_buffer_size(kinc_g4_vertex_buffer_t *buffer) {
	return (kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer));
}

EMSCRIPTEN_KEEPALIVE float *lockVertexBuffer(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	float *vertices = kinc_g4_vertex_buffer_lock(buffer, start, count);
	return vertices;
}

EMSCRIPTEN_KEEPALIVE void unlockVertexBuffer(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_g4_vertex_buffer_unlock(buffer, count);
}

EMSCRIPTEN_KEEPALIVE void setVertexBuffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_set_vertex_buffer(buffer);
}

EMSCRIPTEN_KEEPALIVE void setVertexBuffers(kinc_g4_vertex_buffer_t **buffers, int length) {
	kinc_g4_set_vertex_buffers(buffers, length);
}

EMSCRIPTEN_KEEPALIVE void drawIndexedVertices(int start, int count) {
	if (count < 0) kinc_g4_draw_indexed_vertices();
	else kinc_g4_draw_indexed_vertices_from_to(start, count);
}

EMSCRIPTEN_KEEPALIVE void drawIndexedVerticesInstanced(int instance_count, int start, int count) {
	if (count < 0) kinc_g4_draw_indexed_vertices_instanced(instance_count);
	else kinc_g4_draw_indexed_vertices_instanced_from_to(instance_count, start, count);
}

EMSCRIPTEN_KEEPALIVE kinc_g4_shader_t *createVertexShader(char *data, int length, char *name) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, data, length, KINC_G4_SHADER_TYPE_VERTEX);
	return shader;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_shader_t *createVertexShaderFromSource(char *source) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_VERTEX);
	return shader;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_shader_t *createFragmentShader(char *data, int length, char *name) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, data, length, KINC_G4_SHADER_TYPE_FRAGMENT);
	return shader;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_shader_t *createFragmentShaderFromSource(char *source) {
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_FRAGMENT);
	return shader;
}

EMSCRIPTEN_KEEPALIVE void createGeometryShader() {

}

EMSCRIPTEN_KEEPALIVE void createTessellationControlShader() {

}

EMSCRIPTEN_KEEPALIVE void createTessellationEvaluationShader() {

}

EMSCRIPTEN_KEEPALIVE void deleteShader(kinc_g4_shader_t *shader) {
	kinc_g4_shader_destroy(shader);
	free(shader);
}

EMSCRIPTEN_KEEPALIVE kinc_g4_pipeline_t *createPipeline() {
	kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)malloc(sizeof(kinc_g4_pipeline_t));
	kinc_g4_pipeline_init(pipeline);
	return pipeline;
}

EMSCRIPTEN_KEEPALIVE void deletePipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_pipeline_destroy(pipeline);
	free(pipeline);
}

EMSCRIPTEN_KEEPALIVE void compilePipeline(kinc_g4_pipeline_t *pipeline, char *name0, int data0, char *name1, int data1, char *name2, int data2, char *name3, int data3, char *name4, int data4, char *name5, int data5, char *name6, int data6, char *name7, int data7, kinc_g4_cull_mode_t cull_mode, bool depth_write, kinc_g4_compare_mode_t depth_mode, kinc_g4_shader_t *vertex_shader, kinc_g4_shader_t *fragment_shader) {
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
	kinc_g4_pipeline_compile(pipeline);
}

EMSCRIPTEN_KEEPALIVE void setPipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_set_pipeline(pipeline);
}

bool ends_with(const char *str, const char *suffix) {
	if (!str || !suffix) return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr) return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_texture_t *loadImage(char *file, bool readable) {
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

EMSCRIPTEN_KEEPALIVE void unloadTexture(kinc_g4_texture_t *texture) {
	kinc_g4_texture_destroy(texture);
	free(texture);
}

EMSCRIPTEN_KEEPALIVE void unloadRenderTarget(kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_destroy(render_target);
	free(render_target);
}

EMSCRIPTEN_KEEPALIVE void loadSound() {

}

EMSCRIPTEN_KEEPALIVE void setAudioCallback() {

}

EMSCRIPTEN_KEEPALIVE void audioThread() {

}

EMSCRIPTEN_KEEPALIVE void writeAudioBuffer() {

}

int reader_size = 0;

EMSCRIPTEN_KEEPALIVE int readerSize() {
	return reader_size;
}

EMSCRIPTEN_KEEPALIVE char *loadBlob(char *path) {
	strcpy(temp_buffer, "/Deployment/");
	strcat(temp_buffer, path);

	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, temp_buffer, KINC_FILE_TYPE_ASSET)) return NULL;
	reader_size = (int)kinc_file_reader_size(&reader);

	char *buffer = (char *)malloc(reader_size);
	kinc_file_reader_read(&reader, buffer, reader_size);
	kinc_file_reader_close(&reader);

	return buffer;
}

EMSCRIPTEN_KEEPALIVE void loadUrl(char *url) {
	// kinc_load_url(url);
}

EMSCRIPTEN_KEEPALIVE void copyToClipboard(char *str) {
	kinc_copy_to_clipboard(str);
}

EMSCRIPTEN_KEEPALIVE kinc_g4_constant_location_t *getConstantLocation(kinc_g4_pipeline_t *pipeline, char *name) {
	kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, name);

	kinc_g4_constant_location_t *location_copy = (kinc_g4_constant_location_t *)malloc(sizeof(kinc_g4_constant_location_t));
	memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
	return location_copy;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_texture_unit_t *getTextureUnit(kinc_g4_pipeline_t *pipeline, char *name) {
	kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, name);

	kinc_g4_texture_unit_t *unit_copy = (kinc_g4_texture_unit_t *)malloc(sizeof(kinc_g4_texture_unit_t));
	memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
	return unit_copy;
}

EMSCRIPTEN_KEEPALIVE void setTexture(kinc_g4_texture_unit_t *unit, kinc_g4_texture_t *texture) {
	kinc_g4_set_texture(*unit, texture);
}

EMSCRIPTEN_KEEPALIVE void setRenderTarget(kinc_g4_texture_unit_t *unit, kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_use_color_as_texture(render_target, *unit);
}

EMSCRIPTEN_KEEPALIVE void setTextureDepth(kinc_g4_texture_unit_t *unit, kinc_g4_render_target_t *render_target) {
	kinc_g4_render_target_use_depth_as_texture(render_target, *unit);
}

EMSCRIPTEN_KEEPALIVE void setImageTexture(kinc_g4_texture_unit_t *unit, kinc_g4_texture_t *texture) {
	kinc_g4_set_image_texture(*unit, texture);
}

EMSCRIPTEN_KEEPALIVE void setTextureParameters(kinc_g4_texture_unit_t *unit, int uAddressing, int vAddressing, int minificationFilter, int magnificationFilter, int mipmapFilter) {
	kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)uAddressing);
	kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)vAddressing);
	kinc_g4_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)minificationFilter);
	kinc_g4_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)magnificationFilter);
	kinc_g4_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)mipmapFilter);
}

EMSCRIPTEN_KEEPALIVE void setTexture3DParameters(kinc_g4_texture_unit_t *unit, int uAddressing, int vAddressing, int wAddressing, int minificationFilter, int magnificationFilter, int mipmapFilter) {
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)uAddressing);
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)vAddressing);
	kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)wAddressing);
	kinc_g4_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)minificationFilter);
	kinc_g4_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)magnificationFilter);
	kinc_g4_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)mipmapFilter);
}

EMSCRIPTEN_KEEPALIVE void setTextureCompareMode(kinc_g4_texture_unit_t *unit, bool enabled) {
	kinc_g4_set_texture_compare_mode(*unit, enabled);
}

EMSCRIPTEN_KEEPALIVE void setCubeMapCompareMode(kinc_g4_texture_unit_t *unit, bool enabled) {
	kinc_g4_set_cubemap_compare_mode(*unit, enabled);
}

EMSCRIPTEN_KEEPALIVE void setBool(kinc_g4_constant_location_t *location, bool value) {
	kinc_g4_set_bool(*location, value);
}

EMSCRIPTEN_KEEPALIVE void setInt(kinc_g4_constant_location_t *location, int value) {
	kinc_g4_set_int(*location, value);
}

EMSCRIPTEN_KEEPALIVE void setFloat(kinc_g4_constant_location_t *location, float value) {
	kinc_g4_set_float(*location, value);
}

EMSCRIPTEN_KEEPALIVE void setFloat2(kinc_g4_constant_location_t *location, float value1, float value2) {
	kinc_g4_set_float2(*location, value1, value2);
}

EMSCRIPTEN_KEEPALIVE void setFloat3(kinc_g4_constant_location_t *location, float value1, float value2, float value3) {
	kinc_g4_set_float3(*location, value1, value2, value3);
}

EMSCRIPTEN_KEEPALIVE void setFloat4(kinc_g4_constant_location_t *location, float value1, float value2, float value3, float value4) {
	kinc_g4_set_float4(*location, value1, value2, value3, value4);
}

EMSCRIPTEN_KEEPALIVE void setFloats(kinc_g4_constant_location_t *location, float *from, int count) {
	kinc_g4_set_floats(*location, from, count);
}

EMSCRIPTEN_KEEPALIVE void setMatrix(kinc_g4_constant_location_t *location, float *from) {
	kinc_g4_set_matrix4(*location, (kinc_matrix4x4_t *)from);
}

EMSCRIPTEN_KEEPALIVE void setMatrix3(kinc_g4_constant_location_t *location, float *from) {
	kinc_g4_set_matrix3(*location, (kinc_matrix3x3_t *)from);
}

EMSCRIPTEN_KEEPALIVE double getTime() {
	return kinc_time();
}

EMSCRIPTEN_KEEPALIVE int windowWidth(int id) {
	return kinc_window_width(id);
}

EMSCRIPTEN_KEEPALIVE int windowHeight(int id) {
	return kinc_window_height(id);
}

EMSCRIPTEN_KEEPALIVE void setWindowTitle(int id, char *title) {
	kinc_window_set_title(id, title);
}

EMSCRIPTEN_KEEPALIVE int screenDpi() {
	return kinc_display_current_mode(kinc_primary_display()).pixels_per_inch;
}

EMSCRIPTEN_KEEPALIVE const char *systemId() {
	// return kinc_system_id();
	return NULL;
}

EMSCRIPTEN_KEEPALIVE void requestShutdown() {
	kinc_stop();
}

EMSCRIPTEN_KEEPALIVE int displayCount() {
	return kinc_count_displays();
}

EMSCRIPTEN_KEEPALIVE int displayWidth(int index) {
	return kinc_display_current_mode(index).width;
}

EMSCRIPTEN_KEEPALIVE int displayHeight(int index) {
	return kinc_display_current_mode(index).height;
}

EMSCRIPTEN_KEEPALIVE int displayX(int index) {
	return kinc_display_current_mode(index).x;
}

EMSCRIPTEN_KEEPALIVE int displayY(int index) {
	return kinc_display_current_mode(index).y;
}

EMSCRIPTEN_KEEPALIVE int displayFrequency(int index) {
	return kinc_display_current_mode(index).frequency;
}

EMSCRIPTEN_KEEPALIVE bool displayIsPrimary(int index) {
	return index == kinc_primary_display();
}

EMSCRIPTEN_KEEPALIVE void writeStorage() {

}

EMSCRIPTEN_KEEPALIVE void readStorage() {

}

EMSCRIPTEN_KEEPALIVE kinc_g4_render_target_t *createRenderTarget(int width, int height, int depthBufferBits, int format, int stencilBufferBits, int contextId) {
	kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
	kinc_g4_render_target_init(render_target, width, height, depthBufferBits, false, (kinc_g4_render_target_format_t)format, stencilBufferBits, 0);
	last_width = render_target->width;
	last_height = render_target->height;
	return render_target;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_render_target_t *createRenderTargetCubeMap(int size, int depthBufferBits, int format, int stencilBufferBits, int contextId) {
	kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)malloc(sizeof(kinc_g4_render_target_t));
	kinc_g4_render_target_init_cube(render_target, size, depthBufferBits, false, (kinc_g4_render_target_format_t)format, stencilBufferBits, 0);
	last_width = render_target->width;
	last_height = render_target->height;
	return render_target;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_texture_t *createTexture(int width, int height, int format) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init(texture, width, height, (kinc_image_format_t)format);
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	return texture;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_texture_t *createTexture3D(int width, int height, int depth, int format) {
	kinc_g4_texture_t *texture = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init3d(texture, width, height, depth, (kinc_image_format_t)format);
	last_width = texture->tex_width;
	last_height = texture->tex_height;
	last_depth = texture->tex_depth;
	return texture;
}

EMSCRIPTEN_KEEPALIVE kinc_g4_texture_t *createTextureFromBytes(char *data, int width, int height, int format, bool readable) {
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

EMSCRIPTEN_KEEPALIVE kinc_g4_texture_t *createTextureFromBytes3D(char *data, int width, int height, int depth, int format, bool readable) {
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

EMSCRIPTEN_KEEPALIVE kinc_g4_texture_t *createTextureFromEncodedBytes(unsigned char *content_data, int content_length, char *format, bool readable) {
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

EMSCRIPTEN_KEEPALIVE int format_byte_size(kinc_image_format_t format) {
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

EMSCRIPTEN_KEEPALIVE uint8_t *getTexturePixels(kinc_image_t *image) {
	uint8_t *data = kinc_image_get_pixels(image);
	int byteLength = format_byte_size(image->format) * image->width * image->height * image->depth;
	return data;
}

EMSCRIPTEN_KEEPALIVE void getRenderTargetPixels(kinc_g4_render_target_t *render_target, uint8_t *out) {
	kinc_g4_render_target_get_pixels(render_target, out);
}

EMSCRIPTEN_KEEPALIVE uint8_t *lockTexture(kinc_g4_texture_t *texture, int level) {
	uint8_t *tex = kinc_g4_texture_lock(texture);
	// int byteLength = kinc_g4_texture_stride(texture) * texture->tex_height * texture->tex_depth;
	return tex;
}

EMSCRIPTEN_KEEPALIVE void unlockTexture(kinc_g4_texture_t *texture) {
	kinc_g4_texture_unlock(texture);
}

EMSCRIPTEN_KEEPALIVE void clearTexture(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, int color) {
	// kinc_g4_texture_clear(texture, x, y, z, width, height, depth, color);
}

EMSCRIPTEN_KEEPALIVE void generateTextureMipmaps(kinc_g4_texture_t *texture, int levels) {
	kinc_g4_texture_generate_mipmaps(texture, levels);
}

EMSCRIPTEN_KEEPALIVE void generateRenderTargetMipmaps(kinc_g4_render_target_t *render_target, int levels) {
	kinc_g4_render_target_generate_mipmaps(render_target, levels);
}

EMSCRIPTEN_KEEPALIVE void setMipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level) {
	kinc_g4_texture_set_mipmap(texture, mipmap, level);
}

EMSCRIPTEN_KEEPALIVE void setDepthStencilFrom(kinc_g4_render_target_t *render_target, kinc_g4_render_target_t *source_target) {
	kinc_g4_render_target_set_depth_stencil_from(render_target, source_target);
}

EMSCRIPTEN_KEEPALIVE void viewport(int x, int y, int w, int h) {
	kinc_g4_viewport(x, y, w, h);
}

EMSCRIPTEN_KEEPALIVE void scissor(int x, int y, int w, int h) {
	kinc_g4_scissor(x, y, w, h);
}

EMSCRIPTEN_KEEPALIVE void disableScissor() {
	kinc_g4_disable_scissor();
}

EMSCRIPTEN_KEEPALIVE bool renderTargetsInvertedY() {
	return kinc_g4_render_targets_inverted_y();
}

EMSCRIPTEN_KEEPALIVE void begin(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *renderTarget1, kinc_g4_render_target_t *renderTarget2, kinc_g4_render_target_t *renderTarget3, kinc_g4_render_target_t *renderTarget4, kinc_g4_render_target_t *renderTarget5, kinc_g4_render_target_t *renderTarget6, kinc_g4_render_target_t *renderTarget7) {
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

EMSCRIPTEN_KEEPALIVE void beginFace(kinc_g4_render_target_t *render_target, int face) {
	kinc_g4_set_render_target_face(render_target, face);
}

EMSCRIPTEN_KEEPALIVE void end() {

}

EMSCRIPTEN_KEEPALIVE void fileSaveBytes() {

}

EMSCRIPTEN_KEEPALIVE int sysCommand(char *cmd) {
	return system(cmd);
}

EMSCRIPTEN_KEEPALIVE const char *savePath() {
	// return kinc_internal_save_path();
	return NULL;
}

EMSCRIPTEN_KEEPALIVE int getArgCount() {
	return 0;
}

EMSCRIPTEN_KEEPALIVE const char *getArg(int index) {
	return NULL;
}

EMSCRIPTEN_KEEPALIVE const char *getFilesLocation() {
	return kinc_internal_get_files_location();
}

EMSCRIPTEN_KEEPALIVE void setBoolCompute() {

}

EMSCRIPTEN_KEEPALIVE void setIntCompute() {

}

EMSCRIPTEN_KEEPALIVE void setFloatCompute() {

}

EMSCRIPTEN_KEEPALIVE void setFloat2Compute() {

}

EMSCRIPTEN_KEEPALIVE void setFloat3Compute() {

}

EMSCRIPTEN_KEEPALIVE void setFloat4Compute() {

}

EMSCRIPTEN_KEEPALIVE void setFloatsCompute() {

}

EMSCRIPTEN_KEEPALIVE void setMatrixCompute() {

}

EMSCRIPTEN_KEEPALIVE void setMatrix3Compute() {

}

EMSCRIPTEN_KEEPALIVE void setTextureCompute() {

}

EMSCRIPTEN_KEEPALIVE void setRenderTargetCompute() {

}

EMSCRIPTEN_KEEPALIVE void setSampledTextureCompute() {

}

EMSCRIPTEN_KEEPALIVE void setSampledRenderTargetCompute() {

}

EMSCRIPTEN_KEEPALIVE void setSampledDepthTextureCompute() {

}

EMSCRIPTEN_KEEPALIVE void setTextureParametersCompute() {

}

EMSCRIPTEN_KEEPALIVE void setTexture3DParametersCompute() {

}

EMSCRIPTEN_KEEPALIVE void setShaderCompute() {

}

EMSCRIPTEN_KEEPALIVE void deleteShaderCompute() {

}

EMSCRIPTEN_KEEPALIVE void createShaderCompute() {

}

EMSCRIPTEN_KEEPALIVE void getConstantLocationCompute() {

}

EMSCRIPTEN_KEEPALIVE void getTextureUnitCompute() {

}

EMSCRIPTEN_KEEPALIVE void compute() {

}

EMSCRIPTEN_KEEPALIVE void setSaveAndQuitCallback() {

}

EMSCRIPTEN_KEEPALIVE void setMouseCursor() {

}

EMSCRIPTEN_KEEPALIVE int windowX(int id) {
	return kinc_window_x(id);
}

EMSCRIPTEN_KEEPALIVE int windowY(int id) {
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
	let KromBuffers = new Map();\
	let Krom = {};\
	Krom.init = Module.cwrap('init', null, ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']);\
	Krom.setApplicationName = _setApplicationName;\
	Krom.log = Module.cwrap('krom_log', null, ['string']);\
	Krom.clear = _clear;\
	Krom.setCallback = function(f) { _setCallback(Module.addFunction(f, 'v')); };\
	Krom.setDropFilesCallback = _setDropFilesCallback;\
	Krom.setCutCopyPasteCallback = _setCutCopyPasteCallback;\
	Krom.setApplicationStateCallback = _setApplicationStateCallback;\
	Krom.setKeyboardDownCallback = function(f) { _setKeyboardDownCallback(Module.addFunction(f, 'vi')); };\
	Krom.setKeyboardUpCallback = function(f) { _setKeyboardUpCallback(Module.addFunction(f, 'vi')); };\
	Krom.setKeyboardPressCallback = function(f) { _setKeyboardPressCallback(Module.addFunction(f, 'vi')); };\
	Krom.setMouseMoveCallback = function(f) { _setMouseMoveCallback(Module.addFunction(f, 'viiii')); };\
	Krom.setMouseDownCallback = function(f) { _setMouseDownCallback(Module.addFunction(f, 'viii')); };\
	Krom.setMouseUpCallback = function(f) { _setMouseUpCallback(Module.addFunction(f, 'viii')); };\
	Krom.setMouseWheelCallback = function(f) { _setMouseWheelCallback(Module.addFunction(f, 'vi')); };\
	Krom.setPenDownCallback = _setPenDownCallback;\
	Krom.setPenUpCallback = _setPenUpCallback;\
	Krom.setPenMoveCallback = _setPenMoveCallback;\
	Krom.setGamepadAxisCallback = _setGamepadAxisCallback;\
	Krom.setGamepadButtonCallback = _setGamepadButtonCallback;\
	Krom.setTouchDownCallback = _setTouchDownCallback;\
	Krom.setTouchUpCallback = _setTouchUpCallback;\
	Krom.setTouchMoveCallback = _setTouchMoveCallback;\
	Krom.lockMouse = _lockMouse;\
	Krom.unlockMouse = _unlockMouse;\
	Krom.canLockMouse = _canLockMouse;\
	Krom.isMouseLocked = _isMouseLocked;\
	Krom.setMousePosition = _setMousePosition;\
	Krom.showMouse = _showMouse;\
	Krom.showKeyboard = _showKeyboard;\
	Krom.createIndexBuffer = _createIndexBuffer;\
	Krom.deleteIndexBuffer = _deleteIndexBuffer;\
	Krom.lockIndexBuffer = function(buffer) {\
		return new Uint32Array(Module['HEAPU8'].buffer, _lockIndexBuffer(buffer), _index_buffer_size(buffer));\
	};\
	Krom.unlockIndexBuffer = _unlockIndexBuffer;\
	Krom.setIndexBuffer = _setIndexBuffer;\
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
		return Module.cwrap('createVertexBuffer', 'number', ['number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'number', 'number'])(count, name0, data0, name1, data1, name2, data2, name3, data3, name4, data4, name5, data5, name6, data6, name7, data7, usage, instanceDataStepRate);\
	};\
	Krom.deleteVertexBuffer = _deleteVertexBuffer;\
	Krom.lockVertexBuffer = function(vbuffer, vstart, vcount) {\
		let start = _lockVertexBuffer(vbuffer, vstart, vcount);\
		let byteLength = _vertex_buffer_size(vbuffer);\
		let b = Module['HEAPU8'].buffer.slice(start, start + byteLength);\
		KromBuffers.set(vbuffer, { buffer: b, start: start, byteLength: byteLength });\
		return b;\
	};\
	Krom.unlockVertexBuffer = function(vbuffer, count) {\
		let b = new Uint8Array(KromBuffers.get(vbuffer).buffer);\
		let start = KromBuffers.get(vbuffer).start;\
		let byteLength = KromBuffers.get(vbuffer).byteLength;\
		let heap = new Uint8Array(Module['HEAPU8'].buffer, start, byteLength);\
		for (let i = 0; i < byteLength; ++i) heap[i] = b[i];\
		_unlockVertexBuffer(vbuffer, count);\
	};\
	Krom.setVertexBuffer = _setVertexBuffer;\
	Krom.setVertexBuffers = _setVertexBuffers;\
	Krom.drawIndexedVertices = _drawIndexedVertices;\
	Krom.drawIndexedVerticesInstanced = _drawIndexedVerticesInstanced;\
	Krom.createVertexShader = function(buffer, name) {\
		return Module.cwrap('createVertexShader', 'number', ['number', 'number', 'string'])(KromBuffers.get(buffer).start, KromBuffers.get(buffer).byteLength, name);\
	};\
	Krom.createVertexShaderFromSource = Module.cwrap('createVertexShaderFromSource', 'number', ['string']);\
	Krom.createFragmentShader = function(buffer, name) {\
		return Module.cwrap('createFragmentShader', 'number', ['number', 'number', 'string'])(KromBuffers.get(buffer).start, KromBuffers.get(buffer).byteLength, name);\
	};\
	Krom.createFragmentShaderFromSource = Module.cwrap('createFragmentShaderFromSource', 'number', ['string']);\
	Krom.createGeometryShader = _createGeometryShader;\
	Krom.createTessellationControlShader = _createTessellationControlShader;\
	Krom.createTessellationEvaluationShader = _createTessellationEvaluationShader;\
	Krom.deleteShader = _deleteShader;\
	Krom.createPipeline = _createPipeline;\
	Krom.deletePipeline = _deletePipeline;\
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
		Module.cwrap('compilePipeline', null, ['number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'string', 'number', 'number', 'number', 'number', 'number', 'number'])(pipeline, name0, data0, name1, data1, name2, data2, name3, data3, name4, data4, name5, data5, name6, data6, name7, data7, state.cullMode, state.depthWrite, state.depthMode, vertexShader, fragmentShader);\
	};\
	Krom.setPipeline = _setPipeline;\
	Krom.loadImage = function (file, readable) {\
		return { self: Module.cwrap('loadImage', 'number', ['string', 'number'])(file, readable),\
				 image: _getLastImage(),\
				 width: _getLastWidth(),\
				 height: _getLastHeight()\
		};\
	};\
	Krom.unloadImage = function(image) { image.texture_ != null ? _unloadTexture(image.texture_.self) : _unloadRenderTarget(image.renderTarget_.self); };\
	Krom.loadSound = _loadSound;\
	Krom.setAudioCallback = _setAudioCallback;\
	Krom.audioThread = _audioThread;\
	Krom.writeAudioBuffer = _writeAudioBuffer;\
	Krom.loadBlob = function(path) {\
		let start = Module.cwrap('loadBlob', 'number', ['string'])(path);\
		let b = Module['HEAPU8'].buffer.slice(start, start + _readerSize());\
		KromBuffers.set(b, { start: start, byteLength: _readerSize() });\
		return b;\
	};\
	Krom.loadUrl = Module.cwrap('loadUrl', null, ['string']);\
	Krom.copyToClipboard = Module.cwrap('copyToClipboard', null, ['string']);\
	Krom.getConstantLocation = Module.cwrap('getConstantLocation', 'number', ['number', 'string']);\
	Krom.getTextureUnit = Module.cwrap('getTextureUnit', 'number', ['number', 'string']);\
	Krom.setTexture = function(unit, texture) { _setTexture(unit, texture.self); };\
	Krom.setRenderTarget = function(unit, renderTarget) { _setRenderTarget(unit, renderTarget.self); };\
	Krom.setTextureDepth = function(unit, renderTarget) { _setTextureDepth(unit, renderTarget.self); };\
	Krom.setImageTexture = function(unit, texture) { _setImageTexture(unit, texture.self); };\
	Krom.setTextureParameters = _setTextureParameters;\
	Krom.setTexture3DParameters = _setTexture3DParameters;\
	Krom.setTextureCompareMode = _setTextureCompareMode;\
	Krom.setCubeMapCompareMode = _setCubeMapCompareMode;\
	Krom.setBool = _setBool;\
	Krom.setInt = _setInt;\
	Krom.setFloat = _setFloat;\
	Krom.setFloat2 = _setFloat2;\
	Krom.setFloat3 = _setFloat3;\
	Krom.setFloat4 = _setFloat4;\
	Krom.setFloats = function(location, floats) {\
		let to = new Uint8Array(Module['HEAPU8'].buffer, _getTempBuffer(), 4096);\
		let from = new Uint8Array(floats);\
		for (let i = 0; i < floats.byteLength; ++i) to[i] = from[i];\
		Module.cwrap('setFloats', null, ['number', 'number', 'number'])(location, _getTempBuffer(), floats.byteLength / 4);\
	};\
	Krom.setMatrix = function(location, matrix) {\
		let to = new Uint8Array(Module['HEAPU8'].buffer, _getTempBuffer(), 4096);\
		let from = new Uint8Array(matrix);\
		for (let i = 0; i < matrix.byteLength; ++i) to[i] = from[i];\
		Module.cwrap('setMatrix', null, ['number', 'number'])(location, _getTempBuffer());\
	};\
	Krom.setMatrix3 = function(location, matrix) {\
		let to = new Uint8Array(Module['HEAPU8'].buffer, _getTempBuffer(), 4096);\
		let from = new Uint8Array(matrix);\
		for (let i = 0; i < matrix.byteLength; ++i) to[i] = from[i];\
		Module.cwrap('setMatrix3', null, ['number', 'number'])(location, _getTempBuffer());\
	};\
	Krom.getTime = _getTime;\
	Krom.windowWidth = _windowWidth;\
	Krom.windowHeight = _windowHeight;\
	Krom.setWindowTitle = _setWindowTitle;\
	Krom.screenDpi = _screenDpi;\
	Krom.systemId = _systemId;\
	Krom.requestShutdown = _requestShutdown;\
	Krom.displayCount = _displayCount;\
	Krom.displayWidth = _displayWidth;\
	Krom.displayHeight = _displayHeight;\
	Krom.displayX = _displayX;\
	Krom.displayY = _displayY;\
	Krom.displayFrequency = _displayFrequency;\
	Krom.displayIsPrimary = _displayIsPrimary;\
	Krom.writeStorage = _writeStorage;\
	Krom.readStorage = _readStorage;\
	Krom.createRenderTarget = function(width, height, depthBufferBits, format, stencilBufferBits, contextId) {\
		return { self: _createRenderTarget(width, height, depthBufferBits, format, stencilBufferBits, contextId),\
				 width: _getLastWidth(),\
				 height: _getLastHeight()\
		};\
	};\
	Krom.createRenderTargetCubeMap = function(size, depthBufferBits, format, stencilBufferBits, contextId) {\
		return { self: _createRenderTargetCubeMap(size, depthBufferBits, format, stencilBufferBits, contextId),\
				 width: _getLastWidth(),\
				 height: _getLastHeight()\
		};\
	};\
	Krom.createTexture = function(width, height, format) {\
		return { self: _createTexture(width, height, format),\
				 width: _getLastWidth(),\
				 height: _getLastHeight()\
		};\
	};\
	Krom.createTexture3D = function(width, height, depth, format) {\
		return { self: _createTexture3D(width, height, depth, format),\
				 width: _getLastWidth(),\
				 height: _getLastHeight(),\
				 depth: _getLastDepth()\
		};\
	};\
	Krom.createTextureFromBytes = function(data, width, height, format, readable) {\
		let from = new Uint8Array(data);\
		let start = _c_alloc(data.byteLength);\
		let to = new Uint8Array(Module['HEAPU8'].buffer, start, data.byteLength);\
		for (let i = 0; i < data.byteLength; ++i) to[i] = from[i];\
		return { self: _createTextureFromBytes(start, width, height, format, readable),\
				 width: _getLastWidth(),\
				 height: _getLastHeight()\
		};\
	};\
	Krom.createTextureFromBytes3D = function(data, width, height, depth, format, readable) {\
		let from = new Uint8Array(data);\
		let start = _c_alloc(data.byteLength);\
		let to = new Uint8Array(Module['HEAPU8'].buffer, start, data.byteLength);\
		for (let i = 0; i < data.byteLength; ++i) to[i] = from[i];\
		return { self: _createTextureFromBytes3D(start, width, height, depth, format, readable),\
				 width: _getLastWidth(),\
				 height: _getLastHeight(),\
				 depth: _getLastDepth()\
		};\
	};\
	Krom.createTextureFromEncodedBytes = function(data, format, readable) {\
		let from = new Uint8Array(data);\
		let start = _c_alloc(data.byteLength);\
		let to = new Uint8Array(Module['HEAPU8'].buffer, start, data.byteLength);\
		for (let i = 0; i < data.byteLength; ++i) to[i] = from[i];\
		return { self: Module.cwrap('createTextureFromEncodedBytes', 'number', ['number', 'number', 'string', 'number'])(start, data.byteLength, format, readable),\
				 image: _getLastImage(),\
				 width: _getLastWidth(),\
				 height: _getLastHeight()\
		};\
	};\
	Krom.getTexturePixels = function(texture) { return _getTexturePixels(texture.self); };\
	Krom.getRenderTargetPixels = function(renderTarget, out) { return _getRenderTargetPixels(renderTarget.self, out); };\
	Krom.lockTexture = function(texture, level) { return _lockTexture(texture.self, level); };\
	Krom.unlockTexture = function(texture) { _unlockTexture(texture.self); };\
	Krom.clearTexture = function(texture) { _clearTexture(texture.self); };\
	Krom.generateTextureMipmaps = function(texture) { _generateTextureMipmaps(texture.self); };\
	Krom.generateRenderTargetMipmaps = function(renderTarget) { _generateRenderTargetMipmaps(renderTarget.self); };\
	Krom.setMipmaps = function(texture, mipmaps) { for (let i = 0; i < mipmaps.length; ++i) _setMipmap(texture.self, mipmaps[i].texture_.image, i + 1); };\
	Krom.setDepthStencilFrom = function(renderTarget, sourceTarget) { _setDepthStencilFrom(renderTarget.self, sourceTarget.self); };\
	Krom.viewport = _viewport;\
	Krom.scissor = _scissor;\
	Krom.disableScissor = _disableScissor;\
	Krom.renderTargetsInvertedY = _renderTargetsInvertedY;\
	Krom.begin = function(renderTarget, art) {\
		_begin(renderTarget != null ? renderTarget.renderTarget_.self : null,\
			   (art != null && art.length > 0) ? art[0].renderTarget_.self : null,\
			   (art != null && art.length > 1) ? art[1].renderTarget_.self : null,\
			   (art != null && art.length > 2) ? art[2].renderTarget_.self : null,\
			   (art != null && art.length > 3) ? art[3].renderTarget_.self : null,\
			   (art != null && art.length > 4) ? art[4].renderTarget_.self : null,\
			   (art != null && art.length > 5) ? art[5].renderTarget_.self : null,\
			   (art != null && art.length > 6) ? art[6].renderTarget_.self : null\
		);\
	};\
	Krom.beginFace = function(renderTarget, face) { _beginFace(renderTarget.renderTarget_.self, face); };\
	Krom.end = _end;\
	Krom.fileSaveBytes = _fileSaveBytes;\
	Krom.sysCommand = _sysCommand;\
	Krom.savePath = _savePath;\
	Krom.getArgCount = _getArgCount;\
	Krom.getArg = _getArg;\
	Krom.getFilesLocation = _getFilesLocation;\
	Krom.setBoolCompute = _setBoolCompute;\
	Krom.setIntCompute = _setIntCompute;\
	Krom.setFloatCompute = _setFloatCompute;\
	Krom.setFloat2Compute = _setFloat2Compute;\
	Krom.setFloat3Compute = _setFloat3Compute;\
	Krom.setFloat4Compute = _setFloat4Compute;\
	Krom.setFloatsCompute = _setFloatsCompute;\
	Krom.setMatrixCompute = _setMatrixCompute;\
	Krom.setMatrix3Compute = _setMatrix3Compute;\
	Krom.setTextureCompute = _setTextureCompute;\
	Krom.setRenderTargetCompute = _setRenderTargetCompute;\
	Krom.setSampledTextureCompute = _setSampledTextureCompute;\
	Krom.setSampledRenderTargetCompute = _setSampledRenderTargetCompute;\
	Krom.setSampledDepthTextureCompute = _setSampledDepthTextureCompute;\
	Krom.setTextureParametersCompute = _setTextureParametersCompute;\
	Krom.setTexture3DParametersCompute = _setTexture3DParametersCompute;\
	Krom.setShaderCompute = _setShaderCompute;\
	Krom.deleteShaderCompute = _deleteShaderCompute;\
	Krom.createShaderCompute = _createShaderCompute;\
	Krom.getConstantLocationCompute = _getConstantLocationCompute;\
	Krom.getTextureUnitCompute = _getTextureUnitCompute;\
	Krom.compute = _compute;\
	Krom.setSaveAndQuitCallback = _setSaveAndQuitCallback;\
	Krom.setMouseCursor = _setMouseCursor;\
	Krom.windowX = _windowX;\
	Krom.windowY = _windowY;";

	char *code = (char *)malloc(strlen(code_header) + strlen(code_krom) + 1);
	strcpy(code, code_header);
	strcat(code, code_krom);
	free(code_krom);

	emscripten_run_script(code);

	kinc_start();

	free(code);

	return 0;
}
