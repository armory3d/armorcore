
declare type vec2_t = {
	x: f32;
	y: f32;
};

declare function vec2_new(x: f32, y: f32): vec2_t;
declare function vec2_create(x: f32, y: f32): vec2_t;
declare function vec2_len(v: vec2_t): f32;
declare function vec2_set_len(v: vec2_t, length: f32): vec2_t;
declare function vec2_mult(v: vec2_t, f: f32): vec2_t;
declare function vec2_add(a: vec2_t, b: vec2_t): vec2_t;
declare function vec2_sub(a: vec2_t, b: vec2_t): vec2_t;
declare function vec2_cross(a: vec2_t, b: vec2_t): f32;
declare function vec2_norm(v: vec2_t): vec2_t;
declare function vec2_dot(a: vec2_t, b: vec2_t): f32;
declare function vec2_nan(): vec2_t;
declare function vec2_isnan(a: vec2_t): bool;
