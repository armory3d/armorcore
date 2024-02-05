
class ObjectAnimationRaw {
	base: AnimationRaw;
	object: TBaseObject;
	oactions: scene_t[];
	oaction: obj_t;
	s0: f32 = 0.0;
	bezierFrameIndex = -1;
}

class ObjectAnimation {

	static create(object: TBaseObject, oactions: scene_t[]): ObjectAnimationRaw {
		let raw = new ObjectAnimationRaw();
		raw.base = Animation.create();
		raw.base.ext = raw;
		raw.object = object;
		raw.oactions = oactions;
		raw.base.isSkinned = false;
		ObjectAnimation.play(raw);
		return raw;
	}

	static getAction = (raw: ObjectAnimationRaw, action: string): obj_t => {
		for (let a of raw.oactions) if (a != null && a.objects[0].name == action) return a.objects[0];
		return null;
	}

	static updateObjectAnim = (raw: ObjectAnimationRaw) => {
		ObjectAnimation.updateTransformAnim(raw, raw.oaction.anim, raw.object.transform);
		transform_build_matrix(raw.object.transform);
	}

	static interpolateLinear = (t: f32, t1: f32, t2: f32, v1: f32, v2: f32): f32 => {
		let s = (t - t1) / (t2 - t1);
		return (1.0 - s) * v1 + s * v2;
	}

	static checkFrameIndexT = (raw: ObjectAnimationRaw, frameValues: Uint32Array, t: f32): bool => {
		return raw.base.speed > 0 ?
			raw.base.frameIndex < frameValues.length - 2 && t > frameValues[raw.base.frameIndex + 1] * raw.base.frameTime :
			raw.base.frameIndex > 1 && t > frameValues[raw.base.frameIndex - 1] * raw.base.frameTime;
	}

	static updateTransformAnim = (raw: ObjectAnimationRaw, anim: anim_t, transform: transform_t) => {
		if (anim == null) return;

		let total = anim.end * raw.base.frameTime - anim.begin * raw.base.frameTime;

		if (anim.has_delta) {
			let t = transform;
			if (t.dloc == null) {
				t.dloc = vec4_create();
				t.drot = quat_create();
				t.dscale = vec4_create();
			}
			vec4_set(t.dloc, 0, 0, 0);
			vec4_set(t.dscale, 0, 0, 0);
			t._deulerX = t._deulerY = t._deulerZ = 0.0;
		}

		for (let track of anim.tracks) {

			if (raw.base.frameIndex == -1) Animation.rewind(raw.base, track);
			let sign = raw.base.speed > 0 ? 1 : -1;

			// End of current time range
			let t = raw.base.time + anim.begin * raw.base.frameTime;
			while (ObjectAnimation.checkFrameIndexT(raw, track.frames, t)) raw.base.frameIndex += sign;

			// No data for raw track at current time
			if (raw.base.frameIndex >= track.frames.length) continue;

			// End of track
			if (raw.base.time > total) {
				if (raw.base.onComplete != null) raw.base.onComplete();
				if (raw.base.loop) Animation.rewind(raw.base, track);
				else {
					raw.base.frameIndex -= sign;
					raw.base.paused = true;
				}
				return;
			}

			let ti = raw.base.frameIndex;
			let t1 = track.frames[ti] * raw.base.frameTime;
			let t2 = track.frames[ti + sign] * raw.base.frameTime;
			let v1 = track.values[ti];
			let v2 = track.values[ti + sign];

			let value = ObjectAnimation.interpolateLinear(t, t1, t2, v1, v2);

			switch (track.target) {
				case "xloc": transform.loc.x = value;
				case "yloc": transform.loc.y = value;
				case "zloc": transform.loc.z = value;
				case "xrot": transform_set_rot(transform, value, transform._eulerY, transform._eulerZ);
				case "yrot": transform_set_rot(transform, transform._eulerX, value, transform._eulerZ);
				case "zrot": transform_set_rot(transform, transform._eulerX, transform._eulerY, value);
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

	static play = (raw: ObjectAnimationRaw, action = "", onComplete: ()=>void = null, blendTime = 0.0, speed = 1.0, loop = true) => {
		Animation.playSuper(raw.base, action, onComplete, blendTime, speed, loop);

		if (raw.base.action == "" && raw.oactions[0] != null) {
			raw.base.action = raw.oactions[0].objects[0].name;
		}
		raw.oaction = ObjectAnimation.getAction(raw, raw.base.action);
		if (raw.oaction != null) {
			raw.base.isSampled = raw.oaction.sampled != null && raw.oaction.sampled;
		}
	}

	static update = (raw: ObjectAnimationRaw, delta: f32) => {
		if (!raw.object.visible || raw.object.culled || raw.oaction == null) return;

		Animation.updateSuper(raw.base, delta);

		if (raw.base.paused) return;
		if (!raw.base.isSkinned) ObjectAnimation.updateObjectAnim(raw);
	}

	static isTrackEnd = (raw: ObjectAnimationRaw, track: track_t): bool => {
		return raw.base.speed > 0 ?
			raw.base.frameIndex >= track.frames.length - 2 :
			raw.base.frameIndex <= 0;
	}

	static totalFrames = (raw: ObjectAnimationRaw): i32 => {
		if (raw.oaction == null || raw.oaction.anim == null) return 0;
		return raw.oaction.anim.end - raw.oaction.anim.begin;
	}
}
