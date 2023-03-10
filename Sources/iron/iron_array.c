#include "iron_array.h"

#include <stdlib.h>

static void array_free(void *a) {
	array_u8_t *tmp = (array_u8_t *)a;
	free(tmp->data);
	tmp->data = NULL;
	tmp->length = tmp->capacity = 0;
}

void array_u8_free(array_u8_t *a) {
	array_free(a);
}

void array_i16_free(array_i16_t *a) {
	array_free(a);
}

void array_i32_free(array_i32_t *a) {
	array_free(a);
}

void array_u32_free(array_u32_t *a) {
	array_free(a);
}

void array_f32_free(array_f32_t *a) {
	array_free(a);
}

static void array_alloc(void *a, uint8_t element_size) {
	array_u8_t *tmp = (array_u8_t *)a;
	if (tmp->length == tmp->capacity) {
		if (tmp->capacity == 0) tmp->capacity = 1;
		else tmp->capacity *= 2;
		tmp->data = realloc(tmp->data, tmp->capacity * element_size);
	}
}

void array_u8_push(array_u8_t *a, uint8_t e) {
	array_alloc(a, sizeof(uint8_t));
	a->data[a->length++] = e;
}

void array_i16_push(array_i16_t *a, int16_t e) {
	array_alloc(a, sizeof(int16_t));
	a->data[a->length++] = e;
}

void array_i32_push(array_i32_t *a, int32_t e) {
	array_alloc(a, sizeof(int32_t));
	a->data[a->length++] = e;
}

void array_u32_push(array_u32_t *a, uint32_t e) {
	array_alloc(a, sizeof(uint32_t));
	a->data[a->length++] = e;
}

void array_f32_push(array_f32_t *a, float e) {
	array_alloc(a, sizeof(float));
	a->data[a->length++] = e;
}
