
#include "iron_armpack.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static int encode_pos;

// iron_string_map keys;

void armpack_decode(void *decoded, void *encoded, uint32_t len) {
	int di = 0;
	int ei = 0;

	while (true) {
		uint8_t u8 = *(uint8_t *)(encoded + ei);
		ei++;

		if (u8 == 0xca) { // f32
			*(float *)(decoded + di) = *(float *)(encoded + ei);
			di += 4;
			ei += 4;
		}
		else if (u8 == 0xd2) { // i32
			*(int32_t *)(decoded + di) = *(int32_t *)(encoded + ei);
			di += 4;
			ei += 4;
		}

		if (ei >= len) break;
	}
}

void armpack_encode_u8(void *encoded, uint8_t u8) {
	*(uint8_t *)(encoded + encode_pos) = u8;
	encode_pos += 1;
}

void armpack_encode_f32(void *encoded, float f32) {
	armpack_encode_u8(encoded, 0xca);
	*(float *)(encoded + encode_pos) = f32;
	encode_pos += 4;
}

void armpack_encode_i32(void *encoded, int32_t i32) {
	armpack_encode_u8(encoded, 0xd2);
	*(int32_t *)(encoded + encode_pos) = i32;
	encode_pos += 4;
}

/*
typedef struct test {
	float f;
	int i;
} test_t;

void encode_test() {
	test_t a;
	a.f = 3.6;
	a.i = 9;
	int type_count = 2;

	encode_pos = 0;
	uint32_t struct_len = sizeof(test_t);
	uint8_t *encoded = (uint8_t *)malloc(struct_len + type_count);
	armpack_encode_f32(encoded, a.f);
	earmpack_ncode_i32(encoded, a.i);

	test_t *decoded = (void *)malloc(struct_len);
	armpack_decode(decoded, encoded, struct_len + type_count);

	printf("%d %d\n", a.i, decoded->i);
}
*/
