
class transform_t {
	world: mat4_t;
	local_only = false;
	local: mat4_t;
	loc: vec4_t;
	rot: quat_t;
	scale: vec4_t;
	scale_world: f32 = 1.0;
	world_unpack: mat4_t;
	dirty: bool;
	object: object_t;
	dim: vec4_t;
	radius: f32;

	bone_parent: mat4_t = null;
	last_world: mat4_t = null;

	// Wrong order returned from getEuler(), store last state for animation
	_eulerX: f32;
	_eulerY: f32;
	_eulerZ: f32;

	// Animated delta transform
	dloc: vec4_t = null;
	drot: quat_t = null;
	dscale: vec4_t = null;
	_deulerX: f32;
	_deulerY: f32;
	_deulerZ: f32;
}

let _transform_tmp = mat4_identity();
let _transform_q = quat_create();

function transform_create(object: object_t): transform_t {
	let raw = new transform_t();
	raw.object = object;
	transform_reset(raw);
	return raw;
}

function transform_reset(raw: transform_t) {
	raw.world = mat4_identity();
	raw.world_unpack = mat4_identity();
	raw.local = mat4_identity();
	raw.loc = vec4_create();
	raw.rot = quat_create();
	raw.scale = vec4_create(1.0, 1.0, 1.0);
	raw.dim = vec4_create(2.0, 2.0, 2.0);
	raw.radius = 1.0;
	raw.dirty = true;
}

function transform_update(raw: transform_t) {
	if (raw.dirty) transform_build_matrix(raw);
}

function transform_compose_delta(raw: transform_t) {
	// Delta transform
	vec4_add_vecs(raw.dloc, raw.loc, raw.dloc);
	vec4_add_vecs(raw.dscale, raw.dscale, raw.scale);
	quat_from_euler(raw.drot, raw._deulerX, raw._deulerY, raw._deulerZ);
	quat_mult_quats(raw.drot, raw.rot, raw.drot);
	mat4_compose(raw.local, raw.dloc, raw.drot, raw.dscale);
}

function transform_build_matrix(raw: transform_t) {
	raw.dloc == null ? mat4_compose(raw.local, raw.loc, raw.rot, raw.scale) : transform_compose_delta(raw);

	if (raw.bone_parent != null) mat4_mult_mats(raw.local, raw.bone_parent, raw.local);

	if (raw.object.parent != null && !raw.local_only) {
		mat4_mult_mats3x4(raw.world, raw.local, raw.object.parent.transform.world);
	}
	else {
		mat4_set_from(raw.world, raw.local);
	}

	mat4_set_from(raw.world_unpack, raw.world);
	if (raw.scale_world != 1.0) {
		raw.world_unpack._00 *= raw.scale_world;
		raw.world_unpack._01 *= raw.scale_world;
		raw.world_unpack._02 *= raw.scale_world;
		raw.world_unpack._03 *= raw.scale_world;
		raw.world_unpack._10 *= raw.scale_world;
		raw.world_unpack._11 *= raw.scale_world;
		raw.world_unpack._12 *= raw.scale_world;
		raw.world_unpack._13 *= raw.scale_world;
		raw.world_unpack._20 *= raw.scale_world;
		raw.world_unpack._21 *= raw.scale_world;
		raw.world_unpack._22 *= raw.scale_world;
		raw.world_unpack._23 *= raw.scale_world;
	}

	transform_compute_dim(raw);

	// Update children
	for (let n of raw.object.children) {
		transform_build_matrix(n.transform);
	}

	raw.dirty = false;
}

function transform_translate(raw: transform_t, x: f32, y: f32, z: f32) {
	raw.loc.x += x;
	raw.loc.y += y;
	raw.loc.z += z;
	transform_build_matrix(raw);
}

function transform_set_matrix(raw: transform_t, mat: mat4_t) {
	mat4_set_from(raw.local, mat);
	transform_decompose(raw);
	transform_build_matrix(raw);
}

function transform_mult_matrix(raw: transform_t, mat: mat4_t) {
	mat4_mult_mat(raw.local, mat);
	transform_decompose(raw);
	transform_build_matrix(raw);
}

function transform_decompose(raw: transform_t) {
	mat4_decompose(raw.local, raw.loc, raw.rot, raw.scale);
}

function transform_rotate(raw: transform_t, axis: vec4_t, f: f32) {
	quat_from_axis_angle(_transform_q, axis, f);
	quat_mult_quats(raw.rot, _transform_q, raw.rot);
	transform_build_matrix(raw);
}

function transform_move(raw: transform_t, axis: vec4_t, f = 1.0) {
	vec4_add_f(raw.loc, axis.x * f, axis.y * f, axis.z * f);
	transform_build_matrix(raw);
}

function transform_set_rot(raw: transform_t, x: f32, y: f32, z: f32) {
	quat_from_euler(raw.rot, x, y, z);
	raw._eulerX = x;
	raw._eulerY = y;
	raw._eulerZ = z;
	raw.dirty = true;
}

function transform_compute_radius(raw: transform_t) {
	raw.radius = Math.sqrt(raw.dim.x * raw.dim.x + raw.dim.y * raw.dim.y + raw.dim.z * raw.dim.z);
}

function transform_compute_dim(raw: transform_t) {
	if (raw.object.raw == null) {
		transform_compute_radius(raw);
		return;
	}
	let d = raw.object.raw.dimensions;
	if (d == null) vec4_set(raw.dim, 2 * raw.scale.x, 2 * raw.scale.y, 2 * raw.scale.z);
	else vec4_set(raw.dim, d[0] * raw.scale.x, d[1] * raw.scale.y, d[2] * raw.scale.z);
	transform_compute_radius(raw);
}

function transform_apply_parent_inv(raw: transform_t) {
	let pt = raw.object.parent.transform;
	transform_build_matrix(pt);
	mat4_get_inv(_transform_tmp, pt.world);
	mat4_mult_mat(raw.local, _transform_tmp);
	transform_decompose(raw);
	transform_build_matrix(raw);
}

function transform_apply_parent(raw: transform_t) {
	let pt = raw.object.parent.transform;
	transform_build_matrix(pt);
	mat4_mult_mat(raw.local, pt.world);
	transform_decompose(raw);
	transform_build_matrix(raw);
}

function transform_look(raw: transform_t): vec4_t {
	return mat4_look(raw.world);
}

function transform_right(raw: transform_t): vec4_t {
	return mat4_right(raw.world);
}

function transform_up(raw: transform_t): vec4_t {
	return mat4_up(raw.world);
}

function transform_world_x(raw: transform_t): f32 {
	return raw.world._30;
}

function transform_world_y(raw: transform_t): f32 {
	return raw.world._31;
}

function transform_world_z(raw: transform_t): f32 {
	return raw.world._32;
}
