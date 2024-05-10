#include "zui_nodes.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/input/keyboard.h>
#include "zui.h"
#include "zui_ext.h"
#include "../g2/g2.h"
#include "../g2/g2_ext.h"
#include "../iron/iron_armpack.h"

static zui_nodes_t *current_nodes = NULL;
static bool zui_nodes_elements_baked = false;
static kinc_g4_render_target_t zui_socket_image;
static char *zui_clipboard = "";
static bool zui_box_select = false;
static int zui_box_select_x = 0;
static int zui_box_select_y = 0;
static const int zui_max_buttons = 9;
static zui_handle_t handle;
static void (*zui_on_header_released)(zui_node_t *) = NULL;
static void (*zui_nodes_on_node_remove)(zui_node_t *) = NULL;
static int zui_node_id = -1;

char_ptr_array_t *zui_nodes_exclude_remove = NULL; // No removal for listed node types
bool zui_nodes_socket_released = false;
char **(*zui_nodes_enum_texts)(char *) = NULL; // Retrieve combo items for buttons of type ENUM
void (*zui_nodes_on_custom_button)(int, char *) = NULL; // Call external function
zui_canvas_control_t (*zui_nodes_on_canvas_control)(void) = NULL;
void (*zui_nodes_on_canvas_released)(void) = NULL;
void (*zui_nodes_on_socket_released)(int) = NULL;
void (*zui_nodes_on_link_drag)(int, bool) = NULL;

int zui_popup_x = 0;
int zui_popup_y = 0;
int zui_popup_w = 0;
int zui_popup_h = 0;
void (*zui_popup_commands)(zui_t *, void *, void *) = NULL;
void *zui_popup_data;
void *zui_popup_data2;
int zui_popup_handle_node_id = -1;
int zui_popup_handle_node_socket_id = -1;

char *zui_tr(char *id/*, map<char *, char *> vars*/) {
	return id;
}

void zui_nodes_init(zui_nodes_t *nodes) {
	current_nodes = nodes;
	memset(current_nodes, 0, sizeof(zui_nodes_t));
	current_nodes->zoom = 1.0;
	current_nodes->scale_factor = 1.0;
	current_nodes->ELEMENT_H = 25.0;
	current_nodes->snap_from_id = -1;
	current_nodes->snap_to_id = -1;
	current_nodes->link_drag_id = -1;
	current_nodes->nodes_selected_id = malloc(sizeof(i32_array_t));
}

float ZUI_NODES_SCALE() {
	return current_nodes->scale_factor * current_nodes->zoom;
}

float ZUI_NODES_PAN_X() {
	float zoom_pan = (1.0 - current_nodes->zoom) * current_nodes->uiw / 2.5;
	return current_nodes->pan_x * ZUI_NODES_SCALE() + zoom_pan;
}

float ZUI_NODES_PAN_Y() {
	float zoom_pan = (1.0 - current_nodes->zoom) * current_nodes->uih / 2.5;
	return current_nodes->pan_y * ZUI_NODES_SCALE() + zoom_pan;
}

float ZUI_LINE_H() {
	return current_nodes->ELEMENT_H * ZUI_NODES_SCALE();
}

float ZUI_BUTTONS_H(zui_node_t *node) {
	float h = 0.0;
	for (int i = 0; i < node->buttons->length; ++i) {
		zui_node_button_t *but = node->buttons->buffer[i];
		if (strcmp(but->type, "RGBA") == 0) {
			h += 102.0 * ZUI_NODES_SCALE() + ZUI_LINE_H() * 5.0; // Color wheel + controls
		}
		else if (strcmp(but->type, "VECTOR") == 0) {
			h += ZUI_LINE_H() * 4.0;
		}
		else if (strcmp(but->type, "CUSTOM") == 0) {
			h += ZUI_LINE_H() * but->height;
		}
		else {
			h += ZUI_LINE_H();
		}
	}
	return h;
}

float ZUI_OUTPUTS_H(int sockets_count, int length) {
	float h = 0.0;
	for (int i = 0; i < (length < 0 ? sockets_count : length); ++i) {
		h += ZUI_LINE_H();
	}
	return h;
}

bool zui_input_linked(zui_node_canvas_t *canvas, int node_id, int i) {
	for (int x = 0; x < canvas->links->length; ++x) {
		zui_node_link_t *l = canvas->links->buffer[x];
		if (l->to_id == node_id && l->to_socket == i) {
			return true;
		}
	}
	return false;
}

float ZUI_INPUTS_H(zui_node_canvas_t *canvas, zui_node_socket_t **sockets, int sockets_count, int length) {
	float h = 0.0;
	for (int i = 0; i < (length < 0 ? sockets_count : length); ++i) {
		if (strcmp(sockets[i]->type, "VECTOR") == 0 && sockets[i]->display == 1 && !zui_input_linked(canvas, sockets[i]->node_id, i)) {
			h += ZUI_LINE_H() * 4;
		}
		else {
			h += ZUI_LINE_H();
		}
	}
	return h;
}

float ZUI_NODE_H(zui_node_canvas_t *canvas, zui_node_t *node) {
	return ZUI_LINE_H() * 1.2 + ZUI_INPUTS_H(canvas, node->inputs->buffer, node->inputs->length, -1) + ZUI_OUTPUTS_H(node->outputs->length, -1) + ZUI_BUTTONS_H(node);
}

float ZUI_NODE_W(zui_node_t *node) {
	return (node->width != 0 ? node->width : 140.0) * ZUI_NODES_SCALE();
}

float ZUI_NODE_X(zui_node_t *node) {
	return node->x * ZUI_NODES_SCALE() + ZUI_NODES_PAN_X();
}

float ZUI_NODE_Y(zui_node_t *node) {
	return node->y * ZUI_NODES_SCALE() + ZUI_NODES_PAN_Y();
}

float ZUI_INPUT_Y(zui_node_canvas_t *canvas, zui_node_socket_t **sockets, int sockets_count, int pos) {
	return ZUI_LINE_H() * 1.62 + ZUI_INPUTS_H(canvas, sockets, sockets_count, pos);
}

float ZUI_OUTPUT_Y(int sockets_count, int pos) {
	return ZUI_LINE_H() * 1.62 + ZUI_OUTPUTS_H(sockets_count, pos);
}

float zui_p(float f) {
	return f * ZUI_NODES_SCALE();
}

zui_node_t *zui_get_node(zui_node_array_t *nodes, int id) {
	for (int i = 0; i < nodes->length; ++i) if (nodes->buffer[i]->id == id) return nodes->buffer[i];
	return NULL;
}

int zui_get_node_index(zui_node_t **nodes, int nodes_count, int id) {
	for (int i = 0; i < nodes_count; ++i) if (nodes[i]->id == id) return i;
	return -1;
}

int zui_next_node_id(zui_node_array_t *nodes) {
	if (zui_node_id == -1) {
		for (int i = 0; i < nodes->length; ++i) {
			if (zui_node_id < nodes->buffer[i]->id) {
				zui_node_id = nodes->buffer[i]->id;
			}
		}
	}
	return ++zui_node_id;
}

zui_node_link_t *zui_get_link(zui_node_link_array_t *links, int id) {
	for (int i = 0; i < links->length; ++i) {
		if (links->buffer[i]->id == id) {
			return links->buffer[i];
		}
	}
	return NULL;
}

int zui_get_link_index(zui_node_link_array_t *links, int id) {
	for (int i = 0; i < links->length; ++i) {
		if (links->buffer[i]->id == id) {
			return i;
		}
	}
	return -1;
}

int zui_next_link_id(zui_node_link_array_t *links) {
	int id = 0;
	for (int i = 0; i < links->length; ++i) {
		if (links->buffer[i]->id >= id) {
			id = links->buffer[i]->id + 1;
		}
	}
	return id;
}

int zui_get_socket_id(zui_node_array_t *nodes) {
	int id = 0;
	for (int i = 0; i < nodes->length; ++i) {
		zui_node_t *n = nodes->buffer[i];
		for (int j = 0; j < n->inputs->length; ++j) {
			if (n->inputs->buffer[j]->id >= id) {
				id = n->inputs->buffer[j]->id + 1;
			}
		}
		for (int j = 0; j < n->outputs->length; ++j) {
			if (n->outputs->buffer[j]->id >= id) {
				id = n->outputs->buffer[j]->id + 1;
			}
		}
	}
	return id;
}

void zui_nodes_bake_elements() {
	if (zui_socket_image.width != 0) {
		kinc_g4_render_target_destroy(&zui_socket_image);
	}
	kinc_g4_render_target_init(&zui_socket_image, 24, 24, KINC_G4_RENDER_TARGET_FORMAT_32BIT, 0, 0);
	arm_g2_set_render_target(&zui_socket_image);
	kinc_g4_clear(KINC_G4_CLEAR_COLOR, 0x00000000, 0, 0);
	arm_g2_set_color(0xff000000);
	arm_g2_fill_circle(12, 12, 11, 0);
	arm_g2_set_color(0xffffffff);
	arm_g2_fill_circle(12, 12, 9, 0);
	arm_g2_restore_render_target();
	zui_nodes_elements_baked = true;
}

zui_canvas_control_t zui_on_default_canvas_control() {
	zui_t *current = zui_get_current();
	zui_canvas_control_t c;
	c.pan_x = current->input_down_r ? current->input_dx : 0.0;
	c.pan_y = current->input_down_r ? current->input_dy : 0.0;
	c.zoom = -current->input_wheel_delta / 10.0;
	return c;
}

void zui_draw_link(float x1, float y1, float x2, float y2, bool highlight) {
	zui_t *current = zui_get_current();
	int c1 = current->ops->theme->LABEL_COL;
	int c2 = current->ops->theme->ACCENT_SELECT_COL;
	int c = highlight ? c1 : c2;
	arm_g2_set_color(zui_color(zui_color_r(c), zui_color_g(c), zui_color_b(c), 210));
	if (current->ops->theme->LINK_STYLE == ZUI_LINK_STYLE_LINE) {
		arm_g2_draw_line(x1, y1, x2, y2, 1.0);
		arm_g2_set_color(zui_color(zui_color_r(c), zui_color_g(c), zui_color_b(c), 150)); // AA
		arm_g2_draw_line(x1 + 0.5, y1, x2 + 0.5, y2, 1.0);
		arm_g2_draw_line(x1 - 0.5, y1, x2 - 0.5, y2, 1.0);
		arm_g2_draw_line(x1, y1 + 0.5, x2, y2 + 0.5, 1.0);
		arm_g2_draw_line(x1, y1 - 0.5, x2, y2 - 0.5, 1.0);
	}
	else if (current->ops->theme->LINK_STYLE == ZUI_LINK_STYLE_CUBIC_BEZIER) {
		float x[] = { x1, x1 + fabs(x1 - x2) / 2.0, x2 - fabs(x1 - x2) / 2.0, x2 };
		float y[] = { y1, y1, y2, y2 };
		arm_g2_draw_cubic_bezier(x, y, 30, highlight ? 2.0 : 1.0);
	}
}

static void zui_remove_link_at(zui_node_canvas_t *canvas, int at) {
	canvas->links->buffer[at] = NULL;
	for (int i = at; i < canvas->links->length - 1; ++i) {
		canvas->links->buffer[i] = canvas->links->buffer[i + 1];
	}
	canvas->links->length--;
}

static void zui_remove_node_at(zui_node_canvas_t *canvas, int at) {
	canvas->nodes->buffer[at] = NULL;
	for (int i = at; i < canvas->nodes->length - 1; ++i) {
		canvas->nodes->buffer[i] = canvas->nodes->buffer[i + 1];
	}
	canvas->nodes->length--;
}

void zui_remove_node(zui_node_t *n, zui_node_canvas_t *canvas) {
	if (n == NULL) return;
	int i = 0;
	while (i < canvas->links->length) {
		zui_node_link_t *l = canvas->links->buffer[i];
		if (l->from_id == n->id || l->to_id == n->id) {
			zui_remove_link_at(canvas, i);
		}
		else i++;
	}
	zui_remove_node_at(canvas, zui_get_node_index(canvas->nodes->buffer, canvas->nodes->length, n->id));
	if (zui_nodes_on_node_remove != NULL) {
		(*zui_nodes_on_node_remove)(n);
	}
}

bool zui_is_selected(zui_node_t *node) {
	for (int i = 0; i < current_nodes->nodes_selected_id->length; ++i) {
		if (current_nodes->nodes_selected_id->buffer[i] == node->id) {
			return true;
		}
	}
	return false;
}

static void remove_from_selection(zui_node_t *node) {
	i32_array_remove(current_nodes->nodes_selected_id, node->id);
}

static void add_to_selection(zui_node_t *node) {
	i32_array_push(current_nodes->nodes_selected_id, node->id);
}

void zui_popup(int x, int y, int w, int h, void (*commands)(zui_t *, void *, void *), void *data, void *data2) {
	zui_popup_x = x;
	zui_popup_y = y;
	zui_popup_w = w;
	zui_popup_h = h;
	zui_popup_commands = commands;
	zui_popup_data = data;
	zui_popup_data2 = data2;
}

static void color_picker_callback(uint32_t color) {
	zui_t *current = zui_get_current();
	float *val = (float *)current_nodes->color_picker_callback_data;
	val[0] = zui_color_r(color) / 255.0f;
	val[1] = zui_color_g(color) / 255.0f;
	val[2] = zui_color_b(color) / 255.0f;
	current->changed = true;
}

void zui_color_wheel_picker(void *data) {
	current_nodes->color_picker_callback_data = data;
	current_nodes->color_picker_callback = &color_picker_callback;
}

static void rgba_popup_commands(zui_t *ui, void *data, void *data2) {
	zui_handle_t *nhandle = (zui_handle_t *)data;
	float *val = (float *)data2;
	nhandle->color = zui_color(val[0] * 255.0, val[1] * 255.0, val[2] * 255.0, 255.0);
	zui_color_wheel(nhandle, false, -1, -1, true, &zui_color_wheel_picker, val);
	val[0] = zui_color_r(nhandle->color) / 255.0f;
	val[1] = zui_color_g(nhandle->color) / 255.0f;
	val[2] = zui_color_b(nhandle->color) / 255.0f;
}

void zui_nodes_rgba_popup(zui_handle_t *nhandle, float *val, int x, int y) {
	zui_t *current = zui_get_current();
	zui_popup(x, y, 140.0 * current_nodes->scale_factor, current->ops->theme->ELEMENT_H * 10.0, &rgba_popup_commands, nhandle, val);
}

static char enum_label[32];
static char enum_texts_data[32][64];
static char *enum_texts[32];

void zui_draw_node(zui_node_t *node, zui_node_canvas_t *canvas) {
	zui_t *current = zui_get_current();
	float wx = current->_window_x;
	float wy = current->_window_y;
	float ui_x = current->_x;
	float ui_y = current->_y;
	float ui_w = current->_w;
	float w = ZUI_NODE_W(node);
	float h = ZUI_NODE_H(canvas, node);
	float nx = ZUI_NODE_X(node);
	float ny = ZUI_NODE_Y(node);
	char *text = zui_tr(node->name);
	float lineh = ZUI_LINE_H();

	// Disallow input if node is overlapped by another node
	current_nodes->_input_started = current->input_started;
	if (current->input_started) {
		for (int i = zui_get_node_index(canvas->nodes->buffer, canvas->nodes->length, node->id) + 1; i < canvas->nodes->length; ++i) {
			zui_node_t *n = canvas->nodes->buffer[i];
			if (ZUI_NODE_X(n) < current->input_x - current->_window_x && ZUI_NODE_X(n) + ZUI_NODE_W(n) > current->input_x - current->_window_x &&
				ZUI_NODE_Y(n) < current->input_y - current->_window_y && ZUI_NODE_Y(n) + ZUI_NODE_H(canvas, n) > current->input_y - current->_window_y) {
				current->input_started = false;
				break;
			}
		}
	}

	// Outline
	arm_g2_set_color(zui_is_selected(node) ? current->ops->theme->LABEL_COL : current->ops->theme->CONTEXT_COL);
	zui_draw_rect(true, nx - 1, ny - 1, w + 2, h + 2);

	// Body
	arm_g2_set_color(current->ops->theme->WINDOW_BG_COL);
	zui_draw_rect(true, nx, ny, w, h);

	// Header line
	arm_g2_set_color(node->color);
	arm_g2_fill_rect(nx, ny + lineh - zui_p(3), w, zui_p(3));

	// Title
	arm_g2_set_color(current->ops->theme->LABEL_COL);
	float textw = arm_g2_string_width(current->ops->font, current->font_size, text);
	arm_g2_draw_string(text, nx + zui_p(10), ny + zui_p(6));
	ny += lineh * 0.5;

	// Outputs
	for (int i = 0; i < node->outputs->length; ++i) {
		zui_node_socket_t *out = node->outputs->buffer[i];
		ny += lineh;
		arm_g2_set_color(out->color);
		arm_g2_draw_scaled_render_target(&zui_socket_image, nx + w - zui_p(6), ny - zui_p(3), zui_p(12), zui_p(12));
	}
	ny -= lineh * node->outputs->length;
	arm_g2_set_color(current->ops->theme->LABEL_COL);
	for (int i = 0; i < node->outputs->length; ++i) {
		zui_node_socket_t *out = node->outputs->buffer[i];
		ny += lineh;
		float strw = arm_g2_string_width(current->ops->font, current->font_size, zui_tr(out->name));
		arm_g2_draw_string(zui_tr(out->name), nx + w - strw - zui_p(12), ny - zui_p(3));

		if (zui_nodes_on_socket_released != NULL && current->input_enabled && (current->input_released || current->input_released_r)) {
			if (current->input_x > wx + nx && current->input_x < wx + nx + w && current->input_y > wy + ny && current->input_y < wy + ny + lineh) {
				zui_nodes_on_socket_released(out->id);
				zui_nodes_socket_released = true;
			}
		}
	}

	// Buttons
	zui_handle_t *nhandle = zui_nest(&handle, node->id);
	ny -= lineh / 3.0; // Fix align
	for (int buti = 0; buti < node->buttons->length; ++buti) {
		zui_node_button_t *but = node->buttons->buffer[buti];

		if (strcmp(but->type, "RGBA") == 0) {
			ny += lineh; // 18 + 2 separator
			current->_x = nx + 1; // Offset for node selection border
			current->_y = ny;
			current->_w = w;
			float *val = node->outputs->buffer[but->output]->default_value->buffer;
			nhandle->color = zui_color(val[0] * 255.0, val[1] * 255.0, val[2] * 255.0, 255.0);
			zui_color_wheel(nhandle, false, -1, -1, true, &zui_color_wheel_picker, val);
			val[0] = zui_color_r(nhandle->color) / 255.0f;
			val[1] = zui_color_g(nhandle->color) / 255.0f;
			val[2] = zui_color_b(nhandle->color) / 255.0f;
		}
		else if (strcmp(but->type, "VECTOR") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			float min = but->min;
			float max = but->max;
			float text_off = current->ops->theme->TEXT_OFFSET;
			current->ops->theme->TEXT_OFFSET = 6;
			zui_text(zui_tr(but->name), ZUI_ALIGN_LEFT, 0);
			float *val = (float *)but->default_value->buffer;

			zui_handle_t *h = zui_nest(nhandle, buti);
			zui_handle_t *h0 = zui_nest(h, 0);
			if (h0->init) h0->value = val[0];
			zui_handle_t *h1 = zui_nest(h, 1);
			if (h1->init) h1->value = val[1];
			zui_handle_t *h2 = zui_nest(h, 2);
			if (h2->init) h2->value = val[2];

			val[0] = zui_slider(h0, "X", min, max, true, 100, true, ZUI_ALIGN_LEFT, true);
			val[1] = zui_slider(h1, "Y", min, max, true, 100, true, ZUI_ALIGN_LEFT, true);
			val[2] = zui_slider(h2, "Z", min, max, true, 100, true, ZUI_ALIGN_LEFT, true);
			current->ops->theme->TEXT_OFFSET = text_off;
			if (but->output >= 0) {
				node->outputs->buffer[but->output]->default_value->buffer = but->default_value->buffer;
			}
			ny += lineh * 3.0;
		}
		else if (strcmp(but->type, "VALUE") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			zui_node_socket_t *soc = node->outputs->buffer[but->output];
			float min = but->min;
			float max = but->max;
			float prec = but->precision;
			float text_off = current->ops->theme->TEXT_OFFSET;
			current->ops->theme->TEXT_OFFSET = 6;
			zui_handle_t *soc_handle = zui_nest(nhandle, buti);
			if (soc_handle->init) soc_handle->value = ((float *)soc->default_value->buffer)[0];
			((float *)soc->default_value->buffer)[0] = zui_slider(soc_handle, "Value", min, max, true, prec, true, ZUI_ALIGN_LEFT, true);
			current->ops->theme->TEXT_OFFSET = text_off;
		}
		else if (strcmp(but->type, "STRING") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			zui_node_socket_t *soc = but->output >= 0 ? node->outputs->buffer[but->output] : NULL;
			zui_handle_t *h = zui_nest(nhandle, buti);
			if (h->init) strcpy(h->text, soc != NULL ? soc->default_value->buffer : but->default_value->buffer != NULL ? but->default_value->buffer : "");
			but->default_value->buffer = zui_text_input(h, zui_tr(but->name), ZUI_ALIGN_LEFT, true, false);
			but->default_value->length = strlen(but->default_value->buffer) + 1;
			if (soc != NULL) {
				soc->default_value->buffer = but->default_value->buffer;
			}
		}
		else if (strcmp(but->type, "ENUM") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			char temp_label[32];
			char temp_texts_data[32][64];
			char *temp_texts[32];
			bool combo_selected = current->combo_selected_handle != NULL;
			char *label = combo_selected ? temp_label : enum_label;
			char (*texts_data)[64] = combo_selected ? temp_texts_data : enum_texts_data;
			char **texts = combo_selected ? temp_texts : enum_texts;

			int texts_count = 0;
			if (but->data != NULL && but->data->length > 1) {
				bool string_start = true;
				for (int i = 0; i < but->data->length; ++i) {
					if (string_start) {
						string_start = false;
						strcpy(texts_data[texts_count], zui_tr(&((char *)but->data->buffer)[i]));
						texts[texts_count] = texts_data[texts_count];
						texts_count++;
					}
					if (((char *)but->data->buffer)[i] == '\0') {
						string_start = true;
					}
				}
			}
			else {
				char **strings = (*zui_nodes_enum_texts)(node->type);
				while (strings[texts_count] != NULL) {
					strcpy(texts_data[texts_count], strings[texts_count]);
					texts[texts_count] = texts_data[texts_count];
					texts_count++;
				}
			}
			strcpy(label, zui_tr(but->name));
			zui_handle_t *but_handle = zui_nest(nhandle, buti);
			but_handle->position = ((float *)but->default_value->buffer)[0];

			char_ptr_array_t ar;
			ar.buffer = texts;
			ar.length = texts_count;
			((float *)but->default_value->buffer)[0] = zui_combo(but_handle, &ar, label, false, ZUI_ALIGN_LEFT, true);
		}
		else if (strcmp(but->type, "BOOL") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			zui_handle_t *h = zui_nest(nhandle, buti);
			if (h->init) h->selected = ((uint8_t *)but->default_value->buffer)[0];
			((uint8_t *)but->default_value->buffer)[0] = zui_check(h, zui_tr(but->name), "");
		}
		else if (strcmp(but->type, "CUSTOM") == 0) { // Calls external function for custom button drawing
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			zui_nodes_on_custom_button(node->id, but->name);
			ny += lineh * (but->height - 1); // but->height specifies vertical button size
		}
	}
	ny += lineh / 3.0; // Fix align

	// Inputs
	for (int i = 0; i < node->inputs->length; ++i) {
		zui_node_socket_t *inp = node->inputs->buffer[i];
		ny += lineh;
		arm_g2_set_color(inp->color);
		arm_g2_draw_scaled_render_target(&zui_socket_image, nx - zui_p(6), ny - zui_p(3), zui_p(12), zui_p(12));
		bool is_linked = zui_input_linked(canvas, node->id, i);
		if (!is_linked && strcmp(inp->type, "VALUE") == 0) {
			current->_x = nx + zui_p(6);
			current->_y = ny - zui_p(current_nodes->ELEMENT_H / 3.0);
			current->_w = w - zui_p(6);
			zui_node_socket_t *soc = inp;
			float min = soc->min;
			float max = soc->max;
			float prec = soc->precision;
			float text_off = current->ops->theme->TEXT_OFFSET;
			current->ops->theme->TEXT_OFFSET = 6;

			zui_handle_t *_handle = zui_nest(nhandle, zui_max_buttons);
			zui_handle_t *soc_handle = zui_nest(_handle, i);
			if (soc_handle->init) soc_handle->value = ((float *)soc->default_value->buffer)[0];
			((float *)soc->default_value->buffer)[0] = zui_slider(soc_handle, zui_tr(inp->name), min, max, true, prec, true, ZUI_ALIGN_LEFT, true);
			current->ops->theme->TEXT_OFFSET = text_off;
		}
		else if (!is_linked && strcmp(inp->type, "STRING") == 0) {
			current->_x = nx + zui_p(6);
			current->_y = ny - zui_p(9);
			current->_w = w - zui_p(6);
			zui_node_socket_t *soc = inp;
			float text_off = current->ops->theme->TEXT_OFFSET;
			current->ops->theme->TEXT_OFFSET = 6;
			zui_handle_t *_handle = zui_nest(nhandle, zui_max_buttons);
			zui_handle_t *h = zui_nest(_handle, i);
			if (h->init) strcpy(h->text, soc->default_value->buffer);
			soc->default_value->buffer = zui_text_input(h, zui_tr(inp->name), ZUI_ALIGN_LEFT, true, false);
			current->ops->theme->TEXT_OFFSET = text_off;
		}
		else if (!is_linked && strcmp(inp->type, "RGBA") == 0) {
			arm_g2_set_color(current->ops->theme->LABEL_COL);
			arm_g2_draw_string(zui_tr(inp->name), nx + zui_p(12), ny - zui_p(3));
			zui_node_socket_t *soc = inp;
			arm_g2_set_color(0xff000000);
			arm_g2_fill_rect(nx + w - zui_p(38), ny - zui_p(6), zui_p(36), zui_p(18));
			float *val = (float *)soc->default_value->buffer;
			arm_g2_set_color(zui_color(val[0] * 255, val[1] * 255, val[2] * 255, 255));
			float rx = nx + w - zui_p(37);
			float ry = ny - zui_p(5);
			float rw = zui_p(34);
			float rh = zui_p(16);
			arm_g2_fill_rect(rx, ry, rw, rh);
			float ix = current->input_x - wx;
			float iy = current->input_y - wy;
			if (current->input_started && ix > rx && iy > ry && ix < rx + rw && iy < ry + rh) {
				current_nodes->_input_started = current->input_started = false;
				zui_nodes_rgba_popup(nhandle, soc->default_value->buffer, (int)(rx), (int)(ry + ZUI_ELEMENT_H()));
				zui_popup_handle_node_id = node->id;
				zui_popup_handle_node_socket_id = soc->id;
			}
			if (zui_popup_commands != NULL && zui_popup_handle_node_id == node->id && zui_popup_handle_node_socket_id == soc->id) {
				// armpack data may have been moved in memory
				zui_popup_data = nhandle;
				zui_popup_data2 = soc->default_value->buffer;
			}
		}
		else if (!is_linked && strcmp(inp->type, "VECTOR") == 0 && inp->display == 1) {
			arm_g2_set_color(current->ops->theme->LABEL_COL);
			arm_g2_draw_string(zui_tr(inp->name), nx + zui_p(12), ny - zui_p(3));
			ny += lineh / 2;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			float min = inp->min;
			float max = inp->max;
			float text_off = current->ops->theme->TEXT_OFFSET;
			current->ops->theme->TEXT_OFFSET = 6;
			float *val = (float *)inp->default_value->buffer;
			zui_handle_t *h = zui_nest(nhandle, zui_max_buttons);
			zui_handle_t *hi = zui_nest(h, i);
			zui_handle_t *h0 = zui_nest(hi, 0);
			if (h0->init) h0->value = val[0];
			zui_handle_t *h1 = zui_nest(hi, 1);
			if (h1->init) h1->value = val[1];
			zui_handle_t *h2 = zui_nest(hi, 2);
			if (h2->init) h2->value = val[2];
			val[0] = zui_slider(h0, "X", min, max, true, 100, true, ZUI_ALIGN_LEFT, true);
			val[1] = zui_slider(h1, "Y", min, max, true, 100, true, ZUI_ALIGN_LEFT, true);
			val[2] = zui_slider(h2, "Z", min, max, true, 100, true, ZUI_ALIGN_LEFT, true);
			current->ops->theme->TEXT_OFFSET = text_off;
			ny += lineh * 2.5;
		}
		else {
			arm_g2_set_color(current->ops->theme->LABEL_COL);
			arm_g2_draw_string(zui_tr(inp->name), nx + zui_p(12), ny - zui_p(3));
		}
		if (zui_nodes_on_socket_released != NULL && current->input_enabled && (current->input_released || current->input_released_r)) {
			if (current->input_x > wx + nx && current->input_x < wx + nx + w && current->input_y > wy + ny && current->input_y < wy + ny + lineh) {
				zui_nodes_on_socket_released(inp->id);
				zui_nodes_socket_released = true;
			}
		}
	}

	current->_x = ui_x;
	current->_y = ui_y;
	current->_w = ui_w;
	current->input_started = current_nodes->_input_started;
}

bool zui_is_node_type_excluded(char *type) {
	for (int i = 0; i < zui_nodes_exclude_remove->length; ++i) {
		if (strcmp(zui_nodes_exclude_remove->buffer[i], type) == 0) {
			return true;
		}
	}
	return false;
}

void zui_node_canvas(zui_nodes_t *nodes, zui_node_canvas_t *canvas) {
	current_nodes = nodes;
	zui_t *current = zui_get_current();
	if (!zui_nodes_elements_baked) {
		zui_nodes_bake_elements();
	}
	if (zui_nodes_exclude_remove == NULL) {
		zui_nodes_exclude_remove = malloc(sizeof(char_ptr_array_t));
	}

	float wx = current->_window_x;
	float wy = current->_window_y;
	bool _input_enabled = current->input_enabled;
	current->input_enabled = _input_enabled && zui_popup_commands == NULL;
	zui_canvas_control_t controls = zui_nodes_on_canvas_control != NULL ? zui_nodes_on_canvas_control() : zui_on_default_canvas_control();
	zui_nodes_socket_released = false;

	// Pan canvas
	if (current->input_enabled && (controls.pan_x != 0.0 || controls.pan_y != 0.0)) {
		current_nodes->pan_x += controls.pan_x / ZUI_NODES_SCALE();
		current_nodes->pan_y += controls.pan_y / ZUI_NODES_SCALE();
	}

	// Zoom canvas
	if (current->input_enabled && controls.zoom != 0.0) {
		current_nodes->zoom += controls.zoom;
		if (current_nodes->zoom < 0.1) {
			current_nodes->zoom = 0.1;
		}
		else if (current_nodes->zoom > 1.0) {
			current_nodes->zoom = 1.0;
		}
		current_nodes->zoom = round(current_nodes->zoom * 100.0) / 100.0;
		current_nodes->uiw = current->_w;
		current_nodes->uih = current->_h;
		if (zui_touch_scroll) {
			// Zoom to finger location
			current_nodes->pan_x -= (current->input_x - current->_window_x - current->_window_w / 2.0) * controls.zoom * 5.0 * (1.0 - current_nodes->zoom);
			current_nodes->pan_y -= (current->input_y - current->_window_y - current->_window_h / 2.0) * controls.zoom * 5.0 * (1.0 - current_nodes->zoom);
		}
	}
	current_nodes->scale_factor = ZUI_SCALE();
	current_nodes->ELEMENT_H = current->ops->theme->ELEMENT_H + 2;
	zui_set_scale(ZUI_NODES_SCALE()); // Apply zoomed scale
	current->elements_baked = true;
	arm_g2_set_font(current->ops->font, current->font_size);

	for (int i = 0; i < canvas->links->length; ++i) {
		zui_node_link_t *link = canvas->links->buffer[i];
		zui_node_t *from = zui_get_node(canvas->nodes, link->from_id);
		zui_node_t *to = zui_get_node(canvas->nodes, link->to_id);
		float from_x = from == NULL ? current->input_x : wx + ZUI_NODE_X(from) + ZUI_NODE_W(from);
		float from_y = from == NULL ? current->input_y : wy + ZUI_NODE_Y(from) + ZUI_OUTPUT_Y(from->outputs->length, link->from_socket);
		float to_x = to == NULL ? current->input_x : wx + ZUI_NODE_X(to);
		float to_y = to == NULL ? current->input_y : wy + ZUI_NODE_Y(to) + ZUI_INPUT_Y(canvas, to->inputs->buffer, to->inputs->length, link->to_socket) + ZUI_OUTPUTS_H(to->outputs->length, -1) + ZUI_BUTTONS_H(to);

		// Cull
		float left = to_x > from_x ? from_x : to_x;
		float right = to_x > from_x ? to_x : from_x;
		float top = to_y > from_y ? from_y : to_y;
		float bottom = to_y > from_y ? to_y : from_y;
		if (right < 0 || left > wx + current->_window_w ||
			bottom < 0 || top > wy + current->_window_h) {
			continue;
		}

		// Snap to nearest socket
		if (current_nodes->link_drag_id == link->id) {
			if (current_nodes->snap_from_id != -1) {
				from_x = current_nodes->snap_x;
				from_y = current_nodes->snap_y;
			}
			if (current_nodes->snap_to_id != -1) {
				to_x = current_nodes->snap_x;
				to_y = current_nodes->snap_y;
			}
			current_nodes->snap_from_id = current_nodes->snap_to_id = -1;

			for (int j = 0; j < canvas->nodes->length; ++j) {
				zui_node_t *node = canvas->nodes->buffer[j];
				zui_node_socket_t **inps = node->inputs->buffer;
				zui_node_socket_t **outs = node->outputs->buffer;
				float node_h = ZUI_NODE_H(canvas, node);
				float rx = wx + ZUI_NODE_X(node) - ZUI_LINE_H() / 2;
				float ry = wy + ZUI_NODE_Y(node) - ZUI_LINE_H() / 2;
				float rw = ZUI_NODE_W(node) + ZUI_LINE_H();
				float rh = node_h + ZUI_LINE_H();
				if (zui_input_in_rect(rx, ry, rw, rh)) {
					if (from == NULL && node->id != to->id) { // Snap to output
						for (int k = 0; k < node->outputs->length; ++k) {
							float sx = wx + ZUI_NODE_X(node) + ZUI_NODE_W(node);
							float sy = wy + ZUI_NODE_Y(node) + ZUI_OUTPUT_Y(node->outputs->length, k);
							float rx = sx - ZUI_LINE_H() / 2;
							float ry = sy - ZUI_LINE_H() / 2;
							if (zui_input_in_rect(rx, ry, ZUI_LINE_H(), ZUI_LINE_H())) {
								current_nodes->snap_x = sx;
								current_nodes->snap_y = sy;
								current_nodes->snap_from_id = node->id;
								current_nodes->snap_socket = k;
								break;
							}
						}
					}
					else if (to == NULL && node->id != from->id) { // Snap to input
						for (int k = 0; k < node->inputs->length; ++k) {
							float sx = wx + ZUI_NODE_X(node);
							float sy = wy + ZUI_NODE_Y(node) + ZUI_INPUT_Y(canvas, inps, node->inputs->length, k) + ZUI_OUTPUTS_H(node->outputs->length, -1) + ZUI_BUTTONS_H(node);
							float rx = sx - ZUI_LINE_H() / 2.0;
							float ry = sy - ZUI_LINE_H() / 2.0;
							if (zui_input_in_rect(rx, ry, ZUI_LINE_H(), ZUI_LINE_H())) {
								current_nodes->snap_x = sx;
								current_nodes->snap_y = sy;
								current_nodes->snap_to_id = node->id;
								current_nodes->snap_socket = k;
								break;
							}
						}
					}
				}
			}
		}

		bool selected = false;
		for (int j = 0; j < current_nodes->nodes_selected_id->length; ++j) {
			int n_id = current_nodes->nodes_selected_id->buffer[j];
			if (link->from_id == n_id || link->to_id == n_id) {
				selected = true;
				break;
			}
		}

		zui_draw_link(from_x - wx, from_y - wy, to_x - wx, to_y - wy, selected);
	}

	for (int i = 0; i < canvas->nodes->length; ++i) {
		zui_node_t *node = canvas->nodes->buffer[i];

		// Cull
		if (ZUI_NODE_X(node) > current->_window_w || ZUI_NODE_X(node) + ZUI_NODE_W(node) < 0 ||
			ZUI_NODE_Y(node) > current->_window_h || ZUI_NODE_Y(node) + ZUI_NODE_H(canvas, node) < 0) {
			if (!zui_is_selected(node)) continue;
		}

		zui_node_socket_t **inps = node->inputs->buffer;
		zui_node_socket_t **outs = node->outputs->buffer;

		// Drag node
		float node_h = ZUI_NODE_H(canvas, node);
		if (current->input_enabled && zui_input_in_rect(wx + ZUI_NODE_X(node) - ZUI_LINE_H() / 2.0, wy + ZUI_NODE_Y(node), ZUI_NODE_W(node) + ZUI_LINE_H(), ZUI_LINE_H())) {
			if (current->input_started) {
				if (current->is_shift_down || current->is_ctrl_down) {
					// Add to selection or deselect
					if (zui_is_selected(node)) {
						remove_from_selection(node);
					}
					else {
						add_to_selection(node);
					}
				}
				else if (current_nodes->nodes_selected_id->length <= 1) {
					// Selecting single node, otherwise wait for input release
					current_nodes->nodes_selected_id->length = 0;
					i32_array_push(current_nodes->nodes_selected_id, node->id);
				}
				current_nodes->move_on_top = node; // Place selected node on top
				current_nodes->nodes_drag = true;
				current_nodes->dragged = false;
			}
			else if (current->input_released && !current->is_shift_down && !current->is_ctrl_down && !current_nodes->dragged) {
				// No drag performed, select single node
				current_nodes->nodes_selected_id->length = 0;
				i32_array_push(current_nodes->nodes_selected_id, node->id);
				if (zui_on_header_released != NULL) {
					zui_on_header_released(node);
				}
			}
		}
		if (current->input_started && zui_input_in_rect(wx + ZUI_NODE_X(node) - ZUI_LINE_H() / 2, wy + ZUI_NODE_Y(node) - ZUI_LINE_H() / 2, ZUI_NODE_W(node) + ZUI_LINE_H(), node_h + ZUI_LINE_H())) {
			// Check sockets
			if (current_nodes->link_drag_id == -1) {
				for (int j = 0; j < node->outputs->length; ++j) {
					float sx = wx + ZUI_NODE_X(node) + ZUI_NODE_W(node);
					float sy = wy + ZUI_NODE_Y(node) + ZUI_OUTPUT_Y(node->outputs->length, j);
					if (zui_input_in_rect(sx - ZUI_LINE_H() / 2.0, sy - ZUI_LINE_H() / 2.0, ZUI_LINE_H(), ZUI_LINE_H())) {
						// New link from output
						zui_node_link_t *l = (zui_node_link_t *)malloc(sizeof(zui_node_link_t)); // TODO: store at canvas->links without malloc
						l->id = zui_next_link_id(canvas->links);
						l->from_id = node->id;
						l->from_socket = j;
						l->to_id = -1;
						l->to_socket = -1;
						any_array_push(canvas->links, l);
						current_nodes->link_drag_id = l->id;
						current_nodes->is_new_link = true;
						break;
					}
				}
			}
			if (current_nodes->link_drag_id == -1) {
				for (int j = 0; j < node->inputs->length; ++j) {
					float sx = wx + ZUI_NODE_X(node);
					float sy = wy + ZUI_NODE_Y(node) + ZUI_INPUT_Y(canvas, inps, node->inputs->length, j) + ZUI_OUTPUTS_H(node->outputs->length, -1) + ZUI_BUTTONS_H(node);
					if (zui_input_in_rect(sx - ZUI_LINE_H() / 2.0, sy - ZUI_LINE_H() / 2.0, ZUI_LINE_H(), ZUI_LINE_H())) {
						// Already has a link - disconnect
						for (int k = 0; k < canvas->links->length; ++k) {
							zui_node_link_t *l = canvas->links->buffer[k];
							if (l->to_id == node->id && l->to_socket == j) {
								l->to_id = l->to_socket = -1;
								current_nodes->link_drag_id = l->id;
								current_nodes->is_new_link = false;
								break;
							}
						}
						if (current_nodes->link_drag_id != -1) break;
						// New link from input
						zui_node_link_t *l = (zui_node_link_t *)malloc(sizeof(zui_node_link_t));
						l->id = zui_next_link_id(canvas->links);
						l->from_id = -1;
						l->from_socket = -1;
						l->to_id = node->id;
						l->to_socket = j;
						any_array_push(canvas->links, l);
						current_nodes->link_drag_id = l->id;
						current_nodes->is_new_link = true;
						break;
					}
				}
			}
		}
		else if (current->input_released) {
			if (current_nodes->snap_to_id != -1) { // Connect to input
				// Force single link per input
				for (int j = 0; j < canvas->links->length; ++j) {
					zui_node_link_t *l = canvas->links->buffer[j];
					if (l->to_id == current_nodes->snap_to_id && l->to_socket == current_nodes->snap_socket) {
						zui_remove_link_at(canvas, zui_get_link_index(canvas->links, l->id));
						break;
					}
				}
				zui_node_link_t *link_drag = zui_get_link(canvas->links, current_nodes->link_drag_id);
				link_drag->to_id = current_nodes->snap_to_id;
				link_drag->to_socket = current_nodes->snap_socket;
				current->changed = true;
			}
			else if (current_nodes->snap_from_id != -1) { // Connect to output
				zui_node_link_t *link_drag = zui_get_link(canvas->links, current_nodes->link_drag_id);
				link_drag->from_id = current_nodes->snap_from_id;
				link_drag->from_socket = current_nodes->snap_socket;
				current->changed = true;
			}
			else if (current_nodes->link_drag_id != -1) { // Remove dragged link
				zui_remove_link_at(canvas, zui_get_link_index(canvas->links, current_nodes->link_drag_id));
				current->changed = true;
				if (zui_nodes_on_link_drag != NULL) {
					zui_nodes_on_link_drag(current_nodes->link_drag_id, current_nodes->is_new_link);
				}
			}
			current_nodes->snap_to_id = current_nodes->snap_from_id = -1;
			current_nodes->link_drag_id = -1;
			current_nodes->nodes_drag = false;
		}
		if (current_nodes->nodes_drag && zui_is_selected(node) && !current->input_down_r) {
			if (current->input_dx != 0 || current->input_dy != 0) {
				current_nodes->dragged = true;
				node->x += current->input_dx / ZUI_NODES_SCALE();
				node->y += current->input_dy / ZUI_NODES_SCALE();
			}
		}

		zui_draw_node(node, canvas);
	}

	if (zui_nodes_on_canvas_released != NULL && current->input_enabled && (current->input_released || current->input_released_r) && !zui_nodes_socket_released) {
		zui_nodes_on_canvas_released();
	}

	if (zui_box_select) {
		arm_g2_set_color(0x223333dd);
		arm_g2_fill_rect(zui_box_select_x, zui_box_select_y, current->input_x - zui_box_select_x - current->_window_x, current->input_y - zui_box_select_y - current->_window_y);
		arm_g2_set_color(0x773333dd);
		arm_g2_draw_rect(zui_box_select_x, zui_box_select_y, current->input_x - zui_box_select_x - current->_window_x, current->input_y - zui_box_select_y - current->_window_y, 1);
		arm_g2_set_color(0xffffffff);
	}
	if (current->input_enabled && current->input_started && !current->is_alt_down && current_nodes->link_drag_id == -1 && !current_nodes->nodes_drag && !current->changed) {
		zui_box_select = true;
		zui_box_select_x = current->input_x - current->_window_x;
		zui_box_select_y = current->input_y - current->_window_y;
	}
	else if (zui_box_select && !current->input_down) {
		zui_box_select = false;
		int left = zui_box_select_x;
		int top = zui_box_select_y;
		int right = current->input_x - current->_window_x;
		int bottom = current->input_y - current->_window_y;
		if (left > right) {
			int t = left;
			left = right;
			right = t;
		}
		if (top > bottom) {
			int t = top;
			top = bottom;
			bottom = t;
		}
		zui_node_t *nodes[32];
		int nodes_count = 0;
		for (int j = 0; j < canvas->nodes->length; ++j) {
			zui_node_t *n = canvas->nodes->buffer[j];
			if (ZUI_NODE_X(n) + ZUI_NODE_W(n) > left && ZUI_NODE_X(n) < right &&
				ZUI_NODE_Y(n) + ZUI_NODE_H(canvas, n) > top && ZUI_NODE_Y(n) < bottom) {
				nodes[nodes_count] = n;
				nodes_count++;
			}
		}
		if (current->is_shift_down || current->is_ctrl_down) {
			for (int j = 0; j < nodes_count; ++j) {
				add_to_selection(nodes[j]);
			}
		}
		else {
			current_nodes->nodes_selected_id->length = 0;
			for (int j = 0; j < nodes_count; ++j) {
				add_to_selection(nodes[j]);
			}
		}
	}

	// Place selected node on top
	if (current_nodes->move_on_top != NULL) {
		int index = zui_get_node_index(canvas->nodes->buffer, canvas->nodes->length, current_nodes->move_on_top->id);
		zui_remove_node_at(canvas, index);
		any_array_push(canvas->nodes, current_nodes->move_on_top);
		current_nodes->move_on_top = NULL;
	}

	// Node copy & paste
	bool cut_selected = false;
	if (zui_is_copy) {
		zui_node_t *copy_nodes[32];
		int copy_nodes_count = 0;
		for (int i = 0; i < current_nodes->nodes_selected_id->length; ++i) {
			int id = current_nodes->nodes_selected_id->buffer[i];
			zui_node_t *n = zui_get_node(canvas->nodes, id);
			if (zui_is_node_type_excluded(n->type)) {
				continue;
			}
			copy_nodes[copy_nodes_count] = n;
			copy_nodes_count++;
		}
		zui_node_link_t *copy_links[64];
		int copy_links_count = 0;
		for (int i = 0; i < canvas->links->length; ++i) {
			zui_node_link_t *l = canvas->links->buffer[i];
			zui_node_t *from = NULL;
			for (int j = 0; j < current_nodes->nodes_selected_id->length; ++j) {
				if (current_nodes->nodes_selected_id->buffer[j] == l->from_id) {
					from = zui_get_node(canvas->nodes, l->from_id);
					break;
				}
			}
			zui_node_t *to = NULL;
			for (int j = 0; j < current_nodes->nodes_selected_id->length; ++j) {
				if (current_nodes->nodes_selected_id->buffer[j] == l->to_id) {
					to = zui_get_node(canvas->nodes, l->to_id);
					break;
				}
			}

			if (from != NULL && !zui_is_node_type_excluded(from->type) &&
				to != NULL && !zui_is_node_type_excluded(to->type)) {
				copy_links[copy_links_count] = l;
				copy_links_count++;
			}
		}
		if (copy_nodes_count > 0) {
			zui_node_canvas_t copy_canvas;
			copy_canvas.name = canvas->name;
			copy_canvas.nodes = copy_nodes;
			copy_canvas.nodes->length = copy_nodes_count;
			copy_canvas.links = copy_links;
			copy_canvas.links->length = copy_links_count;
			// zui_clipboard = haxe.Json.stringify(copy_canvas);
		}
		cut_selected = zui_is_cut;
	}

	if (zui_is_paste && !current->is_typing) {
		zui_node_canvas_t *paste_canvas = NULL;
		// Try: clipboard can contain non-json data
		// paste_canvas = haxe.Json.parse(zui_clipboard);
		if (paste_canvas != NULL) {
			for (int i = 0; i < paste_canvas->links->length; ++i) {
				zui_node_link_t *l = paste_canvas->links->buffer[i];
				// Assign unique link id
				l->id = zui_next_link_id(canvas->links);
				any_array_push(canvas->links, l);
			}
			int offset_x = (int)(((int)(current->input_x / ZUI_SCALE()) * ZUI_NODES_SCALE() - wx - ZUI_NODES_PAN_X()) / ZUI_NODES_SCALE()) - paste_canvas->nodes->buffer[paste_canvas->nodes->length - 1]->x;
			int offset_y = (int)(((int)(current->input_y / ZUI_SCALE()) * ZUI_NODES_SCALE() - wy - ZUI_NODES_PAN_Y()) / ZUI_NODES_SCALE()) - paste_canvas->nodes->buffer[paste_canvas->nodes->length - 1]->y;
			for (int i = 0; i < paste_canvas->nodes->length; ++i) {
				zui_node_t *n = paste_canvas->nodes->buffer[i];
				// Assign unique node id
				int old_id = n->id;
				n->id = zui_next_node_id(canvas->nodes);

				for (int j = 0; j < n->inputs->length; ++j) {
					zui_node_socket_t *soc = n->inputs->buffer[j];
					soc->id = zui_get_socket_id(canvas->nodes);
					soc->node_id = n->id;
				}
				for (int j = 0; j < n->outputs->length; ++j) {
					zui_node_socket_t *soc = n->outputs->buffer[j];
					soc->id = zui_get_socket_id(canvas->nodes);
					soc->node_id = n->id;
				}
				for (int j = 0; j < paste_canvas->links->length; ++j) {
					zui_node_link_t *l = paste_canvas->links->buffer[j];
					if (l->from_id == old_id) l->from_id = n->id;
					else if (l->to_id == old_id) l->to_id = n->id;
				}
				n->x += offset_x;
				n->y += offset_y;
				any_array_push(canvas->nodes, n);
			}
			current_nodes->nodes_drag = true;
			current_nodes->nodes_selected_id->length = 0;
			for (int i = 0; i < paste_canvas->nodes->length; ++i) {
				i32_array_push(current_nodes->nodes_selected_id, paste_canvas->nodes->buffer[i]->id);
			}
			current->changed = true;
		}
	}

	// Select all nodes
	if (current->is_ctrl_down && current->key_code == KINC_KEY_A && !current->is_typing) {
		current_nodes->nodes_selected_id->length = 0;
		for (int i = 0; i < canvas->nodes->length; ++i) {
			add_to_selection(canvas->nodes->buffer[i]);
		}
	}

	// Node removal
	if (current->input_enabled && (current->is_backspace_down || current->is_delete_down || cut_selected) && !current->is_typing) {
		int i = current_nodes->nodes_selected_id->length - 1;
		while (i >= 0) {
			int nid = current_nodes->nodes_selected_id->buffer[i--];
			zui_node_t *n = zui_get_node(canvas->nodes, nid);
			if (n == NULL) {
				continue; ////
			}
			if (zui_is_node_type_excluded(n->type)) {
				continue;
			}
			zui_remove_node(n, canvas);
			current->changed = true;
		}
		current_nodes->nodes_selected_id->length = 0;
	}

	zui_set_scale(current_nodes->scale_factor); // Restore non-zoomed scale
	current->elements_baked = true;
	current->input_enabled = _input_enabled;

	if (zui_popup_commands != NULL) {
		current->_x = zui_popup_x;
		current->_y = zui_popup_y;
		current->_w = zui_popup_w;

		zui_fill(-6, -6, current->_w / ZUI_SCALE() + 12, zui_popup_h + 12, current->ops->theme->ACCENT_SELECT_COL);
		zui_fill(-5, -5, current->_w / ZUI_SCALE() + 10, zui_popup_h + 10, current->ops->theme->SEPARATOR_COL);
		(*zui_popup_commands)(current, zui_popup_data, zui_popup_data2);

		bool hide = (current->input_started || current->input_started_r) && (current->input_x - wx < zui_popup_x - 6 || current->input_x - wx > zui_popup_x + zui_popup_w + 6 || current->input_y - wy < zui_popup_y - 6 || current->input_y - wy > zui_popup_y + zui_popup_h * ZUI_SCALE() + 6);
		if (hide || current->is_escape_down) {
			zui_popup_commands = NULL;
		}
	}
}

void zui_node_canvas_encode(void *encoded, zui_node_canvas_t *canvas) {
	armpack_encode_start(encoded);
	armpack_encode_map(3);
	armpack_encode_string("name");
	armpack_encode_string(canvas->name);

	armpack_encode_string("nodes");
	armpack_encode_array(canvas->nodes->length);
	for (int i = 0; i < canvas->nodes->length; ++i) {
		armpack_encode_map(10);
		armpack_encode_string("id");
		armpack_encode_i32(canvas->nodes->buffer[i]->id);
		armpack_encode_string("name");
		armpack_encode_string(canvas->nodes->buffer[i]->name);
		armpack_encode_string("type");
		armpack_encode_string(canvas->nodes->buffer[i]->type);
		armpack_encode_string("x");
		armpack_encode_i32(canvas->nodes->buffer[i]->x);
		armpack_encode_string("y");
		armpack_encode_i32(canvas->nodes->buffer[i]->y);
		armpack_encode_string("color");
		armpack_encode_i32(canvas->nodes->buffer[i]->color);

		armpack_encode_string("inputs");
		armpack_encode_array(canvas->nodes->buffer[i]->inputs->length);
		for (int j = 0; j < canvas->nodes->buffer[i]->inputs->length; ++j) {
			armpack_encode_map(10);
			armpack_encode_string("id");
			armpack_encode_i32(canvas->nodes->buffer[i]->inputs->buffer[j]->id);
			armpack_encode_string("node_id");
			armpack_encode_i32(canvas->nodes->buffer[i]->inputs->buffer[j]->node_id);
			armpack_encode_string("name");
			armpack_encode_string(canvas->nodes->buffer[i]->inputs->buffer[j]->name);
			armpack_encode_string("type");
			armpack_encode_string(canvas->nodes->buffer[i]->inputs->buffer[j]->type);
			armpack_encode_string("color");
			armpack_encode_i32(canvas->nodes->buffer[i]->inputs->buffer[j]->color);
			armpack_encode_string("default_value");
			armpack_encode_array_f32(canvas->nodes->buffer[i]->inputs->buffer[j]->default_value->buffer, canvas->nodes->buffer[i]->inputs->buffer[j]->default_value->length);
			armpack_encode_string("min");
			armpack_encode_f32(canvas->nodes->buffer[i]->inputs->buffer[j]->min);
			armpack_encode_string("max");
			armpack_encode_f32(canvas->nodes->buffer[i]->inputs->buffer[j]->max);
			armpack_encode_string("precision");
			armpack_encode_f32(canvas->nodes->buffer[i]->inputs->buffer[j]->precision);
			armpack_encode_string("display");
			armpack_encode_i32(canvas->nodes->buffer[i]->inputs->buffer[j]->display);
		}

		armpack_encode_string("outputs");
		armpack_encode_array(canvas->nodes->buffer[i]->outputs->length);
		for (int j = 0; j < canvas->nodes->buffer[i]->outputs->length; ++j) {
			armpack_encode_map(10);
			armpack_encode_string("id");
			armpack_encode_i32(canvas->nodes->buffer[i]->outputs->buffer[j]->id);
			armpack_encode_string("node_id");
			armpack_encode_i32(canvas->nodes->buffer[i]->outputs->buffer[j]->node_id);
			armpack_encode_string("name");
			armpack_encode_string(canvas->nodes->buffer[i]->outputs->buffer[j]->name);
			armpack_encode_string("type");
			armpack_encode_string(canvas->nodes->buffer[i]->outputs->buffer[j]->type);
			armpack_encode_string("color");
			armpack_encode_i32(canvas->nodes->buffer[i]->outputs->buffer[j]->color);
			armpack_encode_string("default_value");
			armpack_encode_array_f32(canvas->nodes->buffer[i]->outputs->buffer[j]->default_value->buffer, canvas->nodes->buffer[i]->outputs->buffer[j]->default_value->length);
			armpack_encode_string("min");
			armpack_encode_f32(canvas->nodes->buffer[i]->outputs->buffer[j]->min);
			armpack_encode_string("max");
			armpack_encode_f32(canvas->nodes->buffer[i]->outputs->buffer[j]->max);
			armpack_encode_string("precision");
			armpack_encode_f32(canvas->nodes->buffer[i]->outputs->buffer[j]->precision);
			armpack_encode_string("display");
			armpack_encode_i32(canvas->nodes->buffer[i]->outputs->buffer[j]->display);
		}

		armpack_encode_string("buttons");
		armpack_encode_array(canvas->nodes->buffer[i]->buttons->length);
		for (int j = 0; j < canvas->nodes->buffer[i]->buttons->length; ++j) {
			armpack_encode_map(9);
			armpack_encode_string("name");
			armpack_encode_string(canvas->nodes->buffer[i]->buttons->buffer[j]->name);
			armpack_encode_string("type");
			armpack_encode_string(canvas->nodes->buffer[i]->buttons->buffer[j]->type);
			armpack_encode_string("output");
			armpack_encode_i32(canvas->nodes->buffer[i]->buttons->buffer[j]->output);
			armpack_encode_string("default_value");
			armpack_encode_array_f32(canvas->nodes->buffer[i]->buttons->buffer[j]->default_value->buffer, canvas->nodes->buffer[i]->buttons->buffer[j]->default_value->length);
			armpack_encode_string("data");
			u8_array_t *u8 = canvas->nodes->buffer[i]->buttons->buffer[j]->data;
			armpack_encode_array_u8(u8 != NULL ? u8->buffer : NULL, u8 != NULL ? u8->length : 0);
			armpack_encode_string("min");
			armpack_encode_f32(canvas->nodes->buffer[i]->buttons->buffer[j]->min);
			armpack_encode_string("max");
			armpack_encode_f32(canvas->nodes->buffer[i]->buttons->buffer[j]->max);
			armpack_encode_string("precision");
			armpack_encode_f32(canvas->nodes->buffer[i]->buttons->buffer[j]->precision);
			armpack_encode_string("height");
			armpack_encode_f32(canvas->nodes->buffer[i]->buttons->buffer[j]->height);
		}

		armpack_encode_string("width");
		armpack_encode_i32(canvas->nodes->buffer[i]->width);
	}

	armpack_encode_string("links");
	armpack_encode_array(canvas->links->length);
	for (int i = 0; i < canvas->links->length; ++i) {
		armpack_encode_map(5);
		armpack_encode_string("id");
		armpack_encode_i32(canvas->links->buffer[i]->id);
		armpack_encode_string("from_id");
		armpack_encode_i32(canvas->links->buffer[i]->from_id);
		armpack_encode_string("from_socket");
		armpack_encode_i32(canvas->links->buffer[i]->from_socket);
		armpack_encode_string("to_id");
		armpack_encode_i32(canvas->links->buffer[i]->to_id);
		armpack_encode_string("to_socket");
		armpack_encode_i32(canvas->links->buffer[i]->to_socket);
	}
}

uint32_t zui_node_canvas_encoded_size(zui_node_canvas_t *canvas) {
	uint32_t size = 0;
	size += armpack_size_map();
	size += armpack_size_string("name");
	size += armpack_size_string(canvas->name);

	size += armpack_size_string("nodes");
	size += armpack_size_array();
	for (int i = 0; i < canvas->nodes->length; ++i) {
		size += armpack_size_map();
		size += armpack_size_string("id");
		size += armpack_size_i32();
		size += armpack_size_string("name");
		size += armpack_size_string(canvas->nodes->buffer[i]->name);
		size += armpack_size_string("type");
		size += armpack_size_string(canvas->nodes->buffer[i]->type);
		size += armpack_size_string("x");
		size += armpack_size_i32();
		size += armpack_size_string("y");
		size += armpack_size_i32();
		size += armpack_size_string("color");
		size += armpack_size_i32();

		size += armpack_size_string("inputs");
		size += armpack_size_array();
		for (int j = 0; j < canvas->nodes->buffer[i]->inputs->length; ++j) {
			size += armpack_size_map();
			size += armpack_size_string("id");
			size += armpack_size_i32();
			size += armpack_size_string("node_id");
			size += armpack_size_i32();
			size += armpack_size_string("name");
			size += armpack_size_string(canvas->nodes->buffer[i]->inputs->buffer[j]->name);
			size += armpack_size_string("type");
			size += armpack_size_string(canvas->nodes->buffer[i]->inputs->buffer[j]->type);
			size += armpack_size_string("color");
			size += armpack_size_i32();
			size += armpack_size_string("default_value");
			size += armpack_size_array_f32(canvas->nodes->buffer[i]->inputs->buffer[j]->default_value->length);
			size += armpack_size_string("min");
			size += armpack_size_f32();
			size += armpack_size_string("max");
			size += armpack_size_f32();
			size += armpack_size_string("precision");
			size += armpack_size_f32();
			size += armpack_size_string("display");
			size += armpack_size_i32();
		}

		size += armpack_size_string("outputs");
		size += armpack_size_array();
		for (int j = 0; j < canvas->nodes->buffer[i]->outputs->length; ++j) {
			size += armpack_size_map();
			size += armpack_size_string("id");
			size += armpack_size_i32();
			size += armpack_size_string("node_id");
			size += armpack_size_i32();
			size += armpack_size_string("name");
			size += armpack_size_string(canvas->nodes->buffer[i]->outputs->buffer[j]->name);
			size += armpack_size_string("type");
			size += armpack_size_string(canvas->nodes->buffer[i]->outputs->buffer[j]->type);
			size += armpack_size_string("color");
			size += armpack_size_i32();
			size += armpack_size_string("default_value");
			size += armpack_size_array_f32(canvas->nodes->buffer[i]->outputs->buffer[j]->default_value->length);
			size += armpack_size_string("min");
			size += armpack_size_f32();
			size += armpack_size_string("max");
			size += armpack_size_f32();
			size += armpack_size_string("precision");
			size += armpack_size_f32();
			size += armpack_size_string("display");
			size += armpack_size_i32();
		}

		size += armpack_size_string("buttons");
		size += armpack_size_array();
		for (int j = 0; j < canvas->nodes->buffer[i]->buttons->length; ++j) {
			size += armpack_size_map();
			size += armpack_size_string("name");
			size += armpack_size_string(canvas->nodes->buffer[i]->buttons->buffer[j]->name);
			size += armpack_size_string("type");
			size += armpack_size_string(canvas->nodes->buffer[i]->buttons->buffer[j]->type);
			size += armpack_size_string("output");
			size += armpack_size_i32();
			size += armpack_size_string("default_value");
			size += armpack_size_array_f32(canvas->nodes->buffer[i]->buttons->buffer[j]->default_value->length);
			size += armpack_size_string("data");
			u8_array_t *u8 = canvas->nodes->buffer[i]->buttons->buffer[j]->data;
			size += armpack_size_array_u8(u8 != NULL ? u8->length : 0);
			size += armpack_size_string("min");
			size += armpack_size_f32();
			size += armpack_size_string("max");
			size += armpack_size_f32();
			size += armpack_size_string("precision");
			size += armpack_size_f32();
			size += armpack_size_string("height");
			size += armpack_size_f32();
		}

		size += armpack_size_string("width");
		size += armpack_size_i32();
	}

	size += armpack_size_string("links");
	size += armpack_size_array();
	for (int i = 0; i < canvas->links->length; ++i) {
		size += armpack_size_map();
		size += armpack_size_string("id");
		size += armpack_size_i32();
		size += armpack_size_string("from_id");
		size += armpack_size_i32();
		size += armpack_size_string("from_socket");
		size += armpack_size_i32();
		size += armpack_size_string("to_id");
		size += armpack_size_i32();
		size += armpack_size_string("to_socket");
		size += armpack_size_i32();
	}

	return size;
}
