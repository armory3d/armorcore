
let app_w = function(): i32 { return sys_width(); }
let app_h = function(): i32 { return sys_height(); }
let app_x = function(): i32 { return 0; }
let app_y = function(): i32 { return 0; }

let app_on_resets: (()=>void)[] = null;
let app_on_end_frames: (()=>void)[] = null;
let app_on_inits: (()=>void)[] = [];
let app_on_updates: (()=>void)[] = [];
let app_on_late_updates: (()=>void)[] = [];
let app_on_renders: (()=>void)[] = [];
let app_on_renders_2d: (()=>void)[] = [];
let app_pause_updates = false;
let app_lastw = -1;
let app_lasth = -1;
let app_on_resize: ()=>void = null;

function app_init(done: ()=>void) {
	done();
	sys_notify_on_frames(app_render);
}

function app_reset() {
	app_on_inits = [];
	app_on_updates = [];
	app_on_late_updates = [];
	app_on_renders = [];
	app_on_renders_2d = [];
	if (app_on_resets != null) {
		for (let f of app_on_resets) {
			f();
		}
	}
}

function app_update() {
	if (!_scene_ready) {
		return;
	}
	if (app_pause_updates) {
		return;
	}

	scene_update_frame();

	let i = 0;
	let l = app_on_updates.length;
	while (i < l) {
		if (app_on_inits.length > 0) {
			for (let f of app_on_inits) {
				if (app_on_inits.length > 0) {
					f();
				}
				else {
					break;
				}
			}
			app_on_inits.splice(0, app_on_inits.length);
		}
		app_on_updates[i]();
		// Account for removed traits
		l <= app_on_updates.length ? i++ : l = app_on_updates.length;
	}

	i = 0;
	l = app_on_late_updates.length;
	while (i < l) {
		app_on_late_updates[i]();
		l <= app_on_late_updates.length ? i++ : l = app_on_late_updates.length;
	}

	if (app_on_end_frames != null) {
		for (let f of app_on_end_frames) {
			f();
		}
	}

	// Rebuild projection on window resize
	if (app_lastw == -1) {
		app_lastw = app_w();
		app_lasth = app_h();
	}
	if (app_lastw != app_w() || app_lasth != app_h()) {
		if (app_on_resize != null) {
			app_on_resize();
		}
		else {
			if (scene_camera != null) {
				camera_object_build_projection(scene_camera);
			}
		}
	}
	app_lastw = app_w();
	app_lasth = app_h();
}

function app_render() {
	app_update();

	time_update();

	if (!_scene_ready) {
		app_render_2d();
		return;
	}

	if (app_on_inits.length > 0) {
		for (let f of app_on_inits) {
			if (app_on_inits.length > 0) {
				f();
			}
			else {
				break;
			}
		}
		app_on_inits.splice(0, app_on_inits.length);
	}

	scene_render_frame();

	for (let f of app_on_renders) {
		if (app_on_renders.length > 0) {
			f();
		}
		else {
			break;
		}
	}

	app_render_2d();
}

function app_render_2d() {
	if (app_on_renders_2d.length > 0) {
		g2_begin();
		for (let f of app_on_renders_2d) {
			if (app_on_renders_2d.length > 0) {
				f();
			}
			else {
				break;
			}
		}
		g2_end();
	}
}

// Hooks
function app_notify_on_init(f: ()=>void) {
	app_on_inits.push(f);
}

function app_remove_init(f: ()=>void) {
	array_remove(app_on_inits, f);
}

function app_notify_on_update(f: ()=>void) {
	app_on_updates.push(f);
}

function app_remove_update(f: ()=>void) {
	array_remove(app_on_updates, f);
}

function app_notify_on_late_update(f: ()=>void) {
	app_on_late_updates.push(f);
}

function app_remove_late_update(f: ()=>void) {
	array_remove(app_on_late_updates, f);
}

function app_notify_on_render(f: ()=>void) {
	app_on_renders.push(f);
}

function app_remove_render(f: ()=>void) {
	array_remove(app_on_renders, f);
}

function app_notify_on_render_2d(f: ()=>void) {
	app_on_renders_2d.push(f);
}

function app_remove_render_2d(f: ()=>void) {
	array_remove(app_on_renders_2d, f);
}

function app_notify_on_reset(f: ()=>void) {
	if (app_on_resets == null) {
		app_on_resets = [];
	}
	app_on_resets.push(f);
}

function app_remove_reset(f: ()=>void) {
	array_remove(app_on_resets, f);
}

function app_notify_on_end_frame(f: ()=>void) {
	if (app_on_end_frames == null) {
		app_on_end_frames = [];
	}
	app_on_end_frames.push(f);
}

function app_remove_end_frame(f: ()=>void) {
	array_remove(app_on_end_frames, f);
}
