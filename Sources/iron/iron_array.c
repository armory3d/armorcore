#include "iron_array.h"

#include <stdlib.h>
#include <string.h>
#include "iron_string.h"

#ifdef WITH_MINITS
void *gc_alloc(size_t size);
void *gc_realloc(void *ptr, size_t size);
void gc_free(void *ptr);
#else
static void *gc_alloc(size_t size) { return calloc(size, sizeof(uint8_t)); }
static void *gc_realloc(void *ptr, size_t size) { return realloc(ptr, size); }
static void gc_free(void *ptr) { free(ptr); }
#endif

void array_free(void *a) {
	u8_array_t *tmp = (u8_array_t *)a;
	gc_free(tmp->buffer);
	tmp->buffer = NULL;
	tmp->length = tmp->capacity = 0;
}

static void *gc_realloc_no_free(void *ptr, size_t old_size, size_t new_size) {
	void *buffer = gc_alloc(new_size);
	memcpy(buffer, ptr, old_size);
	return buffer;
}

static void array_alloc(void *a, uint8_t element_size) {
	u8_array_t *tmp = (u8_array_t *)a;
	if (tmp->length >= tmp->capacity) {
		if (tmp->capacity == 0) {
			// If the array was created in armpack, length can already be > 0
			tmp->capacity = tmp->length + 1;
			size_t old_size = tmp->length * element_size;
			size_t new_size = tmp->capacity * element_size;
			tmp->buffer = gc_realloc_no_free(tmp->buffer, old_size, new_size);
		}
		else {
			tmp->capacity *= 2;
			tmp->buffer = gc_realloc(tmp->buffer, tmp->capacity * element_size);
		}
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
	b->buffer = gc_realloc(b->buffer, b->length * sizeof(uint8_t));
}

void array_sort(any_array_t *ar, int (*compare)(const void *, const void *)) {
	qsort(ar->buffer, ar->length, sizeof(ar->buffer[0]), compare);
}

void *array_pop(any_array_t *ar) {
	ar->length--;
	return ar->buffer[ar->length];
}

void *array_shift(any_array_t *ar) {
	void *first = ar->buffer[0];
	for (int i = 0; i < ar->length - 1; ++i) {
		ar->buffer[i] = ar->buffer[i + 1];
	}
	ar->length--;
	return first;
}

void array_splice(any_array_t *ar, int32_t start, int32_t delete_count) {
	for (int i = start; i < delete_count; ++i) {
		ar->buffer[i] = ar->buffer[i + delete_count];
	}
	ar->length -= delete_count;
}

any_array_t *array_concat(any_array_t *a, any_array_t *b) {
	any_array_t *ar = gc_alloc(sizeof(any_array_t));
	ar->length = a->length + b->length;
	any_array_resize(ar, ar->length);
	for (int i = 0; i < a->length; ++i) {
		ar->buffer[i] = a->buffer[i];
	}
	for (int i = 0; i < b->length; ++i) {
		ar->buffer[a->length + i] = b->buffer[i];
	}
	return ar;
}

any_array_t *array_slice(any_array_t *a, int32_t begin, int32_t end) {
	any_array_t *ar = gc_alloc(sizeof(any_array_t));
	ar->length = end - begin;
	any_array_resize(ar, ar->length);
	for (int i = 0; i < ar->length; ++i) {
		ar->buffer[i] = a->buffer[begin + i];
	}
	return ar;
}

void array_insert(any_array_t *a, int at, void *e) {
	array_alloc(a, sizeof(uintptr_t));
	a->length++;
	for (int i = a->length; i > at; --i) {
		a->buffer[i] = a->buffer[i - 1];
	}
	a->buffer[at] = e;
}

void array_remove(any_array_t *ar, void *e) {
	array_splice(ar, array_index_of(ar, e), 1);
}

void i32_array_remove(i32_array_t *ar, int e) {
	array_splice((any_array_t *)ar, i32_array_index_of(ar, e), 1);
}

int array_index_of(any_array_t *ar, void *e) {
	for (int i = 0; i < ar->length; ++i) {
		if (ar->buffer[i] == e) {
			return i;
		}
	}
	return -1;
}

int char_ptr_array_index_of(char_ptr_array_t *ar, char *e) {
	for (int i = 0; i < ar->length; ++i) {
		if (string_equals(ar->buffer[i], e)) {
			return i;
		}
	}
	return -1;
}

int i32_array_index_of(i32_array_t *ar, int e) {
	return array_index_of((any_array_t *)ar, (void *)e);
}

void array_reverse(any_array_t *ar) {
	for (int i = 0; i < ar->length / 2; ++i) {
		void *tmp = ar->buffer[i];
		ar->buffer[i] = ar->buffer[ar->length - 1 - i];
		ar->buffer[ar->length - 1 - i] = tmp;
	}
}

buffer_t *buffer_slice(buffer_t *a, int32_t begin, int32_t end) {
	buffer_t *b = gc_alloc(sizeof(buffer_t));
	buffer_resize(b, end - begin);
	for (int i = 0; i < b->length; ++i) {
		b->buffer[i] = a->buffer[begin + i];
	}
	return b;
}

int32_t buffer_size(buffer_t *b) {
	return b->length;
}

int32_t buffer_view_size(buffer_view_t *v) {
	return v->buffer->length;
}

uint8_t buffer_view_get_u8(buffer_view_t *v, int32_t p) {
	return *(uint8_t *)(v->buffer->buffer + p);
}

int8_t buffer_view_get_i8(buffer_view_t *v, int32_t p) {
	return *(int8_t *)(v->buffer->buffer + p);
}

uint16_t buffer_view_get_u16(buffer_view_t *v, int32_t p) {
	return *(uint16_t *)(v->buffer->buffer + p);
}

int16_t buffer_view_get_i16(buffer_view_t *v, int32_t p) {
	return *(int16_t *)(v->buffer->buffer + p);
}

uint32_t buffer_view_get_u32(buffer_view_t *v, int32_t p) {
	return *(uint32_t *)(v->buffer->buffer + p);
}

int32_t buffer_view_get_i32(buffer_view_t *v, int32_t p) {
	return *(int32_t *)(v->buffer->buffer + p);
}

float buffer_view_get_f32(buffer_view_t *v, int32_t p) {
	return *(float *)(v->buffer->buffer + p);
}

int64_t buffer_view_get_i64(buffer_view_t *v, int32_t p) {
	return *(int64_t *)(v->buffer->buffer + p);
}

void buffer_view_set_u8(buffer_view_t *v, int32_t p, uint8_t n) {
	*(uint8_t *)(v->buffer->buffer + p) = n;
}

void buffer_view_set_i8(buffer_view_t *v, int32_t p, int8_t n) {
	*(int8_t *)(v->buffer->buffer + p) = n;
}

void buffer_view_set_u16(buffer_view_t *v, int32_t p, uint16_t n) {
	*(uint16_t *)(v->buffer->buffer + p) = n;
}

void buffer_view_set_i16(buffer_view_t *v, int32_t p, uint16_t n) {
	*(int16_t *)(v->buffer->buffer + p) = n;
}

void buffer_view_set_u32(buffer_view_t *v, int32_t p, uint32_t n) {
	*(uint32_t *)(v->buffer->buffer + p) = n;
}

void buffer_view_set_i32(buffer_view_t *v, int32_t p, int32_t n) {
	*(int32_t *)(v->buffer->buffer + p) = n;
}

void buffer_view_set_f32(buffer_view_t *v, int32_t p, float n) {
	*(float *)(v->buffer->buffer + p) = n;
}

buffer_t *buffer_create(int32_t length) {
	buffer_t * b = gc_alloc(sizeof(buffer_t));
	buffer_resize(b, length);
	return b;
}

buffer_view_t *buffer_view_create(buffer_t *b) {
	buffer_view_t *view = gc_alloc(sizeof(buffer_view_t));
	view->buffer = b;
	return view;
}

f32_array_t *f32_array_create(int32_t length) {
	f32_array_t *a = gc_alloc(sizeof(f32_array_t));
	if (length > 0) {
		f32_array_resize(a, length);
		a->length = length;
	}
	return a;
}

f32_array_t *f32_array_create_from_buffer(buffer_t *b) {
	f32_array_t *a = gc_alloc(sizeof(f32_array_t));
	a->buffer = b->buffer;
	a->length = b->length / 4;
	a->capacity = b->length / 4;
	return a;
}

f32_array_t *f32_array_create_from_array(f32_array_t *from) {
	f32_array_t *a = f32_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

f32_array_t *f32_array_create_from_raw(float *raw, int length) {
	f32_array_t *a = f32_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}

f32_array_t *f32_array_create_x(float x) {
	f32_array_t *a = f32_array_create(1);
	a->buffer[0] = x;
	return a;
}

f32_array_t *f32_array_create_xy(float x, float y) {
	f32_array_t *a = f32_array_create(2);
	a->buffer[0] = x;
	a->buffer[1] = y;
	return a;
}

f32_array_t *f32_array_create_xyz(float x, float y, float z) {
	f32_array_t *a = f32_array_create(3);
	a->buffer[0] = x;
	a->buffer[1] = y;
	a->buffer[2] = z;
	return a;
}

f32_array_t *f32_array_create_xyzw(float x, float y, float z, float w) {
	f32_array_t *a = f32_array_create(4);
	a->buffer[0] = x;
	a->buffer[1] = y;
	a->buffer[2] = z;
	a->buffer[3] = w;
	return a;
}

f32_array_t *f32_array_create_xyzwv(float x, float y, float z, float w, float v) {
	f32_array_t *a = f32_array_create(5);
	a->buffer[0] = x;
	a->buffer[1] = y;
	a->buffer[2] = z;
	a->buffer[3] = w;
	a->buffer[4] = v;
	return a;
}

u32_array_t *u32_array_create(int32_t length) {
	u32_array_t *a = gc_alloc(sizeof(u32_array_t));
	if (length > 0) {
		u32_array_resize(a, length);
		a->length = length;
	}
	return a;
}

u32_array_t *u32_array_create_from_array(u32_array_t *from) {
	u32_array_t *a = u32_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

u32_array_t *u32_array_create_from_raw(uint32_t *raw, int length) {
	u32_array_t *a = u32_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}

i32_array_t *i32_array_create(int32_t length) {
	i32_array_t *a = gc_alloc(sizeof(i32_array_t));
	if (length > 0) {
		i32_array_resize(a, length);
		a->length = length;
	}
	return a;
}

i32_array_t *i32_array_create_from_array(i32_array_t *from) {
	i32_array_t *a = i32_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

i32_array_t *i32_array_create_from_raw(int32_t *raw, int length) {
	i32_array_t *a = i32_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}

u16_array_t *u16_array_create(int32_t length) {
	u16_array_t *a = gc_alloc(sizeof(u16_array_t));
	if (length > 0) {
		u16_array_resize(a, length);
		a->length = length;
	}
	return a;
}

u16_array_t *u16_array_create_from_raw(uint16_t *raw, int length) {
	u16_array_t *a = u16_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}

i16_array_t *i16_array_create(int32_t length) {
	i16_array_t *a = gc_alloc(sizeof(i16_array_t));
	if (length > 0) {
		i16_array_resize(a, length);
		a->length = length;
	}
	return a;
}

i16_array_t *i16_array_create_from_array(i16_array_t *from) {
	i16_array_t *a = i16_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

i16_array_t *i16_array_create_from_raw(int16_t *raw, int length) {
	i16_array_t *a = i16_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}

u8_array_t *u8_array_create(int32_t length) {
	u8_array_t *a = gc_alloc(sizeof(u8_array_t));
	if (length > 0) {
		u8_array_resize(a, length);
		a->length = length;
	}
	return a;
}

u8_array_t *u8_array_create_from_buffer(buffer_t *b) {
	u8_array_t *a = gc_alloc(sizeof(u8_array_t));
	a->buffer = b->buffer;
	a->length = b->length;
	a->capacity = b->length;
	return a;
}

u8_array_t *u8_array_create_from_array(u8_array_t *from) {
	u8_array_t *a = u8_array_create(from->length);
	for (int i = 0; i < from->length; ++i) {
		a->buffer[i] = from->buffer[i];
	}
	return a;
}

u8_array_t *u8_array_create_from_raw(uint8_t *raw, int length) {
	u8_array_t *a = u8_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}

u8_array_t *u8_array_create_from_string(char *s) {
	u8_array_t *a = u8_array_create(strlen(s) + 1);
    for (int i = 0; i < strlen(s); ++i) {
        a->buffer[i] = s[i];
    }
    return a;
}

char *u8_array_to_string(u8_array_t *a) {
	char *r = gc_alloc(a->length);
	memcpy(r, a->buffer, a->length);
    return r;
}

i8_array_t *i8_array_create(int32_t length) {
	i8_array_t *a = gc_alloc(sizeof(i8_array_t));
	if (length > 0) {
		i8_array_resize(a, length);
		a->length = length;
	}
	return a;
}

i8_array_t *i8_array_create_from_raw(int8_t *raw, int length) {
	i8_array_t *a = i8_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}

any_array_t *any_array_create(int32_t length) {
	any_array_t *a = gc_alloc(sizeof(any_array_t));
	if (length > 0) {
		any_array_resize(a, length);
		a->length = length;
	}
	return a;
}

any_array_t *any_array_create_from_raw(void **raw, int length) {
	any_array_t *a = any_array_create(length);
	for (int i = 0; i < length; ++i) {
		a->buffer[i] = raw[i];
	}
	return a;
}
