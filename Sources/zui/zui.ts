// @ts-nocheck
// Externs for Zui in C

let zui_children: map_t<string, zui_handle_t> = map_create();
// let zui_tr: (id: string, vars: map_t<string, string>)=>string;
let zui_clipboard: string = "";

function zui_SCALE(ui: zui_t): f32 {
	let current: zui_t = zui_get_current();
	zui_set_current(ui);
	let f: f32 = ZUI_SCALE();
	zui_set_current(current);
	return f;
}

function zui_ELEMENT_OFFSET(ui: zui_t): f32 {
	let current: zui_t = zui_get_current();
	zui_set_current(ui);
	let f: f32 = ZUI_ELEMENT_OFFSET();
	zui_set_current(current);
	return f;
}

function zui_ELEMENT_W(ui: zui_t): f32 {
	let current: zui_t = zui_get_current();
	zui_set_current(ui);
	let f: f32 = ZUI_ELEMENT_W();
	zui_set_current(current);
	return f;
}

function zui_ELEMENT_H(ui: zui_t): f32 {
	let current: zui_t = zui_get_current();
	zui_set_current(ui);
	let f: f32 = ZUI_ELEMENT_H();
	zui_set_current(current);
	return f;
}

function zui_MENUBAR_H(ui: zui_t): f32 {
	let button_offset_y: f32 = (ui.ops.theme.ELEMENT_H * zui_SCALE(ui) - ui.ops.theme.BUTTON_H * zui_SCALE(ui)) / 2;
	return ui.ops.theme.BUTTON_H * zui_SCALE(ui) * 1.1 + 2 + button_offset_y;
}

function zui_nodes_LINE_H(): f32 {
	return ZUI_LINE_H();
}

function zui_nodes_SCALE(): f32 {
	return ZUI_NODES_SCALE();
}

function zui_nodes_p(f: f32): f32 {
	return zui_p(f);
}

function zui_nodes_NODE_X(node: zui_node_t): f32 {
	return ZUI_NODE_X(node);
}

function zui_nodes_NODE_Y(node: zui_node_t): f32 {
	return ZUI_NODE_Y(node);
}

function zui_nodes_NODE_W(node: zui_node_t): f32 {
	return ZUI_NODE_W(node);
}

function zui_nodes_NODE_H(canvas: zui_node_canvas_t, node: zui_node_t): f32 {
	return ZUI_NODE_H(canvas, node);
}

function zui_nodes_OUTPUT_Y(sockets_count: i32, pos: i32): f32 {
	return ZUI_OUTPUT_Y(sockets_count, pos);
}

function zui_nodes_INPUT_Y(canvas: zui_node_canvas_t, sockets: zui_node_socket_t[], pos: i32): f32 {
	return ZUI_INPUT_Y(canvas, sockets.buffer, sockets.length, pos);
}

function zui_nodes_OUTPUTS_H(sockets_count: i32, length: i32 = -1): f32 {
	return ZUI_OUTPUTS_H(sockets_count, length);
}

function zui_nodes_BUTTONS_H(node: zui_node_t): f32 {
	return ZUI_BUTTONS_H(node);
}

function zui_nodes_PAN_X(): f32 {
	return ZUI_NODES_PAN_X();
}

function zui_nodes_PAN_Y(): f32 {
	return ZUI_NODES_PAN_Y();
}

function _zui_image(image: image_t, tint: i32 = 0xffffffff, h: f32 = -1.0, sx: i32 = 0, sy: i32 = 0, sw: i32 = 0, sh: i32 = 0): zui_state_t {
	if (image.texture_ != null) {
		return zui_sub_image(image.texture_, false, tint, h, sx, sy, sw, sh);
	}
	else {
		return zui_sub_image(image.render_target_, true, tint, h, sx, sy, sw, sh);
	}
}

function _zui_tooltip_image(image: image_t, max_width: i32 = 0) {
	if (image.texture_ != null) {
		return zui_tooltip_image(image.texture_, max_width);
	}
	else {
		return zui_tooltip_render_target(image.render_target_, max_width);
	}
}

function _zui_set_scale(ui: zui_t, factor: f32) {
	let current: zui_t = zui_get_current();
	zui_set_current(ui);
	zui_set_scale(factor);
	zui_set_current(current);
}

function _zui_end_element(element_size: f32 = -1.0) {
	if (element_size < 0) {
		zui_end_element();
	}
	else {
		zui_end_element_of_size(element_size);
	}
}

function zui_handle(s: string): zui_handle_t {
	let h: zui_handle_t = map_get(zui_children, s);
	if (h == null) {
		h = zui_handle_create();
		map_set(zui_children, s, h);
		return h;
	}
	h.init = false;
	return h;
}

function zui_create(ops: zui_options_t): zui_t {
    let raw: zui_t = {};
    zui_init(raw, ops);
    return raw;
}

function zui_theme_create(): zui_theme_t {
	let raw: zui_theme_t = {};
	zui_theme_default(raw);
	return raw;
}

function nodes_on_custom_button(node_id: i32, button_name: string) {
	// eval(button_name + "(Zui.current, current, current.getNode(currentCanvas.nodes, node_id))");
}

function zui_nodes_create(): zui_nodes_t {
	let raw: zui_nodes_t = {};
	zui_nodes_init(raw);
	// zui_nodes_exclude_remove[0] = "OUTPUT_MATERIAL_PBR";
	// zui_nodes_exclude_remove[1] = "GROUP_OUTPUT";
	// zui_nodes_exclude_remove[2] = "GROUP_INPUT";
	// zui_nodes_exclude_remove[3] = "BrushOutputNode";
	zui_nodes_on_custom_button = nodes_on_custom_button;
	return raw;
}

function zui_get_socket(nodes: zui_node_t[], id: i32): zui_node_socket_t {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: zui_node_t = nodes[i];
		for (let j: i32 = 0; j < n.inputs.length; ++j) {
			let s: zui_node_socket_t = n.inputs[j];
			if (s.id == id) {
				return s;
			}
		}
		for (let j: i32 = 0; j < n.outputs.length; ++j) {
			let s: zui_node_socket_t = n.outputs[j];
			if (s.id == id) {
				return s;
			}
		}
	}
	return null;
}

function zui_set_font(raw: zui_t, font: g2_font_t) {
	g2_font_init(font); // Make sure font_ is ready
	raw.ops.font = font;
}

declare let zui_nodes_enum_texts: (s: string)=>string[];
declare let zui_touch_scroll: bool;
declare let zui_touch_hold : bool;
declare let zui_touch_tooltip: bool;
declare let zui_always_redraw_window: bool;
declare let zui_on_border_hover: any;
declare let zui_on_text_hover: any;
declare let zui_on_deselect_text: any;
declare let zui_on_tab_drop: any;
declare let zui_nodes_on_link_drag: any;
declare let zui_nodes_on_socket_released: any;
declare let zui_nodes_on_canvas_released: any;
declare let zui_nodes_on_canvas_control: any;
declare let zui_text_area_line_numbers: bool;
declare let zui_text_area_scroll_past_end: bool;
declare let zui_text_area_coloring: any;
declare let zui_nodes_socket_released: bool;
declare let zui_is_cut: bool;
declare let zui_is_copy: bool;
declare let zui_is_paste: bool;
declare let zui_nodes_exclude_remove: string[];

declare function zui_nest(handle: zui_handle_t, pos: i32): zui_handle_t;
declare function zui_theme_default(theme: zui_theme_t): void;
declare function zui_tab(handle: zui_handle_t, text: string, vertical: bool = false, color: i32 = -1): bool;
declare function zui_combo(handle: zui_handle_t, texts: string[], label: string = "", show_label: bool = false, align: zui_align_t = zui_align_t.LEFT, search_bar: bool = true): i32;
declare function zui_slider(handle: zui_handle_t, text: string, from: f32 = 0.0, to: f32 = 1.0, filled: bool = false, precision: f32 = 100.0, display_value: bool = true, align: zui_align_t = zui_align_t.RIGHT, text_edit: bool = true): f32;
declare function zui_button(text: string, align: zui_align_t = zui_align_t.CENTER, label: string = ""): bool;
declare function zui_text(text: string, align: zui_align_t = zui_align_t.LEFT, bg: i32 = 0x00000000): zui_state_t;
declare function zui_text_input(handle: zui_handle_t, label: string = "", align: zui_align_t = zui_align_t.LEFT, editable: bool = true, live_update: bool = false): string;
declare function zui_check(handle: zui_handle_t, text: string, label: string = ""): bool;
declare function zui_color_wheel(handle: zui_handle_t, alpha: bool = false, w: f32 = -1.0, h: f32 = -1.0, color_preview: bool = true, picker: ()=>void = null, data: any = null): color_t;
declare function zui_hovered_tab_name(): string;
declare function zui_radio(handle: zui_handle_t, position: i32, text: string, label: string = ""): bool;
declare function zui_start_text_edit(handle: zui_handle_t, align: zui_align_t = zui_align_t.LEFT): void;
declare function zui_tooltip(s: string): void;
declare function zui_tooltip_image(tex: any, max_width: i32 = 0): void;
declare function zui_tooltip_render_target(rt: any, max_width: i32 = 0): void;
declare function zui_separator(h: i32 = 4, fill: bool = true): void;
declare function zui_text_area(handle: zui_handle_t, align: zui_align_t = zui_align_t.LEFT, editable: bool = true, label: string = "", word_wrap: bool = false): string;
declare function zui_window(handle: zui_handle_t, x: i32, y: i32, w: i32, h: i32, drag: bool = false): bool;
declare function zui_begin(ui: zui_t): void;
declare function zui_end(last: bool = true): void;
declare function zui_end_window(bind_global_g: bool = true): void;
declare function zui_end_region(last: bool = true): void;
declare function zui_float_input(handle: zui_handle_t, label: string = "", align: zui_align_t = zui_align_t.LEFT, precision: f32 = 1000.0): f32;
declare function zui_get_current(): zui_t;
declare function zui_remove_node(n: zui_node_t, canvas: zui_node_canvas_t): void;
declare function zui_next_link_id(links: zui_node_link_t[]): i32;
declare function zui_set_hovered_tab_name(s: string): void;
declare function zui_begin_sticky(): void;
declare function zui_end_sticky(): void;
declare function zui_row(r: f32[]): void;
declare function zui_handle_create(): zui_handle_t;
declare function zui_fill(x: i32, y: i32, w: i32, h: i32, color: i32): void;
declare function zui_rect(x: i32, y: i32, w: i32, h: i32, color: i32, strength: f32): void;
declare function zui_draw_rect(filled: bool, x: i32, y: i32, w: i32, h: i32): void;
declare function zui_begin_menu(): void;
declare function zui_end_menu(): void;
declare function zui_menu_button(s: string): bool;
declare function zui_begin_region(ui: zui_t, x: i32, y: i32, w: i32): void;
declare function zui_end_region(last: bool): void;
declare function zui_inline_radio(handle: zui_handle_t, texts: string[], align: i32): int;
declare function zui_end_input(): void;
declare function zui_panel(handle: zui_handle_t, text: string, is_tree: bool, filled: bool, pack: bool): bool;
declare function zui_nodes_rgba_popup(nhandle: zui_handle_t, val: f32[], x: i32, y: i32): void;
declare function zui_get_link(links: zui_node_link_t[], id: i32): zui_node_link_t;
declare function zui_get_node(nodes: zui_node_t[], id: i32): zui_node_t;
declare function zui_input_in_rect(x: f32, y: f32, w: f32, h: f32): bool;
declare function zui_get_socket_id(nodes: zui_node_t[]): i32;
declare function zui_draw_string(text: string, x_offset: f32, y_offset: f32, align: i32, truncation: bool): void;
declare function zui_next_node_id(nodes: zui_node_t[]): i32;
declare function zui_node_canvas(nodes: zui_nodes_t, canvas: zui_node_canvas_t): void;

declare type kinc_g4_texture_t = any;
declare type kinc_g4_render_target_t = any;
declare type zui_theme_t = any;

declare type zui_t = {
	ops: zui_options_t;
	is_hovered: bool;
	is_typing: bool;
	is_escape_down: bool;
	is_delete_down: bool;
	is_return_down: bool;
	is_ctrl_down: bool;
	is_released: bool;
	is_key_pressed: bool;
	is_scrolling: bool;
	key_code: i32;
	input_started: bool;
	input_started_r: bool;
	input_released: bool;
	input_released_r: bool;
	input_x: i32;
	input_y: i32;
	input_started_x: i32;
	input_started_y: i32;
	input_enabled: bool;
	input_down_r: bool;
	input_dx: i32;
	input_dy: i32;
	input_wheel_delta: i32;
	_x: i32;
	_y: i32;
	_w: i32;
	_window_w: i32;
	_window_h: i32;
	_window_x: i32;
	_window_y: i32;
	scroll_enabled: bool;
	input_down: bool;
	font_size: i32;
	image_scroll_align: bool;
	changed: bool;
	font_offset_y: f32;
	enabled: bool;
	scissor: bool;
	text_selected_handle: zui_handle_t;
	submit_text_handle: zui_handle_t;
	combo_selected_handle: zui_handle_t;
	current_ratio: i32;
	image_invert_y: bool;
	elements_baked: bool;
	window_border_right: i32;
	window_border_top: i32;
	window_border_bottom: i32;
};

declare type zui_handle_t = {
	selected: bool;
	position: i32;
	color: u32;
	value: f32;
	text: string;
	// kinc_g4_render_target_t texture;
	redraws: i32;
	scroll_offset: f32;
	scroll_enabled: bool;
	layout: i32;
	last_max_x: f32;
	last_max_y: f32;
	drag_enabled: bool;
	drag_x: i32;
	drag_y: i32;
	changed: bool;
	init: bool;
	children: zui_handle_t[];
};

declare type zui_options_t = {
	font?: g2_font_t;
	theme?: zui_theme_t;
	scale_factor?: f32;
	color_wheel?: kinc_g4_texture_t;
	black_white_gradient?: kinc_g4_texture_t;
};

declare type zui_coloring_t = {
	color?: i32;
	start?: string[];
	end?: string;
	separated?: bool;
};

declare type zui_text_coloring_t = {
	colorings?: zui_coloring_t[];
	default_color?: i32;
};

declare type zui_canvas_control_t = {
	pan_x?: f32;
	pan_y?: f32;
	zoom?: f32;
	controls_down?: bool;
};

declare type zui_node_canvas_t = {
	name?: string;
	nodes?: zui_node_t[];
	links?: zui_node_link_t[];
};

declare type zui_node_t = {
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

declare type zui_node_socket_t = {
	id?: i32;
	node_id?: i32;
	name?: string;
	type?: string;
	color?: i32;
	default_value?: f32_array_t;
	min?: f32;
	max?: f32;
	precision?: f32;
	display?: i32;
};

declare type zui_node_link_t = {
	id?: i32;
	from_id?: i32;
	from_socket?: i32;
	to_id?: i32;
	to_socket?: i32;
};

declare type zui_node_button_t = {
	name?: string;
	type?: string;
	output?: i32;
	default_value?: f32_array_t;
	data?: u8_array_t;
	min?: f32;
	max?: f32;
	precision?: f32;
	height?: f32;
};

declare type zui_nodes_t = {
	color_picker_callback?: (col: color_t)=>void;
	nodes_selected_id?: i32[];
	_input_started?: bool;
	nodes_drag?: bool;
	pan_x?: f32;
	pan_y?: f32;
	zoom?: f32;
	link_drag_id?: i32;
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

let zui_theme_keys: string[] = [
	"WINDOW_BG_COL",
	"WINDOW_TINT_COL",
	"ACCENT_COL",
	"ACCENT_HOVER_COL",
	"ACCENT_SELECT_COL",
	"BUTTON_COL",
	"BUTTON_TEXT_COL",
	"BUTTON_HOVER_COL",
	"BUTTON_PRESSED_COL",
	"TEXT_COL",
	"LABEL_COL",
	"SEPARATOR_COL",
	"HIGHLIGHT_COL",
	"CONTEXT_COL",
	"PANEL_BG_COL",
	"FONT_SIZE",
	"ELEMENT_W",
	"ELEMENT_H",
	"ELEMENT_OFFSET",
	"ARROW_SIZE",
	"BUTTON_H",
	"CHECK_SIZE",
	"CHECK_SELECT_SIZE",
	"SCROLL_W",
	"SCROLL_MINI_W",
	"TEXT_OFFSET",
	"TAB_W",
	"FILL_WINDOW_BG",
	"FILL_BUTTON_BG",
	"FILL_ACCENT_BG",
	"LINK_STYLE",
	"FULL_TABS",
	"ROUND_CORNERS",
];
