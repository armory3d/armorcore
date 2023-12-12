package iron;

import iron.System;

class App {

	public static dynamic function w(): Int { return System.width; }
	public static dynamic function h(): Int { return System.height; }
	public static dynamic function x(): Int { return 0; }
	public static dynamic function y(): Int { return 0; }

	static var onResets: Array<Void->Void> = null;
	static var onEndFrames: Array<Void->Void> = null;
	static var traitInits: Array<Void->Void> = [];
	static var traitUpdates: Array<Void->Void> = [];
	static var traitLateUpdates: Array<Void->Void> = [];
	static var traitRenders: Array<Graphics4->Void> = [];
	static var traitRenders2D: Array<Graphics2->Void> = [];
	public static var pauseUpdates = false;
	static var lastw = -1;
	static var lasth = -1;
	public static var onResize: Void->Void = null;

	public static function init(done: Void->Void) {
		new App(done);
	}

	function new(done: Void->Void) {
		done();
		System.notifyOnFrames(render);
	}

	public static function reset() {
		traitInits = [];
		traitUpdates = [];
		traitLateUpdates = [];
		traitRenders = [];
		traitRenders2D = [];
		if (onResets != null) for (f in onResets) f();
	}

	static function update() {
		if (Scene.active == null || !Scene.active.ready) return;
		if (pauseUpdates) return;

		Scene.active.updateFrame();

		var i = 0;
		var l = traitUpdates.length;
		while (i < l) {
			if (traitInits.length > 0) {
				for (f in traitInits) {
					traitInits.length > 0 ? f() : break;
				}
				traitInits.splice(0, traitInits.length);
			}
			traitUpdates[i]();
			// Account for removed traits
			l <= traitUpdates.length ? i++ : l = traitUpdates.length;
		}

		i = 0;
		l = traitLateUpdates.length;
		while (i < l) {
			traitLateUpdates[i]();
			l <= traitLateUpdates.length ? i++ : l = traitLateUpdates.length;
		}

		if (onEndFrames != null) for (f in onEndFrames) f();

		// Rebuild projection on window resize
		if (lastw == -1) {
			lastw = App.w();
			lasth = App.h();
		}
		if (lastw != App.w() || lasth != App.h()) {
			if (onResize != null) onResize();
			else {
				if (Scene.active != null && Scene.active.camera != null) {
					Scene.active.camera.buildProjection();
				}
			}
		}
		lastw = App.w();
		lasth = App.h();
	}

	static function render(g2: Graphics2, g4: Graphics4) {
		update();

		Time.update();

		if (Scene.active == null || !Scene.active.ready) {
			render2D(g2);
			return;
		}

		if (traitInits.length > 0) {
			for (f in traitInits) {
				traitInits.length > 0 ? f() : break;
			}
			traitInits.splice(0, traitInits.length);
		}

		Scene.active.renderFrame(g4);

		for (f in traitRenders) {
			traitRenders.length > 0 ? f(g4) : break;
		}

		render2D(g2);
	}

	static function render2D(g2: Graphics2) {
		if (traitRenders2D.length > 0) {
			g2.begin(false);
			for (f in traitRenders2D) {
				traitRenders2D.length > 0 ? f(g2) : break;
			}
			g2.end();
		}
	}

	// Hooks
	public static function notifyOnInit(f: Void->Void) {
		traitInits.push(f);
	}

	public static function removeInit(f: Void->Void) {
		traitInits.remove(f);
	}

	public static function notifyOnUpdate(f: Void->Void) {
		traitUpdates.push(f);
	}

	public static function removeUpdate(f: Void->Void) {
		traitUpdates.remove(f);
	}

	public static function notifyOnLateUpdate(f: Void->Void) {
		traitLateUpdates.push(f);
	}

	public static function removeLateUpdate(f: Void->Void) {
		traitLateUpdates.remove(f);
	}

	public static function notifyOnRender(f: Graphics4->Void) {
		traitRenders.push(f);
	}

	public static function removeRender(f: Graphics4->Void) {
		traitRenders.remove(f);
	}

	public static function notifyOnRender2D(f: Graphics2->Void) {
		traitRenders2D.push(f);
	}

	public static function removeRender2D(f: Graphics2->Void) {
		traitRenders2D.remove(f);
	}

	public static function notifyOnReset(f: Void->Void) {
		if (onResets == null) onResets = [];
		onResets.push(f);
	}

	public static function removeReset(f: Void->Void) {
		onResets.remove(f);
	}

	public static function notifyOnEndFrame(f: Void->Void) {
		if (onEndFrames == null) onEndFrames = [];
		onEndFrames.push(f);
	}

	public static function removeEndFrame(f: Void->Void) {
		onEndFrames.remove(f);
	}
}
