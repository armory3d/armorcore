// Bindings to Zui in C

class zui_t {

	_t: theme_t = null;

	get t(): theme_t {
		return this._t;
	}

	set t(theme: theme_t) {
		if (this.t != null) {
			for (let key of Object.getOwnPropertyNames(theme_t.prototype)) {
				if (key == "constructor") {
					continue;
				}
				let t_: any = this.t;
				let theme_: any = theme;
				t_[key] = theme_[key];
			}
			theme.theme_ = this.t.theme_;
		}
		this._t = theme;
	}

	font: g2_font_t;
	zui_: any;

	get is_scrolling(): bool { return krom_zui_get(this.zui_, "is_scrolling"); }

	get is_typing(): bool { return krom_zui_get(this.zui_, "is_typing"); }

	get enabled(): bool { return krom_zui_get(this.zui_, "enabled"); }
	set enabled(a: bool) { krom_zui_set(this.zui_, "enabled", a); }

	get is_hovered(): bool { return krom_zui_get(this.zui_, "is_hovered"); }
	set is_hovered(a: bool) { krom_zui_set(this.zui_, "is_hovered", a); }

	get is_released(): bool { return krom_zui_get(this.zui_, "is_released"); }

	get changed(): bool { return krom_zui_get(this.zui_, "changed"); }
	set changed(a: bool) { krom_zui_set(this.zui_, "changed", a); }

	set image_invert_y(a: bool) { krom_zui_set(this.zui_, "image_invert_y", a); }

	set scroll_enabled(a: bool) { krom_zui_set(this.zui_, "scroll_enabled", a); }

	set window_border_top(a: i32) { krom_zui_set(this.zui_, "window_border_top", a); }

	set window_border_bottom(a: i32) { krom_zui_set(this.zui_, "window_border_bottom", a); }

	set window_border_right(a: i32) { krom_zui_set(this.zui_, "window_border_right", a); }

	get input_enabled(): bool { return krom_zui_get(this.zui_, "input_enabled"); }
	set input_enabled(a: bool) { krom_zui_set(this.zui_, "input_enabled", a); }

	get input_x(): f32 { return krom_zui_get(this.zui_, "input_x"); }
	set input_x(a: f32) { krom_zui_set(this.zui_, "input_x", a); }

	get input_y(): f32 { return krom_zui_get(this.zui_, "input_y"); }
	set input_y(a: f32) { krom_zui_set(this.zui_, "input_y", a); }

	get input_started_x(): f32 { return krom_zui_get(this.zui_, "input_started_x"); }

	get input_started_y(): f32 { return krom_zui_get(this.zui_, "input_started_y"); }

	get input_dx(): f32 { return krom_zui_get(this.zui_, "input_dx"); }

	get input_dy(): f32 { return krom_zui_get(this.zui_, "input_dy"); }

	get input_wheel_delta(): f32 { return krom_zui_get(this.zui_, "input_wheel_delta"); }

	get input_started(): bool { return krom_zui_get(this.zui_, "input_started"); }
	set input_started(a: bool) { krom_zui_set(this.zui_, "input_started", a); }

	get input_started_r(): bool { return krom_zui_get(this.zui_, "input_started_r"); }

	get input_released(): bool { return krom_zui_get(this.zui_, "input_released"); }

	get input_released_r(): bool { return krom_zui_get(this.zui_, "input_released_r"); }

	get input_down(): bool { return krom_zui_get(this.zui_, "input_down"); }

	get input_down_r(): bool { return krom_zui_get(this.zui_, "input_down_r"); }

	get is_key_pressed(): bool { return krom_zui_get(this.zui_, "is_key_pressed"); }

	get is_ctrl_down(): bool { return krom_zui_get(this.zui_, "is_ctrl_down"); }

	get is_delete_down(): bool { return krom_zui_get(this.zui_, "is_delete_down"); }
	set is_delete_down(a: bool) { krom_zui_set(this.zui_, "is_delete_down", a); }

	get is_escape_down(): bool { return krom_zui_get(this.zui_, "is_escape_down"); }

	get is_return_down(): bool { return krom_zui_get(this.zui_, "is_return_down"); }

	get key(): key_code_t { return krom_zui_get(this.zui_, "key_code"); }

	get cur_ratio(): i32 { return krom_zui_get(this.zui_, "current_ratio"); }
	set cur_ratio(a: i32) { krom_zui_set(this.zui_, "current_ratio", a); }

	get font_size(): i32 { return krom_zui_get(this.zui_, "font_size"); }
	set font_size(a: i32) { krom_zui_set(this.zui_, "font_size", a); }

	get font_offset_y(): f32 { return krom_zui_get(this.zui_, "font_offset_y"); }
	set font_offset_y(a: f32) { krom_zui_set(this.zui_, "font_offset_y", a); }

	set image_scroll_align(a: bool) { krom_zui_set(this.zui_, "image_scroll_align", a); }

	get _x(): f32 { return krom_zui_get(this.zui_, "_x"); }
	set _x(a: f32) { krom_zui_set(this.zui_, "_x", a); }

	get _y(): f32 { return krom_zui_get(this.zui_, "_y"); }
	set _y(a: f32) { krom_zui_set(this.zui_, "_y", a); }

	get _w(): i32 { return krom_zui_get(this.zui_, "_w"); }
	set _w(a: i32) { krom_zui_set(this.zui_, "_w", a); }

	get _window_x(): f32 { return krom_zui_get(this.zui_, "_window_x"); }

	get _window_y(): f32 { return krom_zui_get(this.zui_, "_window_y"); }

	get _window_w(): f32 { return krom_zui_get(this.zui_, "_window_w"); }

	get _window_h(): f32 { return krom_zui_get(this.zui_, "_window_h"); }

	get scissor(): bool { return krom_zui_get(this.zui_, "scissor"); }
	set scissor(a: bool) { krom_zui_set(this.zui_, "scissor", a); }

	set elements_baked(a: bool) { krom_zui_set(this.zui_, "elements_baked", a); }

	get text_selected_handle_ptr(): i32 { return krom_zui_get(this.zui_, "text_selected_handle"); }

	get submit_text_handle_ptr(): i32 { return krom_zui_get(this.zui_, "submit_text_handle"); }

	get combo_selected_handle_ptr(): i32 { return krom_zui_get(this.zui_, "combo_selected_handle"); }
}

let zui_current: zui_t = null;
let zui_children: map_t<string, zui_handle_t> = map_create();

function zui_handle(s: string, ops: zui_handle_ops_t = null): zui_handle_t {
	let h = map_get(zui_children, s);
	if (h == null) {
		h = zui_handle_create(ops);
		map_set(zui_children, s, h);
	}
	return h;
}

function zui_nest(raw: zui_handle_t, i: i32, ops: zui_handle_ops_t = null): zui_handle_t {
	if (raw.children == null) raw.children = map_create();
	let c = map_get(raw.children, i);
	if (c == null) {
		c = zui_handle_create(ops);
		map_set(raw.children, i, c);
	}
	return c;
}

function zui_set_on_border_hover(f: (handle_ptr: any, side: i32)=>void) { krom_zui_set_on_border_hover(f); }
function zui_set_on_text_hover(f: ()=>void) { krom_zui_set_on_text_hover(f); }
function zui_set_on_deselect_text(f: ()=>void) { krom_zui_set_on_deselect_text(f); }
function zui_set_on_tab_drop(f: (to_ptr: any, to_pos: i32, from_ptr: any, from_pos: i32)=>void) { krom_zui_set_on_tab_drop(f); }
function zui_set_text_area_line_numbers(a: bool) { krom_zui_set(null, "zui_text_area_line_numbers", a); }
function zui_set_text_area_scroll_past_end(a: bool) { krom_zui_set(null, "zui_text_area_scroll_past_end", a); }
function zui_set_text_area_coloring(coloring: zui_text_coloring_t) {
	krom_zui_text_area_coloring(coloring == null ? null : armpack_encode(coloring));
}
function zui_always_redraw_window(): bool { return krom_zui_get(null, "zui_always_redraw_window"); }
function zui_set_always_redraw_window(a: bool) { krom_zui_set(null, "zui_always_redraw_window", a); }
function zui_touch_scroll(): bool { return krom_zui_get(null, "zui_touch_scroll"); }
function zui_set_touch_scroll(a: bool) { krom_zui_set(null, "zui_touch_scroll", a); }
function zui_touch_hold(): bool { return krom_zui_get(null, "zui_touch_hold"); }
function zui_set_touch_hold(a: bool) { krom_zui_set(null, "zui_touch_hold", a); }
function zui_touch_tooltip(): bool { return krom_zui_get(null, "zui_touch_tooltip"); }
function zui_set_touch_tooltip(a: bool) { krom_zui_set(null, "zui_touch_tooltip", a); }
function zui_set_is_cut(a: bool) { krom_zui_set(null, "zui_is_cut", a); }
function zui_set_is_copy(a: bool) { krom_zui_set(null, "zui_is_copy", a); }
function zui_set_is_paste(a: bool) { krom_zui_set(null, "zui_is_paste", a); }
function zui_is_paste(): bool { return krom_zui_get(null, "zui_is_paste"); }

function zui_create(ops: zui_ops_t): zui_t {
	let raw = new zui_t();
	raw.zui_ = krom_zui_init(
		{
			font: ops.font.font_,
			theme: ops.theme.theme_,
			scale_factor: ops.scale_factor,
			color_wheel: ops.color_wheel != null ? ops.color_wheel.texture_ : null,
			black_white_gradient: ops.black_white_gradient != null ? ops.black_white_gradient.texture_ : null
		}
	);
	zui_current = raw;
	raw.t = ops.theme;
	raw.font = ops.font;
	return raw;
}

function zui_set_font(raw: zui_t, font: g2_font_t) {
	g2_font_init(font); // Make sure font_ is ready
	raw.font = font;
	krom_zui_set_font(raw.zui_, font.font_);
}

function zui_SCALE(raw: zui_t): f32 {
	return krom_zui_get_scale(raw.zui_);
}

function zui_set_scale(raw: zui_t, factor: f32) {
	krom_zui_set_scale(raw.zui_, factor);
}

function zui_begin(raw: zui_t) {
	zui_current = raw;
	krom_zui_begin(raw.zui_);
}

function zui_end(last: bool = true) {
	krom_zui_end(last);
	_g2_current = null;
}

function zui_begin_region(raw: zui_t, x: i32, y: i32, w: i32) {
	zui_current = raw;
	krom_zui_begin_region(raw.zui_, x, y, w);
}

function zui_end_region(last: bool = true) {
	krom_zui_end_region(last);
}

function zui_begin_sticky() {
	krom_zui_begin_sticky();
}

function zui_end_sticky() {
	krom_zui_end_sticky();
}

function zui_end_input() {
	krom_zui_end_input();
}

function zui_window(handle: zui_handle_t, x: i32, y: i32, w: i32, h: i32, drag: bool = false): bool {
	let img = _image_create(null);
	img.render_target_ = handle.texture;
	_g2_current = img;
	return krom_zui_window(handle.handle_, x, y, w, h, drag);
}

function zui_end_window(bind_global_g = true) {
	krom_zui_end_window(bind_global_g);
}

function zui_tab(handle: zui_handle_t, text: string, vertical: bool = false, color: i32 = -1): bool {
	return krom_zui_tab(handle.handle_, text, vertical, color);
}

function zui_panel(handle: zui_handle_t, text: string, is_tree: bool = false, filled: bool = true, pack: bool = true): bool {
	return krom_zui_panel(handle.handle_, text, is_tree, filled, pack);
}

function zui_image(image: image_t, tint: i32 = 0xffffffff, h: f32 = -1.0, sx: i32 = 0, sy: i32 = 0, sw: i32 = 0, sh: i32 = 0): zui_state_t {
	return krom_zui_image(image, tint, math_floor(h), sx, sy, sw, sh);
}

function zui_text(text: string, align: zui_align_t = zui_align_t.LEFT, bg: i32 = 0x00000000): zui_state_t {
	return krom_zui_text(text, align, bg);
}

function zui_text_input(handle: zui_handle_t, label: string = "", align: zui_align_t = zui_align_t.LEFT, editable: bool = true, live_update: bool = false): string {
	return krom_zui_text_input(handle.handle_, label, align, editable, live_update);
}

function zui_button(text: string, align: zui_align_t = zui_align_t.CENTER, label: string = "", icon: image_t = null, sx: i32 = 0, sy: i32 = 0, sw: i32 = 0, sh: i32 = 0): bool {
	return krom_zui_button(text, align, label);
}

function zui_check(handle: zui_handle_t, text: string, label: string = ""): bool {
	return krom_zui_check(handle.handle_, text, label);
}

function zui_radio(handle: zui_handle_t, position: i32, text: string, label: string = ""): bool {
	return krom_zui_radio(handle.handle_, position, text, label);
}

function zui_combo(handle: zui_handle_t, texts: string[], label: string = "", show_label: bool = false, align: zui_align_t = zui_align_t.LEFT, search_bar: bool = true): i32 {
	return krom_zui_combo(handle.handle_, texts, label, show_label, align, search_bar);
}

function zui_slider(handle: zui_handle_t, text: string, from: f32 = 0.0, to: f32 = 1.0, filled: bool = false, precision: f32 = 100.0, display_value: bool = true, align: zui_align_t = zui_align_t.RIGHT, text_edit: bool = true): f32 {
	return krom_zui_slider(handle.handle_, text, from, to, filled, precision, display_value, align, text_edit);
}

function zui_separator(h: i32 = 4, fill: bool = true) {
	krom_zui_separator(h, fill);
}

function zui_tooltip(text: string) {
	krom_zui_tooltip(text);
}

function zui_tooltip_image(image: image_t, max_width: i32 = 0) {
	krom_zui_tooltip_image(image, max_width);
}

function zui_row(ratios: f32[]) {
	krom_zui_row(ratios);
}

function zui_fill(x: f32, y: f32, w: f32, h: f32, color: color_t) {
	krom_zui_fill(x, y, w, h, color);
}

function zui_rect(x: f32, y: f32, w: f32, h: f32, color: color_t, strength: f32 = 1.0) {
	krom_zui_rect(x, y, w, h, color, strength);
}

function zui_draw_rect(fill: bool, x: f32, y: f32, w: f32, h: f32, strength: f32 = 0.0) {
	krom_zui_draw_rect(fill, x, y, w, h, strength);
}

function zui_end_element(element_size: f32 = -1.0) {
	krom_zui_end_element(element_size);
}

function zui_start_text_edit(handle: zui_handle_t, align: zui_align_t = zui_align_t.LEFT) {
	krom_zui_start_text_edit(handle.handle_, align);
}

function zui_get_input_in_rect(x: f32, y: f32, w: f32, h: f32): bool {
	return krom_zui_input_in_rect(x, y, w, h);
}

function zui_draw_string(text: string, x_offset: f32 = -1.0, y_offset: f32 = 0, align: zui_align_t = zui_align_t.LEFT, truncation: bool = true) {
	krom_zui_draw_string(text, x_offset, y_offset, align, truncation);
}

function zui_get_hovered_tab_name(): string {
	return krom_zui_get_hovered_tab_name();
}

function zui_set_hovered_tab_name(name: string) {
	krom_zui_set_hovered_tab_name(name);
}

function zui_ELEMENT_W(raw: zui_t): f32 {
	return raw.t.ELEMENT_W * zui_SCALE(raw);
}

function zui_ELEMENT_H(raw: zui_t): f32 {
	return raw.t.ELEMENT_H * zui_SCALE(raw);
}

function zui_ELEMENT_OFFSET(raw: zui_t): f32 {
	return raw.t.ELEMENT_OFFSET * zui_SCALE(raw);
}

function zui_float_input(handle: zui_handle_t, label: string = "", align: zui_align_t = zui_align_t.LEFT, precision: f32 = 1000.0): f32 {
	return krom_zui_float_input(handle.handle_, label, align, precision);
}

function zui_inline_radio(handle: zui_handle_t, texts: string[], align: zui_align_t = zui_align_t.LEFT): i32 {
	return krom_zui_inline_radio(handle.handle_, texts, align);
}

function zui_color_wheel(handle: zui_handle_t, alpha: bool = false, w: f32 = -1.0, h: f32 = -1.0, color_preview: bool = true, picker: ()=>void = null): color_t {
	return krom_zui_color_wheel(handle.handle_, alpha, w, h, color_preview, picker);
}

function zui_text_area(handle: zui_handle_t, align: zui_align_t = zui_align_t.LEFT, editable: bool = true, label: string = "", word_wrap: bool = false): string {
	return krom_zui_text_area(handle.handle_, align, editable, label, word_wrap);
}

function zui_begin_menu() {
	krom_zui_begin_menu();
}

function zui_end_menu() {
	krom_zui_end_menu();
}

function zui_menu_button(text: string): bool {
	return krom_zui_menu_button(text);
}

function zui_MENUBAR_H(raw: zui_t): f32 {
	let button_offset_y = (raw.t.ELEMENT_H * zui_SCALE(raw) - raw.t.BUTTON_H * zui_SCALE(raw)) / 2;
	return raw.t.BUTTON_H * zui_SCALE(raw) * 1.1 + 2 + button_offset_y;
}

class zui_handle_t {
	handle__: any = null;
	get handle_(): any {
		if (this.handle__ == null) {
			this.handle__ = krom_zui_handle(this.ops);
		}
		return this.handle__;
	}

	ops: zui_handle_ops_t;
	children: map_t<i32, zui_handle_t>;

	get selected(): bool { return krom_zui_handle_get(this.handle_, "selected"); }
	set selected(a: bool) { krom_zui_handle_set(this.handle_, "selected", a); }

	get position(): i32 { return krom_zui_handle_get(this.handle_, "position"); }
	set position(a: i32) { krom_zui_handle_set(this.handle_, "position", a); }

	get color(): i32 { return krom_zui_handle_get(this.handle_, "color"); }
	set color(a: i32) { krom_zui_handle_set(this.handle_, "color", a); }

	get value(): f32 { return krom_zui_handle_get(this.handle_, "value"); }
	set value(a: f32) { krom_zui_handle_set(this.handle_, "value", a); }

	get text(): string { return krom_zui_handle_get(this.handle_, "text"); }
	set text(a: string) { krom_zui_handle_set(this.handle_, "text", a); }

	set redraws(a: i32) { krom_zui_handle_set(this.handle_, "redraws", a); }

	get scroll_offset(): f32 { return krom_zui_handle_get(this.handle_, "scroll_offset"); }

	get drag_x(): i32 { return krom_zui_handle_get(this.handle_, "drag_x"); }
	set drag_x(a: i32) { krom_zui_handle_set(this.handle_, "drag_x", a); }

	get drag_y(): i32 { return krom_zui_handle_get(this.handle_, "drag_y"); }
	set drag_y(a: i32) { krom_zui_handle_set(this.handle_, "drag_y", a); }

	get changed(): bool { return krom_zui_handle_get(this.handle_, "changed"); }
	set changed(a: bool) { krom_zui_handle_set(this.handle_, "changed", a); }

	get texture(): any { return krom_zui_handle_get(this.handle_, "texture"); }

	get ptr(): i32 { return krom_zui_handle_ptr(this.handle_); }
}

function zui_handle_create(ops: zui_handle_ops_t = null): zui_handle_t {
	let raw = new zui_handle_t();
	if (ops == null) {
		ops = {};
	}
	if (ops.selected == null) {
		ops.selected = false;
	}
	if (ops.position == null) {
		ops.position = 0;
	}
	if (ops.value == null) {
		ops.value = 0.0;
	}
	if (ops.text == null) {
		ops.text = "";
	}
	if (ops.color == null) {
		ops.color = 0xffffffff;
	}
	if (ops.layout == null) {
		ops.layout = zui_layout_t.VERTICAL;
	}
	raw.ops = ops;
	return raw;
}

class theme_t {
	theme_: any;

	get WINDOW_BG_COL(): i32 { return krom_zui_theme_get(this.theme_, "WINDOW_BG_COL"); }
	set WINDOW_BG_COL(a: i32) { krom_zui_theme_set(this.theme_, "WINDOW_BG_COL", a); }

	get WINDOW_TINT_COL(): i32 { return krom_zui_theme_get(this.theme_, "WINDOW_TINT_COL"); }
	set WINDOW_TINT_COL(a: i32) { krom_zui_theme_set(this.theme_, "WINDOW_TINT_COL", a); }

	get ACCENT_COL(): i32 { return krom_zui_theme_get(this.theme_, "ACCENT_COL"); }
	set ACCENT_COL(a: i32) { krom_zui_theme_set(this.theme_, "ACCENT_COL", a); }

	get ACCENT_HOVER_COL(): i32 { return krom_zui_theme_get(this.theme_, "ACCENT_HOVER_COL"); }
	set ACCENT_HOVER_COL(a: i32) { krom_zui_theme_set(this.theme_, "ACCENT_HOVER_COL", a); }

	get ACCENT_SELECT_COL(): i32 { return krom_zui_theme_get(this.theme_, "ACCENT_SELECT_COL"); }
	set ACCENT_SELECT_COL(a: i32) { krom_zui_theme_set(this.theme_, "ACCENT_SELECT_COL", a); }

	get BUTTON_COL(): i32 { return krom_zui_theme_get(this.theme_, "BUTTON_COL"); }
	set BUTTON_COL(a: i32) { krom_zui_theme_set(this.theme_, "BUTTON_COL", a); }

	get BUTTON_TEXT_COL(): i32 { return krom_zui_theme_get(this.theme_, "BUTTON_TEXT_COL"); }
	set BUTTON_TEXT_COL(a: i32) { krom_zui_theme_set(this.theme_, "BUTTON_TEXT_COL", a); }

	get BUTTON_HOVER_COL(): i32 { return krom_zui_theme_get(this.theme_, "BUTTON_HOVER_COL"); }
	set BUTTON_HOVER_COL(a: i32) { krom_zui_theme_set(this.theme_, "BUTTON_HOVER_COL", a); }

	get BUTTON_PRESSED_COL(): i32 { return krom_zui_theme_get(this.theme_, "BUTTON_PRESSED_COL"); }
	set BUTTON_PRESSED_COL(a: i32) { krom_zui_theme_set(this.theme_, "BUTTON_PRESSED_COL", a); }

	get TEXT_COL(): i32 { return krom_zui_theme_get(this.theme_, "TEXT_COL"); }
	set TEXT_COL(a: i32) { krom_zui_theme_set(this.theme_, "TEXT_COL", a); }

	get LABEL_COL(): i32 { return krom_zui_theme_get(this.theme_, "LABEL_COL"); }
	set LABEL_COL(a: i32) { krom_zui_theme_set(this.theme_, "LABEL_COL", a); }

	get SEPARATOR_COL(): i32 { return krom_zui_theme_get(this.theme_, "SEPARATOR_COL"); }
	set SEPARATOR_COL(a: i32) { krom_zui_theme_set(this.theme_, "SEPARATOR_COL", a); }

	get HIGHLIGHT_COL(): i32 { return krom_zui_theme_get(this.theme_, "HIGHLIGHT_COL"); }
	set HIGHLIGHT_COL(a: i32) { krom_zui_theme_set(this.theme_, "HIGHLIGHT_COL", a); }

	get CONTEXT_COL(): i32 { return krom_zui_theme_get(this.theme_, "CONTEXT_COL"); }
	set CONTEXT_COL(a: i32) { krom_zui_theme_set(this.theme_, "CONTEXT_COL", a); }

	get PANEL_BG_COL(): i32 { return krom_zui_theme_get(this.theme_, "PANEL_BG_COL"); }
	set PANEL_BG_COL(a: i32) { krom_zui_theme_set(this.theme_, "PANEL_BG_COL", a); }

	get FONT_SIZE(): i32 { return krom_zui_theme_get(this.theme_, "FONT_SIZE"); }
	set FONT_SIZE(a: i32) { krom_zui_theme_set(this.theme_, "FONT_SIZE", a); }

	get ELEMENT_W(): i32 { return krom_zui_theme_get(this.theme_, "ELEMENT_W"); }
	set ELEMENT_W(a: i32) { krom_zui_theme_set(this.theme_, "ELEMENT_W", a); }

	get ELEMENT_H(): i32 { return krom_zui_theme_get(this.theme_, "ELEMENT_H"); }
	set ELEMENT_H(a: i32) { krom_zui_theme_set(this.theme_, "ELEMENT_H", a); }

	get ELEMENT_OFFSET(): i32 { return krom_zui_theme_get(this.theme_, "ELEMENT_OFFSET"); }
	set ELEMENT_OFFSET(a: i32) { krom_zui_theme_set(this.theme_, "ELEMENT_OFFSET", a); }

	get ARROW_SIZE(): i32 { return krom_zui_theme_get(this.theme_, "ARROW_SIZE"); }
	set ARROW_SIZE(a: i32) { krom_zui_theme_set(this.theme_, "ARROW_SIZE", a); }

	get BUTTON_H(): i32 { return krom_zui_theme_get(this.theme_, "BUTTON_H"); }
	set BUTTON_H(a: i32) { krom_zui_theme_set(this.theme_, "BUTTON_H", a); }

	get CHECK_SIZE(): i32 { return krom_zui_theme_get(this.theme_, "CHECK_SIZE"); }
	set CHECK_SIZE(a: i32) { krom_zui_theme_set(this.theme_, "CHECK_SIZE", a); }

	get CHECK_SELECT_SIZE(): i32 { return krom_zui_theme_get(this.theme_, "CHECK_SELECT_SIZE"); }
	set CHECK_SELECT_SIZE(a: i32) { krom_zui_theme_set(this.theme_, "CHECK_SELECT_SIZE", a); }

	get SCROLL_W(): i32 { return krom_zui_theme_get(this.theme_, "SCROLL_W"); }
	set SCROLL_W(a: i32) { krom_zui_theme_set(this.theme_, "SCROLL_W", a); }

	get SCROLL_MINI_W(): i32 { return krom_zui_theme_get(this.theme_, "SCROLL_MINI_W"); }
	set SCROLL_MINI_W(a: i32) { krom_zui_theme_set(this.theme_, "SCROLL_MINI_W", a); }

	get TEXT_OFFSET(): i32 { return krom_zui_theme_get(this.theme_, "TEXT_OFFSET"); }
	set TEXT_OFFSET(a: i32) { krom_zui_theme_set(this.theme_, "TEXT_OFFSET", a); }

	get TAB_W(): i32 { return krom_zui_theme_get(this.theme_, "TAB_W"); }
	set TAB_W(a: i32) { krom_zui_theme_set(this.theme_, "TAB_W", a); }

	get FILL_WINDOW_BG(): bool { return krom_zui_theme_get(this.theme_, "FILL_WINDOW_BG") > 0; }
	set FILL_WINDOW_BG(a: bool) { krom_zui_theme_set(this.theme_, "FILL_WINDOW_BG", a); }

	get FILL_BUTTON_BG(): bool { return krom_zui_theme_get(this.theme_, "FILL_BUTTON_BG") > 0; }
	set FILL_BUTTON_BG(a: bool) { krom_zui_theme_set(this.theme_, "FILL_BUTTON_BG", a); }

	get FILL_ACCENT_BG(): bool { return krom_zui_theme_get(this.theme_, "FILL_ACCENT_BG") > 0; }
	set FILL_ACCENT_BG(a: bool) { krom_zui_theme_set(this.theme_, "FILL_ACCENT_BG", a); }

	get LINK_STYLE(): i32 { return krom_zui_theme_get(this.theme_, "LINK_STYLE"); }
	set LINK_STYLE(a: i32) { krom_zui_theme_set(this.theme_, "LINK_STYLE", a); }

	get FULL_TABS(): bool { return krom_zui_theme_get(this.theme_, "FULL_TABS") > 0; }
	set FULL_TABS(a: bool) { krom_zui_theme_set(this.theme_, "FULL_TABS", a); }

	get ROUND_CORNERS(): bool { return krom_zui_theme_get(this.theme_, "ROUND_CORNERS") > 0; }
	set ROUND_CORNERS(a: bool) { krom_zui_theme_set(this.theme_, "ROUND_CORNERS", a); }
}

function zui_theme_create(): theme_t {
	let raw = new theme_t();
	raw.theme_ = krom_zui_theme_init();
	return raw;
}

class zui_nodes_t {
	nodes_: any;
	color_picker_callback: (col: color_t)=>void = null;

	get nodes_selected_id(): i32[] { return krom_zui_nodes_get(this.nodes_, "nodes_selected_id"); }
	set nodes_selected_id(a: i32[]) { krom_zui_nodes_set(this.nodes_, "nodes_selected_id", a); }

	set _input_started(a: bool) { krom_zui_nodes_set(this.nodes_, "_input_started", a); }

	set nodes_drag(a: bool) { krom_zui_nodes_set(this.nodes_, "nodes_drag", a); }

	get pan_x(): f32 { return krom_zui_nodes_get(this.nodes_, "pan_x"); }
	set pan_x(a: f32) { krom_zui_nodes_set(this.nodes_, "pan_x", a); }

	get pan_y(): f32 { return krom_zui_nodes_get(this.nodes_, "pan_y"); }
	set pan_y(a: f32) { krom_zui_nodes_set(this.nodes_, "pan_y", a); }

	set zoom(a: f32) { krom_zui_nodes_set(this.nodes_, "zoom", a); }

	get link_drag_id(): i32 { return krom_zui_nodes_get(this.nodes_, "link_drag_id"); }
	set link_drag_id(a: i32) { krom_zui_nodes_set(this.nodes_, "link_drag_id", a); }
}

let zui_nodes_current: zui_nodes_t;
let zui_current_canvas: zui_node_canvas_t;
let zui_tr: (id: string, vars?: map_t<string, string>)=>string;

let zui_clipboard: string = "";
let zui_element_h: i32 = 25;
let zui_exclude_remove: string[] = ["OUTPUT_MATERIAL_PBR", "GROUP_OUTPUT", "GROUP_INPUT", "brush_output_node"];
let zui_node_replace: zui_node_t[] = [];
let zui_nodes_eps: f32 = 0.00001;

function zui_set_on_link_drag(f: (link_drag_id: i32, is_new_link: bool)=>void) { krom_zui_nodes_set_on_link_drag(f); }
function zui_set_on_socket_released(f: (socket_id: i32)=>void) { krom_zui_nodes_set_on_socket_released(f); }
function zui_set_on_canvas_released(f: ()=>void) { krom_zui_nodes_set_on_canvas_released(f); }
function zui_set_on_canvas_control(f: ()=>zui_canvas_control_t) { krom_zui_nodes_set_on_canvas_control(f); }
function zui_socket_released(): bool { return krom_zui_nodes_get(null, "socket_released"); }

let zui_enum_texts_js: (node_type: string)=>string[] = null;
function zui_set_enum_texts(f: (node_type: string)=>string[]) { zui_enum_texts_js = f; krom_zui_nodes_set_enum_texts(f); }

function zui_nodes_create(): zui_nodes_t {
	let raw = new zui_nodes_t();
	raw.nodes_ = krom_zui_nodes_init();
	krom_zui_nodes_set_on_custom_button(zui_nodes_on_custom_button);
	return raw;
}

function zui_get_node(nodes: zui_node_t[], id: i32): zui_node_t {
	for (let node of nodes) {
		if (node.id == id) {
			return node;
		}
	}
	return null;
}

function zui_get_node_id(nodes: zui_node_t[]): i32 {
	let id = 0;
	for (let n of nodes) {
		if (n.id >= id) {
			id = n.id + 1;
		}
	}
	return id;
}

function zui_get_link(links: zui_node_link_t[], id: i32): zui_node_link_t {
	for (let link of links) {
		if (link.id == id) {
			return link;
		}
	}
	return null;
}

function zui_get_link_id(links: zui_node_link_t[]): i32 {
	let id = 0;
	for (let l of links) {
		if (l.id >= id) {
			id = l.id + 1;
		}
	}
	return id;
}

function zui_get_socket(nodes: zui_node_t[], id: i32): zui_node_socket_t {
	for (let n of nodes) {
		for (let s of n.inputs) {
			if (s.id == id) {
				return s;
			}
		}
		for (let s of n.outputs) {
			if (s.id == id) {
				return s;
			}
		}
	}
	return null;
}

function zui_get_socket_id(nodes: zui_node_t[]): i32 {
	let id = 0;
	for (let n of nodes) {
		for (let s of n.inputs) {
			if (s.id >= id) {
				id = s.id + 1;
			}
		}
		for (let s of n.outputs) {
			if (s.id >= id) {
				id = s.id + 1;
			}
		}
	}
	return id;
}

function zui_node_canvas(raw: zui_nodes_t, ui: zui_t, canvas: zui_node_canvas_t) {
	zui_nodes_current = raw;
	zui_current_canvas = canvas;

	// Fill in optional values
	zui_nodes_update_canvas_format(canvas);

	// Ensure properties order
	let canvas_: zui_node_canvas_t = {
		name: canvas.name,
		nodes: array_slice(canvas.nodes, 0, canvas.nodes.length),
		nodes_count: canvas.nodes.length,
		links: array_slice(canvas.links, 0, canvas.links.length),
		links_count: canvas.links.length,
	}

	// Convert default data
	for (let n of canvas_.nodes) {
		for (let soc of n.inputs) {
			soc.default_value = zui_nodes_js_to_c(soc.type, soc.default_value);
		}
		for (let soc of n.outputs) {
			soc.default_value = zui_nodes_js_to_c(soc.type, soc.default_value);
		}
		for (let but of n.buttons) {
			but.default_value = zui_nodes_js_to_c(but.type, but.default_value);
			but.data = zui_nodes_js_to_c_data(but.type, but.data);
		}
	}

	// Ensure properties order
	for (let n of canvas_.nodes) {
		n.name = zui_tr(n.name);
		for (let i: i32 = 0; i < n.inputs.length; ++i) {
			n.inputs[i] = {
				id: n.inputs[i].id,
				node_id: n.inputs[i].node_id,
				name: zui_tr(n.inputs[i].name),
				type: n.inputs[i].type,
				color: n.inputs[i].color,
				default_value: n.inputs[i].default_value,
				min: n.inputs[i].min,
				max: n.inputs[i].max,
				precision: n.inputs[i].precision,
				display: n.inputs[i].display,
			};
		}
		for (let i: i32 = 0; i < n.outputs.length; ++i) {
			n.outputs[i] = {
				id: n.outputs[i].id,
				node_id: n.outputs[i].node_id,
				name: zui_tr(n.outputs[i].name),
				type: n.outputs[i].type,
				color: n.outputs[i].color,
				default_value: n.outputs[i].default_value,
				min: n.outputs[i].min,
				max: n.outputs[i].max,
				precision: n.outputs[i].precision,
				display: n.outputs[i].display,
			};
		}
		for (let i: i32 = 0; i < n.buttons.length; ++i) {
			n.buttons[i] = {
				name: zui_tr(n.buttons[i].name),
				type: n.buttons[i].type,
				output: n.buttons[i].output,
				default_value: n.buttons[i].default_value,
				data: n.buttons[i].data,
				min: n.buttons[i].min,
				max: n.buttons[i].max,
				precision: n.buttons[i].precision,
				height: n.buttons[i].height,
			};
		}
	}

	// Reserve capacity
	while (canvas_.nodes.length < 128) {
		array_push(canvas_.nodes, { id: -1, name: "", type: "", x: 0, y: 0, color: 0, inputs: [], outputs: [], buttons: [], width: 0 });
	}
	while (canvas_.links.length < 256) {
		array_push(canvas_.links, { id: -1, from_id: 0, from_socket: 0, to_id: 0, to_socket: 0 });
	}

	let packed: buffer_t = krom_zui_node_canvas(raw.nodes_, armpack_encode(canvas_));
	canvas_ = armpack_decode(packed);
	if (canvas_.nodes == null) {
		canvas_.nodes = [];
	}
	if (canvas_.links == null) {
		canvas_.links = [];
	}

	// Convert default data
	for (let n of canvas_.nodes) {
		for (let soc of n.inputs) {
			soc.default_value = zui_nodes_c_to_js(soc.type, soc.default_value);
		}
		for (let soc of n.outputs) {
			soc.default_value = zui_nodes_c_to_js(soc.type, soc.default_value);
		}
		for (let but of n.buttons) {
			but.default_value = zui_nodes_c_to_js(but.type, but.default_value);
			but.data = zui_nodes_c_to_js_data(but.type, but.data);
		}
	}

	canvas.name = canvas_.name;
	canvas.nodes = canvas_.nodes;
	canvas.links = canvas_.links;

	// Restore nodes modified in js while zui_node_canvas was running
	for (let n of zui_node_replace) {
		for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
			if (canvas.nodes[i].id == n.id) {
				canvas.nodes[i] = n;
				break;
			}
		}
	}
	zui_node_replace = [];

	zui_element_h = ui.t.ELEMENT_H + 2;
}

function zui_nodes_rgba_popup(ui: zui_t, nhandle: zui_handle_t, val: f32_array_t, x: i32, y: i32) {
	krom_zui_nodes_rgba_popup(nhandle.handle_, val.buffer, x, y);
}

function zui_remove_node(n: zui_node_t, canvas: zui_node_canvas_t) {
	if (n == null) {
		return;
	}
	let i: i32 = 0;
	while (i < canvas.links.length) {
		let l: zui_node_link_t = canvas.links[i];
		if (l.from_id == n.id || l.to_id == n.id) {
			array_splice(canvas.links, i, 1);
		}
		else {
			i++;
		}
	}
	array_remove(canvas.nodes, n);
}

function zui_nodes_SCALE(): f32 {
	return krom_zui_nodes_scale();
}

function zui_nodes_PAN_X(): f32 {
	return krom_zui_nodes_pan_x();
}

function zui_nodes_PAN_Y(): f32 {
	return krom_zui_nodes_pan_y();
}

function zui_nodes_NODE_H(canvas: zui_node_canvas_t, node: zui_node_t): i32 {
	return math_floor(zui_nodes_LINE_H() * 1.2 + zui_nodes_INPUTS_H(canvas, node.inputs) + zui_nodes_OUTPUTS_H(node.outputs) + zui_nodes_BUTTONS_H(node));
}

function zui_nodes_NODE_W(node: zui_node_t): i32 {
	return math_floor((node.width != 0 ? node.width : 140) * zui_nodes_SCALE());
}

function zui_nodes_NODE_X(node: zui_node_t): f32 {
	return node.x * zui_nodes_SCALE() + zui_nodes_PAN_X();
}

function zui_nodes_NODE_Y(node: zui_node_t): f32 {
	return node.y * zui_nodes_SCALE() + zui_nodes_PAN_Y();
}

function zui_nodes_BUTTONS_H(node: zui_node_t): i32 {
	let h: f32 = 0.0;
	for (let but of node.buttons) {
		if (but.type == "RGBA") {
			h += 102 * zui_nodes_SCALE() + zui_nodes_LINE_H() * 5; // Color wheel + controls
		}
		else if (but.type == "VECTOR") {
			h += zui_nodes_LINE_H() * 4;
		}
		else if (but.type == "CUSTOM") {
			h += zui_nodes_LINE_H() * but.height;
		}
		else {
			h += zui_nodes_LINE_H();
		}
	}
	return math_floor(h);
}

function zui_nodes_OUTPUTS_H(sockets: zui_node_socket_t[], length: i32 = -1): i32 {
	let h: f32 = 0.0;
	for (let i = 0; i < (length == -1 ? sockets.length : length); ++i) {
		h += zui_nodes_LINE_H();
	}
	return math_floor(h);
}

function zui_nodes_input_linked(canvas: zui_node_canvas_t, node_id: i32, i: i32): bool {
	for (let l of canvas.links) {
		if (l.to_id == node_id && l.to_socket == i) {
			return true;
		}
	}
	return false;
}

function zui_nodes_INPUTS_H(canvas: zui_node_canvas_t, sockets: zui_node_socket_t[], length: i32 = -1): i32 {
	let h: f32 = 0.0;
	for (let i = 0; i < (length == -1 ? sockets.length : length); ++i) {
		if (sockets[i].type == "VECTOR" && sockets[i].display == 1 && !zui_nodes_input_linked(canvas, sockets[i].node_id, i)) {
			h += zui_nodes_LINE_H() * 4;
		}
		else {
			h += zui_nodes_LINE_H();
		}
	}
	return math_floor(h);
}

function zui_nodes_INPUT_Y(canvas: zui_node_canvas_t, sockets: zui_node_socket_t[], pos: i32): i32 {
	return math_floor(zui_nodes_LINE_H() * 1.62) + zui_nodes_INPUTS_H(canvas, sockets, pos);
}

function zui_nodes_OUTPUT_Y(sockets: zui_node_socket_t[], pos: i32): i32 {
	return math_floor(zui_nodes_LINE_H() * 1.62) + zui_nodes_OUTPUTS_H(sockets, pos);
}

function zui_nodes_LINE_H(): i32 {
	return math_floor(zui_element_h * zui_nodes_SCALE());
}

function zui_nodes_p(f: f32): f32 {
	return f * zui_nodes_SCALE();
}

function zui_nodes_on_custom_button(node_id: i32, button_name: string) {
	eval(button_name + "(Zui.current, current, current.getNode(currentCanvas.nodes, node_id))");
}

function zui_nodes_js_to_c(type: string, d: any): u8_array_t {
	if (type == "RGBA") {
		if (d == null) return u8_array_create(16);
		else {
			let f32a: f32_array_t = f32_array_create(4);
			f32a[0] = d[0];
			f32a[1] = d[1];
			f32a[2] = d[2];
			f32a[3] = d[3];
			d = new u8_array_t(f32a.buffer);
		}
		return new u8_array_t(d.buffer);
	}
	if (type == "VECTOR") {
		if (d == null) {
			return u8_array_create(12);
		}
		else {
			let f32a: f32_array_t = f32_array_create(4);
			f32a[0] = d[0];
			f32a[1] = d[1];
			f32a[2] = d[2];
			d = new u8_array_t(f32a.buffer);
		}
		return new u8_array_t(d.buffer);
	}
	if (type == "VALUE") {
		if (d == null) {
			return u8_array_create(4);
		}
		let f32a: f32_array_t = new f32_array_t([d]);
		return new u8_array_t(f32a.buffer);
	}
	if (type == "STRING") {
		if (d == null) {
			return u8_array_create(1);
		}
		let s: string = d;
		let u8a: u8_array_t = u8_array_create(s.length + 1);
		for (let i = 0; i < s.length; ++i) {
			u8a[i] = char_code_at(s, i);
		}
		return u8a;
	}
	if (type == "ENUM") {
		if (d == null) {
			return u8_array_create(1);
		}
		let u32a: u32_array_t = new u32_array_t([d]);
		return new u8_array_t(u32a.buffer);
	}
	if (type == "BOOL") {
		if (d == null) {
			return u8_array_create(1);
		}
		let u8a: u8_array_t = u8_array_create(1);
		u8a[0] = d == true ? 1 : 0;
		return u8a;
	}
	if (type == "CUSTOM") {
		return u8_array_create(1);
	}
	return u8_array_create(1);
}

function zui_nodes_js_to_c_data(type: string, d: any): u8_array_t {
	if (type == "ENUM") {
		if (d == null) {
			return u8_array_create(1);
		}
		let a: string[] = d;
		let length: i32 = 0;
		for (let s of a) {
			length += s.length + 1;
		}
		if (length == 0) {
			return u8_array_create(1);
		}
		let u8a: u8_array_t = u8_array_create(length);
		let pos: i32 = 0;
		for (let s of a) {
			for (let i: i32 = 0; i < s.length; ++i) {
				u8a[pos++] = char_code_at(s, i);
			}
			u8a[pos++] = 0; // '\0'
		}
		return u8a;
	}
	if (type == "CUSTOM") {
		return u8_array_create(1);
	}
	return u8_array_create(1);
}

function zui_nodes_c_to_js(type: string, u8a: u8_array_t): any {
	if (type == "RGBA") {
		return new f32_array_t(u8a.buffer);
	}
	if (type == "VECTOR") {
		return new f32_array_t(u8a.buffer);
	}
	if (type == "VALUE") {
		let f32a: f32_array_t = new f32_array_t(u8a.buffer);
		return f32a[0];
	}
	if (type == "STRING") {
		let s = "";
		for (let i: i32 = 0; i < u8a.length - 1; ++i) {
			s += string_from_char_code(u8a[i]);
		}
		return s;
	}
	if (type == "ENUM") {
		let u32a: u32_array_t = new u32_array_t(u8a.buffer);
		return u32a[0];
	}
	if (type == "BOOL") {
		return u8a[0] > 0 ? true : false;
	}
	if (type == "CUSTOM") {
		return 0;
	}
	return null;
}

function zui_nodes_c_to_js_data(type: string, u8a: u8_array_t): any {
	if (type == "ENUM") {
		let a: string[] = [];
		let s: string = "";
		for (let i: i32 = 0; i < u8a.length; ++i) {
			if (u8a[i] == 0) {
				array_push(a, s);
				s = "";
			}
			else {
				s += string_from_char_code(u8a[i]);
			}
		}
		return a;
	}
	if (type == "CUSTOM") {
		return 0;
	}
	return null;
}

function zui_nodes_update_canvas_format(canvas: zui_node_canvas_t) {
	for (let n of canvas.nodes) {
		for (let soc of n.inputs) {
			if (soc.min == null) {
				soc.min = 0.0;
			}
			if (soc.max == null) {
				soc.max = 1.0;
			}
			if (soc.precision == null) {
				soc.precision = 100.0;
			}
			if (soc.display == null) {
				soc.display = 0;
			}
			if (soc.min - math_floor(soc.min) == 0.0) {
				soc.min += zui_nodes_eps;
			}
			if (soc.max - math_floor(soc.max) == 0.0) {
				soc.max += zui_nodes_eps;
			}
			if (soc.precision - math_floor(soc.precision) == 0.0) {
				soc.precision += zui_nodes_eps;
			}
		}
		for (let soc of n.outputs) {
			if (soc.min == null) {
				soc.min = 0.0;
			}
			if (soc.max == null) {
				soc.max = 1.0;
			}
			if (soc.precision == null) {
				soc.precision = 100.0;
			}
			if (soc.display == null) {
				soc.display = 0;
			}
			if (soc.min - math_floor(soc.min) == 0.0) {
				soc.min += zui_nodes_eps;
			}
			if (soc.max - math_floor(soc.max) == 0.0) {
				soc.max += zui_nodes_eps;
			}
			if (soc.precision - math_floor(soc.precision) == 0.0) {
				soc.precision += zui_nodes_eps;
			}
		}
		for (let but of n.buttons) {
			if (but.output == null) {
				but.output = -1;
			}
			if (but.min == null) {
				but.min = 0.0;
			}
			if (but.max == null) {
				but.max = 1.0;
			}
			if (but.precision == null) {
				but.precision = 100.0;
			}
			if (but.height == null) {
				but.height = 0.0;
			}
			if (but.height - math_floor(but.height) == 0.0) {
				but.height += zui_nodes_eps;
			}
			if (but.min - math_floor(but.min) == 0.0) {
				but.min += zui_nodes_eps;
			}
			if (but.max - math_floor(but.max) == 0.0) {
				but.max += zui_nodes_eps;
			}
			if (but.precision - math_floor(but.precision) == 0.0) {
				but.precision += zui_nodes_eps;
			}
		}
		if (n.width == null) {
			n.width = 0;
		}
	}
}

type zui_ops_t = {
	font?: g2_font_t;
	theme?: theme_t;
	scale_factor?: f32;
	color_wheel?: image_t;
	black_white_gradient?: image_t;
};

type zui_handle_ops_t = {
	selected?: boolean;
	position?: i32;
	value?: f32;
	text?: string;
	color?: color_t;
	layout?: zui_layout_t;
};

type zui_coloring_t = {
	color?: i32;
	start?: string[];
	end?: string;
	separated?: boolean;
};

type zui_text_coloring_t = {
	colorings?: zui_coloring_t[];
	default_color?: i32;
};

type zui_canvas_control_t = {
	pan_x?: f32;
	pan_y?: f32;
	zoom?: f32;
	controls_down?: bool;
};

type zui_node_canvas_t = {
	name?: string;
	nodes?: zui_node_t[];
	nodes_count?: i32;
	links?: zui_node_link_t[];
	links_count?: i32;
};

type zui_node_t = {
	id?: i32;
	name?: string;
	type?: string;
	x?: f32;
	y?: f32;
	color?: i32;
	inputs?: zui_node_socket_t[];
	outputs?: zui_node_socket_t[];
	buttons?: zui_node_button_t[];
	width?: f32;
};

type zui_node_socket_t = {
	id?: i32;
	node_id?: i32;
	name?: string;
	type?: string;
	color?: i32;
	default_value?: any;
	min?: f32;
	max?: f32;
	precision?: f32;
	display?: i32;
};

type zui_node_link_t = {
	id?: i32;
	from_id?: i32;
	from_socket?: i32;
	to_id?: i32;
	to_socket?: i32;
};

type zui_node_button_t = {
	name?: string;
	type?: string;
	output?: i32;
	default_value?: any;
	data?: any;
	min?: f32;
	max?: f32;
	precision?: f32;
	height?: f32;
};

enum zui_layout_t {
	VERTICAL,
	HORIZONTAL,
}

enum zui_align_t {
	LEFT,
	CENTER,
	RIGHT,
}

enum zui_state_t {
	IDLE,
	STARTED,
	DOWN,
	RELEASED,
	HOVERED,
}
