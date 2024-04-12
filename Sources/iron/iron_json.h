#pragma once

#include "iron_array.h"
#include "iron_map.h"

void *json_parse(char *s);
any_map_t *json_parse_to_map(char *s);
char *json_stringify(void *a);
