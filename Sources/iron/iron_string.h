#pragma once

#include <stdint.h>

typedef struct string {
	char *data;
	int length;
} string_t;

void string_init(string_t *s, const char *str);
void string_free(string_t *s);
