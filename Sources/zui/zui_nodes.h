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
	int color;
	void *default_value;
	float min; // optional
	float max; // optional
	float precision; // optional
	int display; // optional
	char *tooltip; // optional
} zui_node_socket_t;

typedef struct zui_node_button {
	char *name;
	char *type;
	int output; // optional
	void *default_value; // optional
	void *data; // optional
	float min; // optional
	float max; // optional
	float precision; // optional
	float height; // optional
	char *tooltip; // optional
} zui_node_button_t;

typedef struct zui_node {
	int id;
	char *name;
	char *type;
	float x;
	float y;
	zui_node_socket_t **inputs;
	int inputs_count;
	zui_node_socket_t **outputs;
	int outputs_count;
	zui_node_button_t **buttons;
	int buttons_count;
	int color;
	float width; // optional
	char *tooltip; // optional
} zui_node_t;

typedef struct zui_node_link {
	int id;
	int from_id;
	int from_socket;
	int to_id;
	int to_socket;
} zui_node_link_t;

typedef struct zui_node_canvas {
	char *name;
	zui_node_t **nodes;
	int nodes_count;
	zui_node_link_t **links;
	int links_count;
} zui_node_canvas_t;

typedef struct zui_nodes {
	bool nodes_drag;
	zui_node_t **nodes_selected; // = [];
	int nodes_selected_count;
	float pan_x;
	float pan_y;
	float zoom;
	int uiw;
	int uih;
	bool _input_started;
	void (*color_picker_callback)(int);
	float scale_factor;
	float ELEMENT_H;
	bool dragged;
	zui_node_t *move_on_top;
	zui_node_link_t *link_drag;
	bool is_new_link;
	int snap_from_id;
	int snap_to_id;
	int snap_socket;
	float snap_x;
	float snap_y;
} zui_nodes_t;

void zui_nodes_init(zui_nodes_t *nodes);
void zui_node_canvas(zui_node_canvas_t *canvas);
