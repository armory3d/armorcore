
// .arm file format parser
// msgpack with typed arrays

#pragma once

#include <stdint.h>

void *armpack_decode(void *encoded, uint32_t len);

void armpack_encode_start(void *encoded);
void armpack_encode_map(uint32_t count);
void armpack_encode_array(uint32_t count);
void armpack_encode_array_f32(float *f32, uint32_t count);
void armpack_encode_array_u8(uint8_t *u8, uint32_t count);
void armpack_encode_string(char *str);
void armpack_encode_i32(int32_t i32);
void armpack_encode_f32(float f32);

int armpack_size_map();
int armpack_size_array();
int armpack_size_array_f32(uint32_t count);
int armpack_size_array_u8(uint32_t count);
int armpack_size_string(char *str);
int armpack_size_i32();
int armpack_size_f32();

/* JS object:

	let test = {
		name: "test",
		point: { x: 2, y: 4 },
		array: new Int32Array([1, 2, 3])
	};
*/

/*	C struct:

	typedef struct point {
		int x;
		int y;
	}__attribute__((packed)) point_t;

	typedef struct test {
		char *name;
		point_t point;
		int *array;
		int array_count;
	}__attribute__((packed)) test_t;

*/

/*
	void encode_decode_test() {
		point_t a;
		a.x = 3;
		a.y = 9;

		uint32_t size = 0;
		size += armpack_size_map();
		size += armpack_size_string("x");
		size += armpack_size_i32();
		size += armpack_size_string("y");
		size += armpack_size_i32();

		void *encoded = malloc(size);
		armpack_encode_start(encoded);
		armpack_encode_map(2);
		armpack_encode_i32(a.x);
		armpack_encode_i32(a.y);

		point_t *decoded = armpack_decode(encoded, size);
	}
*/
