
declare type mat4_t = {
	m?: f32_ptr;
	m00: f32;
	m01: f32;
	m02: f32;
	m03: f32;
	m10: f32;
	m11: f32;
	m12: f32;
	m13: f32;
	m20: f32;
	m21: f32;
	m22: f32;
	m23: f32;
	m30: f32;
	m31: f32;
	m32: f32;
	m33: f32;
};

declare type mat4_decomposed_t = {
	loc: vec4_t;
	rot: quat_t;
	scl: vec4_t;
}

declare function mat4_create(_00: f32, _10: f32, _20: f32, _30: f32,
							 _01: f32, _11: f32, _21: f32, _31: f32,
							 _02: f32, _12: f32, _22: f32, _32: f32,
							 _03: f32, _13: f32, _23: f32, _33: f32): mat4_t;
declare function mat4_identity(): mat4_t;
declare function mat4_from_f32_array(a: f32_array_t, offset: i32 = 0): mat4_t;
declare function mat4_persp(fov_y: f32, aspect: f32, zn: f32, zf: f32): mat4_t;
declare function mat4_ortho(left: f32, right: f32, bottom: f32, top: f32, znear: f32, zfar: f32): mat4_t;
declare function mat4_rot_z(alpha: f32): mat4_t;
declare function mat4_compose(loc: vec4_t, rot: quat_t, scl: vec4_t): mat4_t;
declare function mat4_decompose(m: mat4_t): mat4_decomposed_t;
declare function mat4_set_loc(m: mat4_t, v: vec4_t): mat4_t;
declare function mat4_from_quat(q: quat_t): mat4_t;
declare function mat4_init_translate(x: f32, y: f32, z: f32): mat4_t;
declare function mat4_translate(m: mat4_t, x: f32, y: f32, z: f32): mat4_t;
declare function mat4_scale(m: mat4_t, v: vec4_t): mat4_t;
declare function mat4_mult_mat3x4(a: mat4_t, b: mat4_t): mat4_t;
declare function mat4_mult_mat(a: mat4_t, b: mat4_t): mat4_t;
declare function mat4_inv(a: mat4_t): mat4_t;
declare function mat4_transpose(m: mat4_t): mat4_t;
declare function mat4_transpose3x3(m: mat4_t): mat4_t;
declare function mat4_clone(m: mat4_t): mat4_t;
declare function mat4_get_loc(m: mat4_t): vec4_t;
declare function mat4_get_scale(m: mat4_t): vec4_t;
declare function mat4_mult(m: mat4_t, s: f32): mat4_t;
declare function mat4_to_rot(m: mat4_t): mat4_t;
declare function mat4_right(m: mat4_t): vec4_t;
declare function mat4_look(m: mat4_t): vec4_t;
declare function mat4_up(m: mat4_t): vec4_t;
declare function mat4_to_f32_array(m: mat4_t): f32_array_t;
declare function mat4_cofactor(m0: f32, m1: f32, m2: f32, m3: f32, m4: f32, m5: f32, m6: f32, m7: f32, m8: f32): f32;
declare function mat4_determinant(m: mat4_t): f32;
declare function mat4_nan(): mat4_t;
declare function mat4_isnan(m: mat4_t): bool;
declare let mat4nan: mat4_t;

type mat4_box_t = {
	v: mat4_t;
};
