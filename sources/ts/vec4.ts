
declare type vec4_t = {
	x: f32;
	y: f32;
	z: f32;
	w: f32;
};

declare function vec4_new(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): vec4_t;
declare function vec4_create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): vec4_t; //
declare function vec4_cross(a: vec4_t, b: vec4_t): vec4_t;
declare function vec4_add(a: vec4_t, b: vec4_t): vec4_t;
declare function vec4_fadd(a: vec4_t, x: f32, y: f32, z: f32, w: f32 = 0.0): vec4_t;
declare function vec4_norm(a: vec4_t): vec4_t;
declare function vec4_mult(v: vec4_t, f: f32): vec4_t;
declare function vec4_dot(a: vec4_t, b: vec4_t): f32;
declare function vec4_clone(v: vec4_t): vec4_t;
declare function vec4_lerp(from: vec4_t, to: vec4_t, s: f32): vec4_t;
declare function vec4_apply_proj(a: vec4_t, m: mat4_t): vec4_t;
declare function vec4_apply_mat(a: vec4_t, m: mat4_t): vec4_t;
declare function vec4_apply_mat4(a: vec4_t, m: mat4_t): vec4_t;
declare function vec4_apply_axis_angle(a: vec4_t, axis: vec4_t, angle: f32): vec4_t;
declare function vec4_apply_quat(a: vec4_t, q: quat_t): vec4_t;
declare function vec4_equals(a: vec4_t, b: vec4_t): bool;
declare function vec4_almost_equals(a: vec4_t, b: vec4_t, prec: f32): bool;
declare function vec4_len(a: vec4_t): f32;
declare function vec4_sub(a: vec4_t, b: vec4_t): vec4_t;
declare function vec4_exp(a: vec4_t): vec4_t;
declare function vec4_dist(v1: vec4_t, v2: vec4_t): f32;
declare function vec4_fdist(v1x: f32, v1y: f32, v1z: f32, v2x: f32, v2y: f32, v2z: f32): f32;
declare function vec4_reflect(a: vec4_t, n: vec4_t): vec4_t;
declare function vec4_clamp(a: vec4_t, min: f32, max: f32): vec4_t;
declare function vec4_x_axis(): vec4_t;
declare function vec4_y_axis(): vec4_t;
declare function vec4_z_axis(): vec4_t;
declare function vec4_nan(): vec4_t;
declare function vec4_isnan(a: vec4_t): bool;
