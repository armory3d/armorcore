/// <reference path='./Vec4.ts'/>
/// <reference path='./Quat.ts'/>

class Animation {

	isSkinned: bool;
	isSampled: bool;
	action = "";
	///if arm_skin
	armature: Armature; // Bone
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

	constructor() {
		Scene.active.animations.push(this);
	}

	playSuper = (action = "", onComplete: ()=>void = null, blendTime = 0.0, speed = 1.0, loop = true) => {
		if (blendTime > 0) {
			this.blendTime = blendTime;
			this.blendCurrent = 0.0;
			this.blendAction = this.action;
			this.frameIndex = 0;
			this.time = 0.0;
		}
		else this.frameIndex = -1;
		this.action = action;
		this.onComplete = onComplete;
		this.speed = speed;
		this.loop = loop;
		this.paused = false;
	}

	play = this.playSuper;

	blendSuper = (action1: string, action2: string, factor: f32) => {
		this.blendTime = 1.0; // Enable blending
		this.blendFactor = factor;
	}

	blend = this.blendSuper;

	pause = () => {
		this.paused = true;
	}

	resume = () => {
		this.paused = false;
	}

	remove = () => {
		array_remove(Scene.active.animations, this);
	}

	updateSuper = (delta: f32) => {
		if (this.paused || this.speed == 0.0) return;
		this.time += delta * this.speed;

		if (this.blendTime > 0 && this.blendFactor == 0) {
			this.blendCurrent += delta;
			if (this.blendCurrent >= this.blendTime) this.blendTime = 0.0;
		}
	}

	update = this.updateSuper;

	isTrackEnd = (track: TTrack): bool => {
		return this.speed > 0 ?
			this.frameIndex >= track.frames.length - 1 :
			this.frameIndex <= 0;
	}

	checkFrameIndex = (frameValues: Uint32Array): bool => {
		return this.speed > 0 ?
			((this.frameIndex + 1) < frameValues.length && this.time > frameValues[this.frameIndex + 1] * this.frameTime) :
			((this.frameIndex - 1) > -1 && this.time < frameValues[this.frameIndex - 1] * this.frameTime);
	}

	rewind = (track: TTrack) => {
		this.frameIndex = this.speed > 0 ? 0 : track.frames.length - 1;
		this.time = track.frames[this.frameIndex] * this.frameTime;
	}

	updateTrack = (anim: TAnimation) => {
		if (anim == null) return;

		let track = anim.tracks[0];

		if (this.frameIndex == -1) this.rewind(track);

		// Move keyframe
		let sign = this.speed > 0 ? 1 : -1;
		while (this.checkFrameIndex(track.frames)) this.frameIndex += sign;

		// Marker events
		if (this.markerEvents != null && anim.marker_names != null && this.frameIndex != this.lastFrameIndex) {
			for (let i = 0; i < anim.marker_frames.length; ++i) {
				if (this.frameIndex == anim.marker_frames[i]) {
					let ar = this.markerEvents.get(anim.marker_names[i]);
					if (ar != null) for (let f of ar) f();
				}
			}
			this.lastFrameIndex = this.frameIndex;
		}

		// End of track
		if (this.isTrackEnd(track)) {
			if (this.loop || this.blendTime > 0) {
				this.rewind(track);
			}
			else {
				this.frameIndex -= sign;
				this.paused = true;
			}
			if (this.onComplete != null && this.blendTime == 0) this.onComplete();
		}
	}

	updateAnimSampled = (anim: TAnimation, m: Mat4) => {
		if (anim == null) return;
		let track = anim.tracks[0];
		let sign = this.speed > 0 ? 1 : -1;

		let t = this.time;
		let ti = this.frameIndex;
		let t1 = track.frames[ti] * this.frameTime;
		let t2 = track.frames[ti + sign] * this.frameTime;
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

	setFrame = (frame: i32) => {
		this.time = 0;
		this.frameIndex = frame;
		this.update(frame * this.frameTime);
	}

	notifyOnMarker = (name: string, onMarker: ()=>void) => {
		if (this.markerEvents == null) this.markerEvents = new Map();
		let ar = this.markerEvents.get(name);
		if (ar == null) {
			ar = [];
			this.markerEvents.set(name, ar);
		}
		ar.push(onMarker);
	}

	removeMarker = (name: string, onMarker: ()=>void) => {
		array_remove(this.markerEvents.get(name), onMarker);
	}

	currentFrame = (): i32 => {
		return Math.floor(this.time / this.frameTime);
	}

	totalFrames = (): i32 => {
		return 0;
	}
}
