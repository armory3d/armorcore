
// Externs to Zui in C

///if arm_minits

let zui_current: zui_t = null;
let zui_children: map_t<string, zui_handle_t> = map_create();

// function zui_handle(s: string, ops: zui_handle_ops_t = null): zui_handle_t {
function zui_handle(s: string): zui_handle_t {
	let h: zui_handle_t = map_get(zui_children, s);
	if (h == null) {
		// h = zui_handle_create(ops);
		h = zui_handle_create();
		map_set(zui_children, s, h);
	}
	return h;
}

function zui_handle_create(): zui_handle_t {
	let raw: zui_handle_t = {};
	return raw;
}

function zui_create(ops: zui_options_t): zui_t {
    let raw: zui_t = {};
    zui_init(raw, * ops);
    return raw;
}

///else

declare type zui_t = any;
declare type zui_theme_t = any;
declare type zui_options_t = any;
declare type zui_handle_t = any;
declare type zui_align_t = any;
declare function zui_init(ui: zui_t, ops: zui_options_t): void;
declare function zui_theme_default(t: zui_theme_t): void;
declare function zui_begin(ui: zui_t): void;
declare function zui_end(last: bool): void;
declare function zui_window(handle: zui_handle_t, x: i32, y: i32, w: i32, h: i32, drag: bool): bool;
declare function zui_button(text: string, align: zui_align_t, label: string): bool;

///end
