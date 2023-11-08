#pragma once

#include <stdint.h>

typedef struct array_u8 {
	uint8_t *data;
	int length;
	int capacity;
} array_u8_t;

typedef struct array_i16 {
	int16_t *data;
	int length;
	int capacity;
} array_i16_t;

typedef struct array_i32 {
	int32_t *data;
	int length;
	int capacity;
} array_i32_t;

typedef struct array_u32 {
	uint32_t *data;
	int length;
	int capacity;
} array_u32_t;

typedef struct array_f32 {
	float *data;
	int length;
	int capacity;
} array_f32_t;

typedef struct array_u64 {
	uint64_t *data;
	int length;
	int capacity;
} array_u64_t;

void array_u8_free(array_u8_t *a);
void array_i16_free(array_i16_t *a);
void array_i32_free(array_i32_t *a);
void array_u32_free(array_u32_t *a);
void array_f32_free(array_f32_t *a);
void array_u64_free(array_u64_t *a);

void array_u8_push(array_u8_t *a, uint8_t e);
void array_i16_push(array_i16_t *a, int16_t e);
void array_i32_push(array_i32_t *a, int32_t e);
void array_u32_push(array_u32_t *a, uint32_t e);
void array_f32_push(array_f32_t *a, float e);
void array_u64_push(array_u64_t *a, uint64_t e);
