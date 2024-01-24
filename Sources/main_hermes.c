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
#ifdef WITH_G2
#include "g2/g2.h"
#include "g2/g2_ext.h"
#endif
#include <hermes/VM/static_h.h>

static bool enable_window = true;

static void (*update_func)() = NULL;
static void (*keyboard_down_func)(int) = NULL;
static void (*keyboard_up_func)(int) = NULL;
static void (*keyboard_press_func)(int) = NULL;
static void (*mouse_move_func)(int, int, int, int) = NULL;
static void (*mouse_down_func)(int, int, int) = NULL;
static void (*mouse_up_func)(int, int, int) = NULL;
static void (*mouse_wheel_func)(int) = NULL;

static SHRuntime *shr;

extern SHUnit sh_export_this_unit;
void js_eval(SHRuntime *shr, char *str);
void js_call(SHRuntime *shr, char *fn_name);
void js_array_buffer(SHRuntime *shr, uint8_t *buffer, uint32_t length);

void update(void *data) {
	kinc_g4_begin(0);

	js_call(shr, "krom_callback");

	kinc_g4_end(0);
	kinc_g4_swap_buffers();
}

void key_down(int code, void *data) {

}

void key_up(int code, void *data) {

}

void key_press(unsigned int code, void *data) {

}

void mouse_move(int window, int x, int y, int mx, int my, void *data) {

}

void mouse_down(int window, int button, int x, int y, void *data) {

}

void mouse_up(int window, int button, int x, int y, void *data) {

}

void mouse_wheel(int window, int delta, void *data) {

}

void _krom_init(char *title, int width, int height, bool v_sync, int window_mode, int window_features, int x, int y, int frequency) {
	// kinc_threads_init();
	kinc_display_init();

	kinc_window_options_t win;
	kinc_framebuffer_options_t frame;
	win.title = title;
	win.width = width;
	win.height = height;
	frame.vertical_sync = v_sync;
	win.mode = window_mode;
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
	kinc_init(win.title, win.width, win.height, &win, &frame);
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

	kinc_set_update_callback(update, NULL);
	// kinc_set_drop_files_callback(drop_files, NULL);
	// kinc_set_copy_callback(copy, NULL);
	// kinc_set_cut_callback(cut, NULL);
	// kinc_set_paste_callback(paste, NULL);
	// kinc_set_foreground_callback(foreground, NULL);
	// kinc_set_resume_callback(resume, NULL);
	// kinc_set_pause_callback(pause, NULL);
	// kinc_set_background_callback(background, NULL);
	// kinc_set_shutdown_callback(shutdown, NULL);

	// kinc_keyboard_set_key_down_callback(key_down, NULL);
	// kinc_keyboard_set_key_up_callback(key_up, NULL);
	// kinc_keyboard_set_key_press_callback(key_press, NULL);
	// kinc_mouse_set_move_callback(mouse_move, NULL);
	// kinc_mouse_set_press_callback(mouse_down, NULL);
	// kinc_mouse_set_release_callback(mouse_up, NULL);
	// kinc_mouse_set_scroll_callback(mouse_wheel, NULL);
	// kinc_surface_set_move_callback(touch_move);
	// kinc_surface_set_touch_start_callback(touch_down);
	// kinc_surface_set_touch_end_callback(touch_up);
	// kinc_pen_set_press_callback(pen_down);
	// kinc_pen_set_move_callback(pen_move);
	// kinc_pen_set_release_callback(pen_up);
	// kinc_gamepad_set_axis_callback(gamepad_axis);
	// kinc_gamepad_set_button_callback(gamepad_button);

	#ifdef KORE_ANDROID
	android_check_permissions();
	#endif
}

void _krom_set_callback(char *callback) {

}

void _krom_begin() {
	kinc_g4_restore_render_target();
}

void _krom_end() {

}

void _krom_clear(int flags, int color, float depth, int stencil) {
	kinc_g4_clear(flags, color, depth, stencil);
}

kinc_g4_pipeline_t *_krom_create_pipeline() {
	kinc_g4_pipeline_t *pipeline = (kinc_g4_pipeline_t *)malloc(sizeof(kinc_g4_pipeline_t));
	kinc_g4_pipeline_init(pipeline);
	return pipeline;
}

kinc_g4_shader_t *_krom_create_vertex_shader_from_source(char *source) {

	#ifdef WITH_D3DCOMPILER

	strcpy(temp_string_vs, source);

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
	HRESULT hr = D3DCompile(temp_string_vs, strlen(source) + 1, nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, &shader_buffer, &error_message);
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
	strcat(temp_string_vs, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, temp_string_vs, strlen(temp_string_vs), KINC_G4_SHADER_TYPE_VERTEX);

	#elif defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)

	char *output = (char *)malloc(1024 * 1024);
	int length;
	krafix_compile(source, output, &length, "spirv", "windows", "vert", -1);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_VERTEX);

	#else

	char *str = (char *)malloc(strlen(source) + 1);
	strcpy(str, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, str, strlen(str), KINC_G4_SHADER_TYPE_VERTEX);

	#endif

	return shader;
}

kinc_g4_shader_t *_krom_create_fragment_shader_from_source(char *source) {
	#ifdef WITH_D3DCOMPILER

	strcpy(temp_string_fs, source);

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
	HRESULT hr = D3DCompile(temp_string_fs, strlen(source) + 1, nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, &shader_buffer, &error_message);
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
	strcat(temp_string_fs, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, temp_string_fs, strlen(temp_string_fs), KINC_G4_SHADER_TYPE_FRAGMENT);

	#elif defined(KORE_VULKAN) && defined(KRAFIX_LIBRARY)

	char *output = (char *)malloc(1024 * 1024);
	int length;
	krafix_compile(source, output, &length, "spirv", "windows", "frag", -1);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, output, length, KINC_G4_SHADER_TYPE_FRAGMENT);

	#else

	char *str = (char *)malloc(strlen(source) + 1);
	strcpy(str, source);
	kinc_g4_shader_t *shader = (kinc_g4_shader_t *)malloc(sizeof(kinc_g4_shader_t));
	kinc_g4_shader_init(shader, str, strlen(str), KINC_G4_SHADER_TYPE_FRAGMENT);

	#endif

	return shader;
}

void _krom_compile_pipeline(kinc_g4_pipeline_t *pipeline,
	char *name0, int data0, char *name1, int data1, char *name2, int data2, char *name3, int data3,
	char *name4, int data4, char *name5, int data5, char *name6, int data6, char *name7, int data7,
	kinc_g4_cull_mode_t cull_mode, bool depth_write, kinc_g4_compare_mode_t depth_mode,
	int blend_source, int blend_destination, int alpha_blend_source, int alpha_blend_destination,
	bool color_write_mask_red, bool color_write_mask_green, bool color_write_mask_blue, bool color_write_mask_alpha,
	int color_attachment_count, int depth_attachment_bits,
	kinc_g4_shader_t *vertex_shader, kinc_g4_shader_t *fragment_shader) {

	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	if (name0 != NULL) kinc_g4_vertex_structure_add(&structure, name0, (kinc_g4_vertex_data_t)data0);
	if (name1 != NULL) kinc_g4_vertex_structure_add(&structure, name1, (kinc_g4_vertex_data_t)data1);
	if (name2 != NULL) kinc_g4_vertex_structure_add(&structure, name2, (kinc_g4_vertex_data_t)data2);
	if (name3 != NULL) kinc_g4_vertex_structure_add(&structure, name3, (kinc_g4_vertex_data_t)data3);
	if (name4 != NULL) kinc_g4_vertex_structure_add(&structure, name4, (kinc_g4_vertex_data_t)data4);
	if (name5 != NULL) kinc_g4_vertex_structure_add(&structure, name5, (kinc_g4_vertex_data_t)data5);
	if (name6 != NULL) kinc_g4_vertex_structure_add(&structure, name6, (kinc_g4_vertex_data_t)data6);
	if (name7 != NULL) kinc_g4_vertex_structure_add(&structure, name7, (kinc_g4_vertex_data_t)data7);

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

	kinc_g4_pipeline_compile(pipeline);
}

kinc_g4_vertex_buffer_t *_krom_create_vertex_buffer(int count,
	char *name0, int data0, char *name1, int data1, char *name2, int data2, char *name3, int data3,
	char *name4, int data4, char *name5, int data5, char *name6, int data6, char *name7, int data7,
	int usage, int instance_data_step_rate) {

	kinc_g4_vertex_buffer_t *buffer = (kinc_g4_vertex_buffer_t *)malloc(sizeof(kinc_g4_vertex_buffer_t));

	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	if (name0 != NULL) kinc_g4_vertex_structure_add(&structure, name0, (kinc_g4_vertex_data_t)data0);
	if (name1 != NULL) kinc_g4_vertex_structure_add(&structure, name1, (kinc_g4_vertex_data_t)data1);
	if (name2 != NULL) kinc_g4_vertex_structure_add(&structure, name2, (kinc_g4_vertex_data_t)data2);
	if (name3 != NULL) kinc_g4_vertex_structure_add(&structure, name3, (kinc_g4_vertex_data_t)data3);
	if (name4 != NULL) kinc_g4_vertex_structure_add(&structure, name4, (kinc_g4_vertex_data_t)data4);
	if (name5 != NULL) kinc_g4_vertex_structure_add(&structure, name5, (kinc_g4_vertex_data_t)data5);
	if (name6 != NULL) kinc_g4_vertex_structure_add(&structure, name6, (kinc_g4_vertex_data_t)data6);
	if (name7 != NULL) kinc_g4_vertex_structure_add(&structure, name7, (kinc_g4_vertex_data_t)data7);

	kinc_g4_vertex_buffer_init(buffer, count, &structure, (kinc_g4_usage_t)usage, instance_data_step_rate);
	return buffer;
}

float *_krom_lock_vertex_buffer(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	float *vertices = kinc_g4_vertex_buffer_lock(buffer, start, count);
	vertices[0] = 123.5;
	js_array_buffer(shr, (uint8_t *)vertices, (count * kinc_g4_vertex_buffer_stride(buffer)));
	return vertices;
}

void _krom_unlock_vertex_buffer(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_g4_vertex_buffer_unlock(buffer, count);
}

kinc_g4_index_buffer_t *_krom_create_index_buffer(int count) {
	kinc_g4_index_buffer_t *buffer = (kinc_g4_index_buffer_t *)malloc(sizeof(kinc_g4_index_buffer_t));
	kinc_g4_index_buffer_init(buffer, count, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
	return buffer;
}

void *_krom_lock_index_buffer(kinc_g4_index_buffer_t *buffer) {
	void *vertices = kinc_g4_index_buffer_lock_all(buffer);
	js_array_buffer(shr, (uint8_t *)vertices, kinc_g4_index_buffer_count(buffer) * sizeof(int));
	return vertices;
}

void _krom_unlock_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_unlock_all(buffer);
}

void _krom_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_set_pipeline(pipeline);
}

void _krom_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_set_vertex_buffer(buffer);
}

void _krom_set_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_set_index_buffer(buffer);
}

void _krom_draw_indexed_vertices(int start, int count) {
	#ifdef KORE_DIRECT3D12
	// TODO: Prevent heapIndex overflow in texture.c.h/kinc_g5_internal_set_textures
	waitAfterNextDraw = true;
	#endif
	if (count < 0) kinc_g4_draw_indexed_vertices();
	else kinc_g4_draw_indexed_vertices_from_to(start, count);
}

int kickstart(int argc, char **argv) {

	// libmain.a/main()
	shr = _sh_init(argc, argv);
	_sh_initialize_units(shr, 1, &sh_export_this_unit);
	// _sh_done(shr);

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

	return 0;
}
