#pragma once

#include <stdint.h>

typedef struct i8_array {
	int8_t *data;
	int length;
	int capacity;
} i8_array_t;

typedef struct u8_array {
	uint8_t *data;
	int length;
	int capacity;
} u8_array_t;

typedef struct i16_array {
	int16_t *data;
	int length;
	int capacity;
} i16_array_t;

typedef struct u16_array {
	uint16_t *data;
	int length;
	int capacity;
} u16_array_t;

typedef struct i32_array {
	int32_t *data;
	int length;
	int capacity;
} i32_array_t;

typedef struct u32_array {
	uint32_t *data;
	int length;
	int capacity;
} u32_array_t;

typedef struct f32_array {
	float *data;
	int length;
	int capacity;
} f32_array_t;

typedef struct any_array {
	uintptr_t *data;
	int length;
	int capacity;
} any_array_t;

typedef struct buffer {} buffer_t;
typedef struct buffer_view {} buffer_view_t;

void array_free(void *a);
void i8_array_push(i8_array_t *a, int8_t e);
void u8_array_push(u8_array_t *a, uint8_t e);
void i16_array_push(i16_array_t *a, int16_t e);
void u16_array_push(u16_array_t *a, uint16_t e);
void i32_array_push(i32_array_t *a, int32_t e);
void u32_array_push(u32_array_t *a, uint32_t e);
void f32_array_push(f32_array_t *a, float e);
void any_array_push(any_array_t *a, uintptr_t e);
