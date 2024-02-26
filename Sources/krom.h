#pragma once

// #include <quickjs.h>
// int main(const int argc, const char **argv) {
//     JSRuntime *runtime = JS_NewRuntime();
//     JSContext *ctx = JS_NewContext(runtime);
//     JSValue result = JS_Eval(ctx, "5+2", 3, "mini.js", JS_EVAL_TYPE_GLOBAL);
//     printf("%d\n", JS_VALUE_GET_INT(result));
//     JS_FreeValue(ctx, result);
//     JS_RunGC(runtime);
//     return 0;
// }

#include <math.h>
#include <stdlib.h>
// #include <string.h> // strcmp
#include <kinc/log.h>
#include <iron/iron_string.h>
#include <iron/iron_array.h>
#include <iron/iron_map.h>

void _globals_init();
void start();

int kickstart(int argc, char **argv) {
    _globals_init();
    start();
    return 0;
}

void *gc_alloc (size_t size) {
    return calloc(size, sizeof(uint8_t));
}

void gc_free() {

}

#define f32 float
#define i32 int32_t
#define u32 uint32_t
#define i16 int16_t
#define u16 uint16_t
#define i8 int8_t
#define u8 uint8_t
#define any void *
#define null NULL

map_t *map_create() { return malloc(sizeof(map_t)); }
buffer_t *buffer_create(i32 length) { return malloc(sizeof(buffer_t)); }
buffer_view_t *buffer_view_create(buffer_t *b) { return malloc(sizeof(buffer_view_t)); }
f32_array_t *f32_array_create(i32 length) { return malloc(sizeof(f32_array_t)); }
u32_array_t *u32_array_create(i32 length) { return malloc(sizeof(u32_array_t)); }
i32_array_t *i32_array_create(i32 length) { return malloc(sizeof(i32_array_t)); }
u16_array_t *u16_array_create(i32 length) { return malloc(sizeof(u16_array_t)); }
i16_array_t *i16_array_create(i32 length) { return malloc(sizeof(i16_array_t)); }
u8_array_t *u8_array_create(i32 length) { return malloc(sizeof(u8_array_t)); }
u8_array_t *u8_array_create_from_buffer(buffer_t *b) { return malloc(sizeof(u8_array_t)); }
i8_array_t *i8_array_create(i32 length) { return malloc(sizeof(i8_array_t)); }

f32 math_floor(f32 x) { return floorf(x); }
f32 math_cos(f32 x) { return cosf(x); }
f32 math_sin(f32 x) { return sinf(x); }
f32 math_tan(f32 x) { return tanf(x); }
f32 math_sqrt(f32 x) { return sqrtf(x); }
f32 math_abs(f32 x) { return fabsf(x); }
f32 math_random() { return rand() / RAND_MAX; }
f32 math_atan2(f32 y, f32 x) { return atan2f(y, x); }
f32 math_asin(f32 x) { return asinf(x); }
f32 math_pi() { return 3.14159265358979323846; }
f32 math_pow(f32 x, f32 y) { return powf(x, y); }

// str == str
// str != str
// str = str
// str + str
any map_get(map_t *m, any k) { return NULL; }
void map_set(map_t *m, any k, any v) { }
void map_delete(map_t *m, any k) { }
void array_sort(any ar, void *fn) { }
void array_push(any ar, any e) { }
void array_splice(any ar, i32 start, i32 delete_count) { }
void *array_concat(any a, any b) { return NULL; }
i32 string_index_of(string_t *s, string_t *search) { return 0; }
i32 string_last_index_of(string_t *s, string_t *search) { return 0; }
void *string_split(string_t *s, string_t *sep) { return NULL; }
void string_replace_all(string_t *s, string_t *search, string_t *replace) { }
string_t *substring(string_t *s, i32 start, i32 end) { return NULL; };
string_t *string_from_char_code(i32 c) { return NULL; }
i32 char_code_at(string_t *s, i32 i) { return 0; }
string_t *char_at(string_t *s, i32 i) { return NULL; }
bool starts_with(string_t *s, string_t *start) { return false; }
bool ends_with(string_t *s, string_t *end) { return false; }
string_t *to_lower_case(string_t *s) { return NULL; }
void *map_to_array(map_t *m) { return NULL; }
void *array_slice(void *a, i32 begin, i32 end) { return NULL; }
buffer_t *buffer_slice(buffer_t *a, i32 begin, i32 end) { return NULL; }
i32 buffer_size(buffer_t *b) { return 0; }
i32 buffer_view_size(buffer_view_t *v) { return 0; }
u8 buffer_view_get_u8(buffer_view_t *v, i32 p) { return 0; }
i8 buffer_view_get_i8(buffer_view_t *v, i32 p) { return 0; }
u16 buffer_view_get_u16(buffer_view_t *v, i32 p) { return 0; }
i16 buffer_view_get_i16(buffer_view_t *v, i32 p) { return 0; }
u32 buffer_view_get_u32(buffer_view_t *v, i32 p) { return 0; }
i32 buffer_view_get_i32(buffer_view_t *v, i32 p) { return 0; }
f32 buffer_view_get_f32(buffer_view_t *v, i32 p) { return 0; }
void buffer_view_set_u8(buffer_view_t *v, i32 p, u8 n) { }
void buffer_view_set_i8(buffer_view_t *v, i32 p, i8 n) { }
void buffer_view_set_u16(buffer_view_t *v, i32 p, u16 n) { }
void buffer_view_set_i16(buffer_view_t *v, i32 p, i16 n) { }
void buffer_view_set_u32(buffer_view_t *v, i32 p, u32 n) { }
void buffer_view_set_i32(buffer_view_t *v, i32 p, i32 n) { }
void buffer_view_set_f32(buffer_view_t *v, i32 p, f32 n) { }
bool is_integer(void *a) { return false; } // armpack
bool is_view(void *a) { return false; } // armpack
bool is_array(void *a) { return false; } // armpack
string_t *any_to_string(void *a) { return NULL; } // armpack
void array_remove(void *ar, void *e) { }

string_t *trim_end(string_t *str) {
   return NULL;
}

i32 color_from_floats(f32 r, f32 g, f32 b, f32 a) {
    return ((int)(a * 255) << 24) | ((int)(r * 255) << 16) | ((int)(g * 255) << 8) | (int)(b * 255);
}

u8 color_get_rb(i32 c) {
    return (c & 0x00ff0000) >> 16;
}

u8 color_get_gb(i32 c) {
    return (c & 0x0000ff00) >> 8;
}

u8 color_get_bb(i32 c) {
    return c & 0x000000ff;
}

u8 color_get_ab(i32 c) {
    return c & 0x000000ff;
}

i32 color_set_rb(i32 c, u8 i) {
    return (color_get_ab(c) << 24) | (i << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

i32 color_set_gb(i32 c, u8 i) {
    return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (i << 8) | color_get_bb(c);
}

i32 color_set_bb(i32 c, u8 i) {
    return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | i;
}

i32 color_set_ab(i32 c, u8 i) {
    return (i << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}
