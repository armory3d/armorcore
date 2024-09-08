
declare type quat_t = {
	x: f32;
	y: f32;
	z: f32;
	w: f32;
};

declare function quat_create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): quat_t;
declare function quat_new(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): quat_t;
declare function quat_from_axis_angle(axis: vec4_t, angle: f32): quat_t;
declare function quat_from_mat(m: mat4_t): quat_t;
declare function quat_from_rot_mat(m: mat4_t): quat_t;
declare function quat_mult(a: quat_t, b: quat_t): quat_t;
declare function quat_norm(q: quat_t): quat_t;
declare function quat_clone(q: quat_t): quat_t;
declare function quat_get_euler(q: quat_t): vec4_t;
declare function quat_from_euler(x: f32, y: f32, z: f32): quat_t;
declare function quat_lerp(from: quat_t, to: quat_t, s: f32): quat_t;
declare function quat_dot(a: quat_t, b: quat_t): f32;
declare function quat_from_to(v0: vec4_t, v1: vec4_t): quat_t;
