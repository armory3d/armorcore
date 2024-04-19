
#include "iron_json.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <jsmn.h>

#ifdef WITH_MINITS
void *gc_alloc(size_t size);
#else
static void *gc_alloc(size_t size) { return calloc(size, sizeof(uint8_t)); }
#endif

static const int PTR_SIZE = 8;
static char *source;
static jsmntok_t *tokens;
static int num_tokens;
static uint32_t ti; // token index
static uint8_t *decoded;
static uint32_t wi; // write index
static uint32_t bottom;
static int array_count;

static void store_u8(uint8_t u8) {
	*(uint8_t *)(decoded + wi) = u8;
	wi += 1;
}

static void store_i32(int32_t i32) {
	*(int32_t *)(decoded + wi) = i32;
	wi += 4;
}

static void store_f32(float f32) {
	*(float *)(decoded + wi) = f32;
	wi += 4;
}

static void store_ptr(uint32_t ptr) {
	*(uint64_t *)(decoded + wi) = (uint64_t)decoded + (uint64_t)ptr;
	wi += PTR_SIZE;
}

static void store_string_bytes(char *str, int len) {
	for (int i = 0; i < len; ++i) {
		store_u8(str[i]);
	}
	store_u8('\0');
}

static bool is_key(char *s, jsmntok_t *t) {
	return t->type == JSMN_STRING && s[t->end + 1] == ':';
}

static int token_size(bool with_contents, bool root) {
	jsmntok_t t = tokens[ti];
	while (is_key(source, &t)) {
		ti++;
		t = tokens[ti];
	}

	if (t.type == JSMN_OBJECT) {
		ti++;
		int size = root ? 0 : PTR_SIZE;
		int size_contents = 0;
		for (int i = 0; i < t.size; ++i) {
			size_contents += token_size(with_contents, false);
		}
		return (with_contents || root) ? size + size_contents : size;
	}
	else if (t.type == JSMN_PRIMITIVE) {
		ti++;
		if (source[t.start] == 't' || source[t.start] == 'f') { // bool
			return 1;
		}
		else if (source[t.start] == 'n') { // null
			return PTR_SIZE;
		}
		else { // number
			return 4;
		}
	}
	else if (t.type == JSMN_ARRAY) {
		ti++;
		// Store in any/i32/../_array_t format
		int size = PTR_SIZE; // Pointer to array struct
		// Put array contents at the bottom
		int size_contents = PTR_SIZE; // Pointer to buffer contents
		size_contents += 4; // Element count
		size_contents += 4; // Capacity
		for (int i = 0; i < t.size; ++i) {
			size_contents += token_size(true, false);
		}
		return with_contents ? size + size_contents : size;
	}
	else if (t.type == JSMN_STRING) {
		ti++;
		// Put string at the bottom and store a pointer to it
		int size = PTR_SIZE; // Pointer to string
		if (with_contents) {
			size += t.end - t.start; // String contents
			size += 1; // '\0'
		}
		return size;
	}

	return 0;
}

static bool has_dot(char *str, int len) {
	for (int i = 0; i < len; ++i) {
		if (str[i] == '.') {
			return true;
		}
	}
	return false;
}

static void token_write(bool root) {
	jsmntok_t t = tokens[ti];
	while (is_key(source, &t)) {
		ti++;
		t = tokens[ti];
	}

	if (t.type == JSMN_OBJECT) {
		if (root) {
			int _ti = ti;
			bottom += token_size(false, true) * array_count;
			ti = _ti;
			ti++;
			for (int i = 0; i < t.size; ++i) {
				token_write(false);
			}
		}
		else {
			store_ptr(bottom);

			uint32_t _wi = wi;
			wi = bottom;
			token_write(true);
			bottom = wi;
			wi = _wi;
		}
	}
	else if (t.type == JSMN_PRIMITIVE) {
		ti++;
		if (source[t.start] == 't' || source[t.start] == 'f') {
			store_u8(source[t.start] == 't' ? 1 : 0);
		}
		else if (source[t.start] == 'n') {
			store_i32(0);
			store_i32(0);
		}
		else {
			has_dot(source + t.start, t.end - t.start) ?
				store_f32(strtof(source + t.start, NULL)) :
				store_i32(strtol(source + t.start, NULL, 10));
		}
	}
	else if (t.type == JSMN_ARRAY) {
		ti++;
		store_ptr(bottom);

		uint32_t _wi = wi;
		wi = bottom;
		store_ptr(wi + PTR_SIZE + 4 + 4); // Pointer to buffer contents
		store_i32(t.size); // Element count
		store_i32(t.size); // Capacity
		bottom = wi;

		if (t.size == 0) {
			wi = _wi;
			return;
		}

		// array_count = t.size;
		for (int i = 0; i < t.size; ++i) {
			token_write(false);
		}
		bottom = wi;
		array_count = 1;
		wi = _wi;
	}
	else if (t.type == JSMN_STRING) {
		ti++;
		store_ptr(bottom);

		uint32_t _wi = wi;
		wi = bottom;
		store_string_bytes(source + t.start, t.end - t.start);
		bottom = wi;
		wi = _wi;
	}
}

void *json_parse(char *s) {
	jsmn_parser parser;
	jsmn_init(&parser);
	num_tokens = jsmn_parse(&parser, s, strlen(s), NULL, 0);

	tokens = malloc(sizeof(jsmntok_t) * num_tokens);
	jsmn_init(&parser);
	jsmn_parse(&parser, s, strlen(s), tokens, num_tokens);

	source = s;
	ti = 0;
	int out_size = token_size(true, true);

	decoded = gc_alloc(out_size);
	ti = 0;
	wi = 0;
	bottom = 0;
	array_count = 1;
	token_write(true);

	free(tokens);
	return decoded;
}

any_map_t *json_parse_to_map(char *s) {
	return NULL;
}

char *json_stringify(void *a) {
	return NULL;
}
