#include "zui_nodes.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/input/keyboard.h>
#include "zui.h"
#include "zui_ext.h"
#include "../g2/g2.h"
#include "../g2/g2_ext.h"

static zui_nodes_t *current_nodes = NULL;
static bool zui_nodes_elements_baked = false;
static kinc_g4_render_target_t zui_socket_image;
static bool zui_socket_released = false;
static char *zui_clipboard = "";
static bool zui_box_select = false;
static int zui_box_select_x = 0;
static int zui_box_select_y = 0;
static const int zui_max_buttons = 9;
static zui_handle_t handle;

static char **zui_exclude_remove;// = []; // No removal for listed node types
static void (*zui_on_link_drag)(zui_node_link_t *, bool) = NULL;
static void (*zui_on_header_released)(zui_node_t *) = NULL;
static void (*zui_on_socket_released)(zui_node_socket_t *) = NULL;
static void (*zui_on_canvas_released)(void) = NULL;
static void (*zui_on_node_remove)(zui_node_t *) = NULL;
static zui_canvas_control_t (*zui_on_canvas_control)(void) = NULL; // Pan, zoom
static int zui_node_id = -1;

// Retrieve combo items for buttons of type ENUM
char **(*zui_enum_texts)(char *) = NULL;

int zui_popup_x = 0;
int zui_popup_y = 0;
int zui_popup_w = 0;
int zui_popup_h = 0;
void (*zui_popup_commands)(zui_t *) = NULL;

// #ifdef ZUI_TRANSLATE
// char *zui_tr(char *id, map<char *, char *> vars = NULL) { // non-inline
// 	return id;
// }
// #else
char *zui_tr(char *id/*, map<char *, char *> vars*/) { // inline
	return id;
}
// #endif

void zui_nodes_init(zui_nodes_t *nodes) {
	current_nodes = nodes;
	nodes->zoom = 1.0;
	nodes->scale_factor = 1.0;
	nodes->ELEMENT_H = 25;
	nodes->snap_from_id = -1;
	nodes->snap_to_id = -1;
}

inline float ZUI_NODES_SCALE() {
	return current_nodes->scale_factor * current_nodes->zoom;
}

inline float ZUI_PAN_X() {
	float zoom_pan = (1.0 - current_nodes->zoom) * current_nodes->uiw / 2.5;
	return current_nodes->pan_x * ZUI_NODES_SCALE() + zoom_pan;
}

inline float ZUI_PAN_Y() {
	float zoom_pan = (1.0 - current_nodes->zoom) * current_nodes->uih / 2.5;
	return current_nodes->pan_y * ZUI_NODES_SCALE() + zoom_pan;
}

inline int ZUI_LINE_H() {
	return current_nodes->ELEMENT_H * ZUI_NODES_SCALE();
}

int ZUI_BUTTONS_H(zui_node_t *node) {
	float h = 0.0;
	for (int i = 0; i < node->buttons_count; ++i) {
		zui_node_button_t *but = &node->buttons[i];
		if (strcmp(but->type, "RGBA") == 0) h += 102 * ZUI_NODES_SCALE() + ZUI_LINE_H() * 5; // Color wheel + controls
		else if (strcmp(but->type, "VECTOR") == 0) h += ZUI_LINE_H() * 4;
		else if (strcmp(but->type, "CUSTOM") == 0) h += ZUI_LINE_H() * but->height;
		else h += ZUI_LINE_H();
	}
	return h;
}

int ZUI_OUTPUTS_H(int sockets_count, int length) {
	float h = 0.0;
	for (int i = 0; i < (length == 0 ? sockets_count : length); ++i) {
		h += ZUI_LINE_H();
	}
	return h;
}

bool zui_input_linked(zui_node_canvas_t *canvas, int node_id, int i) {
	for (int x = 0; i < canvas->links_count; ++x) {
		zui_node_link_t *l = &canvas->links[x];
		if (l->to_id == node_id && l->to_socket == i) return true;
	}
	return false;
}

int ZUI_INPUTS_H(zui_node_canvas_t *canvas, zui_node_socket_t **sockets, int sockets_count, int length) {
	float h = 0.0;
	for (int i = 0; i < (length == 0 ? sockets_count : length); ++i) {
		if (strcmp(sockets[i]->type, "VECTOR") == 0 && sockets[i]->display == 1 && !zui_input_linked(canvas, sockets[i]->node_id, i)) h += ZUI_LINE_H() * 4;
		else h += ZUI_LINE_H();
	}
	return h;
}

int ZUI_NODE_H(zui_node_canvas_t *canvas, zui_node_t *node) {
	return ZUI_LINE_H() * 1.2 + ZUI_INPUTS_H(canvas, node->inputs, node->inputs_count, 0) + ZUI_OUTPUTS_H(node->outputs_count, 0) + ZUI_BUTTONS_H(node);
}

int ZUI_NODE_W(zui_node_t *node) {
	return (node->width != 0 ? node->width : 140) * ZUI_NODES_SCALE();
}

float ZUI_NODE_X(zui_node_t *node) {
	return node->x * ZUI_NODES_SCALE() + ZUI_PAN_X();
}

float ZUI_NODE_Y(zui_node_t *node) {
	return node->y * ZUI_NODES_SCALE() + ZUI_PAN_Y();
}

int ZUI_INPUT_Y(zui_node_canvas_t *canvas, zui_node_socket_t **sockets, int sockets_count, int pos) {
	return ZUI_LINE_H() * 1.62 + ZUI_INPUTS_H(canvas, sockets, sockets_count, pos);
}

int ZUI_OUTPUT_Y(zui_node_socket_t **sockets, int pos) {
	return ZUI_LINE_H() * 1.62 + ZUI_OUTPUTS_H(sockets, pos);
}

float zui_p(float f) {
	return f * ZUI_NODES_SCALE();
}

zui_node_t *zui_get_node(zui_node_t **nodes, int nodes_count, int id) {
	for (int i = 0; i < nodes_count; ++i) if (nodes[i]->id == id) return &nodes[i];
	return NULL;
}

int zui_get_node_id(zui_node_t **nodes, int nodes_count) {
	if (zui_node_id == -1) for (int i = 0; i < nodes_count; ++i) if (zui_node_id < nodes[i]->id) zui_node_id = nodes[i]->id;
	return ++zui_node_id;
}

int zui_get_link_id(zui_node_link_t **links, int links_count) {
	int id = 0;
	for (int i = 0; i < links_count; ++i) if (links[i]->id >= id) id = links[i]->id + 1;
	return id;
}

int zui_get_socket_id(zui_node_t **nodes, int nodes_count) {
	int id = 0;
	for (int i = 0; i < nodes_count; ++i) {
		zui_node_t *n = &nodes[i];
		for (int j = 0; j < n->inputs_count; ++j) if (n->inputs[j]->id >= id) id = n->inputs[j]->id + 1;
		for (int j = 0; j < n->outputs_count; ++j) if (n->outputs[j]->id >= id) id = n->outputs[j]->id + 1;
	}
	return id;
}

void zui_nodes_bake_elements() {
	kinc_g4_render_target_init(&zui_socket_image, 24, 24, KINC_G4_RENDER_TARGET_FORMAT_32BIT, 0, 0);
	g2_set_render_target(&zui_socket_image);
	kinc_g4_clear(KINC_G4_CLEAR_COLOR, 0x00000000, 0, 0);
	g2_set_color(0xff000000);
	g2_fill_circle(12, 12, 12, 0);
	g2_set_color(0xffffffff);
	g2_fill_circle(12, 12, 9, 0);
	g2_restore_render_target();
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
	int c1 = current->ops.theme.LABEL_COL;
	int c2 = current->ops.theme.ACCENT_SELECT_COL;
	int c = highlight ? c1 : c2;
	g2_set_color(zui_color(zui_color_r(c), zui_color_g(c), zui_color_b(c), 210));
	if (current->ops.theme.LINK_STYLE == ZUI_LINK_STYLE_LINE) {
		g2_draw_line(x1, y1, x2, y2, 1.0);
		g2_set_color(zui_color(zui_color_r(c), zui_color_g(c), zui_color_b(c), 150)); // AA
		g2_draw_line(x1 + 0.5, y1, x2 + 0.5, y2, 1.0);
		g2_draw_line(x1 - 0.5, y1, x2 - 0.5, y2, 1.0);
		g2_draw_line(x1, y1 + 0.5, x2, y2 + 0.5, 1.0);
		g2_draw_line(x1, y1 - 0.5, x2, y2 - 0.5, 1.0);
	}
	else if (current->ops.theme.LINK_STYLE == ZUI_LINK_STYLE_CUBIC_BEZIER) {
		float x[] = { x1, x1 + fabs(x1 - x2) / 2, x2 - fabs(x1 - x2) / 2, x2 };
		float y[] = { y1, y1, y2, y2 };
		g2_draw_cubic_bezier(x, y, 30, highlight ? 2.0 : 1.0);
	}
}

void zui_remove_node(zui_node_t *n, zui_node_canvas_t *canvas) {
	if (n == NULL) return;
	int i = 0;
	while (i < canvas->links_count) {
		zui_node_link_t *l = &canvas->links[i];
		if (l->from_id == n->id || l->to_id == n->id) {
			// canvas.links.splice(i, 1);
		}
		else i++;
	}
	// canvas->nodes.remove(n);
	if (zui_on_node_remove != NULL) {
		zui_on_node_remove(n);
	}
}

bool zui_is_selected(zui_node_t *node) {
	for (int i = 0; i < current_nodes->nodes_selected_count; ++i) if (&current_nodes->nodes_selected[i] == node) return true;
	return false;
}

void zui_popup(int x, int y, int w, int h, void (*commands)(zui_t *)) {
	zui_popup_x = x;
	zui_popup_y = y;
	zui_popup_w = w;
	zui_popup_h = h;
	zui_popup_commands = commands;
}

void zui_rgba_popup(zui_handle_t *nhandle, float *val, int x, int y) {
	// zui_popup(x, y, 140 * scale_factor, current->ops.theme.ELEMENT_H * 10, function(zui_t *ui) {
	// 	nhandle.color = kha.Color.fromFloats(val[0], val[1], val[2]);
	// 	zui_color_wheel(nhandle, false, NULL, true, function () {
	// 		color_picker_callback = function (int color) {
	// 			val[0] = color.R;
	// 			val[1] = color.G;
	// 			val[2] = color.B;
	// 			current->changed = true;
	// 		};
	// 	});
	// 	val[0] = nhandle.color.R; val[1] = nhandle.color.G; val[2] = nhandle.color.B;
	// });
}

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
	// 	for (i in (canvas->nodes.indexOf(node) + 1)...canvas->nodes_count) {
	// 		zui_node_t *n = canvas->nodes[i];
	// 		if (ZUI_NODE_X(n) < current->input_x - current->_window_x && ZUI_NODE_X(n) + ZUI_NODE_W(n) > current->input_x - current->_window_x &&
	// 			ZUI_NODE_Y(n) < current->input_y - current->_window_y && ZUI_NODE_Y(n) + ZUI_NODE_H(canvas, n) > current->input_y - current->_window_y) {
	// 			current->input_started = false;
	// 			break;
	// 		}
	// 	}
	}

	// Outline
	g2_set_color(zui_is_selected(node) ? current->ops.theme.LABEL_COL : current->ops.theme.CONTEXT_COL);
	g2_fill_rect(nx - 1, ny - 1, w + 2, h + 2);

	// Body
	g2_set_color(current->ops.theme.WINDOW_BG_COL);
	g2_fill_rect(nx, ny, w, h);

	// Header line
	g2_set_color(node->color);
	g2_fill_rect(nx, ny + lineh - zui_p(3), w, zui_p(3));

	// Title
	g2_set_color(current->ops.theme.LABEL_COL);
	float textw = g2_string_width(current->ops.font, current->font_size, text);
	g2_draw_string(text, nx + zui_p(10), ny + zui_p(6));
	current->_x = nx; // Use the whole line for hovering and not just the drawn string.
	current->_y = ny;
	current->_w = w;
	if (zui_get_hover(lineh) && node->tooltip != NULL) zui_tooltip(zui_tr(node->tooltip));

	ny += lineh * 0.5;

	// Outputs
	for (int i = 0; i < node->outputs_count; ++i) {
		zui_node_socket_t *out = &node->outputs[i];
		ny += lineh;
		g2_set_color(out->color);
		g2_draw_scaled_render_target(&zui_socket_image, nx + w - zui_p(6), ny - zui_p(3), zui_p(12), zui_p(12));
	}
	ny -= lineh * node->outputs_count;
	g2_set_color(current->ops.theme.LABEL_COL);
	for (int i = 0; i < node->outputs_count; ++i) {
		zui_node_socket_t *out = &node->outputs[i];
		ny += lineh;
		float strw = g2_string_width(current->ops.font, current->font_size, zui_tr(out->name));
		g2_draw_string(zui_tr(out->name), nx + w - strw - zui_p(12), ny - zui_p(3));
		current->_x = nx;
		current->_y = ny;
		current->_w = w;
		if (zui_get_hover(lineh) && out->tooltip != NULL) zui_tooltip(zui_tr(out->tooltip));

		if (zui_on_socket_released != NULL && current->input_enabled && (current->input_released || current->input_released_r)) {
			if (current->input_x > wx + nx && current->input_x < wx + nx + w && current->input_y > wy + ny && current->input_y < wy + ny + lineh) {
				zui_on_socket_released(out);
				zui_socket_released = true;
			}
		}
	}

	// Buttons
	zui_handle_t *nhandle = zui_nest(&handle, node->id);
	ny -= lineh / 3; // Fix align
	for (int buti = 0; buti < node->buttons_count; ++buti) {
		zui_node_button_t *but = &node->buttons[buti];

		if (strcmp(but->type, "RGBA") == 0) {
			ny += lineh; // 18 + 2 separator
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			float *val = node->outputs[but->output]->default_value;
			nhandle->color = zui_color(val[0] * 255, val[1] * 255, val[2] * 255, 255);
			// zui_color_wheel(nhandle, false, NULL, NULL, true,  function () {
			// 	color_picker_callback = function (color: kha.Color) {
			// 		node->outputs[but->output]->default_value[0] = zui_color_r(color) / 255;
			// 		node->outputs[but->output]->default_value[1] = zui_color_g(color) / 255;
			// 		node->outputs[but->output]->default_value[2] = zui_color_b(color) / 255;
			// 		current->changed = true;
			// 	};
			// });
			val[0] = zui_color_r(nhandle->color) / 255;
			val[1] = zui_color_g(nhandle->color) / 255;
			val[2] = zui_color_b(nhandle->color) / 255;
		}
		else if (strcmp(but->type, "VECTOR") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			float min = but->min != 0 ? but->min : 0.0; // != NULL
			float max = but->max != 1 ? but->max : 1.0; // != NULL
			float text_off = current->ops.theme.TEXT_OFFSET;
			current->ops.theme.TEXT_OFFSET = 6;
			zui_text(zui_tr(but->name), ZUI_ALIGN_LEFT, 0);
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
			float *val = but->default_value;
			// val[0] = zui_slider(nhandle.nest(buti).nest(0, {value: val[0]}), "X", min, max, true, 100, true, Left);
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
			// val[1] = zui_slider(nhandle.nest(buti).nest(1, {value: val[1]}), "Y", min, max, true, 100, true, Left);
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
			// val[2] = zui_slider(nhandle.nest(buti).nest(2, {value: val[2]}), "Z", min, max, true, 100, true, Left);
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
			current->ops.theme.TEXT_OFFSET = text_off;
			if (but->output != NULL) node->outputs[but->output]->default_value = but->default_value;
			ny += lineh * 3;
		}
		else if (strcmp(but->type, "VALUE") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			zui_node_socket_t *soc = &node->outputs[but->output];
			float min = but->min != 0 ? but->min : 0.0; // != NULL
			float max = but->max != 1 ? but->max : 1.0; // != NULL
			float prec = but->precision != 100 ? but->precision : 100.0; // != NULL
			float text_off = current->ops.theme.TEXT_OFFSET;
			current->ops.theme.TEXT_OFFSET = 6;
			// soc->default_value = zui_slider(nhandle.nest(buti, {value: soc->default_value}), "Value", min, max, true, prec, true, Left);
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
			current->ops.theme.TEXT_OFFSET = text_off;
		}
		else if (strcmp(but->type, "STRING") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			zui_node_socket_t *soc = but->output != NULL ? &node->outputs[but->output] : NULL;
			// but->default_value = zui_text_input(nhandle.nest(buti, {text: soc != NULL ? soc->default_value : but->default_value != NULL ? but->default_value : ""}), zui_tr(but->name));
			if (soc != NULL) soc->default_value = but->default_value;
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
		}
		else if (strcmp(but->type, "ENUM") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			// char **texts = Std.isOfType(but->data, Array) ? [for (s in cast(but->data, Array<Dynamic>)) zui_tr(s)] : zui_enum_texts(node->type);
			zui_handle_t *but_handle = zui_nest(nhandle, buti);
			but_handle->position = but->default_value;
			// but->default_value = zui_combo(but_handle, texts, zui_tr(but->name));
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
		}
		else if (strcmp(but->type, "BOOL") == 0) {
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			// but->default_value = zui_check(nhandle.nest(buti, {selected: but->default_value}), zui_tr(but->name));
			if (current->is_hovered && but->tooltip != NULL) zui_tooltip(zui_tr(but->tooltip));
		}
		else if (strcmp(but->type, "CUSTOM") == 0) { // Calls external function for custom button drawing
			ny += lineh;
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			// int dot = strrchr(but->name, '.') - but->name; // TNodeButton.name specifies external function path
			// void *fn = Reflect.field(Type.resolveClass(but->name.substr(0, dot)), but->name.substr(dot + 1));
			// fn(ui, this, node);
			ny += lineh * (but->height - 1); // TNodeButton.height specifies vertical button size
		}
	}
	ny += lineh / 3; // Fix align

	// Inputs
	for (int i = 0; i < node->inputs_count; ++i) {
		zui_node_socket_t *inp = &node->inputs[i];
		ny += lineh;
		g2_set_color(inp->color);
		g2_draw_scaled_render_target(&zui_socket_image, nx - zui_p(6), ny - zui_p(3), zui_p(12), zui_p(12));
		bool is_linked = zui_input_linked(canvas, node->id, i);
		if (!is_linked && strcmp(inp->type, "VALUE") == 0) {
			current->_x = nx + zui_p(6);
			current->_y = ny - zui_p(9);
			current->_w = w - zui_p(6);
			zui_node_socket_t *soc = inp;
			float min = soc->min != 0 ? soc->min : 0.0; // != NULL
			float max = soc->max != 1 ? soc->max : 1.0; // != NULL
			float prec = soc->precision != 100 ? soc->precision : 100.0; // != NULL
			float text_off = current->ops.theme.TEXT_OFFSET;
			current->ops.theme.TEXT_OFFSET = 6;
			// soc->default_value = zui_slider(nhandle.nest(zui_max_buttons).nest(i, {value: soc->default_value}), zui_tr(inp->name), min, max, true, prec, true, Left);
			if (current->is_hovered && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));
			current->ops.theme.TEXT_OFFSET = text_off;
		}
		else if (!is_linked && strcmp(inp->type, "STRING") == 0) {
			current->_x = nx + zui_p(6);
			current->_y = ny - zui_p(9);
			current->_w = w - zui_p(6);
			zui_node_socket_t *soc = inp;
			float text_off = current->ops.theme.TEXT_OFFSET;
			current->ops.theme.TEXT_OFFSET = 6;
			// soc->default_value = zui_text_input(nhandle.nest(zui_max_buttons).nest(i, {text: soc->default_value}), zui_tr(inp->name), Left);
			if (current->is_hovered && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));
			current->ops.theme.TEXT_OFFSET = text_off;
		}
		else if (!is_linked && strcmp(inp->type, "RGBA") == 0) {
			g2_set_color(current->ops.theme.LABEL_COL);
			g2_draw_string(zui_tr(inp->name), nx + zui_p(12), ny - zui_p(3));
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			if (zui_get_hover(lineh) && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));
			zui_node_socket_t *soc = inp;
			g2_set_color(0xff000000);
			g2_fill_rect(nx + w - zui_p(38), ny - zui_p(6), zui_p(36), zui_p(18));
			float *val = soc->default_value;
			g2_set_color(zui_color(val[0] * 255, val[1] * 255, val[2] * 255, 255));
			float rx = nx + w - zui_p(37);
			float ry = ny - zui_p(5);
			float rw = zui_p(34);
			float rh = zui_p(16);
			g2_fill_rect(rx, ry, rw, rh);
			float ix = current->input_x - wx;
			float iy = current->input_y - wy;
			if (current->input_started && ix > rx && iy > ry && ix < rx + rw && iy < ry + rh) {
				current_nodes->_input_started = current->input_started = false;
				zui_rgba_popup(nhandle, soc->default_value, (int)(rx), (int)(ry + ZUI_ELEMENT_H()));
			}
		}
		else if (!is_linked && strcmp(inp->type, "VECTOR") == 0 && inp->display == 1) {
			g2_set_color(current->ops.theme.LABEL_COL);
			g2_draw_string(zui_tr(inp->name), nx + zui_p(12), ny - zui_p(3));
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			if (zui_get_hover(lineh) && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));

			ny += lineh / 2;
			current->_y = ny;
			float min = inp->min != 0 ? inp->min : 0.0; // != NULL
			float max = inp->max != 1 ? inp->max : 1.0; // != NULL
			float text_off = current->ops.theme.TEXT_OFFSET;
			current->ops.theme.TEXT_OFFSET = 6;
			float *val = inp->default_value;
			// val[0] = zui_slider(nhandle.nest(zui_max_buttons).nest(i).nest(0, {value: val[0]}), "X", min, max, true, 100, true, Left);
			if (current->is_hovered && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));
			// val[1] = zui_slider(nhandle.nest(zui_max_buttons).nest(i).nest(1, {value: val[1]}), "Y", min, max, true, 100, true, Left);
			if (current->is_hovered && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));
			// val[2] = zui_slider(nhandle.nest(zui_max_buttons).nest(i).nest(2, {value: val[2]}), "Z", min, max, true, 100, true, Left);
			if (current->is_hovered && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));
			current->ops.theme.TEXT_OFFSET = text_off;
			ny += lineh * 2.5;
		}
		else {
			g2_set_color(current->ops.theme.LABEL_COL);
			g2_draw_string(zui_tr(inp->name), nx + zui_p(12), ny - zui_p(3));
			current->_x = nx;
			current->_y = ny;
			current->_w = w;
			if (zui_get_hover(lineh) && inp->tooltip != NULL) zui_tooltip(zui_tr(inp->tooltip));
		}
		if (zui_on_socket_released != NULL && current->input_enabled && (current->input_released || current->input_released_r)) {
			if (current->input_x > wx + nx && current->input_x < wx + nx + w && current->input_y > wy + ny && current->input_y < wy + ny + lineh) {
				zui_on_socket_released(inp);
				zui_socket_released = true;
			}
		}
	}

	current->_x = ui_x;
	current->_y = ui_y;
	current->_w = ui_w;
	current->input_started = current_nodes->_input_started;
}

void zui_node_canvas(zui_node_canvas_t *canvas) {
	zui_t *current = zui_get_current();
	if (!zui_nodes_elements_baked) zui_nodes_bake_elements();

	float wx = current->_window_x;
	float wy = current->_window_y;
	bool _input_enabled = current->input_enabled;
	current->input_enabled = _input_enabled && zui_popup_commands == NULL;
	zui_canvas_control_t controls = zui_on_canvas_control != NULL ? zui_on_canvas_control() : zui_on_default_canvas_control();
	zui_socket_released = false;

	// Pan canvas
	if (current->input_enabled && (controls.pan_x != 0.0 || controls.pan_y != 0.0)) {
		current_nodes->pan_x += controls.pan_x / ZUI_NODES_SCALE();
		current_nodes->pan_y += controls.pan_y / ZUI_NODES_SCALE();
	}

	// Zoom canvas
	if (current->input_enabled && controls.zoom != 0.0) {
		current_nodes->zoom += controls.zoom;
		if (current_nodes->zoom < 0.1) current_nodes->zoom = 0.1;
		else if (current_nodes->zoom > 1.0) current_nodes->zoom = 1.0;
		current_nodes->zoom = round(current_nodes->zoom * 10) / 10;
		current_nodes->uiw = current->_w;
		current_nodes->uih = current->_h;
	}
	current_nodes->scale_factor = ZUI_SCALE();
	current_nodes->ELEMENT_H = current->ops.theme.ELEMENT_H + 2;
	zui_set_scale(ZUI_NODES_SCALE()); // Apply zoomed scale
	current->elements_baked = true;
	g2_set_font(current->ops.font, current->font_size);

	for (int i = 0; i < canvas->links_count; ++i) {
		zui_node_link_t *link = &canvas->links[i];
		zui_node_t *from = zui_get_node(canvas->nodes, canvas->nodes_count, link->from_id);
		zui_node_t *to = zui_get_node(canvas->nodes, canvas->nodes_count, link->to_id);
		float from_x = from == NULL ? current->input_x : wx + ZUI_NODE_X(from) + ZUI_NODE_W(from);
		float from_y = from == NULL ? current->input_y : wy + ZUI_NODE_Y(from) + ZUI_OUTPUT_Y(from->outputs, from->outputs_count, link->from_socket);
		float to_x = to == NULL ? current->input_x : wx + ZUI_NODE_X(to);
		float to_y = to == NULL ? current->input_y : wy + ZUI_NODE_Y(to) + ZUI_INPUT_Y(canvas, to->inputs, to->inputs_count, link->to_socket) + ZUI_OUTPUTS_H(to->outputs, to->outputs_count) + ZUI_BUTTONS_H(to);

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
		if (current_nodes->link_drag == link) {
			if (current_nodes->snap_from_id != -1) {
				from_x = current_nodes->snap_x;
				from_y = current_nodes->snap_y;
			}
			if (current_nodes->snap_to_id != -1) {
				to_x = current_nodes->snap_x;
				to_y = current_nodes->snap_y;
			}
			current_nodes->snap_from_id = current_nodes->snap_to_id = -1;

			for (int j = 0; j < canvas->nodes_count; ++j) {
				zui_node_t *node = canvas->nodes[j];
				zui_node_socket_t **inps = node->inputs;
				zui_node_socket_t **outs = node->outputs;
				float node_h = ZUI_NODE_H(canvas, node);
				float rx = wx + ZUI_NODE_X(node) - ZUI_LINE_H() / 2;
				float ry = wy + ZUI_NODE_Y(node) - ZUI_LINE_H() / 2;
				float rw = ZUI_NODE_W(node) + ZUI_LINE_H();
				float rh = node_h + ZUI_LINE_H();
				if (zui_input_in_rect(rx, ry, rw, rh)) {
					if (from == NULL && node->id != to->id) { // Snap to output
						for (int k = 0; k < node->outputs_count; ++k) {
							float sx = wx + ZUI_NODE_X(node) + ZUI_NODE_W(node);
							float sy = wy + ZUI_NODE_Y(node) + ZUI_OUTPUT_Y(outs, k);
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
						for (int k = 0; k < node->inputs_count; ++k) {
							float sx = wx + ZUI_NODE_X(node);
							float sy = wy + ZUI_NODE_Y(node) + ZUI_INPUT_Y(canvas, inps, node->inputs_count, k) + ZUI_OUTPUTS_H(outs, node->outputs_count) + ZUI_BUTTONS_H(node);
							float rx = sx - ZUI_LINE_H() / 2;
							float ry = sy - ZUI_LINE_H() / 2;
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
		for (int j = 0; j < current_nodes->nodes_selected_count; ++j) {
			zui_node_t *n = &current_nodes->nodes_selected[j];
			if (link->from_id == n->id || link->to_id == n->id) {
				selected = true;
				break;
			}
		}

		zui_draw_link(from_x - wx, from_y - wy, to_x - wx, to_y - wy, selected);
	}

	for (int i = 0; i < canvas->nodes_count; ++i) {
		zui_node_t *node = canvas->nodes[i];

		// Cull
		if (ZUI_NODE_X(node) > current->_window_w || ZUI_NODE_X(node) + ZUI_NODE_W(node) < 0 ||
			ZUI_NODE_Y(node) > current->_window_h || ZUI_NODE_Y(node) + ZUI_NODE_H(canvas, node) < 0) {
			if (!zui_is_selected(node)) continue;
		}

		zui_node_socket_t **inps = node->inputs;
		zui_node_socket_t **outs = node->outputs;

		// Drag node
		float node_h = ZUI_NODE_H(canvas, node);
		if (current->input_enabled && zui_input_in_rect(wx + ZUI_NODE_X(node) - ZUI_LINE_H() / 2, wy + ZUI_NODE_Y(node), ZUI_NODE_W(node) + ZUI_LINE_H(), ZUI_LINE_H())) {
			if (current->input_started) {
				if (current->is_shift_down || current->is_ctrl_down) {
					// Add to selection or deselect
					// zui_is_selected(node) ?
					// 	current_nodes->nodes_selected.remove(node) :
					// 	current_nodes->nodes_selected.push(node);
				}
				else if (current_nodes->nodes_selected_count <= 1) {
					// Selecting single node, otherwise wait for input release
					// current_nodes->nodes_selected = [node];
					current_nodes->nodes_selected[0] = *node;
					current_nodes->nodes_selected_count = 1;
				}
				current_nodes->move_on_top = node; // Place selected node on top
				current_nodes->nodes_drag = true;
				current_nodes->dragged = false;
			}
			else if (current->input_released && !current->is_shift_down && !current->is_ctrl_down && !current_nodes->dragged) {
				// No drag performed, select single node
				// nodes_selected = [node];
				current_nodes->nodes_selected[0] = *node;
				current_nodes->nodes_selected_count = 1;
				if (zui_on_header_released != NULL) {
					zui_on_header_released(node);
				}
			}
		}
		if (current->input_started && zui_input_in_rect(wx + ZUI_NODE_X(node) - ZUI_LINE_H() / 2, wy + ZUI_NODE_Y(node) - ZUI_LINE_H() / 2, ZUI_NODE_W(node) + ZUI_LINE_H(), node_h + ZUI_LINE_H())) {
			// Check sockets
			if (current_nodes->link_drag == NULL) {
				for (int j = 0; j < node->outputs_count; ++j) {
					float sx = wx + ZUI_NODE_X(node) + ZUI_NODE_W(node);
					float sy = wy + ZUI_NODE_Y(node) + ZUI_OUTPUT_Y(outs, j);
					if (zui_input_in_rect(sx - ZUI_LINE_H() / 2, sy - ZUI_LINE_H() / 2, ZUI_LINE_H(), ZUI_LINE_H())) {
						// New link from output
						// zui_node_link_t *l = { id: zui_get_link_id(canvas.links), from_id: node->id, from_socket: j, to_id: -1, to_socket: -1 };
						// canvas.links.push(l);
						// current_nodes->link_drag = l;
						current_nodes->is_new_link = true;
						break;
					}
				}
			}
			if (current_nodes->link_drag == NULL) {
				for (int j = 0; j < node->inputs_count; ++j) {
					float sx = wx + ZUI_NODE_X(node);
					float sy = wy + ZUI_NODE_Y(node) + ZUI_INPUT_Y(canvas, inps, node->inputs_count, j) + ZUI_OUTPUTS_H(outs, node->outputs_count) + ZUI_BUTTONS_H(node);
					if (zui_input_in_rect(sx - ZUI_LINE_H() / 2, sy - ZUI_LINE_H() / 2, ZUI_LINE_H(), ZUI_LINE_H())) {
						// Already has a link - disconnect
						for (int k = 0; k < canvas->links_count; ++k) {
							zui_node_link_t *l = &canvas->links[k];
							if (l->to_id == node->id && l->to_socket == j) {
								l->to_id = l->to_socket = -1;
								current_nodes->link_drag = l;
								current_nodes->is_new_link = false;
								break;
							}
						}
						if (current_nodes->link_drag != NULL) break;
						// New link from input
						// zui_node_link_t l = {
						// 	id: zui_get_link_id(canvas.links),
						// 	from_id: -1,
						// 	from_socket: -1,
						// 	to_id: node->id,
						// 	to_socket: j
						// };
						// canvas.links.push(l);
						// current_nodes->link_drag = l;
						current_nodes->is_new_link = true;
						break;
					}
				}
			}
		}
		else if (current->input_released) {
			if (current_nodes->snap_to_id != -1) { // Connect to input
				// Force single link per input
				for (int j = 0; j < canvas->links_count; ++j) {
					zui_node_link_t *l = &canvas->links[j];
					if (l->to_id == current_nodes->snap_to_id && l->to_socket == current_nodes->snap_socket) {
						// canvas.links.remove(l);
						break;
					}
				}
				current_nodes->link_drag->to_id = current_nodes->snap_to_id;
				current_nodes->link_drag->to_socket = current_nodes->snap_socket;
				current->changed = true;
			}
			else if (current_nodes->snap_from_id != -1) { // Connect to output
				current_nodes->link_drag->from_id = current_nodes->snap_from_id;
				current_nodes->link_drag->from_socket = current_nodes->snap_socket;
				current->changed = true;
			}
			else if (current_nodes->link_drag != NULL) { // Remove dragged link
				// canvas.links.remove(link_drag);
				current->changed = true;
				if (zui_on_link_drag != NULL) {
					zui_on_link_drag(current_nodes->link_drag, current_nodes->is_new_link);
				}
			}
			current_nodes->snap_to_id = current_nodes->snap_from_id = -1;
			current_nodes->link_drag = NULL;
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

	if (zui_on_canvas_released != NULL && current->input_enabled && (current->input_released || current->input_released_r) && !zui_socket_released) {
		zui_on_canvas_released();
	}

	if (zui_box_select) {
		g2_set_color(0x223333dd);
		g2_fill_rect(zui_box_select_x, zui_box_select_y, current->input_x - zui_box_select_x - current->_window_x, current->input_y - zui_box_select_y - current->_window_y);
		g2_set_color(0x773333dd);
		g2_draw_rect(zui_box_select_x, zui_box_select_y, current->input_x - zui_box_select_x - current->_window_x, current->input_y - zui_box_select_y - current->_window_y, 1);
		g2_set_color(0xffffffff);
	}
	if (current->input_enabled && current->input_started && !current->is_alt_down && current_nodes->link_drag == NULL && !current_nodes->nodes_drag && !current->changed) {
		zui_box_select = true;
		zui_box_select_x = current->input_x - current->_window_x;
		zui_box_select_y = current->input_y - current->_window_y;
	}
	else if (zui_box_select && !current->input_down) {
		zui_box_select = false;
		// zui_node_t **nodes = [];
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
		for (int j = 0; j < canvas->nodes_count; ++j) {
			zui_node_t *n = &canvas->nodes[j];
			if (ZUI_NODE_X(n) + ZUI_NODE_W(n) > left && ZUI_NODE_X(n) < right &&
				ZUI_NODE_Y(n) + ZUI_NODE_H(canvas, n) > top && ZUI_NODE_Y(n) < bottom) {
				// nodes.push(n);
			}
		}
		// (current->is_shift_down || current->is_ctrl_down) ? for (n in nodes) nodes_selected.push(n) : nodes_selected = nodes;
	}

	// Place selected node on top
	if (current_nodes->move_on_top != NULL) {
		// canvas->nodes.remove(current_nodes->move_on_top);
		// canvas->nodes.push(current_nodes->move_on_top);
		current_nodes->move_on_top = NULL;
	}

	// Node copy & paste
	bool cut_selected = false;
	// if (zui_is_copy) {
	if (false) {
		zui_node_t **copy_nodes;
		for (int i = 0; i < current_nodes->nodes_selected_count; ++i) {
			zui_node_t *n = &current_nodes->nodes_selected[i];
			// if (zui_exclude_remove.indexOf(n.type) >= 0) continue;
			// copy_nodes.push(n);
		}
		zui_node_link_t **copy_links;
		for (int i = 0; i < canvas->links_count; ++i) {
			zui_node_link_t *l = &canvas->links[i];
			zui_node_t *from = zui_get_node(current_nodes->nodes_selected, current_nodes->nodes_selected_count, l->from_id);
			zui_node_t *to = zui_get_node(current_nodes->nodes_selected, current_nodes->nodes_selected_count, l->to_id);
			// if (from != NULL && zui_exclude_remove.indexOf(from->type) == -1 &&
				// to != NULL && zui_exclude_remove.indexOf(to->type) == -1) {
				// copy_links.push(l);
			// }
		}
		// if (copy_nodes_count > 0) {
			// zui_node_canvas_t copyCanvas = {
				// name: canvas.name,
				// nodes: copy_nodes,
				// links: copy_links
			// };
			// zui_clipboard = haxe.Json.stringify(copyCanvas);
		// }
		// cut_selected = zui_is_cut;
	}

	// if (zui_is_paste && !current->is_typing) {
	if (false) {
		zui_node_canvas_t *paste_canvas = NULL;
		// Clipboard can contain non-json data
		// try {
		// 	paste_canvas = haxe.Json.parse(zui_clipboard);
		// }
		// catch(_) {}
		if (paste_canvas != NULL) {
			for (int i = 0; i < paste_canvas->links_count; ++i) {
				zui_node_link_t *l = &paste_canvas->links[i];
				// Assign unique link id
				l->id = zui_get_link_id(canvas->links, canvas->links_count);
				// canvas.links.push(l);
			}
			int offset_x = (int)(((int)(current->input_x / ZUI_SCALE()) * ZUI_NODES_SCALE() - wx - ZUI_PAN_X()) / ZUI_NODES_SCALE()) - paste_canvas->nodes[paste_canvas->nodes_count - 1]->x;
			int offset_y = (int)(((int)(current->input_y / ZUI_SCALE()) * ZUI_NODES_SCALE() - wy - ZUI_PAN_Y()) / ZUI_NODES_SCALE()) - paste_canvas->nodes[paste_canvas->nodes_count - 1]->y;
			for (int i = 0; i < paste_canvas->nodes_count; ++i) {
				zui_node_t *n = &paste_canvas->nodes[i];
				// Assign unique node id
				int old_id = n->id;
				n->id = zui_get_node_id(canvas->nodes, canvas->nodes_count);

				for (int j = 0; j < n->inputs_count; ++j) {
					zui_node_socket_t *soc = n->inputs[j];
					soc->id = zui_get_socket_id(canvas->nodes, canvas->nodes_count);
					soc->node_id = n->id;
				}
				for (int j = 0; j < n->outputs_count; ++j) {
					zui_node_socket_t *soc = n->outputs[j];
					soc->id = zui_get_socket_id(canvas->nodes, canvas->nodes_count);
					soc->node_id = n->id;
				}
				for (int j = 0; j < paste_canvas->links_count; ++j) {
					zui_node_link_t *l = &paste_canvas->links[j];
					if (l->from_id == old_id) l->from_id = n->id;
					else if (l->to_id == old_id) l->to_id = n->id;
				}
				n->x += offset_x;
				n->y += offset_y;
				// canvas->nodes.push(n);
			}
			current_nodes->nodes_drag = true;
			current_nodes->nodes_selected = paste_canvas->nodes;
			current->changed = true;
		}
	}

	// // Select all nodes
	if (current->is_ctrl_down && current->key_code == KINC_KEY_A && !current->is_typing) {
		// current_nodes->nodes_selected = [];
		// for (n in canvas->nodes) current_nodes->nodes_selected.push(n);
	}

	// Node removal
	if (current->input_enabled && (current->is_backspace_down || current->is_delete_down || cut_selected) && !current->is_typing) {
		int i = current_nodes->nodes_selected_count - 1;
		while (i >= 0) {
			zui_node_t *n = &current_nodes->nodes_selected[i--];
			// if (zui_exclude_remove.indexOf(n.type) >= 0) continue;
			zui_remove_node(n, canvas);
			current->changed = true;
		}
	}

	zui_set_scale(current_nodes->scale_factor); // Restore non-zoomed scale
	current->elements_baked = true;
	current->input_enabled = _input_enabled;

	if (zui_popup_commands != NULL) {
		current->_x = zui_popup_x;
		current->_y = zui_popup_y;
		current->_w = zui_popup_w;

		zui_fill(-6, -6, current->_w / ZUI_SCALE() + 12, zui_popup_h + 12, current->ops.theme.ACCENT_SELECT_COL);
		zui_fill(-5, -5, current->_w / ZUI_SCALE() + 10, zui_popup_h + 10, current->ops.theme.SEPARATOR_COL);
		zui_popup_commands(current);

		bool hide = (current->input_started || current->input_started_r) && (current->input_x - wx < zui_popup_x - 6 || current->input_x - wx > zui_popup_x + zui_popup_w + 6 || current->input_y - wy < zui_popup_y - 6 || current->input_y - wy > zui_popup_y + zui_popup_h * ZUI_SCALE() + 6);
		if (hide || current->is_escape_down) {
			zui_popup_commands = NULL;
		}
	}
}
