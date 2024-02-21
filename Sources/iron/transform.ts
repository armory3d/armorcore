
type transform_t = {
	world?: mat4_t;
	local_only?: bool;
	local?: mat4_t;
	loc?: vec4_t;
	rot?: quat_t;
	scale?: vec4_t;
	scale_world?: f32;
	world_unpack?: mat4_t;
	dirty?: bool;
	object?: object_t;
	dim?: vec4_t;
	radius?: f32;

	bone_parent?: mat4_t;
	last_world?: mat4_t;

	// Wrong order returned from get_euler(), store last state for animation
	_euler_x?: f32;
	_euler_y?: f32;
	_euler_z?: f32;

	// Animated delta transform
	dloc?: vec4_t;
	drot?: quat_t;
	dscale?: vec4_t;
	_deuler_x?: f32;
	_deuler_y?: f32;
	_deuler_z?: f32;

	type?: string;
};

let _transform_tmp: mat4_t = mat4_identity();
let _transform_q: quat_t = quat_create();

function transform_create(object: object_t): transform_t {
	let raw: transform_t = {};
	raw.type = "transform_t";
	raw.local_only = false;
	raw.scale_world = 1.0;
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
	if (raw.dirty) {
		transform_build_matrix(raw);
	}
}

function transform_compose_delta(raw: transform_t) {
	// Delta transform
	vec4_add_vecs(raw.dloc, raw.loc, raw.dloc);
	vec4_add_vecs(raw.dscale, raw.dscale, raw.scale);
	quat_from_euler(raw.drot, raw._deuler_x, raw._deuler_y, raw._deuler_z);
	quat_mult_quats(raw.drot, raw.rot, raw.drot);
	mat4_compose(raw.local, raw.dloc, raw.drot, raw.dscale);
}

function transform_build_matrix(raw: transform_t) {
	raw.dloc == null ? mat4_compose(raw.local, raw.loc, raw.rot, raw.scale) : transform_compose_delta(raw);

	if (raw.bone_parent != null) {
		mat4_mult_mats(raw.local, raw.bone_parent, raw.local);
	}

	if (raw.object.parent != null && !raw.local_only) {
		mat4_mult_mats3x4(raw.world, raw.local, raw.object.parent.transform.world);
	}
	else {
		mat4_set_from(raw.world, raw.local);
	}

	mat4_set_from(raw.world_unpack, raw.world);
	if (raw.scale_world != 1.0) {
		raw.world_unpack.m[0] *= raw.scale_world;
		raw.world_unpack.m[1] *= raw.scale_world;
		raw.world_unpack.m[2] *= raw.scale_world;
		raw.world_unpack.m[3] *= raw.scale_world;
		raw.world_unpack.m[4] *= raw.scale_world;
		raw.world_unpack.m[5] *= raw.scale_world;
		raw.world_unpack.m[6] *= raw.scale_world;
		raw.world_unpack.m[7] *= raw.scale_world;
		raw.world_unpack.m[8] *= raw.scale_world;
		raw.world_unpack.m[9] *= raw.scale_world;
		raw.world_unpack.m[10] *= raw.scale_world;
		raw.world_unpack.m[11] *= raw.scale_world;
	}

	transform_compute_dim(raw);

	// Update children
	for (let i: i32 = 0; i < raw.object.children.length; ++i) {
		let n: object_t = raw.object.children[i];
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

function transform_move(raw: transform_t, axis: vec4_t, f: f32 = 1.0) {
	vec4_add_f(raw.loc, axis.x * f, axis.y * f, axis.z * f);
	transform_build_matrix(raw);
}

function transform_set_rot(raw: transform_t, x: f32, y: f32, z: f32) {
	quat_from_euler(raw.rot, x, y, z);
	raw._euler_x = x;
	raw._euler_y = y;
	raw._euler_z = z;
	raw.dirty = true;
}

function transform_compute_radius(raw: transform_t) {
	raw.radius = math_sqrt(raw.dim.x * raw.dim.x + raw.dim.y * raw.dim.y + raw.dim.z * raw.dim.z);
}

function transform_compute_dim(raw: transform_t) {
	if (raw.object.raw == null) {
		transform_compute_radius(raw);
		return;
	}
	let d: f32_array_t = raw.object.raw.dimensions;
	if (d == null) {
		vec4_set(raw.dim, 2 * raw.scale.x, 2 * raw.scale.y, 2 * raw.scale.z);
	}
	else {
		vec4_set(raw.dim, d[0] * raw.scale.x, d[1] * raw.scale.y, d[2] * raw.scale.z);
	}
	transform_compute_radius(raw);
}

function transform_apply_parent_inv(raw: transform_t) {
	let pt: transform_t = raw.object.parent.transform;
	transform_build_matrix(pt);
	mat4_get_inv(_transform_tmp, pt.world);
	mat4_mult_mat(raw.local, _transform_tmp);
	transform_decompose(raw);
	transform_build_matrix(raw);
}

function transform_apply_parent(raw: transform_t) {
	let pt: transform_t = raw.object.parent.transform;
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
	return raw.world.m[12];
}

function transform_world_y(raw: transform_t): f32 {
	return raw.world.m[13];
}

function transform_world_z(raw: transform_t): f32 {
	return raw.world.m[14];
}
