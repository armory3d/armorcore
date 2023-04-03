#include "zui.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/input/keyboard.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <kinc/log.h>

static zui_t *current = NULL;
static void (*zui_on_border_hover)(zui_handle_t *, int) = NULL; // Mouse over window border, use for resizing
static void (*zui_on_text_hover)(void) = NULL; // Mouse over text input, use to set I-cursor
static void (*zui_on_deselect_text)(void) = NULL; // Text editing finished
static void (*zui_on_tab_drop)(zui_handle_t *, int, zui_handle_t *, int) = NULL; // Tab reorder via drag and drop
static bool zui_always_redraw_window = true; // Redraw cached window texture each frame or on changes only
static bool zui_key_repeat = true; // Emulate key repeat for non-character keys
static bool zui_dynamic_glyph_load = true; // Allow text input fields to push new glyphs into the font atlas
static bool zui_touch_scroll = false; // Pan with finger to scroll
static bool zui_touch_hold = false; // Touch and hold finger for right click
static bool zui_touch_tooltip = false; // Show extra tooltips above finger / on-screen keyboard
static float zui_key_repeat_time = 0.0;
static char zui_text_to_paste[1024];
static char zui_text_to_copy[1024];
static bool zui_is_cut = false;
static bool zui_is_copy = false;
static bool zui_is_paste = false;
static zui_t *zui_copy_receiver = NULL;
static int zui_copy_frame = 0;
static bool zui_combo_first = true;
static char temp[1024];
static zui_handle_t *zui_nested = NULL;
static int zui_nested_count = 0;
static zui_handle_t zui_combo_search_handle;

inline float ZUI_SCALE() {
	return current->ops.scale_factor;
}

inline float ZUI_ELEMENT_W() {
	return current->ops.theme.ELEMENT_W * ZUI_SCALE();
}

inline float ZUI_ELEMENT_H() {
	return current->ops.theme.ELEMENT_H * ZUI_SCALE();
}

inline float ZUI_ELEMENT_OFFSET() {
	return current->ops.theme.ELEMENT_OFFSET * ZUI_SCALE();
}

inline float ZUI_ARROW_SIZE() {
	return current->ops.theme.ARROW_SIZE * ZUI_SCALE();
}

inline float ZUI_BUTTON_H() {
	return current->ops.theme.BUTTON_H * ZUI_SCALE();
}

inline float ZUI_CHECK_SIZE() {
	return current->ops.theme.CHECK_SIZE * ZUI_SCALE();
}

inline float ZUI_CHECK_SELECT_SIZE() {
	return current->ops.theme.CHECK_SELECT_SIZE * ZUI_SCALE();
}

inline int ZUI_FONT_SIZE() {
	return current->ops.theme.FONT_SIZE * ZUI_SCALE();
}

inline int ZUI_SCROLL_W() {
	return current->ops.theme.SCROLL_W * ZUI_SCALE();
}

inline float ZUI_TEXT_OFFSET() {
	return current->ops.theme.TEXT_OFFSET * ZUI_SCALE();
}

inline int ZUI_TAB_W() {
	return current->ops.theme.TAB_W * ZUI_SCALE();
}

inline int ZUI_HEADER_DRAG_H() {
	return 15 * ZUI_SCALE();
}

inline float ZUI_TOOLTIP_DELAY() {
	return 1.0;
}

zui_t *zui_get_current() {
	return current;
}

zui_handle_t *zui_nest(zui_handle_t *handle, int id) {
	++id; // Start at 1
	if (handle->id == 0) handle->id = zui_nested_count + 1;
	int hash = (handle->id << 16) | id;
	for (int i = 0; i < zui_nested_count; i++) {
		if (zui_nested[i].id == hash) return &zui_nested[i];
	}
	zui_nested = realloc(zui_nested, (++zui_nested_count) * sizeof(zui_handle_t));
	zui_nested[zui_nested_count - 1].id = hash;
	return &zui_nested[zui_nested_count - 1];
}

void zui_fade_color(float alpha) {
	uint32_t color = g2_get_color();
	uint8_t r = (color & 0x00ff0000) >> 16;
	uint8_t g = (color & 0x0000ff00) >> 8;
	uint8_t b = (color & 0x000000ff);
	uint8_t a = (uint8_t)(255 * alpha);
	g2_set_color((a << 24) | (r << 16) | (g << 8) | b);
}

void zui_fill(float x, float y, float w, float h, int color) {
	g2_set_color(color);
	if (!current->enabled) zui_fade_color(0.25);
	g2_fill_rect(current->_x + x * ZUI_SCALE(), current->_y + y * ZUI_SCALE() - 1, w * ZUI_SCALE(), h * ZUI_SCALE());
	g2_set_color(0xffffffff);
}

void zui_rect(float x, float y, float w, float h, int color, float strength) {
	g2_set_color(color);
	if (!current->enabled) zui_fade_color(0.25);
	g2_draw_rect(current->_x + x * ZUI_SCALE(), current->_y + y * ZUI_SCALE(), w * ZUI_SCALE(), h * ZUI_SCALE(), strength);
	g2_set_color(0xffffffff);
}

void zui_draw_rect(bool fill, float x, float y, float w, float h) {
	int strength = 1;
	if (!current->enabled) zui_fade_color(0.25);
	fill ? g2_fill_rect(x, y - 1, w, h + 1) : g2_draw_rect(x, y, w, h, strength);
}

bool zui_is_char(int code) {
	return (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
}

int zui_check_start(int i, char *text, char **start) {
	// for (s in start) if (text.substr(i, s.length) == s) return s.length;
	return 0;
}

void zui_extract_coloring(char *text, char *start, char *end, bool skip_first) {
	zui_text_extract_t e;
	e.colored = "";
	e.uncolored = text;
	return e;

	// bool coloring = false;
	// int start_from = 0;
	// int start_length = 0;
	// for (i in 0...text.length) {
	// 	bool skip_first = false;
	// 	// Check if upcoming text should be colored
	// 	int length = zui_check_start(i, text, col.start);
	// 	// Not touching another character
	// 	var separated_left = i == 0 || !zui_is_char(text.charCodeAt(i - 1));
	// 	var separated_right = i + length >= text.length || !zui_is_char(text.charCodeAt(i + length));
	// 	var is_separated = separated_left && separated_right;
	// 	// Start coloring
	// 	if (length > 0 && (!coloring || col.end == "") && (!col.separated || is_separated)) {
	// 		coloring = true;
	// 		start_from = i;
	// 		start_length = length;
	// 		if (col.end != "" && col.end != "\n") skip_first = true;
	// 	}
	// 	// End coloring
	// 	else if (col.end == "") {
	// 		if (i == start_from + start_length) coloring = false;
	// 	}
	// 	else if (text.substr(i, col.end.length) == col.end) {
	// 		coloring = false;
	// 	}
	// 	// If true, add current character to colored string
	// 	var b = coloring && !skip_first;
	// 	res.colored += b ? text.charAt(i) : " ";
	// 	res.uncolored += b ? " " : text.charAt(i);
	// }
	// return res;
}

void zui_draw_string(char *text, float x_offset, float y_offset, int align, bool truncation) {
	if (text == NULL) return;
	if (truncation) {
		assert(strlen(text) < 1024 - 2);
		const char *full_text = text;
		strcpy(temp, text);
		text = &temp[0];
		while (strlen(text) > 0 && g2_string_width(current->ops.font, current->font_size, text) > current->_w - 6 * ZUI_SCALE()) {
			text[strlen(text) - 1] = 0;
		}
		if (strlen(text) < strlen(full_text)) {
			strcat(text, "..");
			// Strip more to fit ".."
			while (strlen(text) > 2 && g2_string_width(current->ops.font, current->font_size, text) > current->_w - 10 * ZUI_SCALE()) {
				text[strlen(text) - 3] = 0;
				strcat(text, "..");
			}
			if (current->is_hovered) {
				zui_tooltip(full_text);
			}
		}
	}

	if (zui_dynamic_glyph_load) {
		int len = strlen(text);
		for (int i = 0; i < len; ++i) {
			if (text[i] > 126 && !g2_font_has_glyph((int)text[i])) {
				int glyph = text[i];
				// g2_font_add_glyphs(&glyph, 1);
			}
		}
	}

	x_offset *= ZUI_SCALE();
	g2_set_font(current->ops.font, current->font_size);
	if (align == ZUI_ALIGN_CENTER) x_offset = current->_w / 2 - g2_string_width(current->ops.font, current->font_size, text) / 2;
	else if (align == ZUI_ALIGN_RIGHT) x_offset = current->_w - g2_string_width(current->ops.font, current->font_size, text) - ZUI_TEXT_OFFSET();

	if (!current->enabled) zui_fade_color(0.25);

	if (current->text_coloring == NULL) {
		g2_draw_string(text, current->_x + x_offset, current->_y + current->font_offset_y + y_offset);
	}
	else {
		// Monospace fonts only for now
		// for (coloring in current->text_coloring->colorings) {
		// 	zui_text_extract_t result = zui_extract_coloring(text, coloring);
		// 	if (result.colored != "") {
		// 		g2_set_color(coloring->color);
		// 		g2_draw_string(result.colored, current->_x + x_offset, current->_y + current->font_offset_y + y_offset);
		// 	}
		// 	text = result.uncolored;
		// }
		// g2_set_color(current->text_coloring->default_color);
		// g2_draw_string(text, current->_x + x_offset, current->_y + current->font_offset_y + y_offset);
	}
}

bool zui_get_initial_hover(float elem_h) {
	if (current->scissor && current->input_y < current->_window_y + current->window_header_h) return false;
	return current->enabled && current->input_enabled &&
		current->input_started_x >= current->_window_x + current->_x && current->input_started_x < (current->_window_x + current->_x + current->_w) &&
		current->input_started_y >= current->_window_y + current->_y && current->input_started_y < (current->_window_y + current->_y + elem_h);
}

bool zui_get_hover(float elem_h) {
	if (current->scissor && current->input_y < current->_window_y + current->window_header_h) return false;
	current->is_hovered = current->enabled && current->input_enabled &&
		current->input_x >= current->_window_x + (current->highlight_full_row ? 0 : current->_x) && current->input_x < (current->_window_x + current->_x + (current->highlight_full_row ? current->_window_w : current->_w)) &&
		current->input_y >= current->_window_y + current->_y && current->input_y < (current->_window_y + current->_y + elem_h);
	return current->is_hovered;
}

bool zui_get_released(float elem_h) { // Input selection
	current->is_released = current->enabled && current->input_enabled && current->input_released && zui_get_hover(elem_h) && zui_get_initial_hover(elem_h);
	return current->is_released;
}

bool zui_get_pushed(float elem_h) {
	current->is_pushed = current->enabled && current->input_enabled && current->input_down && zui_get_hover(elem_h) && zui_get_initial_hover(elem_h);
	return current->is_pushed;
}

bool zui_get_started(float elem_h) {
	current->is_started = current->enabled && current->input_enabled && current->input_started && zui_get_hover(elem_h);
	return current->is_started;
}

bool zui_is_visible(float elem_h) {
	if (current->current_window == NULL) return true;
	return (current->_y + elem_h > current->window_header_h && current->_y < current->current_window->texture.height);
}

float zui_get_ratio(float ratio, float dyn) {
	return ratio < 0 ? -ratio : ratio * dyn;
}

// Highlight all upcoming elements in the next row on a `mouse-over` event
void zui_highlight_next_row() {
	current->highlight_full_row = true;
}

// Draw the upcoming elements in the same row
// Negative values will be treated as absolute, positive values as ratio to `window width`
void zui_row(float *ratios, int count) {
	assert(count < 32);
	current->ratios = ratios;
	current->ratios_count = count;
	current->current_ratio = 0;
	current->x_before_split = current->_x;
	current->w_before_split = current->_w;
	current->_w = zui_get_ratio(ratios[current->current_ratio], current->_w);
}

void zui_indent() {
	current->_x += ZUI_TAB_W();
	current->_w -= ZUI_TAB_W();
}

void zui_unindent() {
	current->_x -= ZUI_TAB_W();
	current->_w += ZUI_TAB_W();
}

void zui_end_element_of_size(float element_size) {
	if (current->current_window == NULL || current->current_window->layout == ZUI_LAYOUT_VERTICAL) {
		if (current->current_ratio == -1 || (current->ratios_count > 0 && current->current_ratio == current->ratios_count - 1)) { // New line
			current->_y += element_size;

			if ((current->ratios_count > 0 && current->current_ratio == current->ratios_count - 1)) { // Last row element
				current->current_ratio = -1;
				current->ratios_count = 0;
				current->_x = current->x_before_split;
				current->_w = current->w_before_split;
				current->highlight_full_row = false;
			}
		}
		else { // Row
			current->current_ratio++;
			current->_x += current->_w; // More row elements to place
			current->_w = zui_get_ratio(current->ratios[current->current_ratio], current->w_before_split);
		}
	}
	else { // Horizontal
		current->_x += current->_w + ZUI_ELEMENT_OFFSET();
	}
}

void zui_end_element() {
	zui_end_element_of_size(ZUI_ELEMENT_H() + ZUI_ELEMENT_OFFSET());
}

void zui_resize(zui_handle_t *handle, int w, int h) {
	handle->redraws = 2;
	if (handle->texture.width != 0) kinc_g4_render_target_destroy(&handle->texture);
	if (w < 1) w = 1;
	if (h < 1) h = 1;
	kinc_g4_render_target_init(&handle->texture, w, h, KINC_G4_RENDER_TARGET_FORMAT_32BIT, 0, 0);
}

bool zui_input_in_rect(float x, float y, float w, float h) {
	return current->enabled && current->input_enabled &&
		current->input_x >= x && current->input_x < (x + w) &&
		current->input_y >= y && current->input_y < (y + h);
}

bool zui_input_changed() {
	return current->input_dx != 0 || current->input_dy != 0 || current->input_wheel_delta != 0 || current->input_started || current->input_started_r || current->input_released || current->input_released_r || current->input_down || current->input_down_r || current->is_key_pressed;
}

void zui_end_input() {
	if (zui_on_tab_drop != NULL && current->drag_tab_handle != NULL) {
		if (current->input_dx != 0 || current->input_dy != 0) {
			kinc_mouse_set_cursor(1); // Hand
		}
		if (current->input_released) {
			kinc_mouse_set_cursor(0); // Default
			current->drag_tab_handle = NULL;
		}
	}

	current->is_key_pressed = false;
	current->input_started = false;
	current->input_started_r = false;
	current->input_released = false;
	current->input_released_r = false;
	current->input_dx = 0;
	current->input_dy = 0;
	current->input_wheel_delta = 0;
	current->pen_in_use = false;
	if (zui_key_repeat && current->is_key_down && kinc_time() - zui_key_repeat_time > 0.05) {
		if (current->key_code == KINC_KEY_BACKSPACE || current->key_code == KINC_KEY_DELETE || current->key_code == KINC_KEY_LEFT || current->key_code == KINC_KEY_RIGHT || current->key_code == KINC_KEY_UP || current->key_code == KINC_KEY_DOWN) {
			zui_key_repeat_time = kinc_time();
			current->is_key_pressed = true;
		}
	}
	if (zui_touch_hold && current->input_down && current->input_x == current->input_started_x && current->input_y == current->input_started_y && current->input_started_time > 0 && kinc_time() - current->input_started_time > 0.7) {
		current->touch_hold_activated = true;
		current->input_released_r = true;
		current->input_started_time = 0;
	}
}

void zui_scroll(float delta, float full_height) {
	current->current_window->scroll_offset -= delta;
}

int zui_line_count(char *str) {
	if (str == NULL || str[0] == 0) return 0;
	int i = 0;
	int count = 1;
	while (str[i++] != 0) if (str == '\n') count++;
	return count;
}

char *zui_extract_line(char *str, int line) {
	int pos = 0;
	int len = strlen(str);
	int line_i = 0;
	for (int i = 0; i < len; ++i) {
		if (str[i] == '\n') line_i++;
		if (line_i < line) continue;
		if (line_i > line) break;
		temp[pos++] = str[i];
	}
	temp[pos] = 0;
	return temp;
}

 void zui_lower_case(char *dest, char *src) {
	int len = strlen(src);
	assert(len < 1024);
	for (int i = 0; i < len; ++i) {
		dest[i] = tolower(src[i]);
	}
	return dest;
}

void zui_draw_tooltip_text(bool bind_global_g) {
	g2_set_color(current->ops.theme.TEXT_COL);
	int line_count = zui_line_count(current->tooltip_text);
	float tooltip_w = 0.0;
	for (int i = 0; i < line_count; ++i) {
		float line_tooltip_w = g2_string_width(current->ops.font, current->font_size, zui_extract_line(current->tooltip_text, i));
		if (line_tooltip_w > tooltip_w) tooltip_w = line_tooltip_w;
	}
	current->tooltip_x = fmin(current->tooltip_x, kinc_window_width(0) - tooltip_w - 20);
	if (bind_global_g) g2_restore_render_target();
	float font_height = g2_font_height(current->ops.font, current->font_size);
	float off = 0;
	if (current->tooltip_img != NULL) {
		float w = current->tooltip_img->tex_width;
		if (current->tooltip_img_max_width != NULL && w > current->tooltip_img_max_width) w = current->tooltip_img_max_width;
		off = current->tooltip_img->tex_height * (w / current->tooltip_img->tex_width);
	}
	g2_fill_rect(current->tooltip_x, current->tooltip_y + off, tooltip_w + 20, font_height * line_count);
	g2_set_font(current->ops.font, current->font_size);
	g2_set_color(current->ops.theme.ACCENT_COL);
	for (int i = 0; i < line_count; ++i) {
		// g2_draw_string(zui_extract_line(current->tooltip_text, i), current->tooltip_x + 5, current->tooltip_y + off + i * current->font_size);
		g2_draw_string(zui_extract_line(current->tooltip_text, i), current->tooltip_x + 5, current->font_offset_y * 0.7 + current->tooltip_y + off + i * current->font_size);
	}
}

void zui_draw_tooltip_image(bool bind_global_g) {
	int w = current->tooltip_img->tex_width;
	if (current->tooltip_img_max_width != NULL && w > current->tooltip_img_max_width) w = current->tooltip_img_max_width;
	int h = current->tooltip_img->tex_height * (w / current->tooltip_img->tex_width);
	current->tooltip_x = fmin(current->tooltip_x, kinc_window_width(0) - w - 20);
	current->tooltip_y = fmin(current->tooltip_y, kinc_window_height(0) - h - 20);
	if (bind_global_g) g2_restore_render_target();
	g2_set_color(0xff000000);
	g2_fill_rect(current->tooltip_x, current->tooltip_y, w, h);
	g2_set_color(0xffffffff);
	current->tooltip_invert_y ?
		g2_draw_scaled_image(current->tooltip_img, current->tooltip_x, current->tooltip_y + h, w, -h) :
		g2_draw_scaled_image(current->tooltip_img, current->tooltip_x, current->tooltip_y, w, h);
}

void zui_draw_tooltip(bool bind_global_g) {
	if (current->slider_tooltip) {
		if (bind_global_g) g2_restore_render_target();
		g2_set_font(current->ops.font, current->font_size * 2);
		assert(sprintf(NULL, "%f", round(current->scroll_handle->value * 100) / 100) < 1024);
		sprintf(temp, "%f", round(current->scroll_handle->value * 100) / 100);
		char *text = temp;
		float x_off = g2_string_width(current->ops.font, current->font_size * 2, text) / 2;
		float y_off = g2_font_height(current->ops.font, current->font_size * 2);
		float x = fmin(fmax(current->slider_tooltip_x, current->input_x), current->slider_tooltip_x + current->slider_tooltip_w);
		g2_set_color(current->ops.theme.ACCENT_COL);
		g2_fill_rect(x - x_off, current->slider_tooltip_y - y_off, x_off * 2, y_off);
		g2_set_color(current->ops.theme.TEXT_COL);
		g2_draw_string(text, x - x_off, current->slider_tooltip_y - y_off);
	}
	if (zui_touch_tooltip && current->text_selected_handle != NULL) {
		if (bind_global_g) g2_restore_render_target();
		g2_set_font(current->ops.font, current->font_size * 2);
		float x_off = g2_string_width(current->ops.font, current->font_size * 2, current->text_selected) / 2;
		float y_off = g2_font_height(current->ops.font, current->font_size * 2) / 2;
		float x = kinc_window_width(0) / 2;
		float y = kinc_window_height(0) / 3;
		g2_set_color(current->ops.theme.ACCENT_COL);
		g2_fill_rect(x - x_off, y - y_off, x_off * 2, y_off * 2);
		g2_set_color(current->ops.theme.TEXT_COL);
		g2_draw_string(current->text_selected, x - x_off, y - y_off);
	}

	if (current->tooltip_text != NULL || current->tooltip_img != NULL) {
		if (zui_input_changed()) {
			current->tooltip_shown = false;
			current->tooltip_wait = current->input_dx == 0 && current->input_dy == 0; // Wait for movement before showing up again
		}
		if (!current->tooltip_shown) {
			current->tooltip_shown = true;
			current->tooltip_x = current->input_x;
			current->tooltip_time = kinc_time();
		}
		if (!current->tooltip_wait && kinc_time() - current->tooltip_time > ZUI_TOOLTIP_DELAY()) {
			if (current->tooltip_img != NULL) zui_draw_tooltip_image(bind_global_g);
			if (current->tooltip_text != NULL) zui_draw_tooltip_text(bind_global_g);
		}
	}
	else current->tooltip_shown = false;
}

void zui_bake_elements() {
	if (current->check_select_image.width != 0) {
		kinc_g4_render_target_destroy(&current->check_select_image);
	}
	kinc_g4_render_target_init(&current->check_select_image, ZUI_CHECK_SELECT_SIZE(), ZUI_CHECK_SELECT_SIZE(), KINC_G4_RENDER_TARGET_FORMAT_32BIT, 0, 0);
	g2_set_render_target(&current->check_select_image);
	kinc_g4_clear(KINC_G4_CLEAR_COLOR, 0x00000000, 0, 0);
	g2_set_color(current->ops.theme.ACCENT_SELECT_COL);
	g2_draw_line(0, 0, current->check_select_image.width, current->check_select_image.height, 2 * ZUI_SCALE());
	g2_draw_line(current->check_select_image.width, 0, 0, current->check_select_image.height, 2 * ZUI_SCALE());
	g2_restore_render_target();
	current->elements_baked = true;
}

void zui_begin_region(int x, int y, int w) {
	if (!current->elements_baked) {
		zui_bake_elements();
	}
	current->changed = false;
	current->current_window = NULL;
	current->tooltip_text = NULL;
	current->tooltip_img = NULL;
	current->_window_x = 0;
	current->_window_y = 0;
	current->_window_w = w;
	current->_x = x;
	current->_y = y;
	current->_w = w;
}

void zui_end_region(bool last) {
	zui_draw_tooltip(false);
	current->tab_pressed_handle = NULL;
	if (last) zui_end_input();
}

void zui_set_cursor_to_input(int align) {
	float off = align == ZUI_ALIGN_LEFT ? ZUI_TEXT_OFFSET() : current->_w - g2_string_width(current->ops.font, current->font_size, current->text_selected);
	float x = current->input_x - (current->_window_x + current->_x + off);
	current->cursor_x = 0;
	while (current->cursor_x < strlen(current->text_selected) && g2_sub_string_width(current->ops.font, current->font_size, current->text_selected, 0, current->cursor_x) < x) {
		current->cursor_x++;
	}
	current->highlight_anchor = current->cursor_x;
}

void zui_start_text_edit(zui_handle_t *handle, int align) {
	current->is_typing = true;
	current->submit_text_handle = current->text_selected_handle;
	current->text_to_submit = current->text_selected;
	current->text_selected_handle = handle;
	current->text_selected = handle->text;
	current->cursor_x = strlen(handle->text);
	if (current->tab_pressed) {
		current->tab_pressed = false;
		current->is_key_pressed = false; // Prevent text deselect after tab press
	}
	else if (!current->highlight_on_select) { // Set cursor to click location
		zui_set_cursor_to_input(align);
	}
	current->tab_pressed_handle = handle;
	current->highlight_anchor = current->highlight_on_select ? 0 : current->cursor_x;
	kinc_keyboard_show();
}

void zui_submit_text_edit() {
	current->changed = strcmp(current->submit_text_handle->text, current->text_to_submit) != 0;
	current->submit_text_handle->changed = current->changed;
	current->submit_text_handle->text = current->text_to_submit;
	current->submit_text_handle = NULL;
	current->text_to_submit[0] = 0;
	current->text_selected[0] = 0;
}

void zui_deselect_text() {
	if (current->text_selected_handle == NULL) return;
	current->submit_text_handle = current->text_selected_handle;
	current->text_to_submit = current->text_selected;
	current->text_selected_handle = NULL;
	current->is_typing = false;
	if (current->current_window != NULL) current->current_window->redraws = 2;
	kinc_keyboard_hide();
	current->highlight_anchor = current->cursor_x;
	if (zui_on_deselect_text != NULL) zui_on_deselect_text();
}

void zui_update_text_edit(int align, bool editable, bool live_update) {
	const char *text = current->text_selected;
	if (current->is_key_pressed) { // Process input
		if (current->key_code == KINC_KEY_LEFT) { // Move cursor
			if (current->cursor_x > 0) current->cursor_x--;
		}
		else if (current->key_code == KINC_KEY_RIGHT) {
			if (current->cursor_x < strlen(text)) current->cursor_x++;
		}
		else if (editable && current->key_code == KINC_KEY_BACKSPACE) { // Remove char
			if (current->cursor_x > 0 && current->highlight_anchor == current->cursor_x) {
				// text = text.substr(0, current->cursor_x - 1) + text.substr(current->cursor_x, strlen(text)); ////
				current->cursor_x--;
			}
			else if (current->highlight_anchor < current->cursor_x) {
				// text = text.substr(0, current->highlight_anchor) + text.substr(current->cursor_x, strlen(text)); ////
				current->cursor_x = current->highlight_anchor;
			}
			else {
				// text = text.substr(0, current->cursor_x) + text.substr(current->highlight_anchor, strlen(text)); ////
			}
		}
		else if (editable && current->key_code == KINC_KEY_DELETE) {
			if (current->highlight_anchor == current->cursor_x) {
				// text = text.substr(0, current->cursor_x) + text.substr(current->cursor_x + 1); ////
			}
			else if (current->highlight_anchor < current->cursor_x) {
				// text = text.substr(0, current->highlight_anchor) + text.substr(current->cursor_x, strlen(text)); ////
				current->cursor_x = current->highlight_anchor;
			}
			else {
				// text = text.substr(0, current->cursor_x) + text.substr(current->highlight_anchor, strlen(text)); ////
			}
		}
		else if (current->key_code == KINC_KEY_RETURN) { // Deselect
			zui_deselect_text();
		}
		else if (current->key_code == KINC_KEY_ESCAPE) { // Cancel
			current->text_selected = current->text_selected_handle->text;
			zui_deselect_text();
		}
		else if (current->key_code == KINC_KEY_TAB && current->tab_switch_enabled && !current->is_ctrl_down) { // Next field
			current->tab_pressed = true;
			zui_deselect_text();
			current->key_code = 0;
		}
		else if (current->key_code == KINC_KEY_HOME) {
			current->cursor_x = 0;
		}
		else if (current->key_code == KINC_KEY_END) {
			current->cursor_x = strlen(text);
		}
		else if (current->is_ctrl_down && current->is_a_down) { // Select all
			current->cursor_x = strlen(text);
			current->highlight_anchor = 0;
		}
		else if (editable && // Write
				 current->key_code != KINC_KEY_SHIFT &&
				 current->key_code != KINC_KEY_CAPS_LOCK &&
				 current->key_code != KINC_KEY_CONTROL &&
				 current->key_code != KINC_KEY_META &&
				 current->key_code != KINC_KEY_ALT &&
				 current->key_code != KINC_KEY_UP &&
				 current->key_code != KINC_KEY_DOWN &&
				 // current->key_char != 0 &&
				 // current->key_char != "" &&
				 current->key_char >= 32) {
			// text = text.substr(0, current->highlight_anchor) + current->key_char + text.substr(current->cursor_x); ////
			current->cursor_x = current->cursor_x + 1 > strlen(text) ? strlen(text) : current->cursor_x + 1;
		}
		bool selecting = current->is_shift_down && (current->key_code == KINC_KEY_LEFT || current->key_code == KINC_KEY_RIGHT || current->key_code == KINC_KEY_SHIFT);
		// isCtrlDown && isAltDown is the condition for AltGr was pressed
		// AltGr is part of the German keyboard layout and part of key combinations like AltGr + e -> €
		if (!selecting && (!current->is_ctrl_down || (current->is_ctrl_down && current->is_alt_down))) current->highlight_anchor = current->cursor_x;
	}

	if (editable && zui_text_to_paste[0] != 0) { // Process cut copy paste
		// text = text.substr(0, current->highlight_anchor) + zui_text_to_paste + text.substr(current->cursor_x); //// strncpy() + \0
		current->cursor_x += strlen(zui_text_to_paste);
		current->highlight_anchor = current->cursor_x;
		zui_text_to_paste[0] = 0;
		zui_is_paste = false;
	}
	if (current->highlight_anchor == current->cursor_x) strcpy(zui_text_to_copy, text); // Copy
	// else if (current->highlight_anchor < current->cursor_x) zui_text_to_copy = text.substring(current->highlight_anchor, current->cursor_x); ////
	// else zui_text_to_copy = text.substring(current->cursor_x, current->highlight_anchor); ////
	if (editable && zui_is_cut) { // Cut
		if (current->highlight_anchor == current->cursor_x) text = "";
		else if (current->highlight_anchor < current->cursor_x) {
			// text = text.substr(0, current->highlight_anchor) + text.substr(current->cursor_x, strlen(text)); ////
			current->cursor_x = current->highlight_anchor;
		}
		else {
			// text = text.substr(0, current->cursor_x) + text.substr(current->highlight_anchor, strlen(text)); ////
		}
	}

	float off = ZUI_TEXT_OFFSET();
	float line_height = ZUI_ELEMENT_H();
	float cursor_height = line_height - current->button_offset_y * 3.0;
	// Draw highlight
	if (current->highlight_anchor != current->cursor_x) {
		float istart = current->cursor_x;
		float iend = current->highlight_anchor;
		if (current->highlight_anchor < current->cursor_x) {
			istart = current->highlight_anchor;
			iend = current->cursor_x;
		}
		// char *hlstr = text.substr(istart, iend - istart);
		// float hlstrw = g2_string_width(current->ops.font, current->font_size, hlstr);
		// float start_off = g2_string_width(current->ops.font, current->font_size, text.substr(0, istart));
		// float hl_start = align == ZUI_ALIGN_LEFT ? current->_x + start_off + off : current->_x + current->_w - hlstrw - off;
		// if (align == ZUI_ALIGN_RIGHT) {
		// 	hl_start -= g2_string_width(current->ops.font, current->font_size, text.substr(iend, strlen(text)));
		// }
		// g2_set_color(current->ops.theme.ACCENT_SELECT_COL);
		// g2_fill_rect(hl_start, current->_y + current->button_offset_y * 1.5, hlstrw, cursor_height);
	}

	// Draw cursor
	// char *str = align == ZUI_ALIGN_LEFT ? text.substr(0, current->cursor_x) : text.substring(current->cursor_x, strlen(text));
	// float strw = g2_string_width(current->ops.font, current->font_size, str);
	// float cursor_x = align == ZUI_ALIGN_LEFT ? current->_x + strw + off : current->_x + current->_w - strw - off;
	// g2_set_color(current->ops.theme.TEXT_COL); // Cursor
	// g2_fill_rect(current->cursor_x, current->_y + current->button_offset_y * 1.5, 1 * ZUI_SCALE(), cursor_height);

	current->text_selected = text;
	if (live_update && current->text_selected_handle != NULL) {
		current->text_selected_handle->changed = current->text_selected_handle->text != current->text_selected;
		current->text_selected_handle->text = current->text_selected;
	}
}

void zui_draw_tabs() {
	current->input_x = current->restore_x;
	current->input_y = current->restore_y;
	if (current->current_window == NULL) return;
	float tab_x = 0.0;
	float tab_y = 0.0;
	float tab_h_min = ZUI_BUTTON_H() * 1.1;
	float header_h = current->current_window->drag_enabled ? ZUI_HEADER_DRAG_H() : 0;
	float tab_h = (current->ops.theme.FULL_TABS && current->tab_vertical) ? ((current->_window_h - header_h) / current->tab_count) : tab_h_min;
	float orig_y = current->_y;
	current->_y = header_h;
	current->tab_handle->changed = false;

	if (current->is_ctrl_down && current->is_tab_down) { // Next tab
		current->tab_handle->position++;
		if (current->tab_handle->position >= current->tab_count) current->tab_handle->position = 0;
		current->tab_handle->changed = true;
		current->is_tab_down = false;
	}

	if (current->tab_handle->position >= current->tab_count) current->tab_handle->position = current->tab_count - 1;

	g2_set_color(current->ops.theme.SEPARATOR_COL); // Tab background
	if (current->tab_vertical) {
		g2_fill_rect(0, current->_y, ZUI_ELEMENT_W(), current->_window_h);
	}
	else {
		g2_fill_rect(0, current->_y, current->_window_w, current->button_offset_y + tab_h + 2);
	}

	g2_set_color(current->ops.theme.ACCENT_COL); // Underline tab buttons
	if (current->tab_vertical) {
		g2_fill_rect(ZUI_ELEMENT_W(), current->_y, 1, current->_window_h);
	}
	else {
		g2_fill_rect(current->button_offset_y, current->_y + current->button_offset_y + tab_h + 2, current->_window_w - current->button_offset_y * 2, 1);
	}

	float base_y = current->tab_vertical ? current->_y : current->_y + 2;

	for (int i = 0; i < current->tab_count; ++i) {
		current->_x = tab_x;
		current->_y = base_y + tab_y;
		current->_w = current->tab_vertical ? (ZUI_ELEMENT_W() - 1 * ZUI_SCALE()) :
			 		  current->ops.theme.FULL_TABS ? (current->_window_w / current->tab_count) :
					  (g2_string_width(current->ops.font, current->font_size, current->tab_names[i]) + current->button_offset_y * 2 + 18 * ZUI_SCALE());
		bool released = zui_get_released(tab_h);
		bool started = zui_get_started(tab_h);
		bool pushed = zui_get_pushed(tab_h);
		bool hover = zui_get_hover(tab_h);
		if (zui_on_tab_drop != NULL) {
			if (started) {
				current->drag_tab_handle = current->tab_handle;
				current->drag_tab_position = i;
			}
			if (current->drag_tab_handle != NULL && hover && current->input_released) {
				zui_on_tab_drop(current->tab_handle, i, current->drag_tab_handle, current->drag_tab_position);
				current->tab_handle->position = i;
			}
		}
		if (released) {
			zui_handle_t *h = zui_nest(current->tab_handle, current->tab_handle->position); // Restore tab scroll
			h->scroll_offset = current->current_window->scroll_offset;
			h = zui_nest(current->tab_handle, i);
			current->tab_scroll = h->scroll_offset;
			current->tab_handle->position = i; // Set new tab
			current->current_window->redraws = 3;
			current->tab_handle->changed = true;
		}
		bool selected = current->tab_handle->position == i;

		g2_set_color((pushed || hover) ? current->ops.theme.BUTTON_HOVER_COL :
			current->tab_colors[i] != -1 ? current->tab_colors[i] :
			selected ? current->ops.theme.WINDOW_BG_COL :
			current->ops.theme.SEPARATOR_COL);
		if (current->tab_vertical) {
			tab_y += tab_h + 1;
		}
		else {
			tab_x += current->_w + 1;
		}
		zui_draw_rect(true, current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w, tab_h);
		g2_set_color(current->ops.theme.BUTTON_TEXT_COL);
		if (!selected) zui_fade_color(0.65);
		zui_draw_string(current->tab_names[i], current->ops.theme.TEXT_OFFSET, (tab_h - tab_h_min) / 2, (current->ops.theme.FULL_TABS || !current->tab_vertical) ? ZUI_ALIGN_CENTER : ZUI_ALIGN_LEFT, true);

		if (selected) { // Hide underline for active tab
			if (current->tab_vertical) {
				// g2_set_color(current->ops.theme.WINDOW_BG_COL);
				// g2_fill_rect(current->_x + current->button_offset_y + current->_w - 1, current->_y + current->button_offset_y - 1, 2, tab_h + current->button_offset_y);
				g2_set_color(current->ops.theme.HIGHLIGHT_COL);
				g2_fill_rect(current->_x + current->button_offset_y, current->_y + current->button_offset_y - 1, 2, tab_h + current->button_offset_y);
			}
			else {
				g2_set_color(current->ops.theme.WINDOW_BG_COL);
				g2_fill_rect(current->_x + current->button_offset_y + 1, current->_y + current->button_offset_y + tab_h, current->_w - 1, 1);
				g2_set_color(current->ops.theme.HIGHLIGHT_COL);
				g2_fill_rect(current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w, 2);
			}
		}

		if (current->tab_vertical) {
			g2_set_color(current->ops.theme.SEPARATOR_COL - 0x00050505);
			g2_fill_rect(current->_x, current->_y + tab_h, current->_w, 1);
		}
		else {
			g2_set_color(current->ops.theme.SEPARATOR_COL - 0x00050505);
			g2_fill_rect(current->_x + current->button_offset_y + current->_w, current->_y, 1, tab_h);
		}
	}

	if (zui_input_in_rect(current->_window_x, current->_window_y, current->_window_w, current->_window_h)) {
		// current->hovered_tab_name = current->tab_names[current->tab_handle->position]; // !!!!
		current->hovered_tab_x = current->_window_x;
		current->hovered_tab_y = current->_window_y;
		current->hovered_tab_w = current->_window_w;
		current->hovered_tab_h = current->_window_h;
	}

	current->_x = 0; // Restore positions
	current->_y = orig_y;
	current->_w = !current->current_window->scroll_enabled ? current->_window_w : current->_window_w - ZUI_SCROLL_W();
}

void zui_draw_arrow(bool selected) {
	float x = current->_x + current->arrow_offset_x;
	float y = current->_y + current->arrow_offset_y;
	g2_set_color(current->ops.theme.TEXT_COL);
	if (selected) {
		g2_fill_triangle(x, y,
						 x + ZUI_ARROW_SIZE(), y,
						 x + ZUI_ARROW_SIZE() / 2, y + ZUI_ARROW_SIZE());
	}
	else {
		g2_fill_triangle(x, y,
						 x, y + ZUI_ARROW_SIZE(),
						 x + ZUI_ARROW_SIZE(), y + ZUI_ARROW_SIZE() / 2);
	}
}

void zui_draw_tree(bool selected) {
	int SIGN_W = 7 * ZUI_SCALE();
	int x = current->_x + current->arrow_offset_x + 1;
	int y = current->_y + current->arrow_offset_y + 1;
	g2_set_color(current->ops.theme.TEXT_COL);
	if (selected) {
		g2_fill_rect(x, y + SIGN_W / 2 - 1, SIGN_W, SIGN_W / 8);
	}
	else {
		g2_fill_rect(x, y + SIGN_W / 2 - 1, SIGN_W, SIGN_W / 8);
		g2_fill_rect(x + SIGN_W / 2 - 1, y, SIGN_W / 8, SIGN_W);
	}
}

void zui_draw_check(bool selected, bool hover) {
	float x = current->_x + current->check_offset_x;
	float y = current->_y + current->check_offset_y;

	g2_set_color(hover ? current->ops.theme.ACCENT_HOVER_COL : current->ops.theme.ACCENT_COL);
	zui_draw_rect(current->ops.theme.FILL_ACCENT_BG, x, y, ZUI_CHECK_SIZE(), ZUI_CHECK_SIZE()); // Bg

	if (selected) { // Check
		g2_set_color(0xffffffff);
		if (!current->enabled) zui_fade_color(0.25);
		int size = ZUI_CHECK_SELECT_SIZE();
		g2_draw_scaled_render_target(&current->check_select_image, x + current->check_select_offset_x, y + current->check_select_offset_y, size, size);
	}
}

void zui_draw_radio(bool selected, bool hover) {
	float x = current->_x + current->radio_offset_x;
	float y = current->_y + current->radio_offset_y;
	g2_set_color(hover ? current->ops.theme.ACCENT_HOVER_COL : current->ops.theme.ACCENT_COL);
	zui_draw_rect(current->ops.theme.FILL_ACCENT_BG, x, y, ZUI_CHECK_SIZE(), ZUI_CHECK_SIZE()); // Bg

	if (selected) { // Check
		g2_set_color(current->ops.theme.ACCENT_SELECT_COL);
		if (!current->enabled) zui_fade_color(0.25);
		g2_fill_rect(x + current->radio_select_offset_x, y + current->radio_select_offset_y, ZUI_CHECK_SELECT_SIZE(), ZUI_CHECK_SELECT_SIZE());
	}
}

void zui_draw_slider(float value, float from, float to, bool filled, bool hover) {
	float x = current->_x + current->button_offset_y;
	float y = current->_y + current->button_offset_y;
	float w = current->_w - current->button_offset_y * 2;

	g2_set_color(hover ? current->ops.theme.ACCENT_HOVER_COL : current->ops.theme.ACCENT_COL);
	zui_draw_rect(current->ops.theme.FILL_ACCENT_BG, x, y, w, ZUI_BUTTON_H()); // Bg

	g2_set_color(hover ? current->ops.theme.ACCENT_HOVER_COL : current->ops.theme.ACCENT_COL);
	float offset = (value - from) / (to - from);
	float bar_w = 8 * ZUI_SCALE(); // Unfilled bar
	float slider_x = filled ? x : x + (w - bar_w) * offset;
	slider_x = fmax(fmin(slider_x, x + (w - bar_w)), x);
	float slider_w = filled ? w * offset : bar_w;
	slider_w = fmax(fmin(slider_w, w), 0);
	zui_draw_rect(true, slider_x, y, slider_w, ZUI_BUTTON_H());
}

void zui_draw_combo() {
	if (current->combo_selected_handle == NULL) return;
	g2_set_color(current->ops.theme.SEPARATOR_COL);
	g2_restore_render_target();

	float combo_h = (current->combo_selected_count + (current->combo_selected_label != NULL ? 1 : 0) + (current->combo_search_bar ? 1 : 0)) * ZUI_ELEMENT_H();
	float dist_top = current->combo_selected_y - combo_h - ZUI_ELEMENT_H() - current->window_border_top;
	float dist_bottom = kinc_window_height(0) - current->window_border_bottom - (current->combo_selected_y + combo_h );
	bool unroll_up = dist_bottom < 0 && dist_bottom < dist_top;
	zui_begin_region(current->combo_selected_x, current->combo_selected_y, current->combo_selected_w);
	if (current->is_key_pressed || current->input_wheel_delta != 0) {
		int arrow_up = current->is_key_pressed && current->key_code == (unroll_up ? KINC_KEY_DOWN : KINC_KEY_UP);
		int arrow_down = current->is_key_pressed && current->key_code == (unroll_up ? KINC_KEY_UP : KINC_KEY_DOWN);
		int wheel_up = (unroll_up && current->input_wheel_delta > 0) || (!unroll_up && current->input_wheel_delta < 0);
		int wheel_down = (unroll_up && current->input_wheel_delta < 0) || (!unroll_up && current->input_wheel_delta > 0);
		if ((arrow_up || wheel_up) && current->combo_to_submit > 0) {
			int step = 1;
			if (current->combo_search_bar && strlen(current->text_selected) > 0) {
				char search[512];
				char str[512];
				zui_lower_case(search, current->text_selected);
				while (true) {
					zui_lower_case(str, current->combo_selected_texts[current->combo_to_submit - step]);
					if (strstr(str, search) == NULL && current->combo_to_submit - step > 0) {
						++step;
					}
					else break;
				}

				// Corner case: current position is the top one according to the search pattern
				zui_lower_case(str, current->combo_selected_texts[current->combo_to_submit - step]);
				if (strstr(str, search) == NULL) step = 0;
			}
			current->combo_to_submit -= step;
			current->submit_combo_handle = current->combo_selected_handle;
		}
		else if ((arrow_down || wheel_down) && current->combo_to_submit < current->combo_selected_count - 1) {
			int step = 1;
			if (current->combo_search_bar && strlen(current->text_selected) > 0) {
				char search[512];
				char str[512];
				zui_lower_case(search, current->text_selected);
				while (true) {
					zui_lower_case(str, current->combo_selected_texts[current->combo_to_submit + step]);
					if (strstr(str, search) == NULL && current->combo_to_submit + step > 0) {
						++step;
					}
					else break;
				}

				// Corner case: current position is the top one according to the search pattern
				zui_lower_case(str, current->combo_selected_texts[current->combo_to_submit + step]);
				if (strstr(str, search) == NULL) step = 0;
			}

			current->combo_to_submit += step;
			current->submit_combo_handle = current->combo_selected_handle;
		}
		if (current->combo_selected_window != NULL) current->combo_selected_window->redraws = 2;
	}

	current->input_enabled = true;
	int _BUTTON_COL = current->ops.theme.BUTTON_COL;
	int _ELEMENT_OFFSET = current->ops.theme.ELEMENT_OFFSET;
	current->ops.theme.ELEMENT_OFFSET = 0;
	float unroll_right = current->_x + current->combo_selected_w * 2 < kinc_window_width(0) - current->window_border_right ? 1 : -1;
	bool reset_position = false;
	char search[512];
	search[0] = 0;
	if (current->combo_search_bar) {
		if (unroll_up) current->_y -= ZUI_ELEMENT_H() * 2;
		if (zui_combo_first) zui_combo_search_handle.text = ""; ////
		zui_fill(0, 0, current->_w / ZUI_SCALE(), ZUI_ELEMENT_H() / ZUI_SCALE(), current->ops.theme.SEPARATOR_COL);
		// search = zui_text_input(&zui_combo_search_handle, "", ZUI_ALIGN_LEFT, true, true);
		zui_text_input(&zui_combo_search_handle, "", ZUI_ALIGN_LEFT, true, true);
		zui_lower_case(search, search);
		if (current->is_released) zui_combo_first = true; // Keep combo open
		if (zui_combo_first) {
			#if !defined(KORE_ANDROID) && !defined(KORE_IOS)
			zui_start_text_edit(&zui_combo_search_handle, ZUI_ALIGN_LEFT); // Focus search bar
			#endif
		}
		reset_position = zui_combo_search_handle.changed;
	}

	for (int i = 0; i < current->combo_selected_count; ++i) {
		char str[512];
		zui_lower_case(str, current->combo_selected_texts[i]);
		if (strlen(search) > 0 && strstr(str, search) == NULL)
			continue; // Don't show items that don't fit the current search pattern

		if (reset_position) { // The search has changed, select first entry that matches
			current->combo_to_submit = current->combo_selected_handle->position = i;
			current->submit_combo_handle = current->combo_selected_handle;
			reset_position = false;
		}
		if (unroll_up) current->_y -= ZUI_ELEMENT_H() * 2;
		current->ops.theme.BUTTON_COL = i == current->combo_selected_handle->position ? current->ops.theme.ACCENT_SELECT_COL : current->ops.theme.SEPARATOR_COL;
		zui_fill(0, 0, current->_w / ZUI_SCALE(), ZUI_ELEMENT_H() / ZUI_SCALE(), current->ops.theme.SEPARATOR_COL);
		if (zui_button(current->combo_selected_texts[i], current->combo_selected_align, "")) {
			current->combo_to_submit = i;
			current->submit_combo_handle = current->combo_selected_handle;
			if (current->combo_selected_window != NULL) current->combo_selected_window->redraws = 2;
			break;
		}
		if (current->_y + ZUI_ELEMENT_H() > kinc_window_height(0) - current->window_border_bottom || current->_y - ZUI_ELEMENT_H() * 2 < current->window_border_top) {
			current->_x += current->combo_selected_w * unroll_right; // Next column
			current->_y = current->combo_selected_y;
		}
	}
	current->ops.theme.BUTTON_COL = _BUTTON_COL;
	current->ops.theme.ELEMENT_OFFSET = _ELEMENT_OFFSET;

	if (current->combo_selected_label != "") { // Unroll down
		if (unroll_up) {
			current->_y -= ZUI_ELEMENT_H() * 2;
			zui_fill(0, 0, current->_w / ZUI_SCALE(), ZUI_ELEMENT_H() / ZUI_SCALE(), current->ops.theme.SEPARATOR_COL);
			g2_set_color(current->ops.theme.LABEL_COL);
			zui_draw_string(current->combo_selected_label, current->ops.theme.TEXT_OFFSET, 0, ZUI_ALIGN_RIGHT, true);
			current->_y += ZUI_ELEMENT_H();
			zui_fill(0, 0, current->_w / ZUI_SCALE(), 1 * ZUI_SCALE(), current->ops.theme.ACCENT_SELECT_COL); // Separator
		}
		else {
			zui_fill(0, 0, current->_w / ZUI_SCALE(), ZUI_ELEMENT_H() / ZUI_SCALE(), current->ops.theme.SEPARATOR_COL);
			zui_fill(0, 0, current->_w / ZUI_SCALE(), 1 * ZUI_SCALE(), current->ops.theme.ACCENT_SELECT_COL); // Separator
			g2_set_color(current->ops.theme.LABEL_COL);
			zui_draw_string(current->combo_selected_label, current->ops.theme.TEXT_OFFSET, 0, ZUI_ALIGN_RIGHT, true);
		}
	}

	if ((current->input_released || current->input_released_r || current->is_escape_down || current->is_return_down) && !zui_combo_first) {
		current->combo_selected_handle = NULL;
		zui_combo_first = true;
	}
	else zui_combo_first = false;
	current->input_enabled = current->combo_selected_handle == NULL;
	zui_end_region(false);
}

void zui_set_scale(float factor) {
	current->ops.scale_factor = factor;
	current->font_size = ZUI_FONT_SIZE();
	float font_height = g2_font_height(current->ops.font, current->font_size);
	current->font_offset_y = (ZUI_ELEMENT_H() - font_height) / 2; // Precalculate offsets
	current->arrow_offset_y = (ZUI_ELEMENT_H() - ZUI_ARROW_SIZE()) / 2;
	current->arrow_offset_x = current->arrow_offset_y;
	current->title_offset_x = (current->arrow_offset_x * 2 + ZUI_ARROW_SIZE()) / ZUI_SCALE();
	current->button_offset_y = (ZUI_ELEMENT_H() - ZUI_BUTTON_H()) / 2;
	current->check_offset_y = (ZUI_ELEMENT_H() - ZUI_CHECK_SIZE()) / 2;
	current->check_offset_x = current->check_offset_y;
	current->check_select_offset_y = (ZUI_CHECK_SIZE() - ZUI_CHECK_SELECT_SIZE()) / 2;
	current->check_select_offset_x = current->check_select_offset_y;
	current->radio_offset_y = (ZUI_ELEMENT_H() - ZUI_CHECK_SIZE()) / 2;
	current->radio_offset_x = current->radio_offset_y;
	current->radio_select_offset_y = (ZUI_CHECK_SIZE() - ZUI_CHECK_SELECT_SIZE()) / 2;
	current->radio_select_offset_x = current->radio_select_offset_y;
	current->elements_baked = false;
}

void zui_init(zui_t *ui, zui_options_t ops) {
	current = ui;
	memset(current, 0, sizeof(zui_t));
	current->ops = ops;
	zui_set_scale(ops.scale_factor);
	current->enabled = true;
	current->scroll_enabled = true;
	current->highlight_on_select = true;
	current->tab_switch_enabled = true;
	current->input_enabled = true;
	current->current_ratio = -1;
	current->image_scroll_align = true;
	current->window_ended = true;
	current->restore_x = -1;
	current->restore_y = -1;
}

void zui_begin(zui_t *ui) {
	current = ui;
	if (!current->elements_baked) zui_bake_elements();
	current->changed = false;
	current->_x = 0; // Reset cursor
	current->_y = 0;
	current->_w = 0;
	current->_h = 0;
}

// Sticky region ignores window scrolling
void zui_begin_sticky() {
	current->sticky = true;
	current->_y -= current->current_window->scroll_offset;
}

void zui_end_sticky() {
	current->sticky = false;
	current->scissor = true;
	kinc_g4_scissor(0, current->_y, current->_window_w, current->_window_h - current->_y);
	current->window_header_h += current->_y - current->window_header_h;
	current->_y += current->current_window->scroll_offset;
	current->is_hovered = false;
}

void zui_end_window(bool bind_global_g) {
	zui_handle_t *handle = current->current_window;
	if (handle == NULL) return;
	if (handle->redraws > 0 || current->is_scrolling) {
		if (current->scissor) {
			current->scissor = false;
			kinc_g4_disable_scissor();
		}

		if (current->tab_count > 0) zui_draw_tabs();

		if (handle->drag_enabled) { // Draggable header
			g2_set_color(current->ops.theme.SEPARATOR_COL);
			g2_fill_rect(0, 0, current->_window_w, ZUI_HEADER_DRAG_H());
		}

		float wh = current->_window_h - current->window_header_h; // Exclude header
		float full_height = current->_y - handle->scroll_offset - current->window_header_h;
		if (full_height < wh || handle->layout == ZUI_LAYOUT_HORIZONTAL || !current->scroll_enabled) { // Disable scrollbar
			handle->scroll_enabled = false;
			handle->scroll_offset = 0;
		}
		else { // Draw window scrollbar if necessary
			handle->scroll_enabled = true;
			if (current->tab_scroll < 0) { // Restore tab
				handle->scroll_offset = current->tab_scroll;
				current->tab_scroll = 0;
			}
			float wy = current->_window_y + current->window_header_h;
			float amount_to_scroll = full_height - wh;
			float amount_scrolled = -handle->scroll_offset;
			float ratio = amount_scrolled / amount_to_scroll;
			float bar_h = wh * fabs(wh / full_height);
			bar_h = fmax(bar_h, ZUI_ELEMENT_H());

			float total_scrollable_area = wh - bar_h;
			float e = amount_to_scroll / total_scrollable_area;
			float bar_y = total_scrollable_area * ratio + current->window_header_h;
			bool bar_focus = zui_input_in_rect(current->_window_x + current->_window_w - ZUI_SCROLL_W(), bar_y + current->_window_y, ZUI_SCROLL_W(), bar_h);

			if (current->input_started && bar_focus) { // Start scrolling
				current->scroll_handle = handle;
				current->is_scrolling = true;
			}

			float scroll_delta = current->input_wheel_delta;
			if (zui_touch_scroll && current->input_down && current->input_dy != 0 && current->input_x > current->_window_x + current->window_header_w && current->input_y > current->_window_y + current->window_header_h) {
				current->is_scrolling = true;
				scroll_delta = -current->input_dy / 20;
			}
			if (handle == current->scroll_handle) { // Scroll
				zui_scroll(current->input_dy * e, full_height);
			}
			else if (scroll_delta != 0 && current->combo_selected_handle == NULL &&
					 zui_input_in_rect(current->_window_x, wy, current->_window_w, wh)) { // Wheel
				zui_scroll(scroll_delta * ZUI_ELEMENT_H(), full_height);
			}

			// Stay in bounds
			if (handle->scroll_offset > 0) {
				handle->scroll_offset = 0;
			}
			else if (full_height + handle->scroll_offset < wh) {
				handle->scroll_offset = wh - full_height;
			}

			g2_set_color(current->ops.theme.ACCENT_COL); // Bar
			bool scrollbar_focus = zui_input_in_rect(current->_window_x + current->_window_w - ZUI_SCROLL_W(), wy, ZUI_SCROLL_W(), wh);
			float bar_w = (scrollbar_focus || handle == current->scroll_handle) ? ZUI_SCROLL_W() : ZUI_SCROLL_W() / 3;
			g2_fill_rect(current->_window_w - bar_w - current->scroll_align, bar_y, bar_w, bar_h);
		}

		handle->last_max_x = current->_x;
		handle->last_max_y = current->_y;
		if (handle->layout == ZUI_LAYOUT_VERTICAL) handle->last_max_x += current->_window_w;
		else handle->last_max_y += current->_window_h;
		handle->redraws--;
	}

	current->window_ended = true;

	// Draw window texture
	if (zui_always_redraw_window || handle->redraws > -4) {
		if (bind_global_g) g2_restore_render_target();
		g2_set_color(current->ops.theme.WINDOW_TINT_COL);
		g2_draw_render_target(&handle->texture, current->_window_x, current->_window_y);
		if (handle->redraws <= 0) handle->redraws--;
	}
}

bool zui_window_dirty(zui_handle_t *handle, int x, int y, int w, int h) {
	float wx = x + handle->drag_x;
	float wy = y + handle->drag_y;
	float input_changed = zui_input_in_rect(wx, wy, w, h) && zui_input_changed();
	return current->always_redraw || current->is_scrolling || input_changed;
}

bool zui_window(zui_handle_t *handle, int x, int y, int w, int h, bool drag) {
	if (handle->texture.width == 0 || w != handle->texture.width || h != handle->texture.height) {
		zui_resize(handle, w, h);
	}

	if (!current->window_ended) zui_end_window(true); // End previous window if necessary
	current->window_ended = false;

	g2_set_render_target(&handle->texture);
	current->current_window = handle;
	current->_window_x = x + handle->drag_x;
	current->_window_y = y + handle->drag_y;
	current->_window_w = w;
	current->_window_h = h;
	current->window_header_w = 0;
	current->window_header_h = 0;

	if (zui_window_dirty(handle, x, y, w, h)) {
		handle->redraws = 2;
	}

	if (zui_on_border_hover != NULL) {
		if (zui_input_in_rect(current->_window_x - 4, current->_window_y, 8, current->_window_h)) {
			zui_on_border_hover(handle, 0);
		}
		else if (zui_input_in_rect(current->_window_x + current->_window_w - 4, current->_window_y, 8, current->_window_h)) {
			zui_on_border_hover(handle, 1);
		}
		else if (zui_input_in_rect(current->_window_x, current->_window_y - 4, current->_window_w, 8)) {
			zui_on_border_hover(handle, 2);
		}
		else if (zui_input_in_rect(current->_window_x, current->_window_y + current->_window_h - 4, current->_window_w, 8)) {
			zui_on_border_hover(handle, 3);
		}
	}

	if (handle->redraws <= 0) {
		return false;
	}

	current->_x = 0;
	current->_y = handle->scroll_offset;
	if (handle->layout == ZUI_LAYOUT_HORIZONTAL) w = ZUI_ELEMENT_W();
	current->_w = !handle->scroll_enabled ? w : w - ZUI_SCROLL_W(); // Exclude scrollbar if present
	current->_h = h;
	current->tooltip_text = NULL;
	current->tooltip_img = NULL;
	current->tab_count = 0;

	if (current->ops.theme.FILL_WINDOW_BG) {
		kinc_g4_clear(KINC_G4_CLEAR_COLOR, current->ops.theme.WINDOW_BG_COL, 0, 0);
	}
	else {
		kinc_g4_clear(KINC_G4_CLEAR_COLOR, 0x00000000, 0, 0);
		g2_set_color(current->ops.theme.WINDOW_BG_COL);
		g2_fill_rect(current->_x, current->_y - handle->scroll_offset, handle->last_max_x, handle->last_max_y);
	}

	handle->drag_enabled = drag;
	if (drag) {
		if (current->input_started && zui_input_in_rect(current->_window_x, current->_window_y, current->_window_w, ZUI_HEADER_DRAG_H())) {
			current->drag_handle = handle;
		}
		else if (current->input_released) {
			current->drag_handle = NULL;
		}
		if (handle == current->drag_handle) {
			handle->redraws = 2;
			handle->drag_x += current->input_dx;
			handle->drag_y += current->input_dy;
		}
		current->_y += ZUI_HEADER_DRAG_H(); // Header offset
		current->window_header_h += ZUI_HEADER_DRAG_H();
	}

	return true;
}

bool zui_button(const char *text, int align, const char *label) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return false;
	}
	bool released = zui_get_released(ZUI_ELEMENT_H());
	bool pushed = zui_get_pushed(ZUI_ELEMENT_H());
	bool hover = zui_get_hover(ZUI_ELEMENT_H());
	if (released) current->changed = true;

	g2_set_color(pushed ? current->ops.theme.BUTTON_PRESSED_COL :
				 hover ? current->ops.theme.BUTTON_HOVER_COL :
				 current->ops.theme.BUTTON_COL);

	zui_draw_rect(current->ops.theme.FILL_BUTTON_BG, current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w - current->button_offset_y * 2, ZUI_BUTTON_H());

	g2_set_color(current->ops.theme.BUTTON_TEXT_COL);
	zui_draw_string(text, current->ops.theme.TEXT_OFFSET, 0, align, true);
	if (label != "") {
		g2_set_color(current->ops.theme.LABEL_COL);
		zui_draw_string(label, current->ops.theme.TEXT_OFFSET, 0, align == ZUI_ALIGN_RIGHT ? ZUI_ALIGN_LEFT : ZUI_ALIGN_RIGHT, true);
	}

	zui_end_element();
	return released;
}

void zui_split_text(const char *lines, int align, int bg) {
	int count = zui_line_count(lines);
	for (int i = 0; i < count; ++i) zui_text(zui_extract_line(lines, i), align, bg);
}

int zui_text(const char *text, int align, int bg) {
	if (zui_line_count(text) > 1) {
		zui_split_text(text, align, bg);
		return ZUI_STATE_IDLE;
	}
	float h = fmax(ZUI_ELEMENT_H(), g2_font_height(current->ops.font, current->font_size));
	if (!zui_is_visible(h)) {
		zui_end_element(h + ZUI_ELEMENT_OFFSET());
		return ZUI_STATE_IDLE;
	}
	bool started = zui_get_started(h);
	bool down = zui_get_pushed(h);
	bool released = zui_get_released(h);
	bool hover = zui_get_hover(h);
	if (bg != 0x0000000) {
		g2_set_color(bg);
		g2_fill_rect(current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w - current->button_offset_y * 2, ZUI_BUTTON_H());
	}
	g2_set_color(current->ops.theme.TEXT_COL);
	zui_draw_string(text, current->ops.theme.TEXT_OFFSET, 0, align, true);

	zui_end_element(h + ZUI_ELEMENT_OFFSET());
	return started ? ZUI_STATE_STARTED : released ? ZUI_STATE_RELEASED : down ? ZUI_STATE_DOWN : ZUI_STATE_IDLE;
}

bool zui_tab(zui_handle_t *handle, const char *text, bool vertical, uint32_t color) {
	if (current->tab_count == 0) { // First tab
		current->tab_handle = handle;
		current->tab_vertical = vertical;
		current->_w -= current->tab_vertical ? ZUI_ELEMENT_OFFSET() + ZUI_ELEMENT_W() - 1 * ZUI_SCALE() : 0; // Shrink window area by width of vertical tabs
		if (vertical) {
			current->window_header_w += ZUI_ELEMENT_W();
		}
		else {
			current->window_header_h += ZUI_BUTTON_H() + current->button_offset_y + ZUI_ELEMENT_OFFSET();
		}
		current->restore_x = current->input_x; // Mouse in tab header, disable clicks for tab content
		current->restore_y = current->input_y;
		if (!vertical && zui_input_in_rect(current->_window_x, current->_window_y, current->_window_w, current->window_header_h)) {
			current->input_x = current->input_y = -1;
		}
		if (vertical) {
			current->_x += current->window_header_w + 6;
			current->_w -= 6;
		}
		else {
			current->_y += current->window_header_h + 3;
		}
	}
	assert(current->tab_count < 16);
	strcpy(current->tab_names[current->tab_count], text);
	current->tab_colors[current->tab_count] = color;
	current->tab_count++;
	return handle->position == current->tab_count - 1;
}

bool zui_panel(zui_handle_t *handle, const char *text, bool is_tree, bool filled, bool pack) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return handle->selected;
	}
	if (zui_get_released(ZUI_ELEMENT_H())) {
		handle->selected = !handle->selected;
		handle->changed = current->changed = true;
	}
	if (filled) {
		g2_set_color(current->ops.theme.PANEL_BG_COL);
		zui_draw_rect(true, current->_x, current->_y, current->_w, ZUI_ELEMENT_H());
	}

	if (is_tree) {
		zui_draw_tree(handle->selected);
	}
	else {
		zui_draw_arrow(handle->selected);
	}

	g2_set_color(current->ops.theme.LABEL_COL); // Title
	zui_draw_string(text, current->title_offset_x, 0, ZUI_ALIGN_LEFT, true);

	zui_end_element();
	if (pack && !handle->selected) current->_y -= ZUI_ELEMENT_OFFSET();

	return handle->selected;
}

int zui_sub_image(kinc_g4_texture_t *image, int tint, float h, int sx, int sy, int sw, int sh) {
	float iw = (sw > 0 ? sw : image->tex_width) * ZUI_SCALE();
	float ih = (sh > 0 ? sh : image->tex_height) * ZUI_SCALE();
	float w = fmin(iw, current->_w);
	float x = current->_x;
	float scroll = current->current_window != NULL ? current->current_window->scroll_enabled : false;
	float r = current->current_ratio == -1 ? 1.0 : zui_get_ratio(current->ratios[current->current_ratio], 1);
	if (current->image_scroll_align) { // Account for scrollbar size
		w = fmin(iw, current->_w - current->button_offset_y * 2);
		x += current->button_offset_y;
		if (!scroll) {
			w -= ZUI_SCROLL_W() * r;
			x += ZUI_SCROLL_W() * r / 2;
		}
	}
	else if (scroll) w += ZUI_SCROLL_W() * r;

	// Image size
	float ratio = h == -1 ?
		w / iw :
		h / ih;
	if (h == -1) {
		h = ih * ratio;
	}
	else {
		w = iw * ratio;
	}

	if (!zui_is_visible(h)) {
		zui_end_element(h);
		return ZUI_STATE_IDLE;
	}
	bool started = zui_get_started(h);
	bool down = zui_get_pushed(h);
	bool released = zui_get_released(h);
	bool hover = zui_get_hover(h);
	if (current->current_ratio == -1 && (started || down || released || hover)) {
		if (current->input_x < current->_window_x + current->_x || current->input_x > current->_window_x + current->_x + w) {
			started = down = released = hover = false;
		}
	}
	g2_set_color(tint);
	if (!current->enabled) zui_fade_color(0.25);
	if (sw > 0) { // Source rect specified
		if (current->image_invert_y) {
			g2_draw_scaled_sub_image(image, sx, sy, sw, sh, x, current->_y + h, w, -h);
		}
		else {
			g2_draw_scaled_sub_image(image, sx, sy, sw, sh, x, current->_y, w, h);
		}
	}
	else {
		if (current->image_invert_y) {
			g2_draw_scaled_image(image, x, current->_y + h, w, -h);
		}
		else {
			g2_draw_scaled_image(image, x, current->_y, w, h);
		}
	}

	zui_end_element(h);
	return started ? ZUI_STATE_STARTED : released ? ZUI_STATE_RELEASED : down ? ZUI_STATE_DOWN : hover ? ZUI_STATE_HOVERED : ZUI_STATE_IDLE;
}

int zui_image(kinc_g4_texture_t *image, int tint, float h) {
	zui_sub_image(image, tint, h, 0, 0, image->tex_width, image->tex_height);
}

char *zui_text_input(zui_handle_t *handle, char *label, int align, bool editable, bool live_update) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return handle->text;
	}

	bool hover = zui_get_hover(ZUI_ELEMENT_H());
	if (hover && zui_on_text_hover != NULL) zui_on_text_hover();
	g2_set_color(hover ? current->ops.theme.ACCENT_HOVER_COL : current->ops.theme.ACCENT_COL); // Text bg
	zui_draw_rect(current->ops.theme.FILL_ACCENT_BG, current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w - current->button_offset_y * 2, ZUI_BUTTON_H());

	bool released = zui_get_released(ZUI_ELEMENT_H());
	if (current->submit_text_handle == handle && released) { // Keep editing selected text
		current->is_typing = true;
		current->text_selected_handle = current->submit_text_handle;
		current->submit_text_handle = NULL;
		zui_set_cursor_to_input(align);
	}
	bool start_edit = released || current->tab_pressed;
	handle->changed = false;

	if (current->text_selected_handle != handle && start_edit) zui_start_text_edit(handle, align);
	if (current->text_selected_handle == handle) zui_update_text_edit(align, editable, live_update);
	if (current->submit_text_handle == handle) zui_submit_text_edit();

	if (label[0] != 0) {
		g2_set_color(current->ops.theme.LABEL_COL); // Label
		int label_align = align == ZUI_ALIGN_RIGHT ? ZUI_ALIGN_LEFT : ZUI_ALIGN_RIGHT;
		zui_draw_string(label, label_align == ZUI_ALIGN_LEFT ? current->ops.theme.TEXT_OFFSET : 0, 0, label_align, true);
	}

	g2_set_color(current->ops.theme.TEXT_COL); // Text
	if (current->text_selected_handle != handle) {
		zui_draw_string(handle->text, current->ops.theme.TEXT_OFFSET, 0, align, true);
	}
	else {
		zui_draw_string(current->text_selected, current->ops.theme.TEXT_OFFSET, 0, align, false);
	}

	zui_end_element();
	return handle->text;
}

bool zui_check(zui_handle_t *handle, const char *text, const char *label) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return handle->selected;
	}
	if (zui_get_released(ZUI_ELEMENT_H())) {
		handle->selected = !handle->selected;
		handle->changed = current->changed = true;
	}
	else handle->changed = false;

	bool hover = zui_get_hover(ZUI_ELEMENT_H());
	zui_draw_check(handle->selected, hover); // Check

	g2_set_color(current->ops.theme.TEXT_COL); // Text
	zui_draw_string(text, current->title_offset_x, 0, ZUI_ALIGN_LEFT, true);

	if (label[0] != 0) {
		g2_set_color(current->ops.theme.LABEL_COL);
		zui_draw_string(label, current->ops.theme.TEXT_OFFSET, 0, ZUI_ALIGN_RIGHT, true);
	}

	zui_end_element();

	return handle->selected;
}

bool zui_radio(zui_handle_t *handle, int position, const char *text, const char *label) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return handle->position == position;
	}
	if (position == 0) {
		handle->changed = false;
	}
	if (zui_get_released(ZUI_ELEMENT_H())) {
		handle->position = position;
		handle->changed = current->changed = true;
	}

	bool hover = zui_get_hover(ZUI_ELEMENT_H());
	zui_draw_radio(handle->position == position, hover); // Radio

	g2_set_color(current->ops.theme.TEXT_COL); // Text
	zui_draw_string(text, current->title_offset_x, 0, ZUI_ALIGN_LEFT, true);

	if (label[0] != 0) {
		g2_set_color(current->ops.theme.LABEL_COL);
		zui_draw_string(label, current->ops.theme.TEXT_OFFSET, 0, ZUI_ALIGN_RIGHT, true);
	}

	zui_end_element();

	return handle->position == position;
}

int zui_combo(zui_handle_t *handle, char **texts, int count, char *label, bool show_label, int align, bool search_bar) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return handle->position;
	}
	if (zui_get_released(ZUI_ELEMENT_H())) {
		if (current->combo_selected_handle == NULL) {
			current->input_enabled = false;
			current->combo_selected_count = count;
			current->combo_selected_handle = handle;
			current->combo_selected_window = current->current_window;
			current->combo_selected_align = align;
			current->combo_selected_texts = texts;
			current->combo_selected_label = label;
			current->combo_selected_x = current->_x + current->_window_x;
			current->combo_selected_y = current->_y + current->_window_y + ZUI_ELEMENT_H();
			current->combo_selected_w = current->_w;
			current->combo_search_bar = search_bar;
			for (int i = 0; i < count; ++i) { // Adapt combo list width to combo item width
				int w = (int)g2_string_width(current->ops.font, current->font_size, texts[i]) + 10;
				if (current->combo_selected_w < w) current->combo_selected_w = w;
			}
			if (current->combo_selected_w > current->_w * 2) current->combo_selected_w = current->_w * 2;
			if (current->combo_selected_w > current->_w) current->combo_selected_w += ZUI_TEXT_OFFSET();
			current->combo_to_submit = handle->position;
			current->combo_initial_value = handle->position;
		}
	}
	if (handle == current->combo_selected_handle && (current->is_escape_down || current->input_released_r)) {
		handle->position = current->combo_initial_value;
		handle->changed = current->changed = true;
		current->submit_combo_handle = NULL;
	}
	else if (handle == current->submit_combo_handle) {
		handle->position = current->combo_to_submit;
		current->submit_combo_handle = NULL;
		handle->changed = current->changed = true;
	}
	else handle->changed = false;

	bool hover = zui_get_hover(ZUI_ELEMENT_H());
	if (hover) { // Bg
		g2_set_color(current->ops.theme.ACCENT_HOVER_COL);
		zui_draw_rect(current->ops.theme.FILL_ACCENT_BG, current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w - current->button_offset_y * 2, ZUI_BUTTON_H());
	}
	else {
		g2_set_color(current->ops.theme.ACCENT_COL);
		zui_draw_rect(current->ops.theme.FILL_ACCENT_BG, current->_x + current->button_offset_y, current->_y + current->button_offset_y, current->_w - current->button_offset_y * 2, ZUI_BUTTON_H());
	}

	int x = current->_x + current->_w - current->arrow_offset_x - 8;
	int y = current->_y + current->arrow_offset_y + 3;
	g2_fill_triangle(x, y, x + ZUI_ARROW_SIZE(), y, x + ZUI_ARROW_SIZE() / 2, y + ZUI_ARROW_SIZE() / 2);

	if (show_label && label[0] != 0) {
		if (align == ZUI_ALIGN_LEFT) current->_x -= 15;
		g2_set_color(current->ops.theme.LABEL_COL);
		zui_draw_string(label, current->ops.theme.TEXT_OFFSET, 0, align == ZUI_ALIGN_LEFT ? ZUI_ALIGN_RIGHT : ZUI_ALIGN_LEFT, true);
		if (align == ZUI_ALIGN_LEFT) current->_x += 15;
	}

	if (align == ZUI_ALIGN_RIGHT) current->_x -= 15;
	g2_set_color(current->ops.theme.TEXT_COL); // Value
	if (handle->position < count) {
		zui_draw_string(texts[handle->position], current->ops.theme.TEXT_OFFSET, 0, align, true);
	}
	if (align == ZUI_ALIGN_RIGHT) current->_x += 15;

	zui_end_element();
	return handle->position;
}

float zui_slider(zui_handle_t *handle, char *text, float from, float to, bool filled, float precision, bool display_value, int align, bool text_edit) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		zui_end_element();
		return handle->value;
	}
	if (zui_get_started(ZUI_ELEMENT_H())) {
		current->scroll_handle = handle;
		current->is_scrolling = true;
		current->changed = handle->changed = true;
		if (zui_touch_tooltip) {
			current->slider_tooltip = true;
			current->slider_tooltip_x = current->_x + current->_window_x;
			current->slider_tooltip_y = current->_y + current->_window_y;
			current->slider_tooltip_w = current->_w;
		}
	}
	else handle->changed = false;

	#if !defined(KORE_ANDROID) && !defined(KORE_IOS)
	if (handle == current->scroll_handle && current->input_dx != 0) { // Scroll
	#else
	if (handle == current->scroll_handle) { // Scroll
	#endif
		float range = to - from;
		float slider_x = current->_x + current->_window_x + current->button_offset_y;
		float slider_w = current->_w - current->button_offset_y * 2;
		float step = range / slider_w;
		float value = from + (current->input_x - slider_x) * step;
		handle->value = round(value * precision) / precision;
		if (handle->value < from) handle->value = from; // Stay in bounds
		else if (handle->value > to) handle->value = to;
		handle->changed = current->changed = true;
	}

	bool hover = zui_get_hover(ZUI_ELEMENT_H());
	zui_draw_slider(handle->value, from, to, filled, hover); // Slider

	// Text edit
	bool start_edit = (zui_get_released(ZUI_ELEMENT_H()) || current->tab_pressed) && text_edit;
	if (start_edit) { // Mouse did not move
		// sprintf(handle->text, "%f", handle->value);
		zui_start_text_edit(handle, ZUI_ALIGN_LEFT);
		handle->changed = current->changed = true;
	}
	int lalign = align == ZUI_ALIGN_LEFT ? ZUI_ALIGN_RIGHT : ZUI_ALIGN_LEFT;
	if (current->text_selected_handle == handle) {
		zui_update_text_edit(lalign, true, false);
	}
	if (current->submit_text_handle == handle) {
		zui_submit_text_edit();
		// try {
		// 	handle->value = js.Lib.eval(handle.text);
		// }
		// catch(_) {}
		handle->value = atof(handle->text);
		handle->changed = current->changed = true;
	}

	g2_set_color(current->ops.theme.LABEL_COL); // Text
	zui_draw_string(text, current->ops.theme.TEXT_OFFSET, 0, align, true);

	if (display_value) {
		g2_set_color(current->ops.theme.TEXT_COL); // Value
		if (current->text_selected_handle != handle) {
			sprintf(temp, "%f", round(handle->value * precision) / precision);
			zui_draw_string(temp, current->ops.theme.TEXT_OFFSET, 0, lalign, true);
		}
		else {
			zui_draw_string(current->text_selected, current->ops.theme.TEXT_OFFSET, 0, lalign, true);
		}
	}

	zui_end_element();
	return handle->value;
}

void zui_separator(int h, bool fill) {
	if (!zui_is_visible(ZUI_ELEMENT_H())) {
		current->_y += h * ZUI_SCALE();
		return;
	}
	if (fill) {
		g2_set_color(current->ops.theme.SEPARATOR_COL);
		g2_fill_rect(current->_x, current->_y, current->_w, h * ZUI_SCALE());
	}
	current->_y += h * ZUI_SCALE();
}

void zui_tooltip(char *text) {
	current->tooltip_text = text;
	current->tooltip_y = current->_y + current->_window_y;
}

void zui_tooltip_image(kinc_g4_texture_t *image, int max_width) {
	current->tooltip_img = image;
	current->tooltip_img_max_width = max_width;
	current->tooltip_invert_y = current->image_invert_y;
	current->tooltip_y = current->_y + current->_window_y;
}

void zui_end(bool last) {
	if (!current->window_ended) zui_end_window(true);
	zui_draw_combo(); // Handle active combo
	zui_draw_tooltip(true);
	current->tab_pressed_handle = NULL;
	if (last) zui_end_input();
}

void zui_set_input_position(int x, int y) {
	current->input_dx += x - current->input_x;
	current->input_dy += y - current->input_y;
	current->input_x = x;
	current->input_y = y;
}

// Useful for drag and drop operations
char *zui_hovered_tab_name() {
	return zui_input_in_rect(current->hovered_tab_x, current->hovered_tab_y, current->hovered_tab_w, current->hovered_tab_h) ? current->hovered_tab_name : "";
}

void zui_mouse_down(int button, int x, int y) {
	if (current == NULL) return;
	if (current->pen_in_use) return;
	if (button == 0) { current->input_started = current->input_down = true; }
	else 			 { current->input_started_r = current->input_down_r = true; }
	current->input_started_time = kinc_time();
	#if defined(KORE_ANDROID) || defined(KORE_IOS)
	zui_set_input_position(x, y);
	#endif
	current->input_started_x = x;
	current->input_started_y = y;
}

void zui_mouse_move(int x, int y, int movement_x, int movement_y) {
	if (current == NULL) return;
	#if !defined(KORE_ANDROID) && !defined(KORE_IOS)
	zui_set_input_position(x, y);
	#endif
}

void zui_mouse_up(int button, int x, int y) {
	if (current == NULL) return;
	if (current->pen_in_use) return;

	if (current->touch_hold_activated) {
		current->touch_hold_activated = false;
		return;
	}

	if (current->is_scrolling) { // Prevent action when scrolling is active
		current->is_scrolling = false;
		current->scroll_handle = NULL;
		current->slider_tooltip = false;
		if (x == current->input_started_x && y == current->input_started_y) { // Mouse not moved
			if (button == 0) current->input_released = true;
			else current->input_released_r = true;
		}
	}
	else {
		if (button == 0) current->input_released = true;
		else current->input_released_r = true;
	}
	if (button == 0) current->input_down = false;
	else current->input_down_r = false;
	#if defined(KORE_ANDROID) || defined(KORE_IOS)
	zui_set_input_position(x, y);
	#endif
	zui_deselect_text();
}

void zui_mouse_wheel(int delta) {
	if (current == NULL) return;
	current->input_wheel_delta = delta;
}

void zui_pen_down(int x, int y, float pressure) {
	if (current == NULL) return;

	#if defined(KORE_ANDROID) || defined(KORE_IOS)
	return;
	#endif

	zui_mouse_down(0, x, y);
}

void zui_pen_up(int x, int y, float pressure) {
	if (current == NULL) return;

	#if defined(KORE_ANDROID) || defined(KORE_IOS)
	return;
	#endif

	if (current->input_started) { current->input_started = false; current->pen_in_use = true; return; }
	zui_mouse_up(0, x, y);
	current->pen_in_use = true; // On pen release, additional mouse down & up events are fired at once - filter those out
}

void zui_pen_move(int x, int y, float pressure) {
	if (current == NULL) return;

	#if defined(KORE_ANDROID) || defined(KORE_IOS)
	return;
	#endif

	zui_mouse_move(x, y, 0, 0);
}

void zui_key_down(int key_code) {
	if (current == NULL) return;
	current->key_code = key_code;
	current->is_key_pressed = true;
	current->is_key_down = true;
	zui_key_repeat_time = kinc_time() + 0.4;
	switch (key_code) {
		case KINC_KEY_SHIFT: current->is_shift_down = true; break;
		case KINC_KEY_CONTROL: current->is_ctrl_down = true; break;
		#ifdef KORE_DARWIN
		case KINC_KEY_META: current->is_ctrl_down = true; break;
		#endif
		case KINC_KEY_ALT: current->is_alt_down = true; break;
		case KINC_KEY_BACKSPACE: current->is_backspace_down = true; break;
		case KINC_KEY_DELETE: current->is_delete_down = true; break;
		case KINC_KEY_ESCAPE: current->is_escape_down = true; break;
		case KINC_KEY_RETURN: current->is_return_down = true; break;
		case KINC_KEY_TAB: current->is_tab_down = true; break;
		case KINC_KEY_A: current->is_a_down = true; break;
		case KINC_KEY_SPACE: current->key_char = ' '; break;
		#ifdef ZUI_ANDROID_RMB // Detect right mouse button on Android..
		case KINC_KEY_BACK: if (!current->input_down_r) zui_mouse_down(1, current->input_x, current->input_y); break;
		#endif
	}
}

void zui_key_up(int key_code) {
	if (current == NULL) return;
	current->is_key_down = false;
	switch (key_code) {
		case KINC_KEY_SHIFT: current->is_shift_down = false; break;
		case KINC_KEY_CONTROL: current->is_ctrl_down = false; break;
		#ifdef KORE_DARWIN
		case KINC_KEY_META: current->is_ctrl_down = false; break;
		#endif
		case KINC_KEY_ALT: current->is_alt_down = false; break;
		case KINC_KEY_BACKSPACE: current->is_backspace_down = false; break;
		case KINC_KEY_DELETE: current->is_delete_down = false; break;
		case KINC_KEY_ESCAPE: current->is_escape_down = false; break;
		case KINC_KEY_RETURN: current->is_return_down = false; break;
		case KINC_KEY_TAB: current->is_tab_down = false; break;
		case KINC_KEY_A: current->is_a_down = false; break;
		#ifdef ZUI_ANDROID_RMB
		case KINC_KEY_BACK: zui_mouse_down(1, current->input_x, current->input_y); break;
		#endif
	}
}

void zui_key_press(unsigned key_char) {
	if (current == NULL) return;
	current->key_char = key_char;
	current->is_key_pressed = true;
}

#if defined(KORE_ANDROID) || defined(KORE_IOS)
static float zui_pinch_distance = 0.0;
static float zui_pinch_total = 0.0;
static bool zui_pinch_started = false;

void zui_touch_down(int index, int x, int y) {
	if (current == NULL) return;
	// Reset movement delta on touch start
	if (index == 0) {
		current->input_dx = 0;
		current->input_dy = 0;
		current->input_x = x;
		current->input_y = y;
	}
	// Two fingers down - right mouse button
	else if (index == 1) {
		current->input_down = false;
		zui_mouse_down(1, current->input_x, current->input_y);
		zui_pinch_started = true;
		zui_pinch_total = 0.0;
		zui_pinch_distance = 0.0;
	}
	// Three fingers down - middle mouse button
	else if (index == 2) {
		current->input_down_r = false;
		zui_mouse_down(2, current->input_x, current->input_y);
	}
}

void zui_touch_up(int index, int x, int y) {
	if (current == NULL) return;
	if (index == 1) zui_mouse_up(1, current->input_x, current->input_y);
}

void zui_touch_move(int index, int x, int y) {
	if (current == NULL) return;
	if (index == 0) zui_set_input_position(x, y);

	// Pinch to zoom - mouse wheel
	if (index == 1) {
		float last_distance = zui_pinch_distance;
		float dx = current->input_x - x;
		float dy = current->input_y - y;
		zui_pinch_distance = sqrt(dx * dx + dy * dy);
		zui_pinch_total += last_distance != 0 ? last_distance - zui_pinch_distance : 0;
		if (!zui_pinch_started) {
			current->input_wheel_delta = zui_pinch_total / 50;
			if (current->input_wheel_delta != 0) {
				zui_pinch_total = 0.0;
			}
		}
		zui_pinch_started = false;
	}
}
#endif

char *zui_copy() {
	zui_is_copy = true;
	return &zui_text_to_copy[0];
}

char *zui_cut() {
	zui_is_cut = true;
	return zui_copy();
}

void zui_paste(char *s) {
	zui_is_paste = true;
	strcpy(zui_text_to_paste, s);
}

zui_theme_t zui_theme_default() {
	zui_theme_t t;
	t.WINDOW_BG_COL = 0xff292929;
	t.WINDOW_TINT_COL = 0xffffffff;
	t.ACCENT_COL = 0xff383838;
	t.ACCENT_HOVER_COL = 0xff434343;
	t.ACCENT_SELECT_COL = 0xff505050;
	t.BUTTON_COL = 0xff383838;
	t.BUTTON_TEXT_COL = 0xffe8e8e8;
	t.BUTTON_HOVER_COL = 0xff434343;
	t.BUTTON_PRESSED_COL = 0xff222222;
	t.TEXT_COL = 0xffe8e8e8;
	t.LABEL_COL = 0xffc8c8c8;
	t.SEPARATOR_COL = 0xff202020;
	t.HIGHLIGHT_COL = 0xff205d9c;
	t.CONTEXT_COL = 0xff222222;
	t.PANEL_BG_COL = 0xff3b3b3b;
	t.FONT_SIZE = 13;
	t.ELEMENT_W = 100;
	t.ELEMENT_H = 24;
	t.ELEMENT_OFFSET = 4;
	t.ARROW_SIZE = 5;
	t.BUTTON_H = 22;
	t.CHECK_SIZE = 15;
	t.CHECK_SELECT_SIZE = 8;
	t.SCROLL_W = 9;
	t.TEXT_OFFSET = 8;
	t.TAB_W = 6;
	t.FILL_WINDOW_BG = false;
	t.FILL_BUTTON_BG = true;
	t.FILL_ACCENT_BG = false;
	t.LINK_STYLE = ZUI_LINK_STYLE_LINE;
	t.FULL_TABS = false;
	return t;
}
