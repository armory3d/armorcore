
class App {

	static w(): i32 { return sys_width(); }
	static h(): i32 { return sys_height(); }
	static x(): i32 { return 0; }
	static y(): i32 { return 0; }

	static onResets: (()=>void)[] = null;
	static onEndFrames: (()=>void)[] = null;
	static traitInits: (()=>void)[] = [];
	static traitUpdates: (()=>void)[] = [];
	static traitLateUpdates: (()=>void)[] = [];
	static traitRenders: ((g4: g4_t)=>void)[] = [];
	static traitRenders2D: ((g2: g2_t)=>void)[] = [];
	static pauseUpdates = false;
	static lastw = -1;
	static lasth = -1;
	static onResize: ()=>void = null;

	static init = (done: ()=>void) => {
		done();
		sys_notify_on_frames(App.render);
	}

	static reset = () => {
		App.traitInits = [];
		App.traitUpdates = [];
		App.traitLateUpdates = [];
		App.traitRenders = [];
		App.traitRenders2D = [];
		if (App.onResets != null) for (let f of App.onResets) f();
	}

	static update = () => {
		if (!_scene_ready) return;
		if (App.pauseUpdates) return;

		scene_update_frame();

		let i = 0;
		let l = App.traitUpdates.length;
		while (i < l) {
			if (App.traitInits.length > 0) {
				for (let f of App.traitInits) {
					if (App.traitInits.length > 0) f();
					else break;
				}
				App.traitInits.splice(0, App.traitInits.length);
			}
			App.traitUpdates[i]();
			// Account for removed traits
			l <= App.traitUpdates.length ? i++ : l = App.traitUpdates.length;
		}

		i = 0;
		l = App.traitLateUpdates.length;
		while (i < l) {
			App.traitLateUpdates[i]();
			l <= App.traitLateUpdates.length ? i++ : l = App.traitLateUpdates.length;
		}

		if (App.onEndFrames != null) for (let f of App.onEndFrames) f();

		// Rebuild projection on window resize
		if (App.lastw == -1) {
			App.lastw = App.w();
			App.lasth = App.h();
		}
		if (App.lastw != App.w() || App.lasth != App.h()) {
			if (App.onResize != null) App.onResize();
			else {
				if (scene_camera != null) {
					CameraObject.buildProjection(scene_camera);
				}
			}
		}
		App.lastw = App.w();
		App.lasth = App.h();
	}

	static render = (g2: g2_t, g4: g4_t) => {
		App.update();

		time_update();

		if (!_scene_ready) {
			App.render2D(g2);
			return;
		}

		if (App.traitInits.length > 0) {
			for (let f of App.traitInits) {
				if (App.traitInits.length > 0) f();
				else break;
			}
			App.traitInits.splice(0, App.traitInits.length);
		}

		scene_render_frame(g4);

		for (let f of App.traitRenders) {
			if (App.traitRenders.length > 0) f(g4);
			else break;
		}

		App.render2D(g2);
	}

	static render2D = (g2: g2_t) => {
		if (App.traitRenders2D.length > 0) {
			g2_begin(g2, false);
			for (let f of App.traitRenders2D) {
				if (App.traitRenders2D.length > 0) f(g2);
				else break;
			}
			g2_end(g2);
		}
	}

	// Hooks
	static notifyOnInit = (f: ()=>void) => {
		App.traitInits.push(f);
	}

	static removeInit = (f: ()=>void) => {
		array_remove(App.traitInits, f);
	}

	static notifyOnUpdate = (f: ()=>void) => {
		App.traitUpdates.push(f);
	}

	static removeUpdate = (f: ()=>void) => {
		array_remove(App.traitUpdates, f);
	}

	static notifyOnLateUpdate = (f: ()=>void) => {
		App.traitLateUpdates.push(f);
	}

	static removeLateUpdate = (f: ()=>void) => {
		array_remove(App.traitLateUpdates, f);
	}

	static notifyOnRender = (f: (g4: g4_t)=>void) => {
		App.traitRenders.push(f);
	}

	static removeRender = (f: (g4: g4_t)=>void) => {
		array_remove(App.traitRenders, f);
	}

	static notifyOnRender2D = (f: (g2: g2_t)=>void) => {
		App.traitRenders2D.push(f);
	}

	static removeRender2D = (f: (g2: g2_t)=>void) => {
		array_remove(App.traitRenders2D, f);
	}

	static notifyOnReset = (f: ()=>void) => {
		if (App.onResets == null) App.onResets = [];
		App.onResets.push(f);
	}

	static removeReset = (f: ()=>void) => {
		array_remove(App.onResets, f);
	}

	static notifyOnEndFrame = (f: ()=>void) => {
		if (App.onEndFrames == null) App.onEndFrames = [];
		App.onEndFrames.push(f);
	}

	static removeEndFrame = (f: ()=>void) => {
		array_remove(App.onEndFrames, f);
	}
}
