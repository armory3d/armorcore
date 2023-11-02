
// Immediate Mode UI Library
// https://github.com/armory3d/armorcore

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "../g2/g2.h"

#define ZUI_LAYOUT_VERTICAL 0
#define ZUI_LAYOUT_HORIZONTAL 1
#define ZUI_ALIGN_LEFT 0
#define ZUI_ALIGN_CENTER 1
#define ZUI_ALIGN_RIGHT 2
#define ZUI_STATE_IDLE 0
#define ZUI_STATE_STARTED 1
#define ZUI_STATE_DOWN 2
#define ZUI_STATE_RELEASED 3
#define ZUI_STATE_HOVERED 4
#define ZUI_LINK_STYLE_LINE 0
#define ZUI_LINK_STYLE_CUBIC_BEZIER 1

// typedef enum zui_link_style {
// 	ZUI_LINK_STYLE_LINE,
// 	ZUI_LINK_STYLE_CUBIC_BEZIER
// } zui_link_style_t;

typedef struct zui_theme {
	int WINDOW_BG_COL;
	int WINDOW_TINT_COL;
	int ACCENT_COL;
	int ACCENT_HOVER_COL;
	int ACCENT_SELECT_COL;
	int BUTTON_COL;
	int BUTTON_TEXT_COL;
	int BUTTON_HOVER_COL;
	int BUTTON_PRESSED_COL;
	int TEXT_COL;
	int LABEL_COL;
	int SEPARATOR_COL;
	int HIGHLIGHT_COL;
	int CONTEXT_COL;
	int PANEL_BG_COL;
	int FONT_SIZE;
	int ELEMENT_W;
	int ELEMENT_H;
	int ELEMENT_OFFSET;
	int ARROW_SIZE;
	int BUTTON_H;
	int CHECK_SIZE;
	int CHECK_SELECT_SIZE;
	int SCROLL_W;
	int SCROLL_MINI_W;
	int TEXT_OFFSET;
	int TAB_W; // Indentation
	bool FILL_WINDOW_BG;
	bool FILL_BUTTON_BG;
	bool FILL_ACCENT_BG;
	int LINK_STYLE;
	bool FULL_TABS; // Make tabs take full window width
	bool ROUND_CORNERS;
} zui_theme_t;

typedef struct zui_options {
	g2_font_t *font;
	zui_theme_t theme;
	float scale_factor;
	kinc_g4_texture_t *color_wheel;
	kinc_g4_texture_t *black_white_gradient;
} zui_options_t;

typedef struct zui_handle {
	bool selected;
	int position;
	int color; // = 0xffffffff
	float value;
	char text[128];
	kinc_g4_render_target_t texture;
	int redraws; // = 2
	float scroll_offset;
	bool scroll_enabled;
	int layout;
	float last_max_x;
	float last_max_y;
	bool drag_enabled;
	int drag_x;
	int drag_y;
	bool changed;
	int id;
} zui_handle_t;

typedef struct zui_text_extract {
	char *colored;
	char *uncolored;
} zui_text_extract_t;

typedef struct zui_coloring {
	int color;
	char **start;
	char *end;
	bool separated;
} zui_coloring_t;

typedef struct zui_text_coloring {
	zui_coloring_t *colorings;
	int default_color;
} zui_text_coloring_t;

typedef struct zui {
	bool is_scrolling; // Use to limit other activities
	bool is_typing;
	bool enabled; // Current element state
	bool is_started;
	bool is_pushed;
	bool is_hovered;
	bool is_released;
	bool changed; // Global elements change check
	bool image_invert_y;
	bool scroll_enabled;
	bool always_redraw; // Hurts performance
	bool highlight_on_select; // Highlight text edit contents on selection
	bool tab_switch_enabled; // Allow switching focus to the next element by pressing tab
	zui_text_coloring_t *text_coloring; // Set coloring scheme for zui_draw_string() calls
	int window_border_top;
	int window_border_bottom;
	int window_border_left;
	int window_border_right;
	char hovered_tab_name[64];
	float hovered_tab_x;
	float hovered_tab_y;
	float hovered_tab_w;
	float hovered_tab_h;
	bool touch_hold_activated;
	bool slider_tooltip;
	float slider_tooltip_x;
	float slider_tooltip_y;
	float slider_tooltip_w;
	bool input_enabled;
	float input_x; // Input position
	float input_y;
	float input_started_x;
	float input_started_y;
	float input_dx; // Delta
	float input_dy;
	float input_wheel_delta;
	bool input_started; // Buttons
	bool input_started_r;
	bool input_released;
	bool input_released_r;
	bool input_down;
	bool input_down_r;
	bool pen_in_use;
	bool is_key_pressed; // Keys
	bool is_key_down;
	bool is_shift_down;
	bool is_ctrl_down;
	bool is_alt_down;
	bool is_a_down;
	bool is_backspace_down;
	bool is_delete_down;
	bool is_escape_down;
	bool is_return_down;
	bool is_tab_down;
	int key_code;
	int key_char;
	float input_started_time;
	int cursor_x; // Text input
	int highlight_anchor;
	float ratios[32]; // Splitting rows
	int ratios_count;
	int current_ratio;
	float x_before_split;
	int w_before_split;

	zui_options_t ops;
	int font_size;
	float font_offset_y; // Precalculated offsets
	float arrow_offset_x;
	float arrow_offset_y;
	float title_offset_x;
	float button_offset_y;
	float check_offset_x;
	float check_offset_y;
	float check_select_offset_x;
	float check_select_offset_y;
	float radio_offset_x;
	float radio_offset_y;
	float radio_select_offset_x;
	float radio_select_offset_y;
	float scroll_align;
	bool image_scroll_align;

	float _x; // Cursor(stack) position
	float _y;
	int _w;
	int _h;

	float _window_x; // Window state
	float _window_y;
	float _window_w;
	float _window_h;
	zui_handle_t *current_window;
	bool window_ended;
	zui_handle_t *scroll_handle; // Window or slider being scrolled
	zui_handle_t *drag_handle; // Window being dragged
	zui_handle_t *drag_tab_handle; // Tab being dragged
	int drag_tab_position;
	float window_header_w;
	float window_header_h;
	float restore_x;
	float restore_y;

	zui_handle_t *text_selected_handle;
	char *text_selected;
	zui_handle_t *submit_text_handle;
	char *text_to_submit;
	bool tab_pressed;
	zui_handle_t *tab_pressed_handle;
	zui_handle_t *combo_selected_handle;
	zui_handle_t *combo_selected_window;
	int combo_selected_align;
	char **combo_selected_texts;
	char *combo_selected_label;
	int combo_selected_count;
	int combo_selected_x;
	int combo_selected_y;
	int combo_selected_w;
	bool combo_search_bar;
	zui_handle_t *submit_combo_handle;
	int combo_to_submit;
	int combo_initial_value;
	char *tooltip_text;
	kinc_g4_texture_t *tooltip_img;
	kinc_g4_render_target_t *tooltip_rt;
	int tooltip_img_max_width;
	bool tooltip_invert_y;
	float tooltip_x;
	float tooltip_y;
	bool tooltip_shown;
	bool tooltip_wait;
	double tooltip_time;
	char tab_names[16][64];
	int tab_colors[16];
	bool tab_enabled[16];
	int tab_count; // Number of tab calls since window begin
	zui_handle_t *tab_handle;
	float tab_scroll;
	bool tab_vertical;
	bool sticky;
	bool scissor;

	bool elements_baked;
	kinc_g4_render_target_t check_select_image;
	kinc_g4_render_target_t radio_image;
	kinc_g4_render_target_t radio_select_image;
	kinc_g4_render_target_t round_corner_image;
	kinc_g4_render_target_t filled_round_corner_image;
} zui_t;

void zui_init(zui_t *ui, zui_options_t ops);
void zui_begin(zui_t *ui);
void zui_begin_sticky();
void zui_end_sticky();
void zui_begin_region(int x, int y, int w);
void zui_end_region(bool last);
bool zui_window(zui_handle_t *handle, int x, int y, int w, int h, bool drag); // Returns true if redraw is needed
bool zui_button(char *text, int align, char *label);
int zui_text(char *text, int align, int bg);
bool zui_tab(zui_handle_t *handle, char *text, bool vertical, uint32_t color);
bool zui_panel(zui_handle_t *handle, char *text, bool is_tree, bool filled, bool pack);
int zui_sub_image(/*kinc_g4_texture_t kinc_g4_render_target_t*/ void *image, bool is_rt, int tint, int h, int sx, int sy, int sw, int sh);
int zui_image(/*kinc_g4_texture_t kinc_g4_render_target_t*/ void *image, bool is_rt, int tint, int h);
char *zui_text_input(zui_handle_t *handle, char *label, int align, bool editable, bool live_update);
bool zui_check(zui_handle_t *handle, char *text, char *label);
bool zui_radio(zui_handle_t *handle, int position, char *text, char *label);
int zui_combo(zui_handle_t *handle, char **texts, int count, char *label, bool show_label, int align, bool search_bar);
float zui_slider(zui_handle_t *handle, char *text, float from, float to, bool filled, float precision, bool display_value, int align, bool text_edit);
void zui_row(float *ratios, int count);
void zui_separator(int h, bool fill);
void zui_tooltip(char *text);
void zui_tooltip_image(kinc_g4_texture_t *image, int max_width);
void zui_tooltip_render_target(kinc_g4_render_target_t *image, int max_width);
void zui_end(bool last);
void zui_end_window(bool bind_global_g);
char *zui_hovered_tab_name();
void zui_mouse_down(int button, int x, int y); // Input events
void zui_mouse_move(int x, int y, int movement_x, int movement_y);
void zui_mouse_up(int button, int x, int y);
void zui_mouse_wheel(int delta);
void zui_pen_down(int x, int y, float pressure);
void zui_pen_up(int x, int y, float pressure) ;
void zui_pen_move(int x, int y, float pressure);
void zui_key_down(int key_code);
void zui_key_up(int key_code);
void zui_key_press(unsigned character);
#if defined(KORE_ANDROID) || defined(KORE_IOS)
void zui_touch_down(int index, int x, int y);
void zui_touch_up(int index, int x, int y);
void zui_touch_move(int index, int x, int y);
#endif
zui_theme_t zui_theme_default();
zui_t *zui_get_current();
zui_handle_t *zui_nest(zui_handle_t *handle, int id);
void zui_set_scale(float factor);

bool zui_get_hover(float elem_h);
bool zui_get_released(float elem_h);
bool zui_input_in_rect(float x, float y, float w, float h);
void zui_fill(float x, float y, float w, float h, int color);
void zui_rect(float x, float y, float w, float h, int color, float strength);
int zui_line_count(char *str);
char *zui_extract_line(char *str, int line);
bool zui_is_visible(float elem_h);
void zui_end_element();
void zui_end_element_of_size(float element_size);
void zui_end_input();
void zui_fade_color(float alpha);
void zui_draw_string(char *text, float x_offset, float y_offset, int align, bool truncation);
void zui_draw_rect(bool fill, float x, float y, float w, float h);
void zui_start_text_edit(zui_handle_t *handle, int align);

float ZUI_SCALE();
float ZUI_ELEMENT_W();
float ZUI_ELEMENT_H();
float ZUI_ELEMENT_OFFSET();
float ZUI_ARROW_SIZE();
float ZUI_BUTTON_H();
float ZUI_CHECK_SIZE();
float ZUI_CHECK_SELECT_SIZE();
int ZUI_FONT_SIZE();
int ZUI_SCROLL_W();
float ZUI_TEXT_OFFSET();
int ZUI_TAB_W();
int ZUI_HEADER_DRAG_H();
float ZUI_FLASH_SPEED();
float ZUI_TOOLTIP_DELAY();
