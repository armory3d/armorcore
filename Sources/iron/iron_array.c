#include "iron_array.h"

#include <stdlib.h>

void array_free(void *a) {
	u8_array_t *tmp = (u8_array_t *)a;
	free(tmp->data);
	tmp->data = NULL;
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
		tmp->data = realloc(tmp->data, tmp->capacity * element_size);
	}
}

void i8_array_push(i8_array_t *a, int8_t e) {
	array_alloc(a, sizeof(int8_t));
	a->data[a->length++] = e;
}

void u8_array_push(u8_array_t *a, uint8_t e) {
	array_alloc(a, sizeof(uint8_t));
	a->data[a->length++] = e;
}

void i16_array_push(i16_array_t *a, int16_t e) {
	array_alloc(a, sizeof(int16_t));
	a->data[a->length++] = e;
}

void u16_array_push(u16_array_t *a, uint16_t e) {
	array_alloc(a, sizeof(uint16_t));
	a->data[a->length++] = e;
}

void i32_array_push(i32_array_t *a, int32_t e) {
	array_alloc(a, sizeof(int32_t));
	a->data[a->length++] = e;
}

void u32_array_push(u32_array_t *a, uint32_t e) {
	array_alloc(a, sizeof(uint32_t));
	a->data[a->length++] = e;
}

void f32_array_push(f32_array_t *a, float e) {
	array_alloc(a, sizeof(float));
	a->data[a->length++] = e;
}

void any_array_push(any_array_t *a, uintptr_t e) {
	array_alloc(a, sizeof(uintptr_t));
	a->data[a->length++] = e;
}
