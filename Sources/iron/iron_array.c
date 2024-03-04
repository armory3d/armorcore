#include "iron_array.h"

#include <stdlib.h>

#ifdef WITH_MINITS
void *gc_realloc(void *ptr, size_t size);
void gc_free(void *ptr);
#else
static void *gc_realloc(void *ptr, size_t size) { return realloc(ptr, size); }
static void gc_free(void *ptr) { free(ptr); }
#endif

void array_free(void *a) {
	u8_array_t *tmp = (u8_array_t *)a;
	gc_free(tmp->buffer);
	tmp->buffer = NULL;
	tmp->length = tmp->capacity = 0;
}

static void array_alloc(void *a, uint8_t element_size) {
	u8_array_t *tmp = (u8_array_t *)a;
	if (tmp->length == tmp->capacity) {
		if (tmp->capacity == 0) {
			tmp->capacity = 1;
		}
		else {
			tmp->capacity *= 2;
		}
		tmp->buffer = gc_realloc(tmp->buffer, tmp->capacity * element_size);
	}
}

void i8_array_push(i8_array_t *a, int8_t e) {
	array_alloc(a, sizeof(int8_t));
	a->buffer[a->length++] = e;
}

void u8_array_push(u8_array_t *a, uint8_t e) {
	array_alloc(a, sizeof(uint8_t));
	a->buffer[a->length++] = e;
}

void i16_array_push(i16_array_t *a, int16_t e) {
	array_alloc(a, sizeof(int16_t));
	a->buffer[a->length++] = e;
}

void u16_array_push(u16_array_t *a, uint16_t e) {
	array_alloc(a, sizeof(uint16_t));
	a->buffer[a->length++] = e;
}

void i32_array_push(i32_array_t *a, int32_t e) {
	array_alloc(a, sizeof(int32_t));
	a->buffer[a->length++] = e;
}

void u32_array_push(u32_array_t *a, uint32_t e) {
	array_alloc(a, sizeof(uint32_t));
	a->buffer[a->length++] = e;
}

void f32_array_push(f32_array_t *a, float e) {
	array_alloc(a, sizeof(float));
	a->buffer[a->length++] = e;
}

void any_array_push(any_array_t *a, void *e) {
	array_alloc(a, sizeof(uintptr_t));
	a->buffer[a->length++] = e;
}

void i8_array_resize(i8_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(int8_t));
}

void u8_array_resize(u8_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(uint8_t));
}

void i16_array_resize(i16_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(int16_t));
}

void u16_array_resize(u16_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(uint16_t));
}

void i32_array_resize(i32_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(int32_t));
}

void u32_array_resize(u32_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(uint32_t));
}

void f32_array_resize(f32_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(float));
}

void any_array_resize(any_array_t *a, int32_t size) {
	a->capacity = size;
	a->buffer = gc_realloc(a->buffer, a->capacity * sizeof(void *));
}

void buffer_resize(buffer_t *b, int32_t size) {
	b->length = size;
	b->data = gc_realloc(b->data, b->length * sizeof(uint8_t));
}

void array_sort(void *ar, void *fn) {

}

void array_push(void *ar, void *e) {

}

void array_splice(void *ar, int32_t start, int32_t delete_count) {

}

void *array_concat(void *a, void *b) {
	return NULL;
}

void *array_slice(void *a, int32_t begin, int32_t end) {
	return NULL;
}

void array_remove(void *ar, void *e) {

}

buffer_t *buffer_slice(buffer_t *a, int32_t begin, int32_t end) {
	return NULL;
}

int32_t buffer_size(buffer_t *b) {
	return b->length;
}

int32_t buffer_view_size(buffer_view_t *v) {
	return v->buffer->length;
}

uint8_t buffer_view_get_u8(buffer_view_t *v, int32_t p) {
	return *(uint8_t *)(v->buffer->data + p);
}

int8_t buffer_view_get_i8(buffer_view_t *v, int32_t p) {
	return *(int8_t *)(v->buffer->data + p);
}

uint16_t buffer_view_get_u16(buffer_view_t *v, int32_t p) {
	return *(uint16_t *)(v->buffer->data + p);
}

int16_t buffer_view_get_i16(buffer_view_t *v, int32_t p) {
	return *(int16_t *)(v->buffer->data + p);
}

uint32_t buffer_view_get_u32(buffer_view_t *v, int32_t p) {
	return *(uint32_t *)(v->buffer->data + p);
}

int32_t buffer_view_get_i32(buffer_view_t *v, int32_t p) {
	return *(int32_t *)(v->buffer->data + p);
}

float buffer_view_get_f32(buffer_view_t *v, int32_t p) {
	return *(float *)(v->buffer->data + p);
}

void buffer_view_set_u8(buffer_view_t *v, int32_t p, uint8_t n) {
	*(uint8_t *)(v->buffer->data + p) = n;
}

void buffer_view_set_i8(buffer_view_t *v, int32_t p, int8_t n) {
	*(int8_t *)(v->buffer->data + p) = n;
}

void buffer_view_set_u16(buffer_view_t *v, int32_t p, uint16_t n) {
	*(uint16_t *)(v->buffer->data + p) = n;
}

void buffer_view_set_i16(buffer_view_t *v, int32_t p, uint16_t n) {
	*(int16_t *)(v->buffer->data + p) = n;
}

void buffer_view_set_u32(buffer_view_t *v, int32_t p, uint32_t n) {
	*(uint32_t *)(v->buffer->data + p) = n;
}

void buffer_view_set_i32(buffer_view_t *v, int32_t p, int32_t n) {
	*(int32_t *)(v->buffer->data + p) = n;
}

void buffer_view_set_f32(buffer_view_t *v, int32_t p, float n) {
	*(float *)(v->buffer->data + p) = n;
}
