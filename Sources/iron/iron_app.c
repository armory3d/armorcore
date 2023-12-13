
#include <stddef.h>
#include <kinc/window.h>

int iron_app_w() { return kinc_window_width(0); }
int iron_app_h() { return kinc_window_height(0); }
int iron_app_x() { return 0; }
int iron_app_y() { return 0; }

void iron_app_new(void (*done)(void)) {

}

void iron_app_init(void (*done)(void)) {
	iron_app_new(done);
}

void iron_app_reset() {

}

void iron_app_update() {

}
