
// .arm file format parser
// msgpack with typed arrays

#pragma once

#include <stdint.h>

void armpack_decode(void *decoded, void *encoded, uint32_t len);
void armpack_encode_u8(void *encoded, uint8_t u8);
void armpack_encode_f32(void *encoded, float f32);
void armpack_encode_i32(void *encoded, int32_t i32);
