#pragma once

#include <kinc/math/vector.h>
#include <kinc/math/quaternion.h>
#include <kinc/math/matrix.h>

kinc_vector4_t vec4_new(float x, float y, float z, float w);
kinc_vector4_t vec4_cross(kinc_vector4_t a, kinc_vector4_t b);
kinc_vector4_t vec4_set(kinc_vector4_t v, float x, float y, float z, float w);
kinc_vector4_t vec4_add(kinc_vector4_t a, kinc_vector4_t b);
kinc_vector4_t vec4_fadd(kinc_vector4_t a, float x, float y, float z, float w);
kinc_vector4_t vec4_normalize(kinc_vector4_t a);
kinc_vector4_t vec4_mult(kinc_vector4_t v, float f);
float vec4_dot(kinc_vector4_t a, kinc_vector4_t b);
kinc_vector4_t vec4_set_from(kinc_vector4_t a, kinc_vector4_t b);
kinc_vector4_t vec4_clone(kinc_vector4_t v);
kinc_vector4_t vec4_lerp(kinc_vector4_t from, kinc_vector4_t to, float s);
kinc_vector4_t vec4_apply_proj(kinc_vector4_t a, kinc_matrix4x4_t m);
kinc_vector4_t vec4_apply_mat(kinc_vector4_t a, kinc_matrix4x4_t m);
kinc_vector4_t vec4_apply_mat4(kinc_vector4_t a, kinc_matrix4x4_t m);
kinc_vector4_t vec4_apply_axis_angle(kinc_vector4_t a, kinc_vector4_t axis, float angle);
kinc_vector4_t vec4_apply_quat(kinc_vector4_t a, kinc_quaternion_t q);
bool vec4_equals(kinc_vector4_t a, kinc_vector4_t b);
bool vec4_almost_equals(kinc_vector4_t a, kinc_vector4_t b, float prec);
float vec4_length(kinc_vector4_t a);
kinc_vector4_t vec4_sub(kinc_vector4_t a, kinc_vector4_t b);
kinc_vector4_t vec4_exp(kinc_vector4_t a);
float vec4_distance(kinc_vector4_t v1, kinc_vector4_t v2);
float vec4_fdistance(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z);
kinc_vector4_t vec4_reflect(kinc_vector4_t a, kinc_vector4_t n);
kinc_vector4_t vec4_clamp(kinc_vector4_t a, float min, float max);
kinc_vector4_t vec4_x_axis();
kinc_vector4_t vec4_y_axis();
kinc_vector4_t vec4_z_axis();
