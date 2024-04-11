#include "iron_string.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WITH_MINITS
void *gc_alloc(size_t size);
#else
static void *gc_alloc(size_t size) { return calloc(size, sizeof(uint8_t)); }
#endif

char *string_join(char *a, char *b) {
	char *r = gc_alloc(strlen(a) + strlen(b) + 1);
	strcpy(r, a);
	strcat(r, b);
	return r;
}

int string_length(char *str) {
	return strlen(str);
}

bool string_equals(char *a, char *b) {
	if (a == NULL || b == NULL) {
		return false;
	}
	return strcmp(a, b) == 0;
}

char * i32_to_string(int32_t i) {
	int l = snprintf(NULL, 0, "%d", i);
	char *r = gc_alloc(l + 1);
	sprintf(r, "%d", i);
	return r;
}

int32_t string_index_of(char *s, char *search) {
	char *found = strstr(s, search);
	if (found != NULL) {
	    return found - s;
	}
	return -1;
}

int32_t string_last_index_of(char *s, char *search) {
	return -1;
}

any_array_t *string_split(char *s, char *sep) {
	char *r = gc_alloc(strlen(s) + 1);
	strcpy(r, s);
	any_array_t *a = gc_alloc(sizeof(any_array_t));
	char *token = strtok(r, sep);
	while (token != NULL) {
		any_array_push(a, token);
		token = strtok(NULL, sep);
	}
	return a;
}

void string_replace_all(char *s, char *search, char *replace) {

}

char *substring(char *s, int32_t start, int32_t end) {
	return s;
}

char *string_from_char_code(int32_t c) {
	char *r = gc_alloc(2);
	r[0] = c;
	r[1] = '\0';
	return r;
}

int32_t char_code_at(char *s, int32_t i) {
	return s[i];
}

char *char_at(char *s, int32_t i) {
	char *r = gc_alloc(2);
	r[0] = s[i];
	r[1] = '\0';
	return r;
}

bool starts_with(char *s, char *start) {
	return strncmp(start, s, strlen(start)) == 0;
}

bool ends_with(char *s, char *end) {
	size_t len_s = strlen(s);
	size_t len_end = strlen(end);
	return strncmp(s + len_s - len_end, end, len_end) == 0;
}

char *to_lower_case(char *s) {
	char *r = gc_alloc(strlen(s) + 1);
	strcpy(r, s);
	int len = string_length(r);
	for (int i = 0; i < len; ++i) {
		r[i] = tolower(r[i]);
	}
	return r;
}

char *trim_end(char *str) {
	int pos = string_length(str);
	while (pos > 0 && str[pos] == ' ' || str[pos] == '\n') {
		pos--;
	}
	return substring(str, 0, pos);
}
