
#include "iron_armpack.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static const int PTR_SIZE = 8;
static uint32_t di; // Decoded index
static uint32_t ei; // Encoded index
static uint32_t bottom; // Decoded bottom
static void *decoded;
static void *encoded;
static uint32_t capacity;
static uint32_t string_length;
static uint32_t array_count;
static void read_store();

static void store_u8(uint8_t u8) {
	*(uint8_t *)(decoded + di) = u8;
	di += 1;
}

static void store_i16(int16_t i16) {
	*(int16_t *)(decoded + di) = i16;
	di += 4;
}

static void store_i32(int32_t i32) {
	*(int32_t *)(decoded + di) = i32;
	di += 4;
}

static void store_f32(float f32) {
	*(float *)(decoded + di) = f32;
	di += 4;
}

static void store_ptr(uint32_t ptr) {
	*(uint64_t *)(decoded + di) = (uint64_t)decoded + (uint64_t)ptr;
	di += PTR_SIZE;
}

static void store_string_bytes(char *str) {
	for (int i = 0; i < string_length; ++i) {
		store_u8(str[i]);
	}
	store_u8('\0');
}

static void store_string(char *str) {
	// Put string at the bottom and store a pointer to it
	store_ptr(bottom);
	uint32_t _di = di;
	di = bottom;
	store_string_bytes(str);
	bottom = di;
	di = _di;
}

static uint8_t read_u8() {
	uint8_t u8 = *(uint8_t *)(encoded + ei);
	ei += 1;
	return u8;
}

static int16_t read_i16() {
	int16_t i16 = *(int16_t *)(encoded + ei);
	ei += 4;
	return i16;
}

static int32_t read_i32() {
	int32_t i32 = *(int32_t *)(encoded + ei);
	ei += 4;
	return i32;
}

static uint32_t read_u32() {
	uint32_t u32 = *(uint32_t *)(encoded + ei);
	ei += 4;
	return u32;
}

static float read_f32() {
	float f32 = *(float *)(encoded + ei);
	ei += 4;
	return f32;
}

static char *read_string() {
	string_length = read_u32();
	char *str = (char *)(encoded + ei);
	ei += string_length;
	return str;
}

static uint32_t traverse() {
	uint8_t flag = read_u8();
	switch (flag) {
	case 0xc2: // false
		ei += 1;
		return 1;
	case 0xc3: // true
		ei += 1;
		return 1;
	case 0xca: // f32
		ei += 4;
		return 4;
	case 0xd2: // i32
		ei += 4;
		return 4;
	case 0xdf: { // map
		uint32_t len = 0;
		int count = read_i32();
		for (int i = 0; i < count; ++i) {
			read_u8(); // 0xdb string
			read_string(); // key
			len += traverse(); // value
		}
		return len;
	}
	case 0xdb: // string
		ei += read_u32(); // string_length
		return PTR_SIZE;
	case 0xdd: { // array
		int count = read_i32();
		uint8_t flag2 = read_u8();
		switch (flag2) {
		case 0xca: // Typed f32
			ei += 4 * count;
			break;
		case 0xd2: // Typed i32
			ei += 4 * count;
			break;
		case 0xd1: // Typed i16
			ei += 2 * count;
			break;
		default: // Dynamic type-value
			ei -= 1;
			for (int j = 0; j < count; ++j) {
				traverse();
			}
		}
		return PTR_SIZE + 4; // ptr + u32 element count
	}
	default:
		return 0;
	}
}

static uint32_t get_struct_length() {
	uint32_t _ei = ei;
	uint32_t len = traverse();
	ei = _ei;
	return len;
}

static void read_store_map(int count) {
	ei -= 5; // u8 map, i32 count
	bottom += get_struct_length() * array_count;
	ei += 5;
	for (int i = 0; i < count; ++i) {
		read_u8(); // 0xdb string
		read_string(); // key
		read_store(); // value
	}
}

static bool is_typed_array(uint8_t flag) {
	return flag == 0xca || flag == 0xd2 || flag == 0xd1;
}

static uint8_t flag_to_byte_size(uint8_t flag) {
	if (flag == 0xca) return 4; // f32
	if (flag == 0xd2) return 4; // i32
	if (flag == 0xd1) return 2; // i16
	return 0;
}

static void store_typed_array(uint8_t flag, uint32_t count) {
	uint32_t size = flag_to_byte_size(flag) * count;
	memcpy(decoded + di, encoded + ei, size);
	ei += size;
	di += size;
}

static void read_store_array(int count) {
	// Put array contents at the bottom
	// Store pointers to array elements
	// Store element count
	store_ptr(bottom);
	store_i32(count);

	if (count == 0) {
		return;
	}

	uint32_t _di = di;
	di = bottom;

	uint8_t flag = read_u8();
	if (is_typed_array(flag)) {
		store_typed_array(flag, count);
		bottom = di;
	}
	// Dynamic (type - value)
	else {
		ei -= 1;
		array_count = count;

		// Strings
		if (flag == 0xdb) {
			// String pointers
			uint32_t _ei = ei;
			uint32_t strings_length = 0;
			for (int i = 0; i < count; ++i) {
				store_ptr(bottom + count * PTR_SIZE + strings_length);
				if (i < count -1) {
					ei += 1; // String flag
					uint32_t length = read_u32(); // String length
					ei += length;
					strings_length += length;
					strings_length += 1; // '\0'
				}
			}
			ei = _ei;

			// String bytes
			for (int i = 0; i < count; ++i) {
				ei += 1; // String flag
				store_string_bytes(read_string());
			}
			bottom = di;
		}
		// Structs
		else {
			uint32_t size = get_struct_length();
			for (int i = 0; i < count; ++i) {
				store_ptr(bottom + count * PTR_SIZE + i * size);
			}

			bottom = di;
			for (int i = 0; i < count; ++i) {
				read_store();
			}
		}

		array_count = 1;
	}

	di = _di;
}

static void read_store() {
	uint8_t flag = read_u8();
	switch (flag) {
	case 0xc0:
		// store_i32(0); // NULL
		break;
	case 0xc2:
		store_u8(false);
		break;
	case 0xc3:
		store_u8(true);
		break;
	case 0xca:
		store_f32(read_f32());
		break;
	case 0xd2:
		store_i32(read_i32());
		break;
	case 0xdf:
		read_store_map(read_i32());
		break;
	case 0xdb:
		store_string(read_string());
		break;
	case 0xdd:
		read_store_array(read_i32());
		break;
	}
}

void *armpack_decode(void *_encoded, uint32_t len) {
	capacity = len * 2;
	decoded = malloc(capacity);
	encoded = _encoded;
	di = 0;
	ei = 0;
	bottom = 0;
	array_count = 1;
	read_store();
	return decoded;
}

void armpack_encode_start(void *_encoded) {
	encoded = _encoded;
	ei = 0;
}

static void armpack_write_u8(uint8_t u8) {
	*(uint8_t *)(encoded + ei) = u8;
	ei += 1;
}

static void armpack_write_i32(int32_t i32) {
	*(int32_t *)(encoded + ei) = i32;
	ei += 4;
}

static void armpack_write_f32(float f32) {
	*(float *)(encoded + ei) = f32;
	ei += 4;
}

void armpack_encode_map(uint32_t count) {
	armpack_write_u8(0xdf);
	armpack_write_i32(count);
}

void armpack_encode_array(uint32_t count) {
	armpack_write_u8(0xdd);
	armpack_write_i32(count);
}

void armpack_encode_array_f32(float *f32, uint32_t count) {
	armpack_write_u8(0xdd);
	armpack_write_i32(count);
	armpack_write_u8(0xca);
	for (int i = 0; i < count; ++i) {
		armpack_write_f32(f32[i]);
	}
}

void armpack_encode_string(char *str) {
	armpack_write_u8(0xdb);
	size_t len = strlen(str);
	armpack_write_i32(len);
	for (int i = 0; i < len; ++i) {
		armpack_write_u8(str[i]);
	}
}

void armpack_encode_i32(int32_t i32) {
	armpack_write_u8(0xd2);
	armpack_write_i32(i32);
}

void armpack_encode_f32(float f32) {
	armpack_write_u8(0xca);
	armpack_write_f32(f32);
}

int armpack_size_map() {
	return 1 + 4; // u8 tag + i32 count
}

int armpack_size_array() {
	return 1 + 4; // u8 tag + i32 count
}

int armpack_size_array_f32(uint32_t count) {
	return 1 + 4 + 1 + count * 4; // u8 tag + i32 count + u8 flag + f32* contents
}

int armpack_size_string(char *str) {
	return 1 + 4 + strlen(str); // u8 tag + i32 length + contents
}

int armpack_size_i32() {
	return 1 + 4; // u8 tag + i32
}

int armpack_size_f32() {
	return 1 + 4; // u8 tag + f32
}
