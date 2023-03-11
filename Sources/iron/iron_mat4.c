#include "iron_mat4.h"

#include <math.h>
#include <string.h>
#include "iron_vec4.h"

kinc_matrix4x4_t mat4_perspective_projection(float fovy, float aspect, float zn, float zf) {
	float uh = 1.0f / tanf(fovy / 2);
	float uw = uh / aspect;
	kinc_matrix4x4_t m = {
		uw, 0, 0, 0,
		0, uh, 0, 0,
		0, 0, (zf + zn) / (zn - zf), -1,
		0, 0, 2 * zf * zn / (zn - zf), 0
	};
	return m;
}

kinc_matrix4x4_t mat4_look_at(kinc_vector4_t eye, kinc_vector4_t at, kinc_vector4_t up) {
	kinc_vector4_t zaxis = vec4_normalize(vec4_sub(at, eye));
	kinc_vector4_t xaxis = vec4_normalize(vec4_cross(zaxis, up));
	kinc_vector4_t yaxis = vec4_cross(xaxis, zaxis);
	kinc_matrix4x4_t m = {
		xaxis.x, yaxis.x, -zaxis.x, 0,
		xaxis.y, yaxis.y, -zaxis.y, 0,
		xaxis.z, yaxis.z, -zaxis.z, 0,
		-vec4_dot(xaxis, eye), -vec4_dot(yaxis, eye), vec4_dot(zaxis, eye), 1
	};
	return m;
}

kinc_matrix4x4_t mat4_identity(void) {
	kinc_matrix4x4_t m;
	memset(m.m, 0, sizeof(m.m));
	for (unsigned x = 0; x < 4; ++x) {
		kinc_matrix4x4_set(&m, x, x, 1.0f);
	}
	return m;
}
