#pragma once

#include "zui.h"
#include "../iron/iron_armpack.h"

typedef struct zui_canvas_control {
	float pan_x;
	float pan_y;
	float zoom;
} zui_canvas_control_t;

typedef PACK(struct zui_node_socket {
	int id;
	int node_id;
	char *name;
	char *type;
	uint32_t color;
	f32_array_t *default_value;
	float min;
	float max;
	float precision;
	int display;
}) zui_node_socket_t;

typedef PACK(struct zui_node_button {
	char *name;
	char *type;
	int output;
	f32_array_t *default_value;
	u8_array_t *data;
	float min;
	float max;
	float precision;
	float height;
}) zui_node_button_t;

typedef struct zui_node_socket_array {
	zui_node_socket_t **buffer;
	int length;
	int capacity;
} zui_node_socket_array_t;

typedef struct zui_node_button_array {
	zui_node_button_t **buffer;
	int length;
	int capacity;
} zui_node_button_array_t;

typedef PACK(struct zui_node {
	int id;
	char *name;
	char *type;
	int x; // float x;
	int y; // float y;
	uint32_t color;
	zui_node_socket_array_t *inputs;
	zui_node_socket_array_t *outputs;
	zui_node_button_array_t *buttons;
	int width; // float width
}) zui_node_t;

typedef PACK(struct zui_node_link {
	int id;
	int from_id;
	int from_socket;
	int to_id;
	int to_socket;
}) zui_node_link_t;

typedef struct zui_node_array {
	zui_node_t **buffer;
	int length;
	int capacity;
} zui_node_array_t;

typedef struct zui_node_link_array {
	zui_node_link_t **buffer;
	int length;
	int capacity;
} zui_node_link_array_t;

typedef PACK(struct zui_node_canvas {
	char *name;
	zui_node_array_t *nodes;
	zui_node_link_array_t *links;
}) zui_node_canvas_t;

typedef struct zui_nodes {
	bool nodes_drag;
	int nodes_selected_id[32];
	int nodes_selected_count;
	float pan_x;
	float pan_y;
	float zoom;
	int uiw;
	int uih;
	bool _input_started;
	void (*color_picker_callback)(uint32_t);
	void *color_picker_callback_data;
	float scale_factor;
	float ELEMENT_H;
	bool dragged;
	zui_node_t *move_on_top;
	int link_drag_id;
	bool is_new_link;
	int snap_from_id;
	int snap_to_id;
	int snap_socket;
	float snap_x;
	float snap_y;
} zui_nodes_t;

void zui_nodes_init(zui_nodes_t *nodes);
void zui_node_canvas(zui_nodes_t *nodes, zui_node_canvas_t *canvas);
void zui_nodes_rgba_popup(zui_handle_t *nhandle, float *val, int x, int y);
float ZUI_NODES_SCALE();
float ZUI_NODES_PAN_X();
float ZUI_NODES_PAN_Y();
extern char *zui_nodes_exclude_remove[64];
extern bool zui_nodes_socket_released;
extern char **(*zui_nodes_enum_texts)(char *);
extern void (*zui_nodes_on_custom_button)(int, char *);
extern zui_canvas_control_t (*zui_nodes_on_canvas_control)(void);
extern void (*zui_nodes_on_canvas_released)(void);
extern void (*zui_nodes_on_socket_released)(int);
extern void (*zui_nodes_on_link_drag)(int, bool);

void zui_node_canvas_encode(void *encoded, zui_node_canvas_t *canvas);
uint32_t zui_node_canvas_encoded_size(zui_node_canvas_t *canvas);
