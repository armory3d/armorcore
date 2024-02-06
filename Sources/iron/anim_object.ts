
class anim_object_t {
	base: anim_raw_t;
	object: object_t;
	oactions: scene_t[];
	oaction: obj_t;
	s0: f32 = 0.0;
	bezier_frame_index = -1;
}

function anim_object_create(object: object_t, oactions: scene_t[]): anim_object_t {
	let raw = new anim_object_t();
	raw.base = anim_create();
	raw.base.ext = raw;
	raw.object = object;
	raw.oactions = oactions;
	raw.base.is_skinned = false;
	anim_object_play(raw);
	return raw;
}

function anim_object_get_action(raw: anim_object_t, action: string): obj_t {
	for (let a of raw.oactions) if (a != null && a.objects[0].name == action) return a.objects[0];
	return null;
}

function anim_object_update_object_anim(raw: anim_object_t) {
	anim_object_update_transform_anim(raw, raw.oaction.anim, raw.object.transform);
	transform_build_matrix(raw.object.transform);
}

function anim_object_interpolate_linear(t: f32, t1: f32, t2: f32, v1: f32, v2: f32): f32 {
	let s = (t - t1) / (t2 - t1);
	return (1.0 - s) * v1 + s * v2;
}

function anim_object_check_frame_index_t(raw: anim_object_t, frameValues: Uint32Array, t: f32): bool {
	return raw.base.speed > 0 ?
		raw.base.frame_index < frameValues.length - 2 && t > frameValues[raw.base.frame_index + 1] * raw.base.frame_time :
		raw.base.frame_index > 1 && t > frameValues[raw.base.frame_index - 1] * raw.base.frame_time;
}

function anim_object_update_transform_anim(raw: anim_object_t, anim: anim_t, transform: transform_t) {
	if (anim == null) return;

	let total = anim.end * raw.base.frame_time - anim.begin * raw.base.frame_time;

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

		if (raw.base.frame_index == -1) anim_rewind(raw.base, track);
		let sign = raw.base.speed > 0 ? 1 : -1;

		// End of current time range
		let t = raw.base.time + anim.begin * raw.base.frame_time;
		while (anim_object_check_frame_index_t(raw, track.frames, t)) raw.base.frame_index += sign;

		// No data for raw track at current time
		if (raw.base.frame_index >= track.frames.length) continue;

		// End of track
		if (raw.base.time > total) {
			if (raw.base.on_complete != null) raw.base.on_complete();
			if (raw.base.loop) anim_rewind(raw.base, track);
			else {
				raw.base.frame_index -= sign;
				raw.base.paused = true;
			}
			return;
		}

		let ti = raw.base.frame_index;
		let t1 = track.frames[ti] * raw.base.frame_time;
		let t2 = track.frames[ti + sign] * raw.base.frame_time;
		let v1 = track.values[ti];
		let v2 = track.values[ti + sign];

		let value = anim_object_interpolate_linear(t, t1, t2, v1, v2);

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

function anim_object_play(raw: anim_object_t, action = "", on_complete: ()=>void = null, blend_time = 0.0, speed = 1.0, loop = true) {
	anim_play_super(raw.base, action, on_complete, blend_time, speed, loop);

	if (raw.base.action == "" && raw.oactions[0] != null) {
		raw.base.action = raw.oactions[0].objects[0].name;
	}
	raw.oaction = anim_object_get_action(raw, raw.base.action);
	if (raw.oaction != null) {
		raw.base.is_sampled = raw.oaction.sampled != null && raw.oaction.sampled;
	}
}

function anim_object_update(raw: anim_object_t, delta: f32) {
	if (!raw.object.visible || raw.object.culled || raw.oaction == null) return;

	anim_update_super(raw.base, delta);

	if (raw.base.paused) return;
	if (!raw.base.is_skinned) anim_object_update_object_anim(raw);
}

function anim_object_is_track_end(raw: anim_object_t, track: track_t): bool {
	return raw.base.speed > 0 ?
		raw.base.frame_index >= track.frames.length - 2 :
		raw.base.frame_index <= 0;
}

function anim_object_total_frames(raw: anim_object_t): i32 {
	if (raw.oaction == null || raw.oaction.anim == null) return 0;
	return raw.oaction.anim.end - raw.oaction.anim.begin;
}
