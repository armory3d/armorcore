#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct obj_part {
	struct i16_array *posa;
	struct i16_array *nora;
	struct i16_array *texa;
	struct u32_array *inda;
	int vertex_count;
	int index_count;
	float scale_pos;
	float scale_tex;
	char *name;
	bool has_next; // File contains multiple objects
	size_t pos;
	uint32_t **udims; // Indices split per udim tile
	int *udims_count;
	int udims_u; // Number of horizontal udim tiles
	int udims_v;
} obj_part_t;

obj_part_t *io_obj_parse(uint8_t *file_bytes, char split_code, uint32_t start_pos, bool udim);
void io_obj_destroy(obj_part_t *part);
