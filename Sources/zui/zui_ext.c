#include "zui_ext.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <kinc/input/keyboard.h>
#include <kinc/io/filereader.h>
#include <kinc/system.h>

#define MATH_PI 3.14159265358979323846

static char data_path[128] = "";
static char last_path[512];
static zui_handle_t *wheel_selected_handle = NULL;
static zui_handle_t *gradient_selected_handle = NULL;
static zui_handle_t radio_handle;
static int _ELEMENT_OFFSET = 0;
static int _BUTTON_COL = 0;
static int text_area_selection_start = -1;
bool zui_text_area_line_numbers = false;
bool zui_text_area_scroll_past_end = false;
zui_text_coloring_t *zui_text_area_coloring = NULL;

float zui_dist(float x1, float y1, float x2, float y2) {
	float vx = x1 - x2;
	float vy = y1 - y2;
	return sqrt(vx * vx + vy * vy);
}

float zui_fract(float f) {
	return f - (int)f;
}

float zui_mix(float x, float y, float a) {
	return x * (1.0 - a) + y * a;
}

float zui_clamp(float x, float min_val, float max_val) {
	return fmin(fmax(x, min_val), max_val);
}

float zui_step(float edge, float x) {
	return x < edge ? 0.0 : 1.0;
}

static const float kx = 1.0;
static const float ky = 2.0 / 3.0;
static const float kz = 1.0 / 3.0;
static const float kw = 3.0;
static float ar[] = {0.0, 0.0, 0.0};
void zui_hsv_to_rgb(float cr, float cg, float cb, float *out) {
	float px = fabs(zui_fract(cr + kx) * 6.0 - kw);
	float py = fabs(zui_fract(cr + ky) * 6.0 - kw);
	float pz = fabs(zui_fract(cr + kz) * 6.0 - kw);
	out[0] = cb * zui_mix(kx, zui_clamp(px - kx, 0.0, 1.0), cg);
	out[1] = cb * zui_mix(kx, zui_clamp(py - kx, 0.0, 1.0), cg);
	out[2] = cb * zui_mix(kx, zui_clamp(pz - kx, 0.0, 1.0), cg);
}

static const float Kx = 0.0;
static const float Ky = -1.0 / 3.0;
static const float Kz = 2.0 / 3.0;
static const float Kw = -1.0;
static const float e = 1.0e-10;
void zui_rgb_to_hsv(float cr, float cg, float cb, float *out) {
	float px = zui_mix(cb, cg, zui_step(cb, cg));
	float py = zui_mix(cg, cb, zui_step(cb, cg));
	float pz = zui_mix(Kw, Kx, zui_step(cb, cg));
	float pw = zui_mix(Kz, Ky, zui_step(cb, cg));
	float qx = zui_mix(px, cr, zui_step(px, cr));
	float qy = zui_mix(py, py, zui_step(px, cr));
	float qz = zui_mix(pw, pz, zui_step(px, cr));
	float qw = zui_mix(cr, px, zui_step(px, cr));
	float d = qx - fmin(qw, qy);
	out[0] = fabs(qz + (qw - qy) / (6.0 * d + e));
	out[1] = d / (qx + e);
	out[2] = qx;
}

float zui_float_input(zui_handle_t *handle, char *label, int align, float precision) {
	sprintf(handle->text, "%f", round(handle->value * precision) / precision);
	char *text = zui_text_input(handle, label, align, true, false);
	handle->value = atof(text);
	return handle->value;
}

void zui_init_path(zui_handle_t *handle, const char *system_id) {
	strcpy(handle->text, strcmp(system_id, "Windows") == 0 ? "C:\\Users" : "/");
	// %HOMEDRIVE% + %HomePath%
	// ~
}

char *zui_str_replace(char *str, char *what, char *with) {
	char *result = str;
	int what_len = strlen(what);
	int with_len = strlen(with);
	char *ins = str;
	char *tmp;
	int count;
	for (count = 0; (tmp = strstr(ins, what)); ++count) {
		ins = tmp + what_len;
	}

	char buf[512];
	tmp = &buf[0];
	while (count--) {
		ins = strstr(str, what);
		int len_front = ins - str;
		tmp = strncpy(tmp, str, len_front) + len_front;
		tmp = strcpy(tmp, with) + with_len;
		str += len_front + what_len;
	}
	strcpy(tmp, str);
	strcpy(result, buf);
	return result;
}

char *zui_file_browser(zui_handle_t *handle, bool folders_only) {
	zui_t *current = zui_get_current();
	const char *sep = "/";

	char cmd[64];
	strcpy(cmd, "ls ");
	const char *system_id = kinc_system_id();
	if (strcmp(system_id, "Windows") == 0) {
		strcpy(cmd, "dir /b ");
		if (folders_only) strcat(cmd, "/ad ");
		sep = "\\";
		zui_str_replace(handle->text, "\\\\", "\\");
		zui_str_replace(handle->text, "\r", "");
	}
	if (handle->text[0] == '\0') zui_init_path(handle, system_id);

	char save[256];
	strcpy(save, kinc_internal_get_files_location());
	strcat(save, sep);
	strcat(save, data_path);
	strcat(save, "dir.txt");
	if (strcmp(handle->text, last_path) != 0) {
		char str[512];
		strcpy(str, cmd);
		strcat(str, "\"");
		strcat(str, handle->text);
		strcat(str, "\" > \"");
		strcat(str, save);
		strcat(str, "\"");
	}
	strcpy(last_path, handle->text);

	kinc_file_reader_t reader;
	if (!kinc_file_reader_open(&reader, save, KINC_FILE_TYPE_ASSET)) return NULL;
	int reader_size = (int)kinc_file_reader_size(&reader);

	char str[2048]; // reader_size
	kinc_file_reader_read(&reader, str, reader_size);
	kinc_file_reader_close(&reader);

	// Up directory
	int i1 = strstr(handle->text, "/") - handle->text;
	int i2 = strstr(handle->text, "\\") - handle->text;
	bool nested =
		(i1 > -1 && strlen(handle->text) - 1 > i1) ||
		(i2 > -1 && strlen(handle->text) - 1 > i2);
	handle->changed = false;
	if (nested && zui_button("..", ZUI_ALIGN_LEFT, "")) {
		handle->changed = current->changed = true;
		handle->text[strrchr(handle->text, sep[0]) - handle->text] = 0;
		// Drive root
		if (strlen(handle->text) == 2 && handle->text[1] == ':') strcat(handle->text, sep);
	}

	// Directory contents
	int count = zui_line_count(str);
	for (int i = 0; i < count; ++i) {
		char *f = zui_extract_line(str, i);
		if (f[0] == '\0' || f[0] == '.') continue; // Skip hidden
		if (zui_button(f, ZUI_ALIGN_LEFT, "")) {
			handle->changed = current->changed = true;
			if (handle->text[strlen(handle->text) - 1] != sep[0]) strcat(handle->text, sep);
			strcat(handle->text, f);
		}
	}

	return handle->text;
}

int zui_inline_radio(zui_handle_t *handle, char **texts, int count, int align) {
	zui_t *current = zui_get_current();

	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return handle->position;
	}
	float step = current->_w / count;
	int hovered = -1;
	if (zui_get_hover(ZUI_ELEMENT_H())) {
		int ix = current->input_x - current->_x - current->_window_x;
		for (int i = 0; i < count; ++i) {
			if (ix < i * step + step) {
				hovered = i;
				break;
			}
		}
	}
	if (zui_get_released(ZUI_ELEMENT_H())) {
		handle->position = hovered;
		handle->changed = current->changed = true;
	}
	else handle->changed = false;

	for (int i = 0; i < count; ++i) {
		if (handle->position == i) {
			g2_set_color(current->ops.theme->ACCENT_HOVER_COL);
			if (!current->enabled) zui_fade_color(0.25);
			zui_draw_rect(true, current->_x + step * i, current->_y + current->button_offset_y, step, ZUI_BUTTON_H());
		}
		else if (hovered == i) {
			g2_set_color(current->ops.theme->ACCENT_COL);
			if (!current->enabled) zui_fade_color(0.25);
			zui_draw_rect(false, current->_x + step * i, current->_y + current->button_offset_y, step, ZUI_BUTTON_H());
		}
		g2_set_color(current->ops.theme->TEXT_COL); // Text
		current->_x += step * i;
		float _w = current->_w;
		current->_w = (int)step;
		zui_draw_string(texts[i], current->ops.theme->TEXT_OFFSET, 0, align, true);
		current->_x -= step * i;
		current->_w = _w;
	}
	zui_end_element();
	return handle->position;
}

uint8_t zui_color_r(uint32_t color) {
	return (color & 0x00ff0000) >> 16;
}

uint8_t zui_color_g(uint32_t color) {
	return (color & 0x0000ff00) >>  8;
}

uint8_t zui_color_b(uint32_t color) {
	return (color & 0x000000ff);
}

uint8_t zui_color_a(uint32_t color) {
	return (color) >> 24;
}

uint32_t zui_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (a << 24) | (r << 16) | (g << 8) | b;
}

static zui_handle_t temp0;
static zui_handle_t temp1;
static zui_handle_t temp2;

int zui_color_wheel(zui_handle_t *handle, bool alpha, float w, float h, bool color_preview, void (*picker)(void *), void *data) {
	zui_t *current = zui_get_current();
	if (w < 0) w = current->_w;
	float r = zui_color_r(handle->color) / 255.0f;
	float g = zui_color_g(handle->color) / 255.0f;
	float b = zui_color_b(handle->color) / 255.0f;
	zui_rgb_to_hsv(r, g, b, ar);
	float chue = ar[0];
	float csat = ar[1];
	float cval = ar[2];
	float calpha = zui_color_a(handle->color) / 255.0f;
	// Wheel
	float px = current->_x;
	float py = current->_y;
	bool scroll = current->current_window != NULL ? current->current_window->scroll_enabled : false;
	if (!scroll) {
		w -= ZUI_SCROLL_W();
		px += ZUI_SCROLL_W() / 2;
	}
	float _x = current->_x;
	float _y = current->_y;
	float _w = current->_w;
	current->_w = (int)(28 * ZUI_SCALE());
	if (picker != NULL && zui_button("P", ZUI_ALIGN_LEFT, "")) {
		(*picker)(data);
		current->changed = false;
		handle->changed = false;
		return handle->color;
	}
	current->_x = _x;
	current->_y = _y;
	current->_w = _w;

	uint32_t col = zui_color(round(cval * 255), round(cval * 255), round(cval * 255), 255);
	zui_image(current->ops.color_wheel, false, col, -1);
	// Picker
	float ph = current->_y - py;
	float ox = px + w / 2;
	float oy = py + ph / 2;
	float cw = w * 0.7;
	float cwh = cw / 2;
	float cx = ox;
	float cy = oy + csat * cwh; // Sat is distance from center
	float grad_tx = px + 0.897 * w;
	float grad_ty = oy - cwh;
	float grad_w = 0.0777 * w;
	float grad_h = cw;
	// Rotate around origin by hue
	float theta = chue * (MATH_PI * 2.0);
	float cx2 = cos(theta) * (cx - ox) - sin(theta) * (cy - oy) + ox;
	float cy2 = sin(theta) * (cx - ox) + cos(theta) * (cy - oy) + oy;
	cx = cx2;
	cy = cy2;

	current->_x = px - (scroll ? 0 : ZUI_SCROLL_W() / 2);
	current->_y = py;
	zui_image(current->ops.black_white_gradient, false, 0xffffffff, -1);

	g2_set_color(0xff000000);
	g2_fill_rect(cx - 3 * ZUI_SCALE(), cy - 3 * ZUI_SCALE(), 6 * ZUI_SCALE(), 6 * ZUI_SCALE());
	g2_set_color(0xffffffff);
	g2_fill_rect(cx - 2 * ZUI_SCALE(), cy - 2 * ZUI_SCALE(), 4 * ZUI_SCALE(), 4 * ZUI_SCALE());

	g2_set_color(0xff000000);
	g2_fill_rect(grad_tx + grad_w / 2 - 3 * ZUI_SCALE(), grad_ty + (1 - cval) * grad_h - 3 * ZUI_SCALE(), 6 * ZUI_SCALE(), 6 * ZUI_SCALE());
	g2_set_color(0xffffffff);
	g2_fill_rect(grad_tx + grad_w / 2 - 2 * ZUI_SCALE(), grad_ty + (1 - cval) * grad_h - 2 * ZUI_SCALE(), 4 * ZUI_SCALE(), 4 * ZUI_SCALE());

	if (alpha) {
		zui_handle_t *alpha_handle = zui_nest(handle, 1);
		static bool first = true;
		if (first) {
			first = false;
			alpha_handle->value = round(calpha * 100) / 100;
		}
		calpha = zui_slider(alpha_handle, "Alpha", 0.0, 1.0, true, 100, true, ZUI_ALIGN_LEFT, true);
		if (alpha_handle->changed) handle->changed = current->changed = true;
	}
	// Mouse picking for color wheel
	float gx = ox + current->_window_x;
	float gy = oy + current->_window_y;
	if (current->input_started && zui_input_in_rect(gx - cwh, gy - cwh, cw, cw)) wheel_selected_handle = handle;
	if (current->input_released && wheel_selected_handle != NULL) { wheel_selected_handle = NULL; handle->changed = current->changed = true; }
	if (current->input_down && wheel_selected_handle == handle) {
		csat = fmin(zui_dist(gx, gy, current->input_x, current->input_y), cwh) / cwh;
		float angle = atan2(current->input_x - gx, current->input_y - gy);
		if (angle < 0) angle = MATH_PI + (MATH_PI - fabs(angle));
		angle = MATH_PI * 2 - angle;
		chue = angle / (MATH_PI * 2);
		handle->changed = current->changed = true;
	}
	// Mouse picking for cval
	if (current->input_started && zui_input_in_rect(grad_tx + current->_window_x, grad_ty + current->_window_y, grad_w, grad_h)) gradient_selected_handle = handle;
	if (current->input_released && gradient_selected_handle != NULL) { gradient_selected_handle = NULL; handle->changed = current->changed = true; }
	if (current->input_down && gradient_selected_handle == handle) {
		cval = fmax(0.01, fmin(1, 1 - (current->input_y - grad_ty - current->_window_y) / grad_h));
		handle->changed = current->changed = true;
	}
	// Save as rgb
	zui_hsv_to_rgb(chue, csat, cval, ar);
	handle->color = zui_color(round(ar[0] * 255), round(ar[1] * 255), round(ar[2] * 255), round(calpha * 255));

	if (color_preview) zui_text("", ZUI_ALIGN_RIGHT, handle->color);

	char *strings[] = {"RGB", "HSV", "Hex"};
	int pos = zui_inline_radio(&radio_handle, strings, 3, ZUI_ALIGN_LEFT);
	zui_handle_t *h0 = &temp0;
	zui_handle_t *h1 = &temp1;
	zui_handle_t *h2 = &temp2;
	if (pos == 0) {
		h0->value = zui_color_r(handle->color) / 255.0f;
		float r = zui_slider(h0, "R", 0, 1, true, 100, true, ZUI_ALIGN_LEFT, true);
		h1->value = zui_color_g(handle->color) / 255.0f;
		float g = zui_slider(h1, "G", 0, 1, true, 100, true, ZUI_ALIGN_LEFT, true);
		h2->value = zui_color_b(handle->color) / 255.0f;
		float b = zui_slider(h2, "B", 0, 1, true, 100, true, ZUI_ALIGN_LEFT, true);
		handle->color = zui_color(r * 255, g * 255, b * 255, 255);
	}
	else if (pos == 1) {
		zui_rgb_to_hsv(zui_color_r(handle->color) / 255.0f, zui_color_g(handle->color) / 255.0f, zui_color_b(handle->color) / 255.0f, ar);
		h0->value = ar[0];
		h1->value = ar[1];
		h2->value = ar[2];
		float chue = zui_slider(h0, "H", 0, 1, true, 100, true, ZUI_ALIGN_LEFT, true);
		float csat = zui_slider(h1, "S", 0, 1, true, 100, true, ZUI_ALIGN_LEFT, true);
		float cval = zui_slider(h2, "V", 0, 1, true, 100, true, ZUI_ALIGN_LEFT, true);
		zui_hsv_to_rgb(chue, csat, cval, ar);
		handle->color = zui_color(ar[0] * 255, ar[1] * 255, ar[2] * 255, 255);
	}
	else if (pos == 2) {
		sprintf(handle->text, "%x", handle->color);
		char *hex_code = zui_text_input(handle, "#", ZUI_ALIGN_LEFT, true, false);
		if (strlen(hex_code) >= 1 && hex_code[0] == '#') { // Allow # at the beginning
			hex_code = strcpy(hex_code, hex_code + 1);
		}
		if (strlen(hex_code) == 3) { // 3 digit CSS style values like fa0 --> ffaa00
			hex_code[5] = hex_code[2];
			hex_code[4] = hex_code[2];
			hex_code[3] = hex_code[1];
			hex_code[2] = hex_code[1];
			hex_code[1] = hex_code[0];
			hex_code[6] = '\0';
		}
		if (strlen(hex_code) == 4) { // 4 digit CSS style values
			hex_code[7] = hex_code[3];
			hex_code[6] = hex_code[3];
			hex_code[5] = hex_code[2];
			hex_code[4] = hex_code[2];
			hex_code[3] = hex_code[1];
			hex_code[2] = hex_code[1];
			hex_code[1] = hex_code[0];
			hex_code[8] = '\0';
		}
		if (strlen(hex_code) == 6) { // Make the alpha channel optional
			hex_code[7] = hex_code[5];
			hex_code[6] = hex_code[4];
			hex_code[5] = hex_code[3];
			hex_code[4] = hex_code[2];
			hex_code[3] = hex_code[1];
			hex_code[2] = hex_code[0];
			hex_code[0] = 'f';
			hex_code[1] = 'f';
		}
		handle->color = strtol(hex_code, NULL, 16);
	}
	if (h0->changed || h1->changed || h2->changed) handle->changed = current->changed = true;

	// Do not close if user clicks
	if (current->input_released && zui_input_in_rect(current->_window_x + px, current->_window_y + py, w, h < 0 ? (current->_y - py) : h) && current->input_released) {
		current->changed = true;
	}

	return handle->color;
}

static void scroll_align(zui_t *current, zui_handle_t *handle) {
	// Scroll down
	if ((handle->position + 1) * ZUI_ELEMENT_H() + current->current_window->scroll_offset > current->_h - current->window_header_h) {
		current->current_window->scroll_offset -= ZUI_ELEMENT_H();
	}
	// Scroll up
	else if ((handle->position + 1) * ZUI_ELEMENT_H() + current->current_window->scroll_offset < current->window_header_h) {
		current->current_window->scroll_offset += ZUI_ELEMENT_H();
	}
}

static char *right_align_number(char *s, int number, int length) {
	sprintf(s, "%d", number);
	while (strlen(s) < length) {
		for (int i = strlen(s) + 1; i > 0; --i) s[i] = s[i - 1];
		s[0] = ' ';
	}
	return s;
}

static void handle_line_select(zui_t *current, zui_handle_t *handle) {
	if (current->is_shift_down) {
		current->highlight_anchor = 0;
		if (text_area_selection_start == -1) {
			text_area_selection_start = handle->position;
		}
	}
	else text_area_selection_start = -1;
}

static int zui_word_count(char *str) {
	if (str == NULL || str[0] == '\0') return 0;
	int i = 0;
	int count = 1;
	while (str[i] != '\0') {
		if (str[i] == ' ' || str[i] == '\n') count++;
		i++;
	}
	return count;
}

static char temp[128];

static char *zui_extract_word(char *str, int word) {
	int pos = 0;
	int len = strlen(str);
	int word_i = 0;
	for (int i = 0; i < len; ++i) {
		if (str[i] == ' ' || str[i] == '\n') { word_i++; continue; }
		if (word_i < word) continue;
		if (word_i > word) break;
		temp[pos++] = str[i];
	}
	temp[pos] = 0;
	return temp;
}

static int zui_line_pos(char *str, int line) {
	return zui_extract_line(str, line) - str;
}

char *zui_text_area(zui_handle_t *handle, int align, bool editable, char *label, bool word_wrap) {
	zui_t *current = zui_get_current();
	zui_str_replace(handle->text, "\t", "    ");
	bool selected = current->text_selected_handle == handle; // Text being edited

	char lines[512];
	strcpy(lines, handle->text);
	int line_count = zui_line_count(lines);
	bool show_label = (line_count == 1 && lines[0] == '\0');
	bool key_pressed = selected && current->is_key_pressed;
	current->highlight_on_select = false;
	current->tab_switch_enabled = false;

	if (word_wrap && handle->text[0] != '\0') {
		bool cursor_set = false;
		int cursor_pos = current->cursor_x;
		for (int i = 0; i < handle->position; ++i) {
			cursor_pos += strlen(zui_extract_line(lines, i)) + 1; // + '\n'
		}
		int word_count = zui_word_count(lines);
		char line[256];
		line[0] = '\0';
		char new_lines[512];
		new_lines[0] = '\0';
		for (int i = 0; i < word_count; ++i) {
			char *w = zui_extract_word(lines, i);
			float spacew = g2_string_width(current->ops.font, current->font_size, " ");
			float wordw = spacew + g2_string_width(current->ops.font, current->font_size, w);
			float linew = wordw + g2_string_width(current->ops.font, current->font_size, line);
			if (linew > current->_w - 10 && linew > wordw) {
				strcat(new_lines, line);
				strcat(new_lines, "\n");
				line[0] = '\0';
			}

			if (line[0] == '\0') {
				strcpy(line, w);
			}
			else {
				strcat(line, " ");
				strcat(line, w);
			}

			int lines_len = line_count;
			int new_line_count = zui_line_count(new_lines);
			for (int i = 0; i < new_line_count; ++i) {
				lines_len += strlen(zui_extract_line(new_lines, i));
			}

			if (selected && !cursor_set && cursor_pos <= lines_len + strlen(line)) {
				cursor_set = true;
				handle->position = line_count;
				current->cursor_x = current->highlight_anchor = cursor_pos - lines_len;
			}
		}
		strcat(new_lines, line);
		strcat(new_lines, "\n");
		if (selected) {
			strcpy(handle->text, zui_extract_line(new_lines, handle->position));
			strcpy(current->text_selected, handle->text);
		}
		strcpy(lines, new_lines);
	}
	int cursor_start_x = current->cursor_x;

	if (zui_text_area_line_numbers) {
		float _y = current->_y;
		int _TEXT_COL = current->ops.theme->TEXT_COL;
		current->ops.theme->TEXT_COL = current->ops.theme->ACCENT_COL;
		int max_length = ceil(log(line_count + 0.5) / log(10)); // Express log_10 with natural log
		char s[64];
		for (int i = 0; i < line_count; ++i) {
			zui_text(right_align_number(&s[0], i + 1, max_length), ZUI_ALIGN_LEFT, 0x00000000);
			current->_y -= ZUI_ELEMENT_OFFSET();
		}
		current->ops.theme->TEXT_COL = _TEXT_COL;
		current->_y = _y;

		sprintf(s, "%d", line_count);
		int numbers_w = (strlen(s) * 16 + 4) * ZUI_SCALE();
		current->_x += numbers_w;
		current->_w -= numbers_w - ZUI_SCROLL_W();
	}

	g2_set_color(current->ops.theme->SEPARATOR_COL); // Background
	zui_draw_rect(true, current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w - current->button_offset_y * 2, line_count * ZUI_ELEMENT_H() - current->button_offset_y * 2);

	zui_text_coloring_t *_text_coloring = current->text_coloring;
	current->text_coloring = zui_text_area_coloring;

	if (current->input_started) text_area_selection_start = -1;

	for (int i = 0; i < line_count; ++i) { // Draw lines
		char *line = zui_extract_line(lines, i);
		// Text input
		if ((!selected && zui_get_hover(ZUI_ELEMENT_H())) || (selected && i == handle->position)) {
			handle->position = i; // Set active line
			strcpy(handle->text, line);
			current->submit_text_handle = NULL;
			zui_text_input(handle, show_label ? label : "", align, editable, false);
			if (key_pressed && current->key_code != KINC_KEY_RETURN && current->key_code != KINC_KEY_ESCAPE) { // Edit text
				strcpy(line, current->text_selected);
			}
		}
		// Text
		else {
			if (show_label) {
				int TEXT_COL = current->ops.theme->TEXT_COL;
				current->ops.theme->TEXT_COL = current->ops.theme->LABEL_COL;
				zui_text(label, ZUI_ALIGN_RIGHT, 0x00000000);
				current->ops.theme->TEXT_COL = TEXT_COL;
			}
			else {
				// Multi-line selection highlight
				if (text_area_selection_start > -1 &&
					(i >= text_area_selection_start && i < handle->position) ||
					(i <= text_area_selection_start && i > handle->position)) {
					int line_height = ZUI_ELEMENT_H();
					int cursor_height = line_height - current->button_offset_y * 3.0;
					int linew = g2_string_width(current->ops.font, current->font_size, line);
					g2_set_color(current->ops.theme->ACCENT_SELECT_COL);
					g2_fill_rect(current->_x + ZUI_ELEMENT_OFFSET() * 2, current->_y + current->button_offset_y * 1.5, linew, cursor_height);
				}
				zui_text(line, align, 0x00000000);
			}
		}
		current->_y -= ZUI_ELEMENT_OFFSET();
	}
	current->_y += ZUI_ELEMENT_OFFSET();
	current->text_coloring = _text_coloring;

	if (zui_text_area_scroll_past_end) {
		current->_y += current->_h - current->window_header_h - ZUI_ELEMENT_H() - ZUI_ELEMENT_OFFSET();
	}

	if (key_pressed) {
		// Move cursor vertically
		if (current->key_code == KINC_KEY_DOWN && handle->position < line_count - 1) {
			handle_line_select(current, handle);
			handle->position++;
			scroll_align(current, handle);
		}
		if (current->key_code == KINC_KEY_UP && handle->position > 0) {
			handle_line_select(current, handle);
			handle->position--;
			scroll_align(current, handle);
		}
		// New line
		if (editable && current->key_code == KINC_KEY_RETURN && !word_wrap) {
			handle->position++;
			char *tmp = zui_extract_line(lines, handle->position - 1);
			strcat(tmp, "\n");
			zui_insert_chars_at(lines, zui_line_pos(lines, handle->position), tmp + current->cursor_x);
			zui_remove_chars_at(lines, zui_line_pos(lines, handle->position - 1), current->cursor_x);
			zui_start_text_edit(handle, ZUI_ALIGN_LEFT);
			current->cursor_x = current->highlight_anchor = 0;
			scroll_align(current, handle);
		}
		// Delete line
		if (editable && current->key_code == KINC_KEY_BACKSPACE && cursor_start_x == 0 && handle->position > 0) {
			handle->position--;
			current->cursor_x = current->highlight_anchor = strlen(zui_extract_line(lines, handle->position));
			char *tmp = zui_extract_line(lines, handle->position + 1);
			zui_insert_chars_at(lines, zui_line_pos(lines, handle->position), tmp);
			zui_remove_chars_at(lines, zui_line_pos(lines, handle->position + 1), strlen(tmp) + 1);
			scroll_align(current, handle);
		}
		// strcpy(current->text_selected, zui_extract_line(lines, handle->position)); ////
	}

	current->highlight_on_select = true;
	current->tab_switch_enabled = true;
	strcpy(handle->text, lines);
	return handle->text;
}

float ZUI_MENUBAR_H() {
	zui_t *current = zui_get_current();
	return ZUI_BUTTON_H() * 1.1 + 2 + current->button_offset_y;
}

void zui_begin_menu() {
	zui_t *current = zui_get_current();
	_ELEMENT_OFFSET = current->ops.theme->ELEMENT_OFFSET;
	_BUTTON_COL = current->ops.theme->BUTTON_COL;
	current->ops.theme->ELEMENT_OFFSET = 0;
	current->ops.theme->BUTTON_COL = current->ops.theme->SEPARATOR_COL;
	g2_set_color(current->ops.theme->SEPARATOR_COL);
	g2_fill_rect(0, 0, current->_window_w, ZUI_MENUBAR_H());
}

void zui_end_menu() {
	zui_t *current = zui_get_current();
	current->ops.theme->ELEMENT_OFFSET = _ELEMENT_OFFSET;
	current->ops.theme->BUTTON_COL = _BUTTON_COL;
}

bool zui_menu_button(char *text) {
	zui_t *current = zui_get_current();
	current->_w = g2_string_width(current->ops.font, current->font_size, text) + 25 * ZUI_SCALE();
	return zui_button(text, ZUI_ALIGN_LEFT, "");
}
