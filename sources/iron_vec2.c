#include "iron_vec2.h"

#include <math.h>
#include <kinc/math/core.h>

kinc_vector2_t vec2_new(float x, float y) {
	kinc_vector2_t v;
	v.x = x;
	v.y = y;
	return v;
}

void vec2_set(kinc_vector2_t *v, float x, float y) {
	v->x = x;
	v->y = y;
}

float vec2_length(kinc_vector2_t *v) {
	return (float)sqrt(v->x * v->x + v->y * v->y);
}

void vec2_set_length(kinc_vector2_t *v, float length) {
	float current_length = vec2_length(v);
	if (current_length == 0)
		return;
	float mul = length / current_length;
	v->x *= mul;
	v->y *= mul;
}

kinc_vector2_t vec2_sub(kinc_vector2_t *a, kinc_vector2_t *b) {
	kinc_vector2_t v;
	v.x = a->x - b->x;
	v.y = a->y - b->y;
	return v;
}
