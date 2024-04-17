#pragma once

#include <stdint.h>
#include "iron_array.h"

typedef struct i32_map {
	struct { char *key; int value; } *hash;
} i32_map_t;

typedef struct f32_map {
	struct { char *key; float value; } *hash;
} f32_map_t;

typedef struct any_map {
	struct { char *key; void *value; } *hash;
	any_array_t *gc; // gc reference
} any_map_t;

void i32_map_set(i32_map_t *m, char *k, int v);
void f32_map_set(f32_map_t *m, char *k, float v);
void any_map_set(any_map_t *m, char *k, void *v);

int32_t i32_map_get(i32_map_t *m, char *k);
float f32_map_get(f32_map_t *m, char *k);
void *any_map_get(any_map_t *m, char *k);

void map_delete(any_map_t *m, void *k);
any_array_t *map_keys(any_map_t *m);

i32_map_t *i32_map_create();
any_map_t *any_map_create();
