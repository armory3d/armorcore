
#include "iron_map.h"

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#ifdef WITH_MINITS
void *gc_alloc(size_t size);
#else
static void *gc_alloc(size_t size) { return calloc(size, sizeof(uint8_t)); }
#endif

void i32_map_set(i32_map_t *m, char *k, int v) {
	shput(m->hash, k, v);
}

void f32_map_set(f32_map_t *m, char *k, float v) {
	shput(m->hash, k, v);
}

void any_map_set(any_map_t *m, char *k, void *v) {
	shput(m->hash, k, v);
	if (m->gc == NULL) {
		 m->gc = gc_alloc(sizeof(any_map_t));
	}
	any_array_push(m->gc, v); // gc reference
}

int32_t i32_map_get(i32_map_t *m, char *k) {
	return shget(m->hash, k);
}

float f32_map_get(f32_map_t *m, char *k) {
	return shget(m->hash, k);
}

void *any_map_get(any_map_t *m, char *k) {
	return shget(m->hash, k);
}

void map_delete(any_map_t *m, void *k) {
	shdel(m->hash, k);
}

any_array_t *map_keys(any_map_t *m) {
	any_array_t *ar = gc_alloc(sizeof(any_array_t));
	for (int i = 0; i < shlen(m->hash); ++i) {
		any_array_push(ar, m->hash[i].key);
	}
	return ar;
}
