
#include <kinc/window.h>

int iron_app_w() { return kinc_window_width(0); }
int iron_app_h() { return kinc_window_height(0); }
int iron_app_x() { return 0; }
int iron_app_y() { return 0; }

// static var on_resets: Array<Void->Void> = NULL;
// static var on_end_frames: Array<Void->Void> = NULL;
// static var trait_inits: Array<Void->Void> = [];
// static var trait_updates: Array<Void->Void> = [];
// static var trait_late_updates: Array<Void->Void> = [];
// static var trait_renders: Array<kha.graphics4.Graphics->Void> = [];
// static var trait_renders2d: Array<kha.graphics2.Graphics->Void> = [];
// kinc_framebuffer *framebuffer;
bool iron_pause_updates = false;

int iron_last_w = -1;
int iron_last_h = -1;
void (*iron_on_resize)(void) = NULL;

void iron_app_new(void (*done)(void)) {
	done();
	// kha.System.notifyOnFrames(render);
	// kha.Scheduler.addTimeTask(update, 0, iron.system.Time.delta);
}

void iron_app_init(void (*done)(void)) {
	iron_app_new(done);
}

void iron_app_reset() {
	// trait_inits = [];
	// trait_updates = [];
	// trait_late_updates = [];
	// trait_renders = [];
	// trait_renders2d = [];
	// if (on_resets != NULL) for (f in on_resets) f();
}

void iron_app_update() {
	// if (iron_scene_active() == NULL || !iron_scene_active()->ready) return;
	// if (iron_pause_updates) return;

	// iron_scene_active()->update_frame();

	// int i = 0;
	// int l = trait_updates.length;
	// while (i < l) {
	// 	if (trait_inits.length > 0) {
	// 		for (f in trait_inits) {
	// 			trait_inits.length > 0 ? f() : break;
	// 		}
	// 		trait_inits.splice(0, trait_inits.length);
	// 	}
	// 	trait_updates[i]();
	// 	// Account for removed traits
	// 	l <= trait_updates.length ? i++ : l = trait_updates.length;
	// }

	// i = 0;
	// l = trait_late_updates.length;
	// while (i < l) {
	// 	trait_late_updates[i]();
	// 	l <= trait_late_updates.length ? i++ : l = trait_late_updates.length;
	// }

	// if (on_end_frames != NULL) for (f in on_end_frames) f();

	// Rebuild projection on window resize
	/*if (iron_last_w == -1) {
		iron_last_w = iron_app_w();
		iron_last_h = iron_app_h();
	}
	if (iron_last_w != iron_app_w() || iron_last_h != iron_app_h()) {
		if (iron_on_resize != NULL) iron_on_resize();
		else {
			if (iron_scene_active() != NULL && iron_scene_active()->camera != NULL) {
				iron_scene_active()->camera->build_projection();
			}
		}
	}
	iron_last_w = iron_app_w();
	iron_last_h = iron_app_h();*/
}

// void iron_app_render(kinc_framebuffer_t **frames) {
	/*kinc_framebuffer frame = frames[0];
	framebuffer = frame;

	iron_time_update();

	if (iron_scene_active() == NULL || !iron_scene_active()->ready) {
		render2d(frame);
		return;
	}*/

	// if (trait_inits.length > 0) {
	// 	for (f in trait_inits) {
	// 		trait_inits.length > 0 ? f() : break;
	// 	}
	// 	trait_inits.splice(0, trait_inits.length);
	// }

	// iron_scene_active()->render_frame(frame.g4);

	// for (f in trait_renders) {
	// 	trait_renders.length > 0 ? f(frame.g4) : break;
	// }

	// render2d(frame);
// }

// void iron_app_render2d(kinc_framebuffer_t *frame) {
	// if (trait_renders2d.length > 0) {
	// 	frame.g2.begin(false);
	// 	for (f in trait_renders2d) {
	// 		trait_renders2d.length > 0 ? f(frame.g2) : break;
	// 	}
	// 	frame.g2.end();
	// }
// }

// Hooks
/*void iron_app_notify_on_init(void (*f)(void)) {
	// trait_inits.push(f);
}

void iron_app_remove_init(void (*f)(void)) {
	// trait_inits.remove(f);
}

void iron_app_notify_on_update(void (*f)(void)) {
	// trait_updates.push(f);
}

void iron_app_remove_update(void (*f)(void)) {
	// trait_updates.remove(f);
}

void iron_app_notify_on_late_update(void (*f)(void)) {
	// trait_late_updates.push(f);
}

void iron_app_remove_late_update(void (*f)(void)) {
	// trait_late_updates.remove(f);
}

void iron_app_notify_on_render(void (*f)(kinc_graphics4_t *)) {
	// trait_renders.push(f);
}

void iron_app_remove_render(void (*f)(kinc_graphics4_t*)) {
	// trait_renders.remove(f);
}

void iron_app_notify_on_render2d(void (*f)(kinc_graphics2_t*)) {
	// trait_renders2d.push(f);
}

void iron_app_remove_render2d(void (*f)(kinc_graphics2_t*)) {
	// trait_renders2d.remove(f);
}

void iron_app_notify_on_reset(void (*f)(void)) {
	// if (on_resets == NULL) on_resets = [];
	// on_resets.push(f);
}

void iron_app_remove_reset(void (*f)(void)) {
	// on_resets.remove(f);
}

void notify_on_end_frame(void (*f)(void)) {
	// if (on_end_frames == NULL) on_end_frames = [];
	// on_end_frames.push(f);
}

void remove_end_frame(void (*f)(void)) {
	// on_end_frames.remove(f);
}*/
