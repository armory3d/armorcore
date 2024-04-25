
// Externs for Zui in C

///if arm_minits

let zui_current: zui_t = null;
let zui_children: map_t<string, zui_handle_t> = map_create();

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
    zui_init(raw, * ops);
    return raw;
}

type zui_options_t = {
	font?: g2_font_t;
	theme?: zui_theme_t;
	scale_factor?: f32;
	color_wheel?: image_t;
	black_white_gradient?: image_t;
};

type zui_coloring_t = {
	color?: i32;
	start?: string[];
	end?: string;
	separated?: bool;
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
	links?: zui_node_link_t[];
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
	default_value?: f32_array_t;
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
	default_value?: f32_array_t;
	data?: u8_array_t;
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

///end

///if _

declare type zui_t = any;
declare type zui_theme_t = any;
declare type zui_handle_t = any;
declare type zui_nodes_t = any;
declare function zui_handle_create(): zui_handle_t;
declare function zui_init(ui: zui_t, ops: zui_options_t): void;
declare function zui_theme_default(t: zui_theme_t): void;
declare function zui_begin(ui: zui_t): void;
declare function zui_end(last: bool): void;
declare function zui_window(handle: zui_handle_t, x: i32, y: i32, w: i32, h: i32, drag: bool): bool;
declare function zui_button(text: string, align: zui_align_t, label: string): bool;

///end
