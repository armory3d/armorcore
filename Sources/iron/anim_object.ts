
type anim_object_t = {
	base?: anim_raw_t;
	object?: object_t;
	oactions?: scene_t[];
	oaction?: obj_t;
};

function anim_object_create(object: object_t, oactions: scene_t[]): anim_object_t {
	let raw: anim_object_t = {};
	raw.base = anim_create();
	raw.base.ext = raw;
	raw.base.ext_type = "anim_bone_t";
	raw.object = object;
	raw.oactions = oactions;
	raw.base.is_skinned = false;
	anim_object_play(raw);
	return raw;
}

function anim_object_get_action(raw: anim_object_t, action: string): obj_t {
	for (let a of raw.oactions) {
		if (a != null && a.objects[0].name == action) {
			return a.objects[0];
		}
	}
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

function anim_object_check_frame_index_t(raw: anim_object_t, frame_values: Uint32Array, t: f32): bool {
	return raw.base.speed > 0 ?
		raw.base.frame_index < frame_values.length - 2 && t > frame_values[raw.base.frame_index + 1] * raw.base.frame_time :
		raw.base.frame_index > 1 && t > frame_values[raw.base.frame_index - 1] * raw.base.frame_time;
}

function anim_object_update_transform_anim(raw: anim_object_t, anim: anim_t, transform: transform_t) {
	if (anim == null) {
		return;
	}

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
		t._deuler_x = t._deuler_y = t._deuler_z = 0.0;
	}

	for (let track of anim.tracks) {

		if (raw.base.frame_index == -1) {
			anim_rewind(raw.base, track);
		}
		let sign = raw.base.speed > 0 ? 1 : -1;

		// End of current time range
		let t = raw.base.time + anim.begin * raw.base.frame_time;
		while (anim_object_check_frame_index_t(raw, track.frames, t)) {
			raw.base.frame_index += sign;
		}

		// No data for raw track at current time
		if (raw.base.frame_index >= track.frames.length) {
			continue;
		}

		// End of track
		if (raw.base.time > total) {
			if (raw.base.on_complete != null) {
				raw.base.on_complete();
			}
			if (raw.base.loop) {
				anim_rewind(raw.base, track);
			}
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

		let tt = track.target;
		if (tt == "xloc") {
			transform.loc.x = value;
		}
		else if (tt == "yloc") {
			transform.loc.y = value;
		}
		else if (tt == "zloc") {
			transform.loc.z = value;
		}
		else if (tt == "xrot") {
			transform_set_rot(transform, value, transform._euler_y, transform._euler_z);
		}
		else if (tt == "yrot") {
			transform_set_rot(transform, transform._euler_x, value, transform._euler_z);
		}
		else if (tt == "zrot") {
			transform_set_rot(transform, transform._euler_x, transform._euler_y, value);
		}
		else if (tt == "qwrot") {
			transform.rot.w = value;
		}
		else if (tt == "qxrot") {
			transform.rot.x = value;
		}
		else if (tt == "qyrot") {
			transform.rot.y = value;
		}
		else if (tt == "qzrot") {
			transform.rot.z = value;
		}
		else if (tt == "xscl") {
			transform.scale.x = value;
		}
		else if (tt == "yscl") {
			transform.scale.y = value;
		}
		else if (tt == "zscl") {
			transform.scale.z = value;
		}
		// Delta
		else if (tt == "dxloc") {
			transform.dloc.x = value;
		}
		else if (tt == "dyloc") {
			transform.dloc.y = value;
		}
		else if (tt == "dzloc") {
			transform.dloc.z = value;
		}
		else if (tt == "dxrot") {
			transform._deuler_x = value;
		}
		else if (tt == "dyrot") {
			transform._deuler_y = value;
		}
		else if (tt == "dzrot") {
			transform._deuler_z = value;
		}
		else if (tt == "dqwrot") {
			transform.drot.w = value;
		}
		else if (tt == "dqxrot") {
			transform.drot.x = value;
		}
		else if (tt == "dqyrot") {
			transform.drot.y = value;
		}
		else if (tt == "dqzrot") {
			transform.drot.z = value;
		}
		else if (tt == "dxscl") {
			transform.dscale.x = value;
		}
		else if (tt == "dyscl") {
			transform.dscale.y = value;
		}
		else if (tt == "dzscl") {
			transform.dscale.z = value;
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
	if (!raw.object.visible || raw.object.culled || raw.oaction == null) {
		return;
	}

	anim_update_super(raw.base, delta);

	if (raw.base.paused) {
		return;
	}
	if (!raw.base.is_skinned) {
		anim_object_update_object_anim(raw);
	}
}

function anim_object_is_track_end(raw: anim_object_t, track: track_t): bool {
	return raw.base.speed > 0 ?
		raw.base.frame_index >= track.frames.length - 2 :
		raw.base.frame_index <= 0;
}

function anim_object_total_frames(raw: anim_object_t): i32 {
	if (raw.oaction == null || raw.oaction.anim == null) {
		return 0;
	}
	return raw.oaction.anim.end - raw.oaction.anim.begin;
}
