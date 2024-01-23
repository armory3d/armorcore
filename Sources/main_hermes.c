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
#include "hermes/VM/static_h.h"

static bool enable_window = true;

static void (*update_func)() = NULL;
static void (*keyboard_down_func)(int) = NULL;
static void (*keyboard_up_func)(int) = NULL;
static void (*keyboard_press_func)(int) = NULL;
static void (*mouse_move_func)(int, int, int, int) = NULL;
static void (*mouse_down_func)(int, int, int) = NULL;
static void (*mouse_up_func)(int, int, int) = NULL;
static void (*mouse_wheel_func)(int) = NULL;

void update(void *data) {
	kinc_g4_begin(0);

	// update_func();

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

void _krom_set_callback(void *ptr) {

}

void _krom_init(char *title, int w, int h) {
	// kinc_threads_init();
	kinc_display_init();

	kinc_window_options_t win;
	kinc_framebuffer_options_t frame;
	win.title = title;
	win.width = w;
	win.height = h;
	frame.vertical_sync = true;
	win.mode = KINC_WINDOW_MODE_WINDOW;
	win.window_features = 0;
	win.x = -1;
	win.y = -1;
	frame.frequency = 60;

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

extern SHUnit sh_export_this_unit;

int kickstart(int argc, char **argv) {

	// libmain.a/main()
	SHRuntime *shr = _sh_init(argc, argv);
	_sh_initialize_units(shr, 1, &sh_export_this_unit);
	_sh_done(shr);

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
