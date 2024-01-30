
class App {

	static w(): i32 { return System.width; }
	static h(): i32 { return System.height; }
	static x(): i32 { return 0; }
	static y(): i32 { return 0; }

	static onResets: (()=>void)[] = null;
	static onEndFrames: (()=>void)[] = null;
	static traitInits: (()=>void)[] = [];
	static traitUpdates: (()=>void)[] = [];
	static traitLateUpdates: (()=>void)[] = [];
	static traitRenders: ((g4: Graphics4)=>void)[] = [];
	static traitRenders2D: ((g2: Graphics2)=>void)[] = [];
	static pauseUpdates = false;
	static lastw = -1;
	static lasth = -1;
	static onResize: ()=>void = null;

	static init = (done: ()=>void) => {
		done();
		System.notifyOnFrames(App.render);
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
		if (!Scene.ready) return;
		if (App.pauseUpdates) return;

		Scene.updateFrame();

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
				if (Scene.camera != null) {
					Scene.camera.buildProjection();
				}
			}
		}
		App.lastw = App.w();
		App.lasth = App.h();
	}

	static render = (g2: Graphics2, g4: Graphics4) => {
		App.update();

		Time.update();

		if (!Scene.ready) {
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

		Scene.renderFrame(g4);

		for (let f of App.traitRenders) {
			if (App.traitRenders.length > 0) f(g4);
			else break;
		}

		App.render2D(g2);
	}

	static render2D = (g2: Graphics2) => {
		if (App.traitRenders2D.length > 0) {
			g2.begin(false);
			for (let f of App.traitRenders2D) {
				if (App.traitRenders2D.length > 0) f(g2);
				else break;
			}
			g2.end();
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

	static notifyOnRender = (f: (g4: Graphics4)=>void) => {
		App.traitRenders.push(f);
	}

	static removeRender = (f: (g4: Graphics4)=>void) => {
		array_remove(App.traitRenders, f);
	}

	static notifyOnRender2D = (f: (g2: Graphics2)=>void) => {
		App.traitRenders2D.push(f);
	}

	static removeRender2D = (f: (g2: Graphics2)=>void) => {
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
