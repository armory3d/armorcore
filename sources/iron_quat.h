#pragma once

#include <kinc/math/quaternion.h>
#include "iron_vec4.h"

#define quat_t kinc_quaternion_t

quat_t quat_create(float x, float y, float z, float w);
quat_t quat_new(float x, float y, float z, float w);
quat_t quat_from_axis_angle(vec4_t axis, float angle);
quat_t quat_from_mat(mat4_t m);
quat_t quat_from_rot_mat(mat4_t m);
quat_t quat_mult(quat_t a, quat_t b);
quat_t quat_norm(q: quat_t);
quat_t quat_clone(quat_t q);
vec4_t quat_get_euler(quat_t q);
quat_t quat_from_euler(float x, float y, float z);
quat_t quat_lerp(quat_t from, quat_t to, float s);
float quat_dot(quat_t a, quat_t b);
quat_t quat_from_to(vec4_t v0, vec4_t v1);
