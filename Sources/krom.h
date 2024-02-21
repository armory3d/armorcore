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

#include <stdlib.h>
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

#define krom_log(str) kinc_log(KINC_LOG_LEVEL_INFO, "%f", str)
#define f32 float
#define i32 int32_t
#define u32 uint32_t
#define i16 int16_t
#define u16 uint16_t
#define i8 int8_t
#define u8 uint8_t
#define any void *

#define map_t any
#define buffer_t any
#define buffer_view_t any
#define f32_array_t any
#define u32_array_t any
#define i32_array_t any
#define u16_array_t any
#define i16_array_t any
#define u8_array_t any
#define i8_array_t any
#define null NULL

map_t map_create() { return NULL; }
buffer_t buffer_create(i32 length) { return NULL; }
buffer_view_t buffer_view_create(buffer_t b) { return NULL; }
f32_array_t f32_array_create(i32 length) { return NULL; }
u32_array_t u32_array_create(i32 length) { return NULL; }
i32_array_t i32_array_create(i32 length) { return NULL; }
u16_array_t u16_array_create(i32 length) { return NULL; }
i16_array_t i16_array_create(i32 length) { return NULL; }
u8_array_t u8_array_create(i32 length) { return NULL; }
u8_array_t u8_array_create_from_buffer(buffer_t b) { return NULL; }
i8_array_t i8_array_create(i32 length) { return NULL; }

f32 math_floor(f32 x) { return 0.0; }
f32 math_cos(f32 x) { return 0.0; }
f32 math_sin(f32 x) { return 0.0; }
f32 math_tan(f32 x) { return 0.0; }
f32 math_sqrt(f32 x) { return 0.0; }
f32 math_abs(f32 x) { return 0.0; }
f32 math_random() { return 0.0; }
f32 math_atan2(f32 y, f32 x) { return 0.0; }
f32 math_asin(f32 x) { return 0.0; }
f32 math_pi() { return 0.0; }
f32 math_pow(f32 x, f32 y) { return 0.0; }

// str == str
// str = str
// str + str
// any map_get(map_t m, any k) { return NULL; }
// void map_set(map_t m, any k, any v) { }
// void map_delete(map_t m, any k) { }
// void array_sort(array_t ar, void *fn) { }
// void array_push(array_t ar, any e) { }
// void array_splice(array_t ar, i32 start, i32 delete_count) { }
// i32 string_index_of(string_t *s, string_t *search) { return 0; }
// i32 string_last_index_of(string_t *s, string_t *search) { return 0; }
// array_string_t *string_split(string_t *s, string_t *sep) { return NULL; }
// void string_replace_all(string_t *s, string_t *search, string_t *replace) { }
// string_t *substring(string_t *s, i32 start, i32 end) { return NULL; };
// string_t *string_from_char_code(i32 c) { return NULL; }
// i32 char_code_at(string_t *s, i32 i) { return 0; }
// string_t *char_at(string_t *s, i32 i) { return NULL; }
// bool starts_with(string_t *s, string_t *start) { return false; }
// bool ends_with(string_t *s, string_t *end) { return false; }
// string_t *to_lower_case(string_t *s) { return NULL; }
// array_any_t *map_to_array(map_t *m) { return NULL; }
// array_any_t *array_slice(array_any_t *a, i32 begin, i32 end) { return NULL; }
// buffer_t *buffer_slice(buffer_t *a, i32 begin, i32 end) { return NULL; }
// i32 buffer_size(buffer_t *b) { return 0; }
// i32 buffer_view_size(buffer_view_t *v) { return 0; }
// u8 buffer_view_get_u8(buffer_view_t *v, i32 p) { return 0; }
// i8 buffer_view_get_i8(buffer_view_t *v, i32 p) { return 0; }
// u16 buffer_view_get_u16(buffer_view_t *v, i32 p) { return 0; }
// i16 buffer_view_get_i16(buffer_view_t *v, i32 p) { return 0; }
// u32 buffer_view_get_u32(buffer_view_t *v, i32 p) { return 0; }
// i32 buffer_view_get_i32(buffer_view_t *v, i32 p) { return 0; }
// f32 buffer_view_get_f32(buffer_view_t *v, i32 p) { return 0; }
// void buffer_view_set_u8(buffer_view_t *v, i32 p, u8 n) { }
// void buffer_view_set_i8(buffer_view_t *v, i32 p, i8 n) { }
// void buffer_view_set_u16(buffer_view_t *v, i32 p, u16 n) { }
// void buffer_view_set_i16(buffer_view_t *v, i32 p, i16 n) { }
// void buffer_view_set_u32(buffer_view_t *v, i32 p, u32 n) { }
// void buffer_view_set_i32(buffer_view_t *v, i32 p, i32 n) { }
// void buffer_view_set_f32(buffer_view_t *v, i32 p, f32 n) { }
// bool is_integer(void *a) { return false; } // armpack
// bool is_view(void *a) { return false; } // armpack
// bool is_array(void *a) { return false; } // armpack
// string_t *any_to_string(void *a) { return NULL; } // armpack
// void array_remove(array_any_t *ar, void *e) { }

string_t *trim_end(string_t *str) {
   return NULL;
}

i32 color_from_floats(f32 r, f32 g, f32 b, f32 a) {
    return 0;
}

u8 color_get_rb(i32 c) {
    return 0;
}

u8 color_get_gb(i32 c) {
    return 0;
}

u8 color_get_bb(i32 c) {
    return 0;
}

u8 color_get_ab(i32 c) {
    return 0;
}

i32 color_set_rb(i32 c, u8 i) {
    return 0;
}

i32 color_set_gb(i32 c, u8 i) {
    return 0;
}

i32 color_set_bb(i32 c, u8 i) {
    return 0;
}

i32 color_set_ab(i32 c, u8 i) {
    return 0;
}
