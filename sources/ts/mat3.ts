
declare type mat3_t = {
	m?: f32_ptr;
	m00: f32;
	m01: f32;
	m02: f32;
	m10: f32;
	m11: f32;
	m12: f32;
	m20: f32;
	m21: f32;
	m22: f32;
};

declare function mat3_create(_00: f32, _10: f32, _20: f32,
							 _01: f32, _11: f32, _21: f32,
							 _02: f32, _12: f32, _22: f32): mat3_t;
declare function mat3_identity(): mat3_t;
declare function mat3_translation(x: f32, y: f32): mat3_t;
declare function mat3_rotation(alpha: f32): mat3_t;
declare function mat3_set_from4(m4: mat4_t): mat3_t;
declare function mat3_multmat(a: mat3_t, b: mat3_t): mat3_t;
declare function mat3_nan(): mat3_t;
declare function mat3_isnan(m: mat3_t): bool;
