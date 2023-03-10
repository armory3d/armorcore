#pragma once

#include <kinc/math/vector.h>
#include <kinc/math/matrix.h>

kinc_matrix4x4_t mat4_perspective_projection(float fovy, float aspect, float zn, float zf);
kinc_matrix4x4_t mat4_look_at(kinc_vector4_t eye, kinc_vector4_t at, kinc_vector4_t up);
kinc_matrix4x4_t mat4_identity(void);
