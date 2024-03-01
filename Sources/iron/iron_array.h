#pragma once

#include <stdint.h>

typedef struct i8_array {
	int8_t *buffer;
	int length;
	int capacity;
} i8_array_t;

typedef struct u8_array {
	uint8_t *buffer;
	int length;
	int capacity;
} u8_array_t;

typedef struct i16_array {
	int16_t *buffer;
	int length;
	int capacity;
} i16_array_t;

typedef struct u16_array {
	uint16_t *buffer;
	int length;
	int capacity;
} u16_array_t;

typedef struct i32_array {
	int32_t *buffer;
	int length;
	int capacity;
} i32_array_t;

typedef struct u32_array {
	uint32_t *buffer;
	int length;
	int capacity;
} u32_array_t;

typedef struct f32_array {
	float *buffer;
	int length;
	int capacity;
} f32_array_t;

typedef struct any_array {
	void **buffer;
	int length;
	int capacity;
} any_array_t;

typedef struct char_ptr_array {
	char **buffer;
	int length;
	int capacity;
} char_ptr_array_t;

typedef struct buffer {
	uint8_t *data;
	int length;
} buffer_t;

typedef struct buffer_view {
	buffer_t *buffer;
} buffer_view_t;

void array_free(void *a);
void i8_array_push(i8_array_t *a, int8_t e);
void u8_array_push(u8_array_t *a, uint8_t e);
void i16_array_push(i16_array_t *a, int16_t e);
void u16_array_push(u16_array_t *a, uint16_t e);
void i32_array_push(i32_array_t *a, int32_t e);
void u32_array_push(u32_array_t *a, uint32_t e);
void f32_array_push(f32_array_t *a, float e);
void any_array_push(any_array_t *a, void *e);

void i8_array_resize(i8_array_t *a, int32_t size);
void u8_array_resize(u8_array_t *a, int32_t size);
void i16_array_resize(i16_array_t *a, int32_t size);
void u16_array_resize(u16_array_t *a, int32_t size);
void i32_array_resize(i32_array_t *a, int32_t size);
void u32_array_resize(u32_array_t *a, int32_t size);
void f32_array_resize(f32_array_t *a, int32_t size);
void any_array_resize(any_array_t *a, int32_t size);
void buffer_resize(buffer_t *b, int32_t size);

void array_sort(void *ar, void *fn);
void array_push(void *ar, void *e);
void array_splice(void *ar, int32_t start, int32_t delete_count);
void *array_concat(void *a, void *b);
void *array_slice(void *a, int32_t begin, int32_t end);
void array_remove(void *ar, void *e);

buffer_t *buffer_slice(buffer_t *a, int32_t begin, int32_t end);
int32_t buffer_size(buffer_t *b);
int32_t buffer_view_size(buffer_view_t *v);
uint8_t buffer_view_get_u8(buffer_view_t *v, int32_t p);
int8_t buffer_view_get_i8(buffer_view_t *v, int32_t p);
uint16_t buffer_view_get_u16(buffer_view_t *v, int32_t p);
int16_t buffer_view_get_i16(buffer_view_t *v, int32_t p);
uint32_t buffer_view_get_u32(buffer_view_t *v, int32_t p);
int32_t buffer_view_get_i32(buffer_view_t *v, int32_t p);
float buffer_view_get_f32(buffer_view_t *v, int32_t p);
void buffer_view_set_u8(buffer_view_t *v, int32_t p, uint8_t n);
void buffer_view_set_i8(buffer_view_t *v, int32_t p, int8_t n);
void buffer_view_set_u16(buffer_view_t *v, int32_t p, uint16_t n);
void buffer_view_set_i16(buffer_view_t *v, int32_t p, uint16_t n);
void buffer_view_set_u32(buffer_view_t *v, int32_t p, uint32_t n);
void buffer_view_set_i32(buffer_view_t *v, int32_t p, int32_t n);
void buffer_view_set_f32(buffer_view_t *v, int32_t p, float n);
