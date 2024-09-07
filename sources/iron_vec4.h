#pragma once

#include <kinc/math/vector.h>
#include <kinc/math/quaternion.h>
#include <kinc/math/matrix.h>

#define vec4_t kinc_vector4_t

vec4_t vec4_new(float x, float y, float z, float w);
vec4_t vec4_create(float x, float y, float z, float w); //
vec4_t vec4_cross(vec4_t a, vec4_t b);
vec4_t vec4_add(vec4_t a, vec4_t b);
vec4_t vec4_fadd(vec4_t a, float x, float y, float z, float w);
vec4_t vec4_norm(vec4_t a);
vec4_t vec4_mult(vec4_t v, float f);
float vec4_dot(vec4_t a, vec4_t b);
vec4_t vec4_clone(vec4_t v);
vec4_t vec4_lerp(vec4_t from, vec4_t to, float s);
vec4_t vec4_apply_proj(vec4_t a, kinc_matrix4x4_t *m);
vec4_t vec4_apply_mat(vec4_t a, kinc_matrix4x4_t *m);
vec4_t vec4_apply_mat4(vec4_t a, kinc_matrix4x4_t *m);
vec4_t vec4_apply_axis_angle(vec4_t a, vec4_t axis, float angle);
vec4_t vec4_apply_quat(vec4_t a, kinc_quaternion_t *q);
bool vec4_equals(vec4_t a, vec4_t b);
bool vec4_almost_equals(vec4_t a, vec4_t b, float prec);
float vec4_len(vec4_t a);
vec4_t vec4_sub(vec4_t a, vec4_t b);
vec4_t vec4_exp(vec4_t a);
float vec4_dist(vec4_t v1, vec4_t v2);
float vec4_fdist(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z);
vec4_t vec4_reflect(vec4_t a, vec4_t n);
vec4_t vec4_clamp(vec4_t a, float min, float max);
vec4_t vec4_x_axis();
vec4_t vec4_y_axis();
vec4_t vec4_z_axis();
vec4_t vec4_nan();
bool vec4_isnan(vec4_t v);
