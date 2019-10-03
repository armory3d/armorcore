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
#include <kinc/compute/compute.h>
#include <kinc/libs/stb_image.h>

#include "../V8/include/libplatform/libplatform.h"
#include "../V8/include/v8.h"

#include <map>
#include <string>
#include <vector>

#ifdef KORE_WINDOWS
#include <Windows.h> // AttachConsole
#include <Kore/Windows.h> // kinc_windows_window_handle
#endif

#ifdef WITH_D3DCOMPILER
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <strstream>
#endif
#ifdef KORE_DIRECT3D12
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/raytrace.h>
#endif
#ifdef WITH_NFD
#include <nfd.h>
#endif

using namespace v8;

const int KROM_API = 3;

int save_and_quit = 0; // off, save, nosave
void armory_save_and_quit(bool save) { save_and_quit = save ? 1 : 2; }

#ifdef KORE_DIRECT3D12
#ifdef __cplusplus
extern "C" {
#endif
	extern kinc_g5_command_list_t commandList;
	kinc_g5_constant_buffer_t constant_buffer;
	kinc_g5_vertex_buffer_t vertex_buffer;
	kinc_g5_index_buffer_t index_buffer;
	kinc_g4_render_target_t* render_target;
	kinc_raytrace_target_t target;
	kinc_raytrace_pipeline_t pipeline;
	kinc_raytrace_acceleration_structure_t accel;
	const int constant_buffer_size = 20;
#ifdef __cplusplus
}
#endif
#endif

namespace {
	int _argc;
	char** _argv;
	bool enable_sound = true;
	bool enable_window = true;

	Global<Context> global_context;
	Isolate* isolate;
	std::unique_ptr<Platform> plat;
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
	Global<Function> mouse_wheel_func;
	Global<Function> pen_down_func;
	Global<Function> pen_up_func;
	Global<Function> pen_move_func;
	Global<Function> gamepad_axis_func;
	Global<Function> gamepad_button_func;
	Global<Function> audio_func;
	Global<Function> save_and_quit_func;

	kinc_mutex_t mutex;
	kinc_a2_buffer_t audio_buffer;
	int audio_read_location = 0;

	void update();
	void update_audio(kinc_a2_buffer_t *buffer, int samples);
	void drop_files(wchar_t* file_path);
	char* cut();
	char* copy();
	void paste(char* data);
	void foreground();
	void resume();
	void pause();
	void background();
	void shutdown();
	void key_down(int code);
	void key_up(int code);
	void key_press(unsigned int character);
	void mouse_move(int window, int x, int y, int mx, int my);
	void mouse_down(int window, int button, int x, int y);
	void mouse_up(int window, int button, int x, int y);
	void mouse_wheel(int window, int delta);
	void pen_down(int window, int x, int y, float pressure);
	void pen_up(int window, int x, int y, float pressure);
	void pen_move(int window, int x, int y, float pressure);
	void gamepad_axis(int gamepad, int axis, float value);
	void gamepad_button(int gamepad, int button, float value);

	char temp_string_vs[1024 * 1024];
	char temp_string_fs[1024 * 1024];
	char temp_string_vstruct[4][32][32];
	std::string assetsdir;

	void krom_init(const v8::FunctionCallbackInfo<v8::Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value title(isolate, arg);
		int width = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int samples_per_pixel = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		bool vertical_sync = args[4]->ToBoolean(isolate)->Value();
		int window_mode = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int window_features = args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int api_version = args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int x = args[8]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[9]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

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
		win.title = *title;
		win.x = x;
		win.y = y;
		win.width = width;
		win.height = height;
		win.display_index = -1;
		win.visible = enable_window;
		win.window_features = window_features;
		win.mode = (kinc_window_mode_t)window_mode;
		kinc_framebuffer_options_t frame;
		frame.frequency = 60;
		frame.vertical_sync = vertical_sync;
		frame.color_bits = 32;
		frame.depth_bits = 16;
		frame.stencil_bits = 8;
		frame.samples_per_pixel = samples_per_pixel;
		kinc_init(*title, width, height, &win, &frame);
		kinc_mutex_init(&mutex);
		kinc_random_init((int)(kinc_time() * 1000));

		#ifdef WITH_AUDIO
		if (enable_sound) {
			kinc_a2_init();
			kinc_a2_set_callback(update_audio);
			audio_buffer.read_location = 0;
			audio_buffer.write_location = 0;
			audio_buffer.data_size = 128 * 1024;
			audio_buffer.data = new uint8_t[audio_buffer.data_size];
		}
		#endif

		kinc_set_update_callback(update);
		kinc_set_drop_files_callback(drop_files);
		kinc_set_copy_callback(copy);
		kinc_set_cut_callback(cut);
		kinc_set_paste_callback(paste);
		kinc_set_foreground_callback(foreground);
		kinc_set_resume_callback(resume);
		kinc_set_pause_callback(pause);
		kinc_set_background_callback(background);
		kinc_set_shutdown_callback(shutdown);

		kinc_keyboard_key_down_callback = key_down;
		kinc_keyboard_key_up_callback = key_up;
		kinc_keyboard_key_press_callback = key_press;
		kinc_mouse_move_callback = mouse_move;
		kinc_mouse_press_callback = mouse_down;
		kinc_mouse_release_callback = mouse_up;
		kinc_mouse_scroll_callback = mouse_wheel;
		kinc_pen_press_callback = pen_down;
		kinc_pen_move_callback = pen_move;
		kinc_pen_release_callback = pen_up;
		kinc_gamepad_axis_callback = gamepad_axis;
		kinc_gamepad_button_callback = gamepad_button;
	}

	void krom_log(const v8::FunctionCallbackInfo<v8::Value>& args) {
		if (args.Length() < 1) return;
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		String::Utf8Value value(isolate, arg);
		kinc_log(KINC_LOG_LEVEL_INFO, *value);
	}

	void krom_clear(const v8::FunctionCallbackInfo<v8::Value>& args) {
		HandleScope scope(args.GetIsolate());
		int flags = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float depth = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int stencil = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_clear(flags, color, depth, stencil);
	}

	void krom_set_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		update_func.Reset(isolate, func);
	}

	void krom_set_drop_files_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		drop_files_func.Reset(isolate, func);
	}

	void krom_set_cut_copy_paste_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> cutArg = args[0];
		Local<Function> cutFunc = Local<Function>::Cast(cutArg);
		cut_func.Reset(isolate, cutFunc);
		Local<Value> copyArg = args[1];
		Local<Function> copyFunc = Local<Function>::Cast(copyArg);
		copy_func.Reset(isolate, copyFunc);
		Local<Value> pasteArg = args[2];
		Local<Function> pasteFunc = Local<Function>::Cast(pasteArg);
		paste_func.Reset(isolate, pasteFunc);
	}

	void krom_set_application_state_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> foregroundArg = args[0];
		Local<Function> foregroundFunc = Local<Function>::Cast(foregroundArg);
		foreground_func.Reset(isolate, foregroundFunc);
		Local<Value> resumeArg = args[1];
		Local<Function> resumeFunc = Local<Function>::Cast(resumeArg);
		resume_func.Reset(isolate, resumeFunc);
		Local<Value> pauseArg = args[2];
		Local<Function> pauseFunc = Local<Function>::Cast(pauseArg);
		pause_func.Reset(isolate, pauseFunc);
		Local<Value> backgroundArg = args[3];
		Local<Function> backgroundFunc = Local<Function>::Cast(backgroundArg);
		background_func.Reset(isolate, backgroundFunc);
		Local<Value> shutdownArg = args[4];
		Local<Function> shutdownFunc = Local<Function>::Cast(shutdownArg);
		shutdown_func.Reset(isolate, shutdownFunc);
	}

	void krom_set_keyboard_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_down_func.Reset(isolate, func);
	}

	void krom_set_keyboard_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_up_func.Reset(isolate, func);
	}

	void krom_set_keyboard_press_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		keyboard_press_func.Reset(isolate, func);
	}

	void krom_set_mouse_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_down_func.Reset(isolate, func);
	}

	void krom_set_mouse_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_up_func.Reset(isolate, func);
	}

	void krom_set_mouse_move_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_move_func.Reset(isolate, func);
	}

	void krom_set_mouse_wheel_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		mouse_wheel_func.Reset(isolate, func);
	}

	void krom_set_pen_down_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_down_func.Reset(isolate, func);
	}

	void krom_set_pen_up_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_up_func.Reset(isolate, func);
	}

	void krom_set_pen_move_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		pen_move_func.Reset(isolate, func);
	}

	void krom_set_gamepad_axis_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepad_axis_func.Reset(isolate, func);
	}

	void krom_set_gamepad_button_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		gamepad_button_func.Reset(isolate, func);
	}

	void krom_lock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_mouse_lock(0);
	}

	void krom_unlock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_mouse_unlock(0);
	}

	void krom_can_lock_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, kinc_mouse_can_lock(0)));
	}

	void krom_is_mouse_locked(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, kinc_mouse_is_locked(0)));
	}

	void krom_show_mouse(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args[0]->ToBoolean(isolate)->Value() ? kinc_mouse_show() : kinc_mouse_hide();
	}

	void krom_set_audio_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		audio_func.Reset(isolate, func);
	}

	void krom_audio_thread(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		bool lock = args[0]->ToBoolean(isolate)->Value();
		if (lock) kinc_mutex_lock(&mutex);    //v8::Locker::Locker(isolate);
		else kinc_mutex_unlock(&mutex);       //v8::Unlocker(args.GetIsolate());
	}

	void krom_create_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)malloc(sizeof(kinc_g4_index_buffer_t));
		kinc_g4_index_buffer_init(buffer, args[0]->Int32Value(isolate->GetCurrentContext()).FromJust());
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, buffer));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		kinc_g4_index_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_lock_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		int* vertices = kinc_g4_index_buffer_lock(buffer);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, vertices, kinc_g4_index_buffer_count(buffer) * sizeof(int));
		args.GetReturnValue().Set(Uint32Array::New(abuffer, 0, kinc_g4_index_buffer_count(buffer)));
	}

	void krom_unlock_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		kinc_g4_index_buffer_unlock(buffer);
	}

	void krom_set_indexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_index_buffer_t* buffer = (kinc_g4_index_buffer_t*)field->Value();
		kinc_g4_set_index_buffer(buffer);
	}

	kinc_g4_vertex_data_t convert_vertex_data(int num) {
		switch (num) {
		case 0:
			return KINC_G4_VERTEX_DATA_FLOAT1;
		case 1:
			return KINC_G4_VERTEX_DATA_FLOAT2;
		case 2:
			return KINC_G4_VERTEX_DATA_FLOAT3;
		case 3:
			return KINC_G4_VERTEX_DATA_FLOAT4;
		case 4:
			return KINC_G4_VERTEX_DATA_FLOAT4X4;
		case 5:
			return KINC_G4_VERTEX_DATA_SHORT2_NORM;
		case 6:
			return KINC_G4_VERTEX_DATA_SHORT4_NORM;
		}
		return KINC_G4_VERTEX_DATA_FLOAT1;
	}

	void krom_create_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> jsstructure = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_vertex_structure_t structure;
		kinc_g4_vertex_structure_init(&structure);
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> element = jsstructure->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<Value> str = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
			String::Utf8Value utf8_value(isolate, str);
			int32_t data = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "data").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			strcpy(temp_string_vstruct[0][i], *utf8_value);
			kinc_g4_vertex_structure_add(&structure, temp_string_vstruct[0][i], convert_vertex_data(data));
		}
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)malloc(sizeof(kinc_g4_vertex_buffer_t));
		kinc_g4_vertex_buffer_init(buffer, args[0]->Int32Value(isolate->GetCurrentContext()).FromJust(), &structure, (kinc_g4_usage_t)args[2]->Int32Value(isolate->GetCurrentContext()).FromJust(), args[3]->Int32Value(isolate->GetCurrentContext()).FromJust());
		obj->SetInternalField(0, External::New(isolate, buffer));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		kinc_g4_vertex_buffer_destroy(buffer);
		free(buffer);
	}

	void krom_lock_vertex_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		float* vertices = kinc_g4_vertex_buffer_lock_all(buffer);
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, vertices, kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer));
		args.GetReturnValue().Set(Float32Array::New(abuffer, 0, kinc_g4_vertex_buffer_count(buffer) * kinc_g4_vertex_buffer_stride(buffer) / 4));
	}

	void krom_unlock_vertex_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		kinc_g4_vertex_buffer_unlock_all(buffer);
	}

	void krom_set_vertexbuffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)field->Value();
		kinc_g4_set_vertex_buffer(buffer);
	}

	void krom_set_vertexbuffers(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_vertex_buffer_t* vertex_buffers[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		Local<Object> jsarray = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> bufferobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "buffer").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> bufferfield = Local<External>::Cast(bufferobj->GetInternalField(0));
			kinc_g4_vertex_buffer_t* buffer = (kinc_g4_vertex_buffer_t*)bufferfield->Value();
			vertex_buffers[i] = buffer;
		}
		kinc_g4_set_vertex_buffers(vertex_buffers, length);
	}

	void krom_draw_indexed_vertices(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int start = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (count < 0) kinc_g4_draw_indexed_vertices();
		else kinc_g4_draw_indexed_vertices_from_to(start, count);
	}

	void krom_draw_indexed_vertices_instanced(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int instance_count = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int start = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int count = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		if (count < 0) kinc_g4_draw_indexed_vertices_instanced(instance_count);
		else kinc_g4_draw_indexed_vertices_instanced_from_to(instance_count, start, count);
	}

	void krom_create_vertex_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_VERTEX);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_vertex_shader_from_source(const FunctionCallbackInfo<Value>& args) {

		#ifdef WITH_D3DCOMPILER

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		strcpy(temp_string_vs, *utf8_value);

		ID3DBlob* error_message;
		ID3DBlob* shader_buffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(temp_string_vs, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, &shader_buffer, &error_message);
		if (hr != S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char*)error_message->GetBufferPointer());
			return;
		}

		// bool hasBone = strstr(temp_string_vs, "bone :") != NULL;
		// bool hasCol = strstr(temp_string_vs, "col :") != NULL;
		bool hasNor = strstr(temp_string_vs, "nor :") != NULL;
		bool hasPos = strstr(temp_string_vs, "pos :") != NULL;
		// bool hasTang = strstr(temp_string_vs, "tang :") != NULL;
		bool hasTex = strstr(temp_string_vs, "tex :") != NULL;
		// bool hasWeight = strstr(temp_string_vs, "weight :") != NULL;

		std::map<std::string, int> attributes;
		int index = 0;
		// if (hasBone) attributes["bone"] = index++;
		// if (hasCol) attributes["col"] = index++;
		if (hasNor) attributes["nor"] = index++;
		if (hasPos) attributes["pos"] = index++;
		// if (hasTang) attributes["tang"] = index++;
		if (hasTex) attributes["tex"] = index++;
		// if (hasWeight) attributes["weight"] = index++;

		char* output = temp_string_vs;
		std::ostrstream file(output, 1024 * 1024);
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

		ID3D11ShaderReflection* reflector = nullptr;
		D3DReflect(shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

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

		ID3D11ShaderReflectionConstantBuffer* constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables);
			output_len += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable* variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name;
					output_len += strlen(variableDesc.Name);
					file.put(0);
					output_len += 1;
					file.write((char*)&variableDesc.StartOffset, 4);
					output_len += 4;
					file.write((char*)&variableDesc.Size, 4);
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
		file.write((char*)shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize());
		output_len += shader_buffer->GetBufferSize();
		shader_buffer->Release();
		reflector->Release();

		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, (int)output_len, KINC_G4_SHADER_TYPE_VERTEX);

		#else

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		char* source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_VERTEX);

		#endif

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		Local<String> name = String::NewFromUtf8(isolate, "").ToLocalChecked();
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_fragment_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_FRAGMENT);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_fragment_shader_from_source(const FunctionCallbackInfo<Value>& args) {

		#ifdef WITH_D3DCOMPILER

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		strcpy(temp_string_fs, *utf8_value);

		ID3DBlob* error_message;
		ID3DBlob* shader_buffer;
		UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;// D3DCOMPILE_OPTIMIZATION_LEVEL0
		HRESULT hr = D3DCompile(temp_string_fs, strlen(*utf8_value) + 1, nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, &shader_buffer, &error_message);
		if (hr != S_OK) {
			kinc_log(KINC_LOG_LEVEL_INFO, "%s", (char*)error_message->GetBufferPointer());
			return;
		}

		std::map<std::string, int> attributes;

		char* output = temp_string_fs;
		std::ostrstream file(output, 1024 * 1024);
		size_t output_len = 0;

		file.put((char)attributes.size());
		output_len += 1;

		ID3D11ShaderReflection* reflector = nullptr;
		D3DReflect(shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

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

		ID3D11ShaderReflectionConstantBuffer* constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
			file.put(bufferDesc.Variables);
			output_len += 1;
			for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
				ID3D11ShaderReflectionVariable* variable = constants->GetVariableByIndex(i);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				hr = variable->GetDesc(&variableDesc);
				if (hr == S_OK) {
					(file) << variableDesc.Name;
					output_len += strlen(variableDesc.Name);
					file.put(0);
					output_len += 1;
					file.write((char*)&variableDesc.StartOffset, 4);
					output_len += 4;
					file.write((char*)&variableDesc.Size, 4);
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
		file.write((char*)shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize());
		output_len += shader_buffer->GetBufferSize();
		shader_buffer->Release();
		reflector->Release();

		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, output, (int)output_len, KINC_G4_SHADER_TYPE_FRAGMENT);

		#else

		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		char* source = new char[strlen(*utf8_value) + 1];
		strcpy(source, *utf8_value);
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init_from_source(shader, source, KINC_G4_SHADER_TYPE_FRAGMENT);

		#endif

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		Local<String> name = String::NewFromUtf8(isolate, "").ToLocalChecked();
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_geometry_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_GEOMETRY);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_tessellation_control_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_create_tessellation_evaluation_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)malloc(sizeof(kinc_g4_shader_t));
		kinc_g4_shader_init(shader, content.Data(), (int)content.ByteLength(), KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked(), args[1]);
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_shader(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_shader_t* shader = (kinc_g4_shader_t*)field->Value();
		kinc_g4_shader_destroy(shader);
		free(shader);
	}

	void krom_create_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)malloc(sizeof(kinc_g4_pipeline_t));
		kinc_g4_pipeline_init(pipeline);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(8);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, pipeline));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();
		kinc_g4_pipeline_destroy(pipeline);
		free(pipeline);
	}

	void krom_compile_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();

		kinc_g4_vertex_structure_t s0, s1, s2, s3;
		kinc_g4_vertex_structure_init(&s0);
		kinc_g4_vertex_structure_init(&s1);
		kinc_g4_vertex_structure_init(&s2);
		kinc_g4_vertex_structure_init(&s3);
		kinc_g4_vertex_structure_t* structures[4] = { &s0, &s1, &s2, &s3 };

		int32_t size = args[5]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i1 = 0; i1 < size; ++i1) {
			Local<Object> jsstructure = args[i1 + 1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			structures[i1]->instanced = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "instanced").ToLocalChecked()).ToLocalChecked()->ToBoolean(isolate)->Value();
			Local<Object> elements = jsstructure->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			int32_t length = elements->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
			for (int32_t i2 = 0; i2 < length; ++i2) {
				Local<Object> element = elements->Get(isolate->GetCurrentContext(), i2).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				Local<Value> str = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
				String::Utf8Value utf8_value(isolate, str);
				int32_t data = element->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "data").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

				strcpy(temp_string_vstruct[i1][i2], *utf8_value);
				kinc_g4_vertex_structure_add(structures[i1], temp_string_vstruct[i1][i2], convert_vertex_data(data));
			}
		}

		pipeobj->SetInternalField(1, External::New(isolate, structures));
		pipeobj->SetInternalField(2, External::New(isolate, &size));

		Local<External> vsfield = Local<External>::Cast(args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_shader_t* vertexShader = (kinc_g4_shader_t*)vsfield->Value();
		pipeobj->SetInternalField(3, External::New(isolate, vertexShader));
		pipeobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "vsname").ToLocalChecked(), args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());

		Local<External> fsfield = Local<External>::Cast(args[7]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_shader_t* fragmentShader = (kinc_g4_shader_t*)fsfield->Value();
		pipeobj->SetInternalField(4, External::New(isolate, fragmentShader));
		pipeobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "fsname").ToLocalChecked(), args[7]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());

		pipeline->vertex_shader = vertexShader;
		pipeline->fragment_shader = fragmentShader;

		if (!args[8]->IsNull() && !args[8]->IsUndefined()) {
			Local<External> gsfield = Local<External>::Cast(args[8]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t* geometryShader = (kinc_g4_shader_t*)gsfield->Value();
			pipeobj->SetInternalField(5, External::New(isolate, geometryShader));
			pipeobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "gsname").ToLocalChecked(), args[8]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
			pipeline->geometry_shader = geometryShader;
		}

		if (!args[9]->IsNull() && !args[9]->IsUndefined()) {
			Local<External> tcsfield = Local<External>::Cast(args[9]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t* tessellationControlShader = (kinc_g4_shader_t*)tcsfield->Value();
			pipeobj->SetInternalField(6, External::New(isolate, tessellationControlShader));
			pipeobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "tcsname").ToLocalChecked(), args[9]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
			pipeline->tessellation_control_shader = tessellationControlShader;
		}

		if (!args[10]->IsNull() && !args[10]->IsUndefined()) {
			Local<External> tesfield = Local<External>::Cast(args[10]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_shader_t* tessellationEvaluationShader = (kinc_g4_shader_t*)tesfield->Value();
			pipeobj->SetInternalField(7, External::New(isolate, tessellationEvaluationShader));
			pipeobj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "tesname").ToLocalChecked(), args[10]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
			pipeline->tessellation_evaluation_shader = tessellationEvaluationShader;
		}

		for (int i = 0; i < size; ++i) {
			pipeline->input_layout[i] = structures[i];
		}
		pipeline->input_layout[size] = nullptr;

		pipeline->cull_mode = (kinc_g4_cull_mode_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "cullMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->depth_write = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depthWrite").ToLocalChecked()).ToLocalChecked()->BooleanValue(isolate);
		pipeline->depth_mode = (kinc_g4_compare_mode_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depthMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->stencil_mode = (kinc_g4_compare_mode_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilMode").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencil_both_pass = (kinc_g4_stencil_action_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilBothPass").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencil_depth_fail = (kinc_g4_stencil_action_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilDepthFail").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencil_fail = (kinc_g4_stencil_action_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilFail").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencil_reference_value = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilReferenceValue").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencil_read_mask = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilReadMask").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->stencil_write_mask = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "stencilWriteMask").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		pipeline->blend_source = (kinc_g4_blending_operation_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->blend_destination = (kinc_g4_blending_operation_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "blendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alpha_blend_source = (kinc_g4_blending_operation_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendSource").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		pipeline->alpha_blend_destination = (kinc_g4_blending_operation_t)args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "alphaBlendDestination").ToLocalChecked()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();

		Local<Object> maskRedArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskRed").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskGreenArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskGreen").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskBlueArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskBlue").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Object> maskAlphaArray = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "colorWriteMaskAlpha").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		for (int i = 0; i < 8; ++i) {
			pipeline->color_write_mask_red[i] = maskRedArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->color_write_mask_green[i] = maskGreenArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->color_write_mask_blue[i] = maskBlueArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
			pipeline->color_write_mask_alpha[i] = maskAlphaArray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->BooleanValue(isolate);
		}

		pipeline->conservative_rasterization = args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "conservativeRasterization").ToLocalChecked()).ToLocalChecked()->BooleanValue(isolate);

		kinc_g4_pipeline_compile(pipeline);
	}

	void krom_set_pipeline(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> pipeobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> pipefield = Local<External>::Cast(pipeobj->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();
		kinc_g4_set_pipeline(pipeline);
	}

	bool ends_with(const char* str, const char* suffix) {
		if (!str || !suffix) return 0;
		size_t lenstr = strlen(str);
		size_t lensuffix = strlen(suffix);
		if (lensuffix > lenstr) return 0;
		return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
	}

	void load_image(kinc_file_reader_t& reader, const char* filename, unsigned char*& output, int& width, int& height, kinc_image_format_t& format) {
		format = KINC_IMAGE_FORMAT_RGBA32;
		int size = (int)kinc_file_reader_size(&reader);
		int comp;
		unsigned char* data = (unsigned char*)malloc(size);
		kinc_file_reader_read(&reader, data, size);
		if (ends_with(filename, "png")) {
			output = stbi_load_from_memory(data, size, &width, &height, &comp, 4);
			if (output == nullptr) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			}
			for (int y = 0; y < height; ++y) {
				int row = y * width * 4;
				for (int x = 0; x < width; ++x) {
					int col = x * 4;
					unsigned char r = output[row + col    ];
					unsigned char g = output[row + col + 1];
					unsigned char b = output[row + col + 2];
					float         a = output[row + col + 3] / 255.0f;
					b = (unsigned char)(b * a);
					g = (unsigned char)(g * a);
					r = (unsigned char)(r * a);
					output[row + col    ] = r;
					output[row + col + 1] = g;
					output[row + col + 2] = b;
				}
			}
		}
		else if (ends_with(filename, "hdr")) {
			output = (unsigned char*)stbi_loadf_from_memory(data, size, &width, &height, &comp, 4);
			if (output == nullptr) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			}
			format = KINC_IMAGE_FORMAT_RGBA128;
		}
		else {
			output = stbi_load_from_memory(data, size, &width, &height, &comp, 4);
			if (output == nullptr) {
				kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			}
		}
		free(data);
	}

	void krom_load_image(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);
		bool readable = args[1]->ToBoolean(isolate)->Value();

		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t* image = (kinc_image_t*)malloc(sizeof(kinc_image_t));

		// TODO: make kinc_image load faster
		// size_t byte_size = kinc_image_size_from_file(*utf8_value);
		// void* memory = malloc(byte_size);
		// kinc_image_init_from_file(image, memory, *utf8_value);

		kinc_file_reader_t reader;
		kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET);
		unsigned char* image_data;
		int image_width;
		int image_height;
		kinc_image_format_t image_format;
		load_image(reader, *utf8_value, image_data, image_width, image_height, image_format);
		kinc_image_init(image, image_data, image_width, image_height, image_format);

		kinc_g4_texture_init_from_image(texture, image);
		if (!readable) {
			delete[] image->data;
			kinc_image_destroy(image);
			free(image);
			// free(memory);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "filename").ToLocalChecked(), args[0]);
		args.GetReturnValue().Set(obj);
	}

	void krom_unload_image(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNull() || args[0]->IsUndefined()) return;
		Local<Object> image = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<Value> tex = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked();
		Local<Value> rt = image->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked();

		if (tex->IsObject()) {
			Local<External> texfield = Local<External>::Cast(tex->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
			kinc_g4_texture_destroy(texture);
			free(texture);
		}
		else if (rt->IsObject()) {
			Local<External> rtfield = Local<External>::Cast(rt->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
			kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
			kinc_g4_render_target_destroy(render_target);
			free(render_target);
		}
	}

	void krom_load_sound(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		kinc_a1_sound_t* sound = kinc_a1_sound_create(*utf8_value);
		Local<ArrayBuffer> buffer;
		ArrayBuffer::Contents content;
		buffer = ArrayBuffer::New(isolate, sound->size * 2 * sizeof(float));
		content = buffer->GetContents();
		float* to = (float*)content.Data();

		int16_t* left = (int16_t*)&sound->left[0];
		int16_t* right = (int16_t*)&sound->right[0];
		for (int i = 0; i < sound->size; i += 1) {
			to[i * 2 + 0] = (float)(left [i] / 32767.0);
			to[i * 2 + 1] = (float)(right[i] / 32767.0);
		}

		args.GetReturnValue().Set(buffer);
		kinc_a1_sound_destroy(sound);
	}

	void krom_write_audio_buffer(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		int samples = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		for (int i = 0; i < samples; ++i) {
			float* values = (float*)content.Data();
			float value = values[audio_read_location / 4];
			audio_read_location += 4;
			if (audio_read_location >= content.ByteLength()) audio_read_location = 0;

			*(float*)&audio_buffer.data[audio_buffer.write_location] = value;
			audio_buffer.write_location += 4;
			if (audio_buffer.write_location >= audio_buffer.data_size) audio_buffer.write_location = 0;
		}
	}

	void krom_load_blob(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_value(isolate, args[0]);

		kinc_file_reader_t reader;
		if (!kinc_file_reader_open(&reader, *utf8_value, KINC_FILE_TYPE_ASSET)) return;
		int reader_size = (int)kinc_file_reader_size(&reader);

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader_size);
		ArrayBuffer::Contents contents = buffer->GetContents();
		kinc_file_reader_read(&reader, contents.Data(), reader_size);
		kinc_file_reader_close(&reader);

		args.GetReturnValue().Set(buffer);
	}

	void krom_get_constant_location(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> pipefield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(pipeline, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_g4_constant_location_t* location_copy = (kinc_g4_constant_location_t*)malloc(sizeof(kinc_g4_constant_location_t));
		memcpy(location_copy, &location, sizeof(kinc_g4_constant_location_t)); // TODO
		obj->SetInternalField(0, External::New(isolate, location_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> pipefield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_pipeline_t* pipeline = (kinc_g4_pipeline_t*)pipefield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_g4_texture_unit_t unit = kinc_g4_pipeline_get_texture_unit(pipeline, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_g4_texture_unit_t* unit_copy = (kinc_g4_texture_unit_t*)malloc(sizeof(kinc_g4_texture_unit_t));
		memcpy(unit_copy, &unit, sizeof(kinc_g4_texture_unit_t)); // TODO
		obj->SetInternalField(0, External::New(isolate, unit_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_set_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		kinc_g4_set_texture(*unit, texture);
	}

	void krom_set_render_target(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_g4_render_target_use_color_as_texture(render_target, *unit);
	}

	void krom_set_texture_depth(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_g4_render_target_use_depth_as_texture(render_target, *unit);
	}

	void krom_set_image_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		kinc_g4_set_image_texture(*unit, texture);
	}

	void krom_set_texture_parameters(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture3d_parameters(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_g4_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture_compare_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_texture_compare_mode(*unit, args[1]->ToBoolean(isolate)->Value());
	}

	void krom_set_cube_map_compare_mode(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_unit_t* unit = (kinc_g4_texture_unit_t*)unitfield->Value();
		kinc_g4_set_cubemap_compare_mode(*unit, args[1]->ToBoolean(isolate)->Value());
	}

	void krom_set_bool(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_bool(*location, value != 0);
	}

	void krom_set_int(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_int(*location, value);
	}

	void krom_set_float(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float(*location, value);
	}

	void krom_set_float2(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float2(*location, value1, value2);
	}

	void krom_set_float3(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float3(*location, value1, value2, value3);
	}

	void krom_set_float4(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_set_float4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		float* from = (float*)content.Data();
		kinc_g4_set_floats(*location, from, int(content.ByteLength() / 4));
	}

	void krom_set_matrix(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_g4_set_matrix4(*location, (kinc_matrix4x4_t*)from);
	}

	void krom_set_matrix3(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_constant_location_t* location = (kinc_g4_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_g4_set_matrix3(*location, (kinc_matrix3x3_t*)from);
	}

	void krom_get_time(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Number::New(isolate, kinc_time()));
	}

	void krom_window_width(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_width(windowId)));
	}

	void krom_window_height(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_window_height(windowId)));
	}

	void krom_set_window_title(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		String::Utf8Value title(isolate, args[1]);
		kinc_window_set_title(windowId, *title);
	}

	void krom_screen_dpi(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int ppi = kinc_display_current_mode(kinc_primary_display()).pixels_per_inch;
		args.GetReturnValue().Set(Int32::New(isolate, ppi));
	}

	void krom_system_id(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_system_id()).ToLocalChecked());
	}

	void krom_request_shutdown(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_stop();
	}

	void krom_display_count(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, kinc_count_displays()));
	}

	void krom_display_width(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).width));
	}

	void krom_display_height(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).height));
	}

	void krom_display_x(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).x));
	}

	void krom_display_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Int32::New(isolate, kinc_display_current_mode(index).y));
	}

	void krom_display_is_primary(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(Boolean::New(isolate, index == kinc_primary_display()));
	}

	void krom_write_storage(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		kinc_file_writer_t writer;
		kinc_file_writer_open(&writer, *utf8_name);
		kinc_file_writer_write(&writer, content.Data(), (int)content.ByteLength());
		kinc_file_writer_close(&writer);
	}

	void krom_read_storage(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_name(isolate, args[0]);

		kinc_file_reader_t reader;
		if (!kinc_file_reader_open(&reader, *utf8_name, KINC_FILE_TYPE_SAVE)) return;
		int reader_size = (int)kinc_file_reader_size(&reader);

		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, reader_size);
		ArrayBuffer::Contents contents = buffer->GetContents();
		kinc_file_reader_read(&reader, contents.Data(), reader_size);
		kinc_file_reader_close(&reader);

		args.GetReturnValue().Set(buffer);
	}

	void krom_create_render_target(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)malloc(sizeof(kinc_g4_render_target_t));
		kinc_g4_render_target_init(render_target, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false, (kinc_g4_render_target_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), 0);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, render_target));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, render_target->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, render_target->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_render_target_cube_map(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)malloc(sizeof(kinc_g4_render_target_t));
		kinc_g4_render_target_init_cube(render_target, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), false, (kinc_g4_render_target_format_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), 0);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, render_target));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, render_target->width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, render_target->height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init(texture, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture3d(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_g4_texture_init3d(texture, args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->tex_depth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		// content = buffer->GetContents();
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t* image = (kinc_image_t*)malloc(sizeof(kinc_image_t));
		kinc_image_init(image, content.Data(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
		kinc_g4_texture_init_from_image(texture, image);
		bool readable = args[4]->ToBoolean(isolate)->Value();
		if (!readable) {
			delete[] content.Data();
			kinc_image_destroy(image);
			free(image);
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(readable ? 2 : 1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		if (readable) obj->SetInternalField(1, External::New(isolate, image));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_bytes3d(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		// content = buffer->GetContents();
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)malloc(sizeof(kinc_g4_texture_t));
		kinc_image_t image;
		kinc_image_init3d(&image, content.Data(), args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value(), (kinc_image_format_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
		kinc_g4_texture_init_from_image3d(texture, &image);
		kinc_image_destroy(&image);
		bool readable = args[5]->ToBoolean(isolate)->Value();
		if (!readable) {
			delete[] content.Data();
		}

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);
		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, texture));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "width").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "height").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "depth").ToLocalChecked(), Int32::New(isolate, texture->tex_depth));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realWidth").ToLocalChecked(), Int32::New(isolate, texture->tex_width));
		obj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "realHeight").ToLocalChecked(), Int32::New(isolate, texture->tex_height));
		args.GetReturnValue().Set(obj);
	}

	void krom_create_texture_from_encoded_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		if (buffer->IsExternal()) content = buffer->GetContents();
		else content = buffer->Externalize();
		// content = buffer->GetContents();
		String::Utf8Value format(isolate, args[1]);
		// TODO
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

	void krom_get_texture_pixels(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(1));
		kinc_image_t* image = (kinc_image_t*)field->Value();

		uint8_t* data = kinc_image_get_pixels(image);
		int byteLength = format_byte_size(image->format) * image->width * image->height * image->depth;
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, data, byteLength);
		args.GetReturnValue().Set(buffer);
	}

	void krom_get_render_target_pixels(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* rt = (kinc_g4_render_target_t*)field->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		uint8_t* b = (uint8_t*)content.Data();
		kinc_g4_render_target_get_pixels(rt, b);
	}

	void krom_lock_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		uint8_t* tex = kinc_g4_texture_lock(texture);

		int byteLength = format_byte_size(texture->format) * texture->tex_width * texture->tex_height * texture->tex_depth;
		Local<ArrayBuffer> abuffer = ArrayBuffer::New(isolate, tex, byteLength);
		args.GetReturnValue().Set(abuffer);
	}

	void krom_unlock_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		kinc_g4_texture_unlock(texture);
	}

	void krom_clear_texture(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		int x = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int width = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int height = args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int depth = args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int color = args[7]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_g4_texture_clear(texture, x, y, z, width, height, depth, color);
	}

	void krom_generate_texture_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();
		kinc_g4_texture_generate_mipmaps(texture, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_generate_render_target_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* rt = (kinc_g4_render_target_t*)field->Value();
		kinc_g4_render_target_generate_mipmaps(rt, args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value());
	}

	void krom_set_mipmaps(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> field = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)field->Value();

		Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		int32_t length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		for (int32_t i = 0; i < length; ++i) {
			Local<Object> mipmapobj = jsarray->Get(isolate->GetCurrentContext(), i).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "texture_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> mipmapfield = Local<External>::Cast(mipmapobj->GetInternalField(1));
			kinc_image_t* mipmap = (kinc_image_t*)mipmapfield->Value();
			kinc_g4_texture_set_mipmap(texture, mipmap, i + 1);
		}
	}

	void krom_set_depth_stencil_from(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> targetfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)targetfield->Value();
		Local<External> sourcefield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* source_target = (kinc_g4_render_target_t*)sourcefield->Value();
		kinc_g4_render_target_set_depth_stencil_from(render_target, source_target);
	}

	void krom_viewport(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_viewport(x, y, w, h);
	}

	void krom_scissor(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int w = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		int h = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_scissor(x, y, w, h);
	}

	void krom_disable_scissor(const FunctionCallbackInfo<Value>& args) {
		kinc_g4_disable_scissor();
	}

	void krom_render_targets_inverted_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Boolean::New(isolate, kinc_g4_render_targets_inverted_y()));
	}

	void krom_begin(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		if (args[0]->IsNull() || args[0]->IsUndefined()) {
			kinc_g4_restore_render_target();
		}
		else {
			Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
			Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
			kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();

			int32_t length = 1;
			kinc_g4_render_target_t* render_targets[8] = { render_target, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
			if (!args[1]->IsNull() && !args[1]->IsUndefined()) {
				Local<Object> jsarray = args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
				length = jsarray->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked()->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value() + 1;
				if (length > 8) length = 8;
				for (int32_t i = 1; i < length; ++i) {
					Local<Object> artobj = jsarray->Get(isolate->GetCurrentContext(), i - 1).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
					Local<External> artfield = Local<External>::Cast(artobj->GetInternalField(0));
					kinc_g4_render_target_t* art = (kinc_g4_render_target_t*)artfield->Value();
					render_targets[i] = art;
				}
			}
			kinc_g4_set_render_targets(render_targets, length);
		}
	}

	void krom_begin_face(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> obj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "renderTarget_").ToLocalChecked()).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> rtfield = Local<External>::Cast(obj->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		int face = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_g4_set_render_target_face(render_target, face);
	}

	void krom_end(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
	}

	void krom_file_save_bytes(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_path(isolate, args[0]);

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();

		FILE* file = fopen(*utf8_path, "wb");
		if (file == nullptr) return;
		fwrite(content.Data(), 1, (int)content.ByteLength(), file);
		fclose(file);
	}

	void krom_sys_command(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value utf8_cmd(isolate, args[0]);
		int result = system(*utf8_cmd);
		args.GetReturnValue().Set(Int32::New(isolate, result));
	}

	void krom_save_path(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_internal_save_path()).ToLocalChecked());
	}

	void krom_get_arg_count(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(Int32::New(isolate, _argc));
	}

	void krom_get_arg(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int index = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, _argv[index]).ToLocalChecked());
	}

	void krom_get_files_location(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, kinc_internal_get_files_location()).ToLocalChecked());
	}

	void krom_set_bool_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_bool(*location, value != 0);
	}

	void krom_set_int_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		int32_t value = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_int(*location, value);
	}

	void krom_set_float_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float(*location, value);
	}

	void krom_set_float2_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float2(*location, value1, value2);
	}

	void krom_set_float3_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float3(*location, value1, value2, value3);
	}

	void krom_set_float4_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		float value1 = (float)args[1]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value2 = (float)args[2]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value3 = (float)args[3]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		float value4 = (float)args[4]->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute_set_float4(*location, value1, value2, value3, value4);
	}

	void krom_set_floats_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();

		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_compute_set_floats(*location, from, int(content.ByteLength() / 4));
	}

	void krom_set_matrix_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_compute_set_matrix4(*location, (kinc_matrix4x4_t*)from);
	}

	void krom_set_matrix3_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> locationfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_constant_location_t* location = (kinc_compute_constant_location_t*)locationfield->Value();
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		float* from = (float*)content.Data();
		kinc_compute_set_matrix3(*location, (kinc_matrix3x3_t*)from);
	}

	void krom_set_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_compute_set_texture(*unit, texture, (kinc_compute_access_t)access);
	}

	void krom_set_render_target_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		int access = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
		kinc_compute_set_render_target(*unit, render_target, (kinc_compute_access_t)access);
	}

	void krom_set_sampled_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> texfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texture = (kinc_g4_texture_t*)texfield->Value();
		kinc_compute_set_sampled_texture(*unit, texture);
	}

	void krom_set_sampled_render_target_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_compute_set_sampled_render_target(*unit, render_target);
	}

	void krom_set_sampled_depth_texture_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		Local<External> rtfield = Local<External>::Cast(args[1]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* render_target = (kinc_g4_render_target_t*)rtfield->Value();
		kinc_compute_set_sampled_depth_from_render_target(*unit, render_target);
	}

	void krom_set_texture_parameters_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		kinc_compute_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_minification_filter(*unit, (kinc_g4_texture_filter_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_texture3d_parameters_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> unitfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_texture_unit_t* unit = (kinc_compute_texture_unit_t*)unitfield->Value();
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_U, (kinc_g4_texture_addressing_t)args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_V, (kinc_g4_texture_addressing_t)args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_addressing(*unit, KINC_G4_TEXTURE_DIRECTION_W, (kinc_g4_texture_addressing_t)args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_minification_filter(*unit, (kinc_g4_texture_filter_t)args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_magnification_filter(*unit, (kinc_g4_texture_filter_t)args[5]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
		kinc_compute_set_texture3d_mipmap_filter(*unit, (kinc_g4_mipmap_filter_t)args[6]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust());
	}

	void krom_set_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();
		kinc_compute_set_shader(shader);
	}

	void krom_create_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents content;
		// if (buffer->IsExternal()) content = buffer->GetContents();
		// else content = buffer->Externalize();
		content = buffer->GetContents();
		kinc_compute_shader* shader = (kinc_compute_shader*)malloc(sizeof(kinc_compute_shader));
		kinc_compute_shader_init(shader, content.Data(), (int)content.ByteLength());

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		obj->SetInternalField(0, External::New(isolate, shader));
		args.GetReturnValue().Set(obj);
	}

	void krom_delete_shader_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Object> shaderobj = args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		Local<External> shaderfield = Local<External>::Cast(shaderobj->GetInternalField(0));
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();
		kinc_compute_shader_destroy(shader);
		free(shader);
	}

	void krom_get_constant_location_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_compute_constant_location_t location = kinc_compute_shader_get_constant_location(shader, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_compute_constant_location_t* location_copy = (kinc_compute_constant_location_t*)malloc(sizeof(kinc_compute_constant_location_t)); // TODO
		memcpy(location_copy, &location, sizeof(kinc_compute_constant_location_t));
		obj->SetInternalField(0, External::New(isolate, location_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_get_texture_unit_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<External> shaderfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_compute_shader* shader = (kinc_compute_shader*)shaderfield->Value();

		String::Utf8Value utf8_value(isolate, args[1]);
		kinc_compute_texture_unit_t unit = kinc_compute_shader_get_texture_unit(shader, *utf8_value);

		Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
		templ->SetInternalFieldCount(1);

		Local<Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
		kinc_compute_texture_unit_t* unit_copy = (kinc_compute_texture_unit_t*)malloc(sizeof(kinc_compute_texture_unit_t)); // TODO
		memcpy(unit_copy, &unit, sizeof(kinc_compute_texture_unit_t));
		obj->SetInternalField(0, External::New(isolate, unit_copy));
		args.GetReturnValue().Set(obj);
	}

	void krom_compute(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int x = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int y = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int z = args[2]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		kinc_compute(x, y, z);
	}

	void krom_set_save_and_quit_callback(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<Value> arg = args[0];
		Local<Function> func = Local<Function>::Cast(arg);
		save_and_quit_func.Reset(isolate, func);
	}

	void krom_set_mouse_cursor(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int i = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		#ifdef KORE_WINDOWS
		SetCursor(LoadCursor(NULL,
			i == 1 ? IDC_HAND     :
			i == 2 ? IDC_CROSS    :
			i == 3 ? IDC_IBEAM    :
			i == 4 ? IDC_WAIT     :
			i == 5 ? IDC_SIZENS   :
			i == 6 ? IDC_SIZEWE   :
			i == 7 ? IDC_SIZENWSE :
			i == 8 ? IDC_SIZENESW :
					 IDC_ARROW
		));
		#endif
	}

	#ifdef WITH_NFD
	void krom_open_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value filterList(isolate, args[0]);
		String::Utf8Value defaultPath(isolate, args[1]);
		nfdchar_t *outPath = NULL;
		nfdresult_t result = NFD_OpenDialog(*filterList, *defaultPath, &outPath);
		if (result == NFD_OKAY) {
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, outPath).ToLocalChecked());
			free(outPath);
		}
		else if (result == NFD_CANCEL) {}
		else {
			kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
		}
	}

	void krom_save_dialog(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		String::Utf8Value filterList(isolate, args[0]);
		String::Utf8Value defaultPath(isolate, args[1]);
		nfdchar_t *outPath = NULL;
		nfdresult_t result = NFD_SaveDialog(*filterList, *defaultPath, &outPath);
		if (result == NFD_OKAY) {
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, outPath).ToLocalChecked());
			free(outPath);
		}
		else if (result == NFD_CANCEL) {}
		else {
			kinc_log(KINC_LOG_LEVEL_INFO, "Error: %s\n", NFD_GetError());
		}
	}
	#endif

	#ifdef KORE_DIRECT3D12
	void krom_raytrace_init(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		Local<ArrayBuffer> shader_buffer = Local<ArrayBuffer>::Cast(args[0]);
		ArrayBuffer::Contents shader_content;
		// if (shader_buffer->IsExternal()) shader_content = shader_buffer->GetContents();
		// else shader_content = shader_buffer->Externalize();
		shader_content = shader_buffer->GetContents();

		Local<ArrayBuffer> vb_buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents vb_content;
		// if (vb_buffer->IsExternal()) vb_content = vb_buffer->GetContents();
		// else vb_content = vb_buffer->Externalize();
		vb_content = vb_buffer->GetContents();
		float* vb = (float*)vb_content.Data();

		Local<ArrayBuffer> ib_buffer = Local<ArrayBuffer>::Cast(args[2]);
		ArrayBuffer::Contents ib_content;
		// if (ib_buffer->IsExternal()) ib_content = ib_buffer->GetContents();
		// else ib_content = ib_buffer->Externalize();
		ib_content = ib_buffer->GetContents();
		int* ib = (int*)ib_content.Data();

		kinc_g5_constant_buffer_init(&constant_buffer, constant_buffer_size * 4);

		kinc_raytrace_pipeline_init(&pipeline, &commandList, shader_content.Data(), (int)shader_content.ByteLength(), &constant_buffer);

		kinc_g5_vertex_structure_t structure;
		kinc_g4_vertex_structure_init(&structure);
		kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_FLOAT3);
		kinc_g4_vertex_structure_add(&structure, "nor", KINC_G4_VERTEX_DATA_FLOAT3);
		kinc_g4_vertex_structure_add(&structure, "tex", KINC_G4_VERTEX_DATA_FLOAT2);

		int vcount = (int)vb_content.ByteLength() / 4;
		kinc_g5_vertex_buffer_init(&vertex_buffer, vcount / 8, &structure, true, 0);
		float *vertices = kinc_g5_vertex_buffer_lock_all(&vertex_buffer);
		for (int i = 0; i < vcount; ++i) {
			vertices[i] = vb[i];
		}
		kinc_g5_vertex_buffer_unlock_all(&vertex_buffer);

		int icount = (int)ib_content.ByteLength() / 4;
		kinc_g5_index_buffer_init(&index_buffer, icount, true);
		int *indices = kinc_g5_index_buffer_lock(&index_buffer);
		for (int i = 0; i < icount; ++i) {
			indices[i] = ib[i];
		}
		kinc_g5_index_buffer_unlock(&index_buffer);

		kinc_raytrace_acceleration_structure_init(&accel, &commandList, &vertex_buffer, &index_buffer);

		int32_t target_w = args[3]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int32_t target_h = args[4]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();

		Local<External> rtfield0 = Local<External>::Cast(args[5]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texpaint0 = (kinc_g4_render_target_t*)rtfield0->Value();

		Local<External> rtfield1 = Local<External>::Cast(args[6]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texpaint1 = (kinc_g4_render_target_t*)rtfield1->Value();

		Local<External> rtfield2 = Local<External>::Cast(args[7]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texpaint2 = (kinc_g4_render_target_t*)rtfield2->Value();

		Local<External> envfield = Local<External>::Cast(args[8]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_texture_t* texenv = (kinc_g4_texture_t*)envfield->Value();

		Local<External> sobolfield = Local<External>::Cast(args[9]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texsobol = (kinc_g4_render_target_t*)sobolfield->Value();

		Local<External> scramblefield = Local<External>::Cast(args[10]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texscramble = (kinc_g4_render_target_t*)scramblefield->Value();

		Local<External> rankfield = Local<External>::Cast(args[11]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		kinc_g4_render_target_t* texrank = (kinc_g4_render_target_t*)rankfield->Value();

		kinc_raytrace_target_init(&target, target_w, target_h, &texpaint0->impl._renderTarget, &texpaint1->impl._renderTarget, &texpaint2->impl._renderTarget, &texenv->impl._texture, &texsobol->impl._renderTarget, &texscramble->impl._renderTarget, &texrank->impl._renderTarget);
	}

	void krom_raytrace_dispatch_rays(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());

		Local<External> rtfield = Local<External>::Cast(args[0]->ToObject(isolate->GetCurrentContext()).ToLocalChecked()->GetInternalField(0));
		render_target = (kinc_g4_render_target_t*)rtfield->Value();

		Local<ArrayBuffer> cb_buffer = Local<ArrayBuffer>::Cast(args[1]);
		ArrayBuffer::Contents cb_content;
		// if (cb_buffer->IsExternal()) cb_content = cb_buffer->GetContents();
		// else cb_content = cb_buffer->Externalize();
		cb_content = cb_buffer->GetContents();
		float* cb = (float*)cb_content.Data();

		kinc_g5_constant_buffer_lock_all(&constant_buffer);
		for (int i = 0; i < constant_buffer_size; ++i) {
			kinc_g5_constant_buffer_set_float(&constant_buffer, i * 4, cb[i]);
		}
		kinc_g5_constant_buffer_unlock(&constant_buffer);

		kinc_raytrace_set_acceleration_structure(&accel);
		kinc_raytrace_set_pipeline(&pipeline);
		kinc_raytrace_set_target(&target);
		kinc_raytrace_dispatch_rays(&commandList);
		kinc_raytrace_copy_target(&commandList, &render_target->impl._renderTarget, &target);
	}
	#endif

	void krom_window_x(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		// args.GetReturnValue().Set(Int32::New(isolate, kinc_window_x(windowId))); // Returns window creation pos
		#if KORE_WINDOWS
		RECT rect;
		GetWindowRect(kinc_windows_window_handle(windowId), &rect);
		args.GetReturnValue().Set(Int32::New(isolate, rect.left));
		#endif
	}

	void krom_window_y(const FunctionCallbackInfo<Value>& args) {
		HandleScope scope(args.GetIsolate());
		int windowId = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		// args.GetReturnValue().Set(Int32::New(isolate, kinc_window_y(windowId)));
		#if KORE_WINDOWS
		RECT rect;
		GetWindowRect(kinc_windows_window_handle(windowId), &rect);
		args.GetReturnValue().Set(Int32::New(isolate, rect.top));
		#endif
	}

	void start_v8(const char* bindir) {
		plat = platform::NewDefaultPlatform();
		V8::InitializePlatform(plat.get());
		V8::Initialize();

		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		isolate = Isolate::New(create_params);

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);

		Local<ObjectTemplate> krom = ObjectTemplate::New(isolate);
		krom->Set(String::NewFromUtf8(isolate, "init").ToLocalChecked(), FunctionTemplate::New(isolate, krom_init));
		krom->Set(String::NewFromUtf8(isolate, "log").ToLocalChecked(), FunctionTemplate::New(isolate, krom_log));
		krom->Set(String::NewFromUtf8(isolate, "clear").ToLocalChecked(), FunctionTemplate::New(isolate, krom_clear));
		krom->Set(String::NewFromUtf8(isolate, "setCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_callback));
		krom->Set(String::NewFromUtf8(isolate, "setDropFilesCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_drop_files_callback));
		krom->Set(String::NewFromUtf8(isolate, "setCutCopyPasteCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_cut_copy_paste_callback));
		krom->Set(String::NewFromUtf8(isolate, "setApplicationStateCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_application_state_callback));
		krom->Set(String::NewFromUtf8(isolate, "setKeyboardDownCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_keyboard_down_callback));
		krom->Set(String::NewFromUtf8(isolate, "setKeyboardUpCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_keyboard_up_callback));
		krom->Set(String::NewFromUtf8(isolate, "setKeyboardPressCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_keyboard_press_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseDownCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_down_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseUpCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_up_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseMoveCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_move_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseWheelCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_wheel_callback));
		krom->Set(String::NewFromUtf8(isolate, "setPenDownCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pen_down_callback));
		krom->Set(String::NewFromUtf8(isolate, "setPenUpCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pen_up_callback));
		krom->Set(String::NewFromUtf8(isolate, "setPenMoveCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pen_move_callback));
		krom->Set(String::NewFromUtf8(isolate, "setGamepadAxisCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_gamepad_axis_callback));
		krom->Set(String::NewFromUtf8(isolate, "setGamepadButtonCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_gamepad_button_callback));
		krom->Set(String::NewFromUtf8(isolate, "lockMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_mouse));
		krom->Set(String::NewFromUtf8(isolate, "unlockMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_mouse));
		krom->Set(String::NewFromUtf8(isolate, "canLockMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_can_lock_mouse));
		krom->Set(String::NewFromUtf8(isolate, "isMouseLocked").ToLocalChecked(), FunctionTemplate::New(isolate, krom_is_mouse_locked));
		krom->Set(String::NewFromUtf8(isolate, "showMouse").ToLocalChecked(), FunctionTemplate::New(isolate, krom_show_mouse));
		krom->Set(String::NewFromUtf8(isolate, "createIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "deleteIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "lockIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "unlockIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "setIndexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_indexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "createVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_vertexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "deleteVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_vertexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "lockVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_vertex_buffer));
		krom->Set(String::NewFromUtf8(isolate, "unlockVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_vertex_buffer));
		krom->Set(String::NewFromUtf8(isolate, "setVertexBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_vertexbuffer));
		krom->Set(String::NewFromUtf8(isolate, "setVertexBuffers").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_vertexbuffers));
		krom->Set(String::NewFromUtf8(isolate, "drawIndexedVertices").ToLocalChecked(), FunctionTemplate::New(isolate, krom_draw_indexed_vertices));
		krom->Set(String::NewFromUtf8(isolate, "drawIndexedVerticesInstanced").ToLocalChecked(), FunctionTemplate::New(isolate, krom_draw_indexed_vertices_instanced));
		krom->Set(String::NewFromUtf8(isolate, "createVertexShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_vertex_shader));
		krom->Set(String::NewFromUtf8(isolate, "createVertexShaderFromSource").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_vertex_shader_from_source));
		krom->Set(String::NewFromUtf8(isolate, "createFragmentShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_fragment_shader));
		krom->Set(String::NewFromUtf8(isolate, "createFragmentShaderFromSource").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_fragment_shader_from_source));
		krom->Set(String::NewFromUtf8(isolate, "createGeometryShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_geometry_shader));
		krom->Set(String::NewFromUtf8(isolate, "createTessellationControlShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_tessellation_control_shader));
		krom->Set(String::NewFromUtf8(isolate, "createTessellationEvaluationShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_tessellation_evaluation_shader));
		krom->Set(String::NewFromUtf8(isolate, "deleteShader").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_shader));
		krom->Set(String::NewFromUtf8(isolate, "createPipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "deletePipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "compilePipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_compile_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "setPipeline").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_pipeline));
		krom->Set(String::NewFromUtf8(isolate, "loadImage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_image));
		krom->Set(String::NewFromUtf8(isolate, "unloadImage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unload_image));
		krom->Set(String::NewFromUtf8(isolate, "loadSound").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_sound));
		krom->Set(String::NewFromUtf8(isolate, "setAudioCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_audio_callback));
		krom->Set(String::NewFromUtf8(isolate, "audioThread").ToLocalChecked(), FunctionTemplate::New(isolate, krom_audio_thread));
		krom->Set(String::NewFromUtf8(isolate, "writeAudioBuffer").ToLocalChecked(), FunctionTemplate::New(isolate, krom_write_audio_buffer));
		krom->Set(String::NewFromUtf8(isolate, "loadBlob").ToLocalChecked(), FunctionTemplate::New(isolate, krom_load_blob));
		krom->Set(String::NewFromUtf8(isolate, "getConstantLocation").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_constant_location));
		krom->Set(String::NewFromUtf8(isolate, "getTextureUnit").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_unit));
		krom->Set(String::NewFromUtf8(isolate, "setTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture));
		krom->Set(String::NewFromUtf8(isolate, "setRenderTarget").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_render_target));
		krom->Set(String::NewFromUtf8(isolate, "setTextureDepth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_depth));
		krom->Set(String::NewFromUtf8(isolate, "setImageTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_image_texture));
		krom->Set(String::NewFromUtf8(isolate, "setTextureParameters").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_parameters));
		krom->Set(String::NewFromUtf8(isolate, "setTexture3DParameters").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture3d_parameters));
		krom->Set(String::NewFromUtf8(isolate, "setTextureCompareMode").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_compare_mode));
		krom->Set(String::NewFromUtf8(isolate, "setCubeMapCompareMode").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_cube_map_compare_mode));
		krom->Set(String::NewFromUtf8(isolate, "setBool").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_bool));
		krom->Set(String::NewFromUtf8(isolate, "setInt").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_int));
		krom->Set(String::NewFromUtf8(isolate, "setFloat").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float));
		krom->Set(String::NewFromUtf8(isolate, "setFloat2").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float2));
		krom->Set(String::NewFromUtf8(isolate, "setFloat3").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float3));
		krom->Set(String::NewFromUtf8(isolate, "setFloat4").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float4));
		krom->Set(String::NewFromUtf8(isolate, "setFloats").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_floats));
		krom->Set(String::NewFromUtf8(isolate, "setMatrix").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix));
		krom->Set(String::NewFromUtf8(isolate, "setMatrix3").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix3));
		krom->Set(String::NewFromUtf8(isolate, "getTime").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_time));
		krom->Set(String::NewFromUtf8(isolate, "windowWidth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_width));
		krom->Set(String::NewFromUtf8(isolate, "windowHeight").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_height));
		krom->Set(String::NewFromUtf8(isolate, "setWindowTitle").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_window_title));
		krom->Set(String::NewFromUtf8(isolate, "screenDpi").ToLocalChecked(), FunctionTemplate::New(isolate, krom_screen_dpi));
		krom->Set(String::NewFromUtf8(isolate, "systemId").ToLocalChecked(), FunctionTemplate::New(isolate, krom_system_id));
		krom->Set(String::NewFromUtf8(isolate, "requestShutdown").ToLocalChecked(), FunctionTemplate::New(isolate, krom_request_shutdown));
		krom->Set(String::NewFromUtf8(isolate, "displayCount").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_count));
		krom->Set(String::NewFromUtf8(isolate, "displayWidth").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_width));
		krom->Set(String::NewFromUtf8(isolate, "displayHeight").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_height));
		krom->Set(String::NewFromUtf8(isolate, "displayX").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_x));
		krom->Set(String::NewFromUtf8(isolate, "displayY").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_y));
		krom->Set(String::NewFromUtf8(isolate, "displayIsPrimary").ToLocalChecked(), FunctionTemplate::New(isolate, krom_display_is_primary));
		krom->Set(String::NewFromUtf8(isolate, "writeStorage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_write_storage));
		krom->Set(String::NewFromUtf8(isolate, "readStorage").ToLocalChecked(), FunctionTemplate::New(isolate, krom_read_storage));
		krom->Set(String::NewFromUtf8(isolate, "createRenderTarget").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_render_target));
		krom->Set(String::NewFromUtf8(isolate, "createRenderTargetCubeMap").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_render_target_cube_map));
		krom->Set(String::NewFromUtf8(isolate, "createTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture));
		krom->Set(String::NewFromUtf8(isolate, "createTexture3D").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture3d));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromBytes").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_bytes));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromBytes3D").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_bytes3d));
		krom->Set(String::NewFromUtf8(isolate, "createTextureFromEncodedBytes").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_texture_from_encoded_bytes));
		krom->Set(String::NewFromUtf8(isolate, "getTexturePixels").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_pixels));
		krom->Set(String::NewFromUtf8(isolate, "getRenderTargetPixels").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_render_target_pixels));
		krom->Set(String::NewFromUtf8(isolate, "lockTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_lock_texture));
		krom->Set(String::NewFromUtf8(isolate, "unlockTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_unlock_texture));
		krom->Set(String::NewFromUtf8(isolate, "clearTexture").ToLocalChecked(), FunctionTemplate::New(isolate, krom_clear_texture));
		krom->Set(String::NewFromUtf8(isolate, "generateTextureMipmaps").ToLocalChecked(), FunctionTemplate::New(isolate, krom_generate_texture_mipmaps));
		krom->Set(String::NewFromUtf8(isolate, "generateRenderTargetMipmaps").ToLocalChecked(), FunctionTemplate::New(isolate, krom_generate_render_target_mipmaps));
		krom->Set(String::NewFromUtf8(isolate, "setMipmaps").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mipmaps));
		krom->Set(String::NewFromUtf8(isolate, "setDepthStencilFrom").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_depth_stencil_from));
		krom->Set(String::NewFromUtf8(isolate, "viewport").ToLocalChecked(), FunctionTemplate::New(isolate, krom_viewport));
		krom->Set(String::NewFromUtf8(isolate, "scissor").ToLocalChecked(), FunctionTemplate::New(isolate, krom_scissor));
		krom->Set(String::NewFromUtf8(isolate, "disableScissor").ToLocalChecked(), FunctionTemplate::New(isolate, krom_disable_scissor));
		krom->Set(String::NewFromUtf8(isolate, "renderTargetsInvertedY").ToLocalChecked(), FunctionTemplate::New(isolate, krom_render_targets_inverted_y));
		krom->Set(String::NewFromUtf8(isolate, "begin").ToLocalChecked(), FunctionTemplate::New(isolate, krom_begin));
		krom->Set(String::NewFromUtf8(isolate, "beginFace").ToLocalChecked(), FunctionTemplate::New(isolate, krom_begin_face));
		krom->Set(String::NewFromUtf8(isolate, "end").ToLocalChecked(), FunctionTemplate::New(isolate, krom_end));
		krom->Set(String::NewFromUtf8(isolate, "fileSaveBytes").ToLocalChecked(), FunctionTemplate::New(isolate, krom_file_save_bytes));
		krom->Set(String::NewFromUtf8(isolate, "sysCommand").ToLocalChecked(), FunctionTemplate::New(isolate, krom_sys_command));
		krom->Set(String::NewFromUtf8(isolate, "savePath").ToLocalChecked(), FunctionTemplate::New(isolate, krom_save_path));
		krom->Set(String::NewFromUtf8(isolate, "getArgCount").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_arg_count));
		krom->Set(String::NewFromUtf8(isolate, "getArg").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_arg));
		krom->Set(String::NewFromUtf8(isolate, "getFilesLocation").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_files_location));
		krom->Set(String::NewFromUtf8(isolate, "setBoolCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_bool_compute));
		krom->Set(String::NewFromUtf8(isolate, "setIntCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_int_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloatCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloat2Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float2_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloat3Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float3_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloat4Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_float4_compute));
		krom->Set(String::NewFromUtf8(isolate, "setFloatsCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_floats_compute));
		krom->Set(String::NewFromUtf8(isolate, "setMatrixCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix_compute));
		krom->Set(String::NewFromUtf8(isolate, "setMatrix3Compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_matrix3_compute));
		krom->Set(String::NewFromUtf8(isolate, "setTextureCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_compute));
		krom->Set(String::NewFromUtf8(isolate, "setRenderTargetCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_render_target_compute));
		krom->Set(String::NewFromUtf8(isolate, "setSampledTextureCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_sampled_texture_compute));
		krom->Set(String::NewFromUtf8(isolate, "setSampledRenderTargetCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_sampled_render_target_compute));
		krom->Set(String::NewFromUtf8(isolate, "setSampledDepthTextureCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_sampled_depth_texture_compute));
		krom->Set(String::NewFromUtf8(isolate, "setTextureParametersCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture_parameters_compute));
		krom->Set(String::NewFromUtf8(isolate, "setTexture3DParametersCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_texture3d_parameters_compute));
		krom->Set(String::NewFromUtf8(isolate, "setShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "deleteShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_delete_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "createShaderCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_create_shader_compute));
		krom->Set(String::NewFromUtf8(isolate, "getConstantLocationCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_constant_location_compute));
		krom->Set(String::NewFromUtf8(isolate, "getTextureUnitCompute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_get_texture_unit_compute));
		krom->Set(String::NewFromUtf8(isolate, "compute").ToLocalChecked(), FunctionTemplate::New(isolate, krom_compute));
		//
		krom->Set(String::NewFromUtf8(isolate, "setSaveAndQuitCallback").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_save_and_quit_callback));
		krom->Set(String::NewFromUtf8(isolate, "setMouseCursor").ToLocalChecked(), FunctionTemplate::New(isolate, krom_set_mouse_cursor));
		#ifdef WITH_NFD
		krom->Set(String::NewFromUtf8(isolate, "openDialog").ToLocalChecked(), FunctionTemplate::New(isolate, krom_open_dialog));
		krom->Set(String::NewFromUtf8(isolate, "saveDialog").ToLocalChecked(), FunctionTemplate::New(isolate, krom_save_dialog));
		#endif
		#ifdef KORE_DIRECT3D12
		krom->Set(String::NewFromUtf8(isolate, "raytraceInit").ToLocalChecked(), FunctionTemplate::New(isolate, krom_raytrace_init));
		krom->Set(String::NewFromUtf8(isolate, "raytraceDispatchRays").ToLocalChecked(), FunctionTemplate::New(isolate, krom_raytrace_dispatch_rays));
		#endif
		krom->Set(String::NewFromUtf8(isolate, "windowX").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_x));
		krom->Set(String::NewFromUtf8(isolate, "windowY").ToLocalChecked(), FunctionTemplate::New(isolate, krom_window_y));

		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		global->Set(String::NewFromUtf8(isolate, "Krom").ToLocalChecked(), krom);

		Local<Context> context = Context::New(isolate, NULL, global);
		global_context.Reset(isolate, context);
	}

	bool start_krom(char* scriptfile) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		Local<String> source = String::NewFromUtf8(isolate, scriptfile, NewStringType::kNormal).ToLocalChecked();

		TryCatch try_catch(isolate);
		Local<Script> compiled_script = Script::Compile(isolate->GetCurrentContext(), source).ToLocalChecked();

		Local<Value> result;
		if (!compiled_script->Run(context).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
			return false;
		}

		return true;
	}

	void runV8() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		v8::MicrotasksScope microtasks_scope(isolate, v8::MicrotasksScope::kRunMicrotasks);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<v8::Function> func = Local<v8::Function>::New(isolate, update_func);
		Local<Value> result;

		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}

		if (save_and_quit > 0) {
			v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, save_and_quit_func);
			Local<Value> result;
			const int argc = 1;
			Local<Value> argv[argc] = {Boolean::New(isolate, save_and_quit == 1)};
			if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
				v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
				kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
			}
			save_and_quit = 0;
		}
	}

	void end_v8() {
		update_func.Reset();
		global_context.Reset();
		isolate->Dispose();
		V8::Dispose();
		V8::ShutdownPlatform();
	}

	#ifdef WITH_AUDIO
	void update_audio(kinc_a2_buffer_t *buffer, int samples) {
		// kinc_mutex_lock(&mutex);
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		v8::MicrotasksScope microtasks_scope(isolate, v8::MicrotasksScope::kRunMicrotasks);
		HandleScope handle_scope(isolate);
		Local<Context> context = Local<Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		Local<v8::Function> func = Local<v8::Function>::New(isolate, audio_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, samples)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}

		for (int i = 0; i < samples; ++i) {
			float sample = *(float*)&audio_buffer.data[audio_buffer.read_location];
			audio_buffer.read_location += 4;
			if (audio_buffer.read_location >= audio_buffer.data_size) {
				audio_buffer.read_location = 0;
			}

			*(float*)&buffer->data[buffer->write_location] = sample;
			buffer->write_location += 4;
			if (buffer->write_location >= buffer->data_size) {
				buffer->write_location = 0;
			}
		}

		// kinc_mutex_unlock(&mutex);
	}
	#endif

	void update() {
		#ifdef WITH_AUDIO
		if (enable_sound) kinc_a2_update();
		#endif

		kinc_g4_begin(0);

		// kinc_mutex_lock(&mutex);
		runV8();
		// kinc_mutex_unlock(&mutex);

		kinc_g4_end(0);
		kinc_g4_swap_buffers();
	}

	void drop_files(wchar_t* file_path) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, drop_files_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc];
		if (sizeof(wchar_t) == 2) {
			argv[0] = {String::NewFromTwoByte(isolate, (const uint16_t*)file_path).ToLocalChecked()};
		}
		else {
			size_t len = wcslen(file_path);
			uint16_t* str = new uint16_t[len + 1];
			for (int i = 0; i < len; i++) str[i] = file_path[i];
			str[len] = 0;
			argv[0] = {String::NewFromTwoByte(isolate, str).ToLocalChecked()};
			delete str;
		}
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	char* copy() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, copy_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
		String::Utf8Value cutCopyString(isolate, result);
		return *cutCopyString;
	}

	char* cut() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, cut_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
		String::Utf8Value cutCopyString(isolate, result);
		return *cutCopyString;
	}

	void paste(char* data) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, paste_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {String::NewFromUtf8(isolate, data).ToLocalChecked()};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void foreground() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, foreground_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void resume() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, resume_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void pause() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pause_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void background() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, background_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void shutdown() {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, shutdown_func);
		Local<Value> result;
		if (!func->Call(context, context->Global(), 0, NULL).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void key_down(int code) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboard_down_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void key_up(int code) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboard_up_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, code)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void key_press(unsigned int character) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, keyboard_press_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, character)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void mouse_move(int window, int x, int y, int mx, int my) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_move_func);
		Local<Value> result;
		const int argc = 4;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Int32::New(isolate, mx), Int32::New(isolate, my)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void mouse_down(int window, int button, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void mouse_up(int window, int button, int x, int y) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, button), Int32::New(isolate, x), Int32::New(isolate, y)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void mouse_wheel(int window, int delta) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, mouse_wheel_func);
		Local<Value> result;
		const int argc = 1;
		Local<Value> argv[argc] = {Int32::New(isolate, delta)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void pen_down(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pen_down_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void pen_up(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pen_up_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void pen_move(int window, int x, int y, float pressure) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, pen_move_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, x), Int32::New(isolate, y), Number::New(isolate, pressure)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void gamepad_axis(int gamepad, int axis, float value) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, gamepad_axis_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, axis), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}

	void gamepad_button(int gamepad, int button, float value) {
		v8::Locker locker{isolate};

		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, global_context);
		Context::Scope context_scope(context);

		TryCatch try_catch(isolate);
		v8::Local<v8::Function> func = v8::Local<v8::Function>::New(isolate, gamepad_button_func);
		Local<Value> result;
		const int argc = 3;
		Local<Value> argv[argc] = {Int32::New(isolate, gamepad), Int32::New(isolate, button), Number::New(isolate, value)};
		if (!func->Call(context, context->Global(), argc, argv).ToLocal(&result)) {
			v8::String::Utf8Value stack_trace(isolate, try_catch.StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
			kinc_log(KINC_LOG_LEVEL_INFO, "Trace: %s", *stack_trace);
		}
	}
}

int kickstart(int argc, char** argv) {
	_argc = argc;
	_argv = argv;
	std::string bindir(argv[0]);
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

	bool read_stdout_path = false;
	bool read_console_pid = false;
	for (int i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "--nosound") == 0) {
			enable_sound = false;
		}
		else if (strcmp(argv[i], "--nowindow") == 0) {
			enable_window = false;
		}
		else if (read_stdout_path) {
			freopen(argv[i], "w", stdout);
			read_stdout_path = false;
		}
		else if (strcmp(argv[i], "--stdout") == 0) {
			read_stdout_path = true;
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

	kinc_internal_set_files_location(&assetsdir[0u]);

	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, "krom.bin", KINC_FILE_TYPE_ASSET)) {
		if (!kinc_file_reader_open(&reader, "krom.js", KINC_FILE_TYPE_ASSET)) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Could not load krom.js, aborting.");
			exit(1);
		}
	}

	int reader_size = (int)kinc_file_reader_size(&reader);
	char* code = (char*)malloc(reader_size + 1);
	kinc_file_reader_read(&reader, code, reader_size);
	code[reader_size] = 0;
	kinc_file_reader_close(&reader);

	#ifdef KORE_WINDOWS
	char dirsep = '\\';
	#else
	char dirsep = '/';
	#endif
	start_v8((bindir + dirsep).c_str());

	kinc_threads_init();

	start_krom(code);
	kinc_start();

	// #ifdef WITH_AUDIO
	// if (enable_sound) {
		// kinc_a2_shutdown();
		// kinc_mutex_lock(&mutex); // Prevent audio thread from running
	// }
	// #endif

	exit(0); // TODO

	free(code);
	end_v8();
	return 0;
}
