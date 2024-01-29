#include "iron_string.h"

#include <stdlib.h>
#include <string.h>

void string_init(string_t *s, const char *str) {
	s->data = realloc(s->data, strlen(str));
	strcpy(s->data, str);
}

void string_free(string_t *s) {
	free(s->data);
	s->data = NULL;
	s->length = 0;
}
