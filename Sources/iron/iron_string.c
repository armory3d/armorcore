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

char *i32_to_string(int32_t i) {
	int l = snprintf(NULL, 0, "%d", i);
	char *r = gc_alloc(l + 1);
	sprintf(r, "%d", i);
	return r;
}

char *i32_to_string_hex(int32_t i) {
	int l = snprintf(NULL, 0, "%X", i);
	char *r = gc_alloc(l + 1);
	sprintf(r, "%X", i);
	return r;
}

int32_t string_index_of_pos(char *s, char *search, int pos) {
	char *found = strstr(s + pos, search);
	if (found != NULL) {
	    return found - s;
	}
	return -1;
}

int32_t string_index_of(char *s, char *search) {
	return string_index_of_pos(s, search, 0);
}

int32_t string_last_index_of(char *s, char *search) {
	char *found = NULL;
    while (1) {
        char *p = strstr(s, search);
        if (p == NULL) {
            break;
		}
        found = p;
        s = p + 1;
    }
	if (found != NULL) {
	    return found - s;
	}
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

char *string_array_join(any_array_t *a, char *separator) {
	int len = 0;
	int len_sep = strlen(separator);
	for (int i = 0; i < a->length; ++i) {
		len += strlen(a->buffer[i]);
		if (i < a->length - 1) {
			len += len_sep;
		}
	}

	char *r = gc_alloc(len + 1);
	for (int i = 0; i < a->length; ++i) {
		strcat(r, a->buffer[i]);
		if (i < a->length - 1) {
			strcat(r, separator);
		}
	}
	return r;
}

char *string_replace_all(char *s, char *search, char *replace) {
	char *buffer = gc_alloc(1024);
    char *buffer_pos = buffer;
    size_t search_len = strlen(search);
    size_t replace_len = strlen(replace);
    while (1) {
        char *p = strstr(s, search);
        if (p == NULL) {
            strcpy(buffer_pos, s);
            break;
        }
        memcpy(buffer_pos, s, p - s);
        buffer_pos += p - s;
        memcpy(buffer_pos, replace, replace_len);
        buffer_pos += replace_len;
        s = p + search_len;
    }
    return buffer;
}

char *substring(char *s, int32_t start, int32_t end) {
	char *buffer = gc_alloc(end - start + 1);
	for (int i = 0; i < end - start; ++i) {
		buffer[i] = s[start + i];
	}
	return buffer;
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
