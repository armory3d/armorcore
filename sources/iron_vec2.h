#pragma once

#include <kinc/math/vector.h>

kinc_vector2_t vec2_new(float x, float y);
void vec2_set(kinc_vector2_t *v, float x, float y);
float vec2_length(kinc_vector2_t *v);
void vec2_set_length(kinc_vector2_t *v, float length);
kinc_vector2_t vec2_sub(kinc_vector2_t *a, kinc_vector2_t *b);
