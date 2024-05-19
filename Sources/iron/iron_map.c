
#include "iron_map.h"

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

void *gc_alloc(size_t size);

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
	any_array_push(m->gc, k); // gc reference
	any_array_push(m->gc, v); // gc reference
}

int32_t i32_map_get(i32_map_t *m, char *k) {
	return shget(m->hash, k);
}

float f32_map_get(f32_map_t *m, char *k) {
	return shget(m->hash, k);
}

void *any_map_get(any_map_t *m, char *k) {
	if (k == NULL) {
		return NULL;
	}
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

i32_map_t *i32_map_create() {
	i32_map_t *r = gc_alloc(sizeof(i32_map_t));
	hmdefault(r->hash, -1);
	return r;
}

any_map_t *any_map_create() {
	return gc_alloc(sizeof(any_map_t));
}

// imap

void i32_imap_set(i32_imap_t *m, int k, int v) {
	hmput(m->hash, k, v);
}

void any_imap_set(any_imap_t *m, int k, void *v) {
	hmput(m->hash, k, v);
	if (m->gc == NULL) {
		 m->gc = gc_alloc(sizeof(any_imap_t));
	}
	any_array_push(m->gc, v); // gc reference
}

int32_t i32_imap_get(i32_imap_t *m, int k) {
	return hmget(m->hash, k);
}

void *any_imap_get(any_imap_t *m, int k) {
	return hmget(m->hash, k);
}

void imap_delete(any_imap_t *m, int k) {
	hmdel(m->hash, k);
}

i32_imap_t *i32_imap_create() {
	return gc_alloc(sizeof(i32_imap_t));
}

any_imap_t *any_imap_create() {
	return gc_alloc(sizeof(any_imap_t));
}
