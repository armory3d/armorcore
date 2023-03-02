
#include <kinc/math/vector.h>
#include <kinc/math/quaternion.h>
#include <kinc/math/matrix.h>
#include <kinc/math/core.h>

kinc_vector4_t iron_vec4_new(float x, float y, float z, float w) {
	kinc_vector4_t v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

kinc_vector4_t iron_vec4_cross(kinc_vector4_t v) {
	float ax = x; float ay = y; float az = z;
	float vx = v.x; float vy = v.y; float vz = v.z;
	x = ay * vz - az * vy;
	y = az * vx - ax * vz;
	z = ax * vy - ay * vx;
	return this;
}

kinc_vector4_t iron_vec4_crossvecs(kinc_vector4_t a, kinc_vector4_t b) {
	float ax = a.x; float ay = a.y; float az = a.z;
	float bx = b.x; float by = b.y; float bz = b.z;
	x = ay * bz - az * by;
	y = az * bx - ax * bz;
	z = ax * by - ay * bx;
	return this;
}

kinc_vector4_t iron_vec4_set(float x, float y, float z, float w) {
	this.x = x;
	this.y = y;
	this.z = z;
	this.w = w;
	return this;
}

kinc_vector4_t iron_vec4_add(kinc_vector4_t v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return this;
}

kinc_vector4_t iron_vec4_addf(float x, float y, float z) {
	this.x += x;
	this.y += y;
	this.z += z;
	return this;
}

kinc_vector4_t iron_vec4_addvecs(kinc_vector4_t a, kinc_vector4_t b) {
	x = a.x + b.x;
	y = a.y + b.y;
	z = a.z + b.z;
	return this;
}

kinc_vector4_t iron_vec4_subvecs(kinc_vector4_t a, kinc_vector4_t b) {
	x = a.x - b.x;
	y = a.y - b.y;
	z = a.z - b.z;
	return this;
}

kinc_vector4_t iron_vec4_normalize() {
	float n = iron_vec4_length();
	if (n > 0.0) {
		float inv_n = 1.0 / n;
		this.x *= inv_n;
		this.y *= inv_n;
		this.z *= inv_n;
	}
	return this;
}

kinc_vector4_t iron_vec4_mult(float f) {
	x *= f;
	y *= f;
	z *= f;
	return this;
}

float iron_vec4_dot(kinc_vector4_t v) {
	return x * v.x + y * v.y + z * v.z;
}

kinc_vector4_t iron_vec4_set_from(kinc_vector4_t v) {
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
	return this;
}

kinc_vector4_t iron_vec4_clone() {
	return iron_vec4_new(x, y, z, w);
}

kinc_vector4_t iron_vec4_lerp(kinc_vector4_t from, kinc_vector4_t to, float s) {
	x = from.x + (to.x - from.x) * s;
	y = from.y + (to.y - from.y) * s;
	z = from.z + (to.z - from.z) * s;
	return this;
}

kinc_vector4_t iron_vec4_applyproj(kinc_matrix4x4_t m) {
	float x = this.x; float y = this.y; float z = this.z;
	float d = 1.0 / (m._03 * x + m._13 * y + m._23 * z + m._33); // Perspective divide
	this.x = (m._00 * x + m._10 * y + m._20 * z + m._30) * d;
	this.y = (m._01 * x + m._11 * y + m._21 * z + m._31) * d;
	this.z = (m._02 * x + m._12 * y + m._22 * z + m._32) * d;
	return this;
}

kinc_vector4_t iron_vec4_applymat(kinc_matrix4x4_t m) {
	float x = this.x; float y = this.y; float z = this.z;
	this.x = m._00 * x + m._10 * y + m._20 * z + m._30;
	this.y = m._01 * x + m._11 * y + m._21 * z + m._31;
	this.z = m._02 * x + m._12 * y + m._22 * z + m._32;
	return this;
}

kinc_vector4_t iron_vec4_applymat4(kinc_matrix4x4_t m) {
	float x = this.x; float y = this.y; float z = this.z; float w = this.w;
	this.x = m._00 * x + m._10 * y + m._20 * z + m._30 * w;
	this.y = m._01 * x + m._11 * y + m._21 * z + m._31 * w;
	this.z = m._02 * x + m._12 * y + m._22 * z + m._32 * w;
	this.w = m._03 * x + m._13 * y + m._23 * z + m._33 * w;
	return this;
}

kinc_vector4_t iron_vec4_applyAxisAngle(kinc_vector4_t axis, float angle) {
	kinc_quaternion_t quat = new kinc_quaternion_t();
	quat.from_axis_angle(axis, angle);
	return iron_vec4_applyQuat(quat);
}

kinc_vector4_t iron_vec4_applyQuat(kinc_quaternion_t q) {
	float ix = q.w * x + q.y * z - q.z * y;
	float iy = q.w * y + q.z * x - q.x * z;
	float iz = q.w * z + q.x * y - q.y * x;
	float iw = -q.x * x - q.y * y - q.z * z;
	x = ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y;
	y = iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z;
	z = iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x;
	return this;
}

bool iron_vec4_equals(kinc_vector4_t v) {
	return x == v.x && y == v.y && z == v.z;
}

bool iron_vec4_almost_equals(kinc_vector4_t v, float prec) {
	return kinc_abs(x - v.x) < prec && kinc_abs(y - v.y) < prec && kinc_abs(z - v.z) < prec;
}

float iron_vec4_length() {
	return sqrt(x * x + y * y + z * z);
}

kinc_vector4_t iron_vec4_sub(kinc_vector4_t v) {
	x -= v.x; y -= v.y; z -= v.z;
	return this;
}

kinc_vector4_t iron_vec4_exp(kinc_vector4_t v) {
	x = expf(v.x);
	y = expf(v.y);
	z = expf(v.z);
	return this;
}

float iron_vec4_distance(kinc_vector4_t v1, kinc_vector4_t v2) {
	return distancef(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
}

float iron_vec4_distancef(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z) {
	float vx = v1x - v2x;
	float vy = v1y - v2y;
	float vz = v1z - v2z;
	return sqrt(vx * vx + vy * vy + vz * vz);
}

float iron_vec4_distance_to(kinc_vector4_t p) {
	return sqrt((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y) + (p.z - z) * (p.z - z));
}

kinc_vector4_t iron_vec4_reflect(kinc_vector4_t n) {
	float d = 2 * this.iron_vec4_dot(n);
	x = x - d * n.x;
	y = y - d * n.y;
	z = z - d * n.z;
	return this;
}

kinc_vector4_t iron_vec4_clamp(float min, float max) {
	float l = iron_vec4_length();
	if (l < min) iron_vec4_normalize().iron_vec4_mult(min);
	else if (l > max) iron_vec4_normalize().iron_vec4_mult(max);
	return this;
}

kinc_vector4_t iron_vec4_x_axis() {
	return iron_vec4_new(1.0, 0.0, 0.0);
}

kinc_vector4_t iron_vec4_y_axis() {
	return iron_vec4_new(0.0, 1.0, 0.0);
}

kinc_vector4_t iron_vec4_z_axis() {
	return iron_vec4_new(0.0, 0.0, 1.0);
}

char *iron_vec4_to_string() {
	// return "(" + this.x + ", " + this.y + ", " + this.z + ", " + this.w + ")";
}
