
class ObjectAnimation extends Animation {

	object: BaseObject;
	oactions: TSceneFormat[];
	oaction: TObj;
	s0: f32 = 0.0;
	bezierFrameIndex = -1;

	constructor(object: BaseObject, oactions: TSceneFormat[]) {
		super();
		this.object = object;
		this.oactions = oactions;
		this.isSkinned = false;
		this.play();
	}

	getAction = (action: string): TObj => {
		for (let a of this.oactions) if (a != null && a.objects[0].name == action) return a.objects[0];
		return null;
	}

	updateObjectAnim = () => {
		this.updateTransformAnim(this.oaction.anim, this.object.transform);
		this.object.transform.buildMatrix();
	}

	interpolateLinear = (t: f32, t1: f32, t2: f32, v1: f32, v2: f32): f32 => {
		let s = (t - t1) / (t2 - t1);
		return (1.0 - s) * v1 + s * v2;
	}

	checkFrameIndexT = (frameValues: Uint32Array, t: f32): bool => {
		return this.speed > 0 ?
			this.frameIndex < frameValues.length - 2 && t > frameValues[this.frameIndex + 1] * this.frameTime :
			this.frameIndex > 1 && t > frameValues[this.frameIndex - 1] * this.frameTime;
	}

	updateTransformAnim = (anim: TAnimation, transform: Transform) => {
		if (anim == null) return;

		let total = anim.end * this.frameTime - anim.begin * this.frameTime;

		if (anim.has_delta) {
			let t = transform;
			if (t.dloc == null) {
				t.dloc = new Vec4();
				t.drot = new Quat();
				t.dscale = new Vec4();
			}
			t.dloc.set(0, 0, 0);
			t.dscale.set(0, 0, 0);
			t._deulerX = t._deulerY = t._deulerZ = 0.0;
		}

		for (let track of anim.tracks) {

			if (this.frameIndex == -1) this.rewind(track);
			let sign = this.speed > 0 ? 1 : -1;

			// End of current time range
			let t = this.time + anim.begin * this.frameTime;
			while (this.checkFrameIndexT(track.frames, t)) this.frameIndex += sign;

			// No data for this track at current time
			if (this.frameIndex >= track.frames.length) continue;

			// End of track
			if (this.time > total) {
				if (this.onComplete != null) this.onComplete();
				if (this.loop) this.rewind(track);
				else {
					this.frameIndex -= sign;
					this.paused = true;
				}
				return;
			}

			let ti = this.frameIndex;
			let t1 = track.frames[ti] * this.frameTime;
			let t2 = track.frames[ti + sign] * this.frameTime;
			let v1 = track.values[ti];
			let v2 = track.values[ti + sign];

			let value = this.interpolateLinear(t, t1, t2, v1, v2);

			switch (track.target) {
				case "xloc": transform.loc.x = value;
				case "yloc": transform.loc.y = value;
				case "zloc": transform.loc.z = value;
				case "xrot": transform.setRotation(value, transform._eulerY, transform._eulerZ);
				case "yrot": transform.setRotation(transform._eulerX, value, transform._eulerZ);
				case "zrot": transform.setRotation(transform._eulerX, transform._eulerY, value);
				case "qwrot": transform.rot.w = value;
				case "qxrot": transform.rot.x = value;
				case "qyrot": transform.rot.y = value;
				case "qzrot": transform.rot.z = value;
				case "xscl": transform.scale.x = value;
				case "yscl": transform.scale.y = value;
				case "zscl": transform.scale.z = value;
				// Delta
				case "dxloc": transform.dloc.x = value;
				case "dyloc": transform.dloc.y = value;
				case "dzloc": transform.dloc.z = value;
				case "dxrot": transform._deulerX = value;
				case "dyrot": transform._deulerY = value;
				case "dzrot": transform._deulerZ = value;
				case "dqwrot": transform.drot.w = value;
				case "dqxrot": transform.drot.x = value;
				case "dqyrot": transform.drot.y = value;
				case "dqzrot": transform.drot.z = value;
				case "dxscl": transform.dscale.x = value;
				case "dyscl": transform.dscale.y = value;
				case "dzscl": transform.dscale.z = value;
			}
		}
	}

	override play = (action = "", onComplete: ()=>void = null, blendTime = 0.0, speed = 1.0, loop = true) => {
		this.playSuper(action, onComplete, blendTime, speed, loop);
		if (this.action == "" && this.oactions[0] != null) this.action = this.oactions[0].objects[0].name;
		this.oaction = this.getAction(this.action);
		if (this.oaction != null) {
			this.isSampled = this.oaction.sampled != null && this.oaction.sampled;
		}
	}

	override update = (delta: f32) => {
		if (!this.object.visible || this.object.culled || this.oaction == null) return;

		this.updateSuper(delta);
		if (this.paused) return;
		if (!this.isSkinned) this.updateObjectAnim();
	}

	override isTrackEnd = (track: TTrack): bool => {
		return this.speed > 0 ?
			this.frameIndex >= track.frames.length - 2 :
			this.frameIndex <= 0;
	}

	override totalFrames = (): i32 => {
		if (this.oaction == null || this.oaction.anim == null) return 0;
		return this.oaction.anim.end - this.oaction.anim.begin;
	}
}
