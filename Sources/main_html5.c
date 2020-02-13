#include <emscripten.h>
#include <string.h>
#include <kinc/pch.h>
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
// #include <kinc/compute/compute.h>
#include <kinc/libs/stb_image.h>
#ifdef KORE_LZ4X
int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxOutputSize);
#else
#include <kinc/io/lz4/lz4.h>
#endif

const int KROM_API = 3;

static void (*update_func)() = NULL;

void update() {

	// #ifdef WITH_AUDIO
	// if (enable_sound) kinc_a2_update();
	// #endif

	kinc_g4_begin(0);

	// kinc_mutex_lock(&mutex);
	
	update_func();

	// kinc_mutex_unlock(&mutex);

	kinc_g4_end(0);
	kinc_g4_swap_buffers();
}

EMSCRIPTEN_KEEPALIVE void init(char* title, int width, int height, int samples_per_pixel, bool vertical_sync, int window_mode, int window_features, int api_version, int x, int y) {

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
	// kinc_mutex_init(&mutex);
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

	// kinc_keyboard_key_down_callback = key_down;
	// kinc_keyboard_key_up_callback = key_up;
	// kinc_keyboard_key_press_callback = key_press;
	// kinc_mouse_move_callback = mouse_move;
	// kinc_mouse_press_callback = mouse_down;
	// kinc_mouse_release_callback = mouse_up;
	// kinc_mouse_scroll_callback = mouse_wheel;
	// kinc_pen_press_callback = pen_down;
	// kinc_pen_move_callback = pen_move;
	// kinc_pen_release_callback = pen_up;
	// kinc_gamepad_axis_callback = gamepad_axis;
	// kinc_gamepad_button_callback = gamepad_button;
}

EMSCRIPTEN_KEEPALIVE void setApplicationName() {

}

EMSCRIPTEN_KEEPALIVE void krom_log(char* message) {
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

EMSCRIPTEN_KEEPALIVE void setKeyboardDownCallback() {

}

EMSCRIPTEN_KEEPALIVE void setKeyboardUpCallback() {

}

EMSCRIPTEN_KEEPALIVE void setKeyboardPressCallback() {

}

EMSCRIPTEN_KEEPALIVE void setMouseDownCallback() {

}

EMSCRIPTEN_KEEPALIVE void setMouseUpCallback() {

}

EMSCRIPTEN_KEEPALIVE void setMouseMoveCallback() {

}

EMSCRIPTEN_KEEPALIVE void setMouseWheelCallback() {

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

}

EMSCRIPTEN_KEEPALIVE void unlockMouse() {

}

EMSCRIPTEN_KEEPALIVE void canLockMouse() {

}

EMSCRIPTEN_KEEPALIVE void isMouseLocked() {

}

EMSCRIPTEN_KEEPALIVE void setMousePosition() {

}

EMSCRIPTEN_KEEPALIVE void showMouse() {

}

EMSCRIPTEN_KEEPALIVE void showKeyboard() {

}

EMSCRIPTEN_KEEPALIVE kinc_g4_index_buffer_t* createIndexBuffer(int count) {
	kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)malloc(sizeof(kinc_g4_index_buffer_t));
	kinc_g4_index_buffer_init(buffer, count);
	return buffer;
}

EMSCRIPTEN_KEEPALIVE void deleteIndexBuffer() {

}

EMSCRIPTEN_KEEPALIVE int index_buffer_size(kinc_g4_index_buffer_t* buffer) {
	return kinc_g4_index_buffer_count(buffer);
}

EMSCRIPTEN_KEEPALIVE int* lockIndexBuffer(kinc_g4_index_buffer_t* buffer) {
	int* vertices = kinc_g4_index_buffer_lock(buffer);
	return vertices;
}

EMSCRIPTEN_KEEPALIVE void unlockIndexBuffer(kinc_g4_index_buffer_t* buffer) {
	kinc_g4_index_buffer_unlock(buffer);
}

EMSCRIPTEN_KEEPALIVE void setIndexBuffer(kinc_g4_index_buffer_t* buffer) {
	kinc_g4_set_index_buffer(buffer);
}

EMSCRIPTEN_KEEPALIVE kinc_g4_vertex_buffer_t* createVertexBuffer(int count/*, structure: Array<kha.graphics4.VertexElement>, int usage, int instanceDataStepRate*/) {
	kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)malloc(sizeof(kinc_g4_vertex_buffer_t));
	
	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_FLOAT3);

	kinc_g4_vertex_buffer_init(buffer, count, &structure, (kinc_g4_usage_t)0 /*KINC_G4_USAGE_STATIC*/, 0);
	return buffer;
}

EMSCRIPTEN_KEEPALIVE void deleteVertexBuffer() {

}

EMSCRIPTEN_KEEPALIVE int vertex_buffer_size(kinc_g4_vertex_buffer_t* buffer) {
	return (kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer)) / 4;
}

EMSCRIPTEN_KEEPALIVE float* lockVertexBuffer(kinc_g4_vertex_buffer_t* buffer) {
	float* vertices = kinc_g4_vertex_buffer_lock_all(buffer);
	return vertices;
}

EMSCRIPTEN_KEEPALIVE void unlockVertexBuffer(kinc_g4_vertex_buffer_t* buffer) {
	kinc_g4_vertex_buffer_unlock_all(buffer);
}

EMSCRIPTEN_KEEPALIVE void setVertexBuffer(kinc_g4_vertex_buffer_t* buffer) {
	kinc_g4_set_vertex_buffer(buffer);
}

EMSCRIPTEN_KEEPALIVE void setVertexBuffers() {

}

EMSCRIPTEN_KEEPALIVE void drawIndexedVertices(int start, int count) {
	if (count < 0) kinc_g4_draw_indexed_vertices();
	else kinc_g4_draw_indexed_vertices_from_to(start, count);
}

EMSCRIPTEN_KEEPALIVE void drawIndexedVerticesInstanced() {

}

EMSCRIPTEN_KEEPALIVE void createVertexShader() {

}

EMSCRIPTEN_KEEPALIVE kinc_g4_shader_t* createVertexShaderFromSource(char* source) {
	// char* source = new char[strlen(*utf8_value) + 1];
	// strcpy(source, *utf8_value);
	kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_VERTEX);
	return shader;
}

EMSCRIPTEN_KEEPALIVE void createFragmentShader() {

}

EMSCRIPTEN_KEEPALIVE kinc_g4_shader_t* createFragmentShaderFromSource(char* source) {
	// char* source = new char[strlen(*utf8_value) + 1];
	// strcpy(source, *utf8_value);
	kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_FRAGMENT);
	return shader;
}

EMSCRIPTEN_KEEPALIVE void createGeometryShader() {

}

EMSCRIPTEN_KEEPALIVE void createTessellationControlShader() {

}

EMSCRIPTEN_KEEPALIVE void createTessellationEvaluationShader() {

}

EMSCRIPTEN_KEEPALIVE void deleteShader() {

}

EMSCRIPTEN_KEEPALIVE kinc_g4_pipeline_t* createPipeline() {
	kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)malloc(sizeof(kinc_g4_pipeline_t));
	kinc_g4_pipeline_init(pipeline);
	return pipeline;
}

EMSCRIPTEN_KEEPALIVE void deletePipeline() {

}

EMSCRIPTEN_KEEPALIVE void compilePipeline(kinc_g4_pipeline_t* pipeline, kinc_g4_shader_t* vertex_shader, kinc_g4_shader_t* fragment_shader) {
	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_FLOAT3);
	pipeline->vertex_shader = vertex_shader;
	pipeline->fragment_shader = fragment_shader;
	pipeline->input_layout[0] = &structure;
	pipeline->input_layout[1] = NULL;
	kinc_g4_pipeline_compile(pipeline);
}

EMSCRIPTEN_KEEPALIVE void setPipeline(kinc_g4_pipeline_t* pipeline) {
	kinc_g4_set_pipeline(pipeline);
}

EMSCRIPTEN_KEEPALIVE void loadImage() {

}

EMSCRIPTEN_KEEPALIVE void unloadImage() {

}

EMSCRIPTEN_KEEPALIVE void loadSound() {

}

EMSCRIPTEN_KEEPALIVE void setAudioCallback() {

}

EMSCRIPTEN_KEEPALIVE void audioThread() {

}

EMSCRIPTEN_KEEPALIVE void writeAudioBuffer() {

}

EMSCRIPTEN_KEEPALIVE void loadBlob() {

}

EMSCRIPTEN_KEEPALIVE void loadUrl() {

}

EMSCRIPTEN_KEEPALIVE void getConstantLocation() {

}

EMSCRIPTEN_KEEPALIVE void getTextureUnit() {

}

EMSCRIPTEN_KEEPALIVE void setTexture() {

}

EMSCRIPTEN_KEEPALIVE void setRenderTarget() {

}

EMSCRIPTEN_KEEPALIVE void setTextureDepth() {

}

EMSCRIPTEN_KEEPALIVE void setImageTexture() {

}

EMSCRIPTEN_KEEPALIVE void setTextureParameters() {

}

EMSCRIPTEN_KEEPALIVE void setTexture3DParameters() {

}

EMSCRIPTEN_KEEPALIVE void setTextureCompareMode() {

}

EMSCRIPTEN_KEEPALIVE void setCubeMapCompareMode() {

}

EMSCRIPTEN_KEEPALIVE void setBool() {

}

EMSCRIPTEN_KEEPALIVE void setInt() {

}

EMSCRIPTEN_KEEPALIVE void setFloat() {

}

EMSCRIPTEN_KEEPALIVE void setFloat2() {

}

EMSCRIPTEN_KEEPALIVE void setFloat3() {

}

EMSCRIPTEN_KEEPALIVE void setFloat4() {

}

EMSCRIPTEN_KEEPALIVE void setFloats() {

}

EMSCRIPTEN_KEEPALIVE void setMatrix() {

}

EMSCRIPTEN_KEEPALIVE void setMatrix3() {

}

EMSCRIPTEN_KEEPALIVE void getTime() {

}

EMSCRIPTEN_KEEPALIVE void windowWidth() {

}

EMSCRIPTEN_KEEPALIVE void windowHeight() {

}

EMSCRIPTEN_KEEPALIVE void setWindowTitle() {

}

EMSCRIPTEN_KEEPALIVE void screenDpi() {

}

EMSCRIPTEN_KEEPALIVE void systemId() {

}

EMSCRIPTEN_KEEPALIVE void requestShutdown() {

}

EMSCRIPTEN_KEEPALIVE void displayCount() {

}

EMSCRIPTEN_KEEPALIVE void displayWidth() {

}

EMSCRIPTEN_KEEPALIVE void displayHeight() {

}

EMSCRIPTEN_KEEPALIVE void displayX() {

}

EMSCRIPTEN_KEEPALIVE void displayY() {

}

EMSCRIPTEN_KEEPALIVE void displayIsPrimary() {

}

EMSCRIPTEN_KEEPALIVE void writeStorage() {

}

EMSCRIPTEN_KEEPALIVE void readStorage() {

}

EMSCRIPTEN_KEEPALIVE void createRenderTarget() {

}

EMSCRIPTEN_KEEPALIVE void createRenderTargetCubeMap() {

}

EMSCRIPTEN_KEEPALIVE void createTexture() {

}

EMSCRIPTEN_KEEPALIVE void createTexture3D() {

}

EMSCRIPTEN_KEEPALIVE void createTextureFromBytes() {

}

EMSCRIPTEN_KEEPALIVE void createTextureFromBytes3D() {

}

EMSCRIPTEN_KEEPALIVE void createTextureFromEncodedBytes() {

}

EMSCRIPTEN_KEEPALIVE void getTexturePixels() {

}

EMSCRIPTEN_KEEPALIVE void getRenderTargetPixels() {

}

EMSCRIPTEN_KEEPALIVE void lockTexture() {

}

EMSCRIPTEN_KEEPALIVE void unlockTexture() {

}

EMSCRIPTEN_KEEPALIVE void clearTexture() {

}

EMSCRIPTEN_KEEPALIVE void generateTextureMipmaps() {

}

EMSCRIPTEN_KEEPALIVE void generateRenderTargetMipmaps() {

}

EMSCRIPTEN_KEEPALIVE void setMipmaps() {

}

EMSCRIPTEN_KEEPALIVE void setDepthStencilFrom() {

}

EMSCRIPTEN_KEEPALIVE void viewport() {

}

EMSCRIPTEN_KEEPALIVE void scissor() {

}

EMSCRIPTEN_KEEPALIVE void disableScissor() {

}

EMSCRIPTEN_KEEPALIVE void renderTargetsInvertedY() {

}

EMSCRIPTEN_KEEPALIVE void begin(/*renderTarget, additionalRenderTargets*/) {
	// if (renderTarget == NULL) {
		kinc_g4_restore_render_target();
	// }
	// else {
		// kinc_g4_set_render_targets();
	// }
}

EMSCRIPTEN_KEEPALIVE void beginFace() {

}

EMSCRIPTEN_KEEPALIVE void end() {

}

EMSCRIPTEN_KEEPALIVE void fileSaveBytes() {

}

EMSCRIPTEN_KEEPALIVE void sysCommand() {

}

EMSCRIPTEN_KEEPALIVE void savePath() {

}

EMSCRIPTEN_KEEPALIVE void getArgCount() {

}

EMSCRIPTEN_KEEPALIVE void getArg() {

}

EMSCRIPTEN_KEEPALIVE void getFilesLocation() {

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

// #ifdef WITH_NFD
// EMSCRIPTEN_KEEPALIVE void openDialog() {

// }

// EMSCRIPTEN_KEEPALIVE void saveDialog() {

// }
// #endif

// #ifdef WITH_TINYDIR
// EMSCRIPTEN_KEEPALIVE void readDirectory() {

// }
// #endif

// #ifdef KORE_DIRECT3D12
// EMSCRIPTEN_KEEPALIVE void raytraceInit() {

// }

// EMSCRIPTEN_KEEPALIVE void raytraceSetTextures() {

// }

// EMSCRIPTEN_KEEPALIVE void raytraceDispatchRays() {

// }
// #endif

EMSCRIPTEN_KEEPALIVE void windowX() {

}

EMSCRIPTEN_KEEPALIVE void windowY() {

}

int kickstart(int argc, char** argv) {
	
	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, "krom.js", KINC_FILE_TYPE_ASSET)) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not load krom.js, aborting.");
		exit(1);
	}

	int reader_size = (int)kinc_file_reader_size(&reader);
	char* code_krom = (char*)malloc(reader_size + 1);
	kinc_file_reader_read(&reader, code_krom, reader_size);
	code_krom[reader_size] = 0;
	kinc_file_reader_close(&reader);

	char* code_header = "Krom = {};\
	Krom.init = Module.cwrap('init', null, ['string', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']);\
	Krom.setApplicationName = _setApplicationName;\
	Krom.log = _krom_log;\
	Krom.clear = _clear;\
	Krom.setCallback = function(f) { _setCallback(addFunctionWasm(f, 'v')) };\
	Krom.setDropFilesCallback = _setDropFilesCallback;\
	Krom.setCutCopyPasteCallback = _setCutCopyPasteCallback;\
	Krom.setApplicationStateCallback = _setApplicationStateCallback;\
	Krom.setKeyboardDownCallback = _setKeyboardDownCallback;\
	Krom.setKeyboardUpCallback = _setKeyboardUpCallback;\
	Krom.setKeyboardPressCallback = _setKeyboardPressCallback;\
	Krom.setMouseDownCallback = _setMouseDownCallback;\
	Krom.setMouseUpCallback = _setMouseUpCallback;\
	Krom.setMouseMoveCallback = _setMouseMoveCallback;\
	Krom.setMouseWheelCallback = _setMouseWheelCallback;\
	Krom.setPenDownCallback = _setPenDownCallback;\
	Krom.setPenUpCallback = _setPenUpCallback;\
	Krom.setPenMoveCallback = _setPenMoveCallback;\
	Krom.setGamepadAxisCallback = _setGamepadAxisCallback;\
	Krom.setGamepadButtonCallback = _setGamepadButtonCallback;\
	Krom.lockMouse = _lockMouse;\
	Krom.unlockMouse = _unlockMouse;\
	Krom.canLockMouse = _canLockMouse;\
	Krom.isMouseLocked = _isMouseLocked;\
	Krom.setMousePosition = _setMousePosition;\
	Krom.showMouse = _showMouse;\
	Krom.showKeyboard = _showKeyboard;\
	Krom.createIndexBuffer = _createIndexBuffer;\
	Krom.deleteIndexBuffer = _deleteIndexBuffer;\
	Krom.lockIndexBuffer = function(buffer) { return new Uint32Array(Module['HEAPU8'].buffer, _lockIndexBuffer(buffer), _index_buffer_size(buffer)); };\
	Krom.unlockIndexBuffer = _unlockIndexBuffer;\
	Krom.setIndexBuffer = _setIndexBuffer;\
	Krom.createVertexBuffer = _createVertexBuffer;\
	Krom.deleteVertexBuffer = _deleteVertexBuffer;\
	Krom.lockVertexBuffer = function(buffer) { return new Float32Array(Module['HEAPU8'].buffer, _lockVertexBuffer(buffer), _vertex_buffer_size(buffer)); };\
	Krom.unlockVertexBuffer = _unlockVertexBuffer;\
	Krom.setVertexBuffer = _setVertexBuffer;\
	Krom.setVertexBuffers = _setVertexBuffers;\
	Krom.drawIndexedVertices = _drawIndexedVertices;\
	Krom.drawIndexedVerticesInstanced = _drawIndexedVerticesInstanced;\
	Krom.createVertexShader = _createVertexShader;\
	Krom.createVertexShaderFromSource = Module.cwrap('createVertexShaderFromSource', 'number', ['string']);\
	Krom.createFragmentShader = _createFragmentShader;\
	Krom.createFragmentShaderFromSource = Module.cwrap('createFragmentShaderFromSource', 'number', ['string']);\
	Krom.createGeometryShader = _createGeometryShader;\
	Krom.createTessellationControlShader = _createTessellationControlShader;\
	Krom.createTessellationEvaluationShader = _createTessellationEvaluationShader;\
	Krom.deleteShader = _deleteShader;\
	Krom.createPipeline = _createPipeline;\
	Krom.deletePipeline = _deletePipeline;\
	Krom.compilePipeline = function(pipeline, structure0, structure1, structure2, structure3, length, vertexShader, fragmentShader, geometryShader, tessellationControlShader, tessellationEvaluationShader, state) { _compilePipeline(pipeline, vertexShader, fragmentShader); };\
	Krom.setPipeline = _setPipeline;\
	Krom.loadImage = _loadImage;\
	Krom.unloadImage = _unloadImage;\
	Krom.loadSound = _loadSound;\
	Krom.setAudioCallback = _setAudioCallback;\
	Krom.audioThread = _audioThread;\
	Krom.writeAudioBuffer = _writeAudioBuffer;\
	Krom.loadBlob = _loadBlob;\
	Krom.loadUrl = _loadUrl;\
	Krom.getConstantLocation = _getConstantLocation;\
	Krom.getTextureUnit = _getTextureUnit;\
	Krom.setTexture = _setTexture;\
	Krom.setRenderTarget = _setRenderTarget;\
	Krom.setTextureDepth = _setTextureDepth;\
	Krom.setImageTexture = _setImageTexture;\
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
	Krom.setFloats = _setFloats;\
	Krom.setMatrix = _setMatrix;\
	Krom.setMatrix3 = _setMatrix3;\
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
	Krom.displayIsPrimary = _displayIsPrimary;\
	Krom.writeStorage = _writeStorage;\
	Krom.readStorage = _readStorage;\
	Krom.createRenderTarget = _createRenderTarget;\
	Krom.createRenderTargetCubeMap = _createRenderTargetCubeMap;\
	Krom.createTexture = _createTexture;\
	Krom.createTexture3D = _createTexture3D;\
	Krom.createTextureFromBytes = _createTextureFromBytes;\
	Krom.createTextureFromBytes3D = _createTextureFromBytes3D;\
	Krom.createTextureFromEncodedBytes = _createTextureFromEncodedBytes;\
	Krom.getTexturePixels = _getTexturePixels;\
	Krom.getRenderTargetPixels = _getRenderTargetPixels;\
	Krom.lockTexture = _lockTexture;\
	Krom.unlockTexture = _unlockTexture;\
	Krom.clearTexture = _clearTexture;\
	Krom.generateTextureMipmaps = _generateTextureMipmaps;\
	Krom.generateRenderTargetMipmaps = _generateRenderTargetMipmaps;\
	Krom.setMipmaps = _setMipmaps;\
	Krom.setDepthStencilFrom = _setDepthStencilFrom;\
	Krom.viewport = _viewport;\
	Krom.scissor = _scissor;\
	Krom.disableScissor = _disableScissor;\
	Krom.renderTargetsInvertedY = _renderTargetsInvertedY;\
	Krom.begin = _begin;\
	Krom.beginFace = _beginFace;\
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
	// #ifdef WITH_NFD
	// Krom.openDialog = _openDialog;
	// Krom.saveDialog = _saveDialog;
	// #endif
	// #ifdef WITH_TINYDIR
	// Krom.readDirectory = _readDirectory;
	// #endif
	// #ifdef KORE_DIRECT3D12
	// Krom.raytraceInit = _raytraceInit;
	// Krom.raytraceSetTextures = _raytraceSetTextures;
	// Krom.raytraceDispatchRays = _raytraceDispatchRays;
	// #endif

	char* code = (char*)malloc(strlen(code_header) + strlen(code_krom) + 1);
	strcpy(code, code_header);
	strcat(code, code_krom);
	free(code_krom);

	emscripten_run_script(code);

	kinc_start();

	free(code);

	return 0;
}
