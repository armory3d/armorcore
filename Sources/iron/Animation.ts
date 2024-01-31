/// <reference path='./Vec4.ts'/>
/// <reference path='./Quat.ts'/>

class AnimationRaw {
	ext: any; // BoneAnimation | ObjectAnimation
	isSkinned: bool;
	isSampled: bool;
	action = "";
	///if arm_skin
	armature: TArmature; // Bone
	///end
	time: f32 = 0.0;
	speed: f32 = 1.0;
	loop = true;
	frameIndex = 0;
	onComplete: ()=>void = null;
	paused = false;
	frameTime: f32 = 1 / 60;
	blendTime: f32 = 0.0;
	blendCurrent: f32 = 0.0;
	blendAction = "";
	blendFactor: f32 = 0.0;
	lastFrameIndex = -1;
	markerEvents: Map<string, (()=>void)[]> = null;
}

class Animation {

	// Lerp
	static m1 = Mat4.identity();
	static m2 = Mat4.identity();
	static vpos = new Vec4();
	static vpos2 = new Vec4();
	static vscl = new Vec4();
	static vscl2 = new Vec4();
	static q1 = new Quat();
	static q2 = new Quat();
	static q3 = new Quat();
	static vp = new Vec4();
	static vs = new Vec4();

	static create(): AnimationRaw {
		let raw = new AnimationRaw();
		Scene.animations.push(raw);
		return raw;
	}

	static play = (raw: AnimationRaw, action = "", onComplete: ()=>void = null, blendTime = 0.0, speed = 1.0, loop = true) => {
		if (blendTime > 0) {
			raw.blendTime = blendTime;
			raw.blendCurrent = 0.0;
			raw.blendAction = raw.action;
			raw.frameIndex = 0;
			raw.time = 0.0;
		}
		else raw.frameIndex = -1;
		raw.action = action;
		raw.onComplete = onComplete;
		raw.speed = speed;
		raw.loop = loop;
		raw.paused = false;

		if (raw.ext.constructor.play != null) {
			raw.ext.constructor.play(raw.ext, action, onComplete, blendTime, speed, loop);
		}
	}

	static blend = (raw: AnimationRaw, action1: string, action2: string, factor: f32) => {
		if (raw.ext.constructor.blend != null) {
			raw.ext.constructor.blend(raw.ext, action1, action2, factor);
		}

		raw.blendTime = 1.0; // Enable blending
		raw.blendFactor = factor;
	}

	static pause = (raw: AnimationRaw) => {
		raw.paused = true;
	}

	static resume = (raw: AnimationRaw) => {
		raw.paused = false;
	}

	static remove = (raw: AnimationRaw) => {
		array_remove(Scene.animations, raw);
	}

	static updateSuper = (raw: AnimationRaw, delta: f32) => {
		if (raw.paused || raw.speed == 0.0) return;
		raw.time += delta * raw.speed;

		if (raw.blendTime > 0 && raw.blendFactor == 0) {
			raw.blendCurrent += delta;
			if (raw.blendCurrent >= raw.blendTime) raw.blendTime = 0.0;
		}
	}

	static update = (raw: AnimationRaw, delta: f32) => {
		if (raw.ext.constructor.update != null) {
			raw.ext.constructor.update(raw, delta);
		}
		else {
			Animation.updateSuper(raw, delta);
		}
	}

	static isTrackEnd = (raw: AnimationRaw, track: TTrack): bool => {
		return raw.speed > 0 ?
			raw.frameIndex >= track.frames.length - 1 :
			raw.frameIndex <= 0;
	}

	static checkFrameIndex = (raw: AnimationRaw, frameValues: Uint32Array): bool => {
		return raw.speed > 0 ?
			((raw.frameIndex + 1) < frameValues.length && raw.time > frameValues[raw.frameIndex + 1] * raw.frameTime) :
			((raw.frameIndex - 1) > -1 && raw.time < frameValues[raw.frameIndex - 1] * raw.frameTime);
	}

	static rewind = (raw: AnimationRaw, track: TTrack) => {
		raw.frameIndex = raw.speed > 0 ? 0 : track.frames.length - 1;
		raw.time = track.frames[raw.frameIndex] * raw.frameTime;
	}

	static updateTrack = (raw: AnimationRaw, anim: TAnimation) => {
		if (anim == null) return;

		let track = anim.tracks[0];

		if (raw.frameIndex == -1) Animation.rewind(raw, track);

		// Move keyframe
		let sign = raw.speed > 0 ? 1 : -1;
		while (Animation.checkFrameIndex(raw, track.frames)) raw.frameIndex += sign;

		// Marker events
		if (raw.markerEvents != null && anim.marker_names != null && raw.frameIndex != raw.lastFrameIndex) {
			for (let i = 0; i < anim.marker_frames.length; ++i) {
				if (raw.frameIndex == anim.marker_frames[i]) {
					let ar = raw.markerEvents.get(anim.marker_names[i]);
					if (ar != null) for (let f of ar) f();
				}
			}
			raw.lastFrameIndex = raw.frameIndex;
		}

		// End of track
		if (Animation.isTrackEnd(raw, track)) {
			if (raw.loop || raw.blendTime > 0) {
				Animation.rewind(raw, track);
			}
			else {
				raw.frameIndex -= sign;
				raw.paused = true;
			}
			if (raw.onComplete != null && raw.blendTime == 0) raw.onComplete();
		}
	}

	static updateAnimSampled = (raw: AnimationRaw, anim: TAnimation, m: Mat4) => {
		if (anim == null) return;
		let track = anim.tracks[0];
		let sign = raw.speed > 0 ? 1 : -1;

		let t = raw.time;
		let ti = raw.frameIndex;
		let t1 = track.frames[ti] * raw.frameTime;
		let t2 = track.frames[ti + sign] * raw.frameTime;
		let s: f32 = (t - t1) / (t2 - t1); // Linear

		Animation.m1.setF32(track.values, ti * 16); // Offset to 4x4 matrix array
		Animation.m2.setF32(track.values, (ti + sign) * 16);

		// Decompose
		Animation.m1.decompose(Animation.vpos, Animation.q1, Animation.vscl);
		Animation.m2.decompose(Animation.vpos2, Animation.q2, Animation.vscl2);

		// Lerp
		Animation.vp.lerp(Animation.vpos, Animation.vpos2, s);
		Animation.vs.lerp(Animation.vscl, Animation.vscl2, s);
		Animation.q3.lerp(Animation.q1, Animation.q2, s);

		// Compose
		m.fromQuat(Animation.q3);
		m.scale(Animation.vs);
		m._30 = Animation.vp.x;
		m._31 = Animation.vp.y;
		m._32 = Animation.vp.z;
	}

	static setFrame = (raw: AnimationRaw, frame: i32) => {
		raw.time = 0;
		raw.frameIndex = frame;
		Animation.update(raw, frame * raw.frameTime);
	}

	static notifyOnMarker = (raw: AnimationRaw, name: string, onMarker: ()=>void) => {
		if (raw.markerEvents == null) raw.markerEvents = new Map();
		let ar = raw.markerEvents.get(name);
		if (ar == null) {
			ar = [];
			raw.markerEvents.set(name, ar);
		}
		ar.push(onMarker);
	}

	static removeMarker = (raw: AnimationRaw, name: string, onMarker: ()=>void) => {
		array_remove(raw.markerEvents.get(name), onMarker);
	}

	static currentFrame = (raw: AnimationRaw): i32 => {
		return Math.floor(raw.time / raw.frameTime);
	}

	static totalFrames = (raw: AnimationRaw): i32 => {
		return 0;
	}
}
