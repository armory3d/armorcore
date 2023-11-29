#pragma once

#include "zui.h"

typedef struct zui_canvas_control {
	float pan_x;
	float pan_y;
	float zoom;
} zui_canvas_control_t;

typedef struct zui_node_socket {
	int id;
	int node_id;
	char *name;
	char *type;
	uint32_t color;
	void *default_value;
	int default_value_count; // Byte length
	float min;
	float max;
	float precision;
	int display;
}__attribute__((packed)) zui_node_socket_t;

typedef struct zui_node_button {
	char *name;
	char *type;
	int output;
	void *default_value;
	int default_value_count; // Byte length
	void *data;
	int data_count; // Byte length
	float min;
	float max;
	float precision;
	float height;
}__attribute__((packed)) zui_node_button_t;

typedef struct zui_node {
	int id;
	char *name;
	char *type;
	int x; // float x;
	int y; // float y;
	uint32_t color;
	zui_node_socket_t **inputs;
	int inputs_count;
	zui_node_socket_t **outputs;
	int outputs_count;
	zui_node_button_t **buttons;
	int buttons_count;
	int width; // float width
}__attribute__((packed)) zui_node_t;

typedef struct zui_node_link {
	int id;
	int from_id;
	int from_socket;
	int to_id;
	int to_socket;
}__attribute__((packed)) zui_node_link_t;

typedef struct zui_node_canvas {
	char *name;
	zui_node_t **nodes;
	int nodes_capacity; // 128
	int nodes_count;
	zui_node_link_t **links;
	int links_capacity; // 256
	int links_count;
}__attribute__((packed)) zui_node_canvas_t;

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
