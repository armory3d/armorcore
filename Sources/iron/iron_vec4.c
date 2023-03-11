#include "iron_vec4.h"

#include <math.h>
#include <kinc/math/core.h>

kinc_vector4_t vec4_new(float x, float y, float z, float w) {
	kinc_vector4_t v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

kinc_vector4_t vec4_cross(kinc_vector4_t a, kinc_vector4_t b) {
	kinc_vector4_t v;
	v.x = a.y * b.z - a.z * b.y;
	v.y = a.z * b.x - a.x * b.z;
	v.z = a.x * b.y - a.y * b.x;
	return v;
}

kinc_vector4_t vec4_set(kinc_vector4_t v, float x, float y, float z, float w) {
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

kinc_vector4_t vec4_add(kinc_vector4_t a, kinc_vector4_t b) {
	kinc_vector4_t v;
	v.x = a.x + b.x;
	v.y = a.y + b.y;
	v.z = a.z + b.z;
	v.w = a.w + b.w;
	return v;
}

kinc_vector4_t vec4_fadd(kinc_vector4_t a, float x, float y, float z, float w) {
	kinc_vector4_t v;
	v.x = a.x + x;
	v.y = a.y + y;
	v.z = a.z + z;
	v.w = a.w + w;
	return v;
}

kinc_vector4_t vec4_normalize(kinc_vector4_t a) {
	float n = vec4_length(a);
	if (n > 0.0) {
		float inv_n = 1.0f / n;
		a.x *= inv_n;
		a.y *= inv_n;
		a.z *= inv_n;
	}
	return a;
}

kinc_vector4_t vec4_mult(kinc_vector4_t v, float f) {
	v.x *= f;
	v.y *= f;
	v.z *= f;
	v.w *= f;
	return v;
}

float vec4_dot(kinc_vector4_t a, kinc_vector4_t b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

kinc_vector4_t vec4_set_from(kinc_vector4_t a, kinc_vector4_t b) {
	a.x = b.x;
	a.y = b.y;
	a.z = b.z;
	a.w = b.w;
	return a;
}

kinc_vector4_t vec4_clone(kinc_vector4_t v) {
	return vec4_new(v.x, v.y, v.z, v.w);
}

kinc_vector4_t vec4_lerp(kinc_vector4_t from, kinc_vector4_t to, float s) {
	kinc_vector4_t v;
	v.x = from.x + (to.x - from.x) * s;
	v.y = from.y + (to.y - from.y) * s;
	v.z = from.z + (to.z - from.z) * s;
	return v;
}

kinc_vector4_t vec4_apply_proj(kinc_vector4_t a, kinc_matrix4x4_t m) {
	kinc_vector4_t v;
	// float d = 1.0 / (m._03 * a.x + m._13 * a.y + m._23 * a.z + m._33); // Perspective divide
	// v.x = (m._00 * a.x + m._10 * a.y + m._20 * a.z + m._30) * d;
	// v.y = (m._01 * a.x + m._11 * a.y + m._21 * a.z + m._31) * d;
	// v.z = (m._02 * a.x + m._12 * a.y + m._22 * a.z + m._32) * d;
	return v;
}

kinc_vector4_t vec4_apply_mat(kinc_vector4_t a, kinc_matrix4x4_t m) {
	kinc_vector4_t v;
	// v.x = m._00 * a.x + m._10 * a.y + m._20 * a.z + m._30;
	// v.y = m._01 * a.x + m._11 * a.y + m._21 * a.z + m._31;
	// v.z = m._02 * a.x + m._12 * a.y + m._22 * a.z + m._32;
	return v;
}

kinc_vector4_t vec4_apply_mat4(kinc_vector4_t a, kinc_matrix4x4_t m) {
	kinc_vector4_t v;
	// v.x = m._00 * a.x + m._10 * a.y + m._20 * a.z + m._30 * a.w;
	// v.y = m._01 * a.x + m._11 * a.y + m._21 * a.z + m._31 * a.w;
	// v.z = m._02 * a.x + m._12 * a.y + m._22 * a.z + m._32 * a.w;
	// v.w = m._03 * a.x + m._13 * a.y + m._23 * a.z + m._33 * a.w;
	return v;
}

kinc_vector4_t vec4_apply_axis_angle(kinc_vector4_t a, kinc_vector4_t axis, float angle) {
	kinc_vector4_t v;
	// kinc_quaternion_t quat = quat_from_axis_angle(axis, angle);
	// return vec4_apply_quat(a, quat);
	return v;
}

kinc_vector4_t vec4_apply_quat(kinc_vector4_t a, kinc_quaternion_t q) {
	kinc_vector4_t v;
	float ix = q.w * a.x + q.y * a.z - q.z * a.y;
	float iy = q.w * a.y + q.z * a.x - q.x * a.z;
	float iz = q.w * a.z + q.x * a.y - q.y * a.x;
	float iw = -q.x * a.x - q.y * a.y - q.z * a.z;
	v.x = ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y;
	v.y = iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z;
	v.z = iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x;
	return v;
}

bool vec4_equals(kinc_vector4_t a, kinc_vector4_t b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool vec4_almost_equals(kinc_vector4_t a, kinc_vector4_t b, float prec) {
	return fabs(a.x - b.x) < prec && fabs(a.y - b.y) < prec && fabs(a.z - b.z) < prec;
}

float vec4_length(kinc_vector4_t a) {
	return (float)sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

kinc_vector4_t vec4_sub(kinc_vector4_t a, kinc_vector4_t b) {
	kinc_vector4_t v;
	v.x = a.x - b.x;
	v.y = a.y - b.y;
	v.z = a.z - b.z;
	return v;
}

kinc_vector4_t vec4_exp(kinc_vector4_t a) {
	kinc_vector4_t v;
	v.x = expf(a.x);
	v.y = expf(a.y);
	v.z = expf(a.z);
	return v;
}

float vec4_distance(kinc_vector4_t v1, kinc_vector4_t v2) {
	return vec4_fdistance(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
}

float vec4_fdistance(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z) {
	float vx = v1x - v2x;
	float vy = v1y - v2y;
	float vz = v1z - v2z;
	return (float)sqrt(vx * vx + vy * vy + vz * vz);
}

kinc_vector4_t vec4_reflect(kinc_vector4_t a, kinc_vector4_t n) {
	kinc_vector4_t v;
	float d = 2 * vec4_dot(a, n);
	v.x = a.x - d * n.x;
	v.y = a.y - d * n.y;
	v.z = a.z - d * n.z;
	return v;
}

kinc_vector4_t vec4_clamp(kinc_vector4_t a, float min, float max) {
	float l = vec4_length(a);
	if (l < min) return vec4_mult(vec4_normalize(a), min);
	else if (l > max) return vec4_mult(vec4_normalize(a), max);
	return a;
}

kinc_vector4_t vec4_x_axis() {
	return vec4_new(1.0, 0.0, 0.0, 1.0);
}

kinc_vector4_t vec4_y_axis() {
	return vec4_new(0.0, 1.0, 0.0, 1.0);
}

kinc_vector4_t vec4_z_axis() {
	return vec4_new(0.0, 0.0, 1.0, 1.0);
}
