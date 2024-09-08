#pragma once

#include <kinc/math/vector.h>
#include "iron_vec4.h"

vec2_t vec2_new(float x, float y);
vec2_t vec2_create(float x, float y);
float vec2_len(vec2_t v);
vec2_t vec2_set_len(vec2_t v, float length);
vec2_t vec2_sub(vec2_t a, vec2_t b);
vec2_t vec2_nan();
bool vec2_isnan(vec2_t v);
