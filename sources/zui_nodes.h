#pragma once

#include "zui.h"
#include "iron_armpack.h"

typedef struct zui_canvas_control {
	float pan_x;
	float pan_y;
	float zoom;
	bool controls_down;
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
	i32_array_t *nodes_selected_id;
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

void zui_remove_node(zui_node_t *n, zui_node_canvas_t *canvas);

float ZUI_NODES_SCALE();
float ZUI_NODES_PAN_X();
float ZUI_NODES_PAN_Y();
extern char_ptr_array_t *zui_nodes_exclude_remove;
extern bool zui_nodes_socket_released;
extern char **(*zui_nodes_enum_texts)(char *);
extern void (*zui_nodes_on_custom_button)(int, char *);
extern zui_canvas_control_t (*zui_nodes_on_canvas_control)(void);
extern void (*zui_nodes_on_canvas_released)(void);
extern void (*zui_nodes_on_socket_released)(int);
extern void (*zui_nodes_on_link_drag)(int, bool);

void zui_node_canvas_encode(void *encoded, zui_node_canvas_t *canvas);
uint32_t zui_node_canvas_encoded_size(zui_node_canvas_t *canvas);

float ZUI_NODE_X(zui_node_t *node);
float ZUI_NODE_Y(zui_node_t *node);
float ZUI_NODE_W(zui_node_t *node);
float ZUI_NODE_H(zui_node_canvas_t *canvas, zui_node_t *node);
float ZUI_OUTPUT_Y(int sockets_count, int pos);
float ZUI_INPUT_Y(zui_node_canvas_t *canvas, zui_node_socket_t **sockets, int sockets_count, int pos);
float ZUI_OUTPUTS_H(int sockets_count, int length);
float ZUI_BUTTONS_H(zui_node_t *node);
float ZUI_LINE_H();
float ZUI_NODES_PAN_X();
float ZUI_NODES_PAN_Y();

float zui_p(float f);
int zui_get_socket_id(zui_node_array_t *nodes);
zui_node_link_t *zui_get_link(zui_node_link_array_t *links, int id);
int zui_next_link_id(zui_node_link_array_t *links);
zui_node_t *zui_get_node(zui_node_array_t *nodes, int id);
int zui_next_node_id(zui_node_array_t *nodes);
