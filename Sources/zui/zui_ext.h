#pragma once

#include "zui.h"

float zui_float_input(zui_handle_t *handle, char *label, int align, float precision);
char *zui_file_browser(zui_handle_t *handle, bool folders_only);
int zui_inline_radio(zui_handle_t *handle, char **texts, int count, int align);
int zui_color_wheel(zui_handle_t *handle, bool alpha, float w, float h, bool color_preview, void (*picker)(void *), void *data);
char *zui_text_area(zui_handle_t *handle, int align, bool editable, char *label, bool word_wrap);
void zui_begin_menu();
void zui_end_menu();
bool zui_menu_button(char *text);
void zui_hsv_to_rgb(float cr, float cg, float cb, float *out);
void zui_rgb_to_hsv(float cr, float cg, float cb, float *out);

uint8_t zui_color_r(uint32_t color);
uint8_t zui_color_g(uint32_t color);
uint8_t zui_color_b(uint32_t color);
uint8_t zui_color_a(uint32_t color);
uint32_t zui_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
