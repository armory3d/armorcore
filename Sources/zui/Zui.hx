// Bindings to Zui in C

package zui;

import kha.input.KeyCode;
import kha.graphics2.Graphics;

typedef ZuiOptions = {
	font: kha.Font,
	theme: zui.Theme,
	scaleFactor: Float,
	color_wheel: kha.Image,
	black_white_gradient: kha.Image,
}

class Zui {
	public static var current: Zui = null;

	public static var onBorderHover: Handle->Int->Void = null;
	public static var onTextHover: Void->Void = null;
	public static var onDeselectText: Void->Void = null;
	public static var onTabDrop: Handle->Int->Handle->Int->Void = null;

	public static var alwaysRedrawWindow(get, never): Bool;
	static function get_alwaysRedrawWindow(): Bool { return Krom.zui_get(null, "zui_always_redraw_window"); }

	public static var touchScroll(get, set): Bool;
	static function get_touchScroll(): Bool { return Krom.zui_get(null, "zui_touch_scroll"); }
	static function set_touchScroll(a: Bool): Bool { Krom.zui_set(null, "zui_touch_scroll", a); return a; }

	public static var touchHold(get, set): Bool;
	static function get_touchHold(): Bool { return Krom.zui_get(null, "zui_touch_hold"); }
	static function set_touchHold(a: Bool): Bool { Krom.zui_set(null, "zui_touch_hold", a); return a; }

	public static var touchTooltip(get, set): Bool;
	static function get_touchTooltip(): Bool { return Krom.zui_get(null, "zui_touch_tooltip"); }
	static function set_touchTooltip(a: Bool): Bool { Krom.zui_set(null, "zui_touch_tooltip", a); return a; }

	public static var isCut(never, set): Bool;
	static function set_isCut(a: Bool): Bool { Krom.zui_set(null, "zui_is_cut", a); return a; }

	public static var isCopy(never, set): Bool;
	static function set_isCopy(a: Bool): Bool { Krom.zui_set(null, "zui_is_copy", a); return a; }

	public static var isPaste(get, set): Bool;
	static function get_isPaste(): Bool { return Krom.zui_get(null, "zui_is_paste"); }
	static function set_isPaste(a: Bool): Bool { Krom.zui_set(null, "zui_is_paste", a); return a; }

	public var isScrolling(get, null): Bool;
	function get_isScrolling(): Bool { return Krom.zui_get(zui_, "is_scrolling"); }

	public var isTyping(get, never): Bool;
	function get_isTyping(): Bool { return Krom.zui_get(zui_, "is_typing"); }

	public var enabled(get, set): Bool;
	function get_enabled(): Bool { return Krom.zui_get(zui_, "enabled"); }
	function set_enabled(a: Bool) { Krom.zui_set(zui_, "enabled", a); return a; }

	public var isHovered(get, set): Bool;
	function get_isHovered(): Bool { return Krom.zui_get(zui_, "is_hovered"); }
	function set_isHovered(a: Bool) { Krom.zui_set(zui_, "is_hovered", a); return a; }

	public var isReleased(get, never): Bool;
	function get_isReleased(): Bool { return Krom.zui_get(zui_, "is_released"); }

	public var changed(get, set): Bool;
	function get_changed(): Bool { return Krom.zui_get(zui_, "changed"); }
	function set_changed(a: Bool) { Krom.zui_set(zui_, "changed", a); return a; }

	public var imageInvertY(never, set): Bool;
	function set_imageInvertY(a: Bool) { Krom.zui_set(zui_, "image_invert_y", a); return a; }

	public var scrollEnabled(never, set): Bool;
	function set_scrollEnabled(a: Bool) { Krom.zui_set(zui_, "scroll_enabled", a); return a; }

	public var windowBorderTop(never, set): Int;
	function set_windowBorderTop(a: Int) { Krom.zui_set(zui_, "window_border_top", a); return a; }

	public var windowBorderBottom(never, set): Int;
	function set_windowBorderBottom(a: Int) { Krom.zui_set(zui_, "window_border_bottom", a); return a; }

	public var windowBorderRight(never, set): Int;
	function set_windowBorderRight(a: Int) { Krom.zui_set(zui_, "window_border_right", a); return a; }

	public var inputEnabled(get, set): Bool;
	function get_inputEnabled(): Bool { return Krom.zui_get(zui_, "input_enabled"); }
	function set_inputEnabled(a: Bool) { Krom.zui_set(zui_, "input_enabled", a); return a; }

	public var inputX(get, set): Float;
	function get_inputX(): Float { return Krom.zui_get(zui_, "input_x"); }
	function set_inputX(a: Float) { Krom.zui_set(zui_, "input_x", a); return a; }

	public var inputY(get, set): Float;
	function get_inputY(): Float { return Krom.zui_get(zui_, "input_y"); }
	function set_inputY(a: Float) { Krom.zui_set(zui_, "input_y", a); return a; }

	public var inputStartedX(get, never): Float;
	function get_inputStartedX(): Float { return Krom.zui_get(zui_, "input_started_x"); }

	public var inputStartedY(get, never): Float;
	function get_inputStartedY(): Float { return Krom.zui_get(zui_, "input_started_y"); }

	public var inputDX(get, never): Float;
	function get_inputDX(): Float { return Krom.zui_get(zui_, "input_dx"); }

	public var inputDY(get, never): Float;
	function get_inputDY(): Float { return Krom.zui_get(zui_, "input_dy"); }

	public var inputWheelDelta(get, never): Float;
	function get_inputWheelDelta(): Float { return Krom.zui_get(zui_, "input_wheel_delta"); }

	public var inputStarted(get, set): Bool;
	function get_inputStarted(): Bool { return Krom.zui_get(zui_, "input_started"); }
	function set_inputStarted(a: Bool) { Krom.zui_set(zui_, "input_started", a); return a; }

	public var inputStartedR(get, never): Bool;
	function get_inputStartedR(): Bool { return Krom.zui_get(zui_, "input_started_r"); }

	public var inputReleased(get, never): Bool;
	function get_inputReleased(): Bool { return Krom.zui_get(zui_, "input_released"); }

	public var inputReleasedR(get, never): Bool;
	function get_inputReleasedR(): Bool { return Krom.zui_get(zui_, "input_released_r"); }

	public var inputDown(get, never): Bool;
	function get_inputDown(): Bool { return Krom.zui_get(zui_, "input_down"); }

	public var inputDownR(get, never): Bool;
	function get_inputDownR(): Bool { return Krom.zui_get(zui_, "input_down_r"); }

	public var isKeyPressed(get, never): Bool;
	function get_isKeyPressed(): Bool { return Krom.zui_get(zui_, "is_key_pressed"); }

	public var isCtrlDown(get, never): Bool;
	function get_isCtrlDown(): Bool { return Krom.zui_get(zui_, "is_ctrl_down"); }

	public var isDeleteDown(get, set): Bool;
	function get_isDeleteDown(): Bool { return Krom.zui_get(zui_, "is_delete_down"); }
	function set_isDeleteDown(a: Bool) { Krom.zui_set(zui_, "is_delete_down", a); return a; }

	public var isEscapeDown(get, never): Bool;
	function get_isEscapeDown(): Bool { return Krom.zui_get(zui_, "is_escape_down"); }

	public var isReturnDown(get, never): Bool;
	function get_isReturnDown(): Bool { return Krom.zui_get(zui_, "is_return_down"); }

	public var key(get, never): KeyCode;
	function get_key(): KeyCode { return cast Krom.zui_get(zui_, "key_code"); }

	public var curRatio(get, set): Int;
	function get_curRatio(): Int { return Krom.zui_get(zui_, "current_ratio"); }
	function set_curRatio(a: Int) { Krom.zui_set(zui_, "current_ratio", a); return a; }

	public var fontSize(get, set): Int;
	function get_fontSize(): Int { return Krom.zui_get(zui_, "font_size"); }
	function set_fontSize(a: Int) { Krom.zui_set(zui_, "font_size", a); return a; }

	public var fontOffsetY(get, set): Float;
	function get_fontOffsetY(): Float { return Krom.zui_get(zui_, "font_offset_y"); }
	function set_fontOffsetY(a: Float) { Krom.zui_set(zui_, "font_offset_y", a); return a; }

	public var imageScrollAlign(never, set): Bool;
	function set_imageScrollAlign(a: Bool) { Krom.zui_set(zui_, "image_scroll_align", a); return a; }

	public var _x(get, set): Float;
	function get__x(): Float { return Krom.zui_get(zui_, "_x"); }
	function set__x(a: Float) { Krom.zui_set(zui_, "_x", a); return a; }

	public var _y(get, set): Float;
	function get__y(): Float { return Krom.zui_get(zui_, "_y"); }
	function set__y(a: Float) { Krom.zui_set(zui_, "_y", a); return a; }

	public var _w(get, set): Int;
	function get__w(): Int { return Krom.zui_get(zui_, "_w"); }
	function set__w(a: Int) { Krom.zui_set(zui_, "_w", a); return a; }

	public var _windowX(get, never): Float;
	function get__windowX(): Float { return Krom.zui_get(zui_, "_window_x"); }

	public var _windowY(get, never): Float;
	function get__windowY(): Float { return Krom.zui_get(zui_, "_window_y"); }

	public var _windowW(get, never): Float;
	function get__windowW(): Float { return Krom.zui_get(zui_, "_window_w"); }

	public var _windowH(get, never): Float;
	function get__windowH(): Float { return Krom.zui_get(zui_, "_window_h"); }

	public var scissor(get, set): Bool;
	function get_scissor(): Bool { return Krom.zui_get(zui_, "scissor"); }
	function set_scissor(a: Bool) { Krom.zui_set(zui_, "scissor", a); return a; }

	public var elementsBaked(never, set): Bool;
	function set_elementsBaked(a: Bool) { Krom.zui_set(zui_, "elements_baked", a); return a; }

	public var textSelectedHandle: Handle = null;
	public var submitTextHandle: Handle = null;
	public var comboSelectedHandle: Handle = null;

	public var g: Graphics;
	public var t: zui.Theme;
	public var ops: ZuiOptions; ////

	public var zui_: Dynamic;

	public function new(ops: ZuiOptions) {
		this.ops = ops; ////
		zui_ = Krom.zui_init({ font: ops.font.font_, theme: ops.theme.theme_, scaleFactor: ops.scaleFactor, color_wheel: ops.color_wheel, black_white_gradient: ops.black_white_gradient });
		current = this;
		t = ops.theme;
	}

	// getFont()
	// getScaleFactor

	public function SCALE(): Float {
		// return ops.scaleFactor;
		return 1.0;
	}

	public function setScale(factor: Float) {
		Krom.zui_set_scale(factor);
	}

	public function begin(g: Graphics) {
		current = this;
		this.g = g;
		Krom.zui_begin(zui_);
	}

	public function end(last = true) {
		Krom.zui_end(last);
	}

	public function beginRegion(g: Graphics, x: Int, y: Int, w: Int) {
		this.g = g;
		Krom.zui_begin_region(zui_, x, y, w);
	}

	public function endRegion(last = true) {
		Krom.zui_end_region(last);
	}

	public function beginSticky() {
		Krom.zui_begin_sticky();
	}

	public function endSticky() {
		Krom.zui_end_sticky();
	}

	public function endInput() {
		Krom.zui_end_input();
	}

	public function window(handle: Handle, x: Int, y: Int, w: Int, h: Int, drag = false): Bool {
		return Krom.zui_window(handle.handle_, x, y, w, h, drag);
	}

	public function endWindow(bindGlobalG = true) {
		Krom.zui_end_window(bindGlobalG);
	}

	public function tab(handle: Handle, text: String, vertical = false, color: Int = -1): Bool {
		return Krom.zui_tab(handle.handle_, text, vertical, color);
	}

	public function panel(handle: Handle, text: String, isTree = false, filled = true, pack = true): Bool {
		return Krom.zui_panel(handle.handle_, text, isTree, filled, pack);
	}

	public function image(image: kha.Image, tint = 0xffffffff, h: Null<Float> = null, sx = 0, sy = 0, sw = 0, sh = 0): State {
		return Krom.zui_image(image, tint, h == null ? -1 : Std.int(h), sx, sy, sw, sh);
	}

	public function text(text: String, align = Align.Left, bg = 0x00000000): State {
		return Krom.zui_text(text, align, bg);
	}

	public function textInput(handle: Handle, label = "", align = Align.Left, editable = true, liveUpdate = false): String {
		return Krom.zui_text_input(handle.handle_, label, align, editable, liveUpdate);
	}

	public function button(text: String, align = Align.Center, label = "", icon: kha.Image = null, sx = 0, sy = 0, sw = 0, sh = 0): Bool {
		return Krom.zui_button(text, align, label);
	}

	public function check(handle: Handle, text: String, label: String = ""): Bool {
		return Krom.zui_check(handle.handle_, text, label);
	}

	public function radio(handle: Handle, position: Int, text: String, label: String = ""): Bool {
		return Krom.zui_radio(handle.handle_, position, text, label);
	}

	public function combo(handle: Handle, texts: Array<String>, label = "", showLabel = false, align = Align.Left, searchBar = true): Int {
		return Krom.zui_combo(handle.handle_, texts, label, showLabel, align, searchBar);
	}

	public function slider(handle: Handle, text: String, from = 0.0, to = 1.0, filled = false, precision = 100.0, displayValue = true, align = Align.Right, textEdit = true): Float {
		return Krom.zui_slider(handle.handle_, text, from, to, filled, precision, displayValue, align, textEdit);
	}

	public function separator(h = 4, fill = true) {
		Krom.zui_separator(h, fill);
	}

	public function tooltip(text: String) {
		Krom.zui_tooltip(text);
	}

	public function tooltipImage(image: kha.Image, maxWidth: Null<Int> = null) {
		Krom.zui_tooltip_image(image, maxWidth == null ? 0 : maxWidth);
	}

	public function row(ratios: Array<Float>) {
		Krom.zui_row(ratios);
	}

	public function fill(x: Float, y: Float, w: Float, h: Float, color: kha.Color) {
		Krom.zui_fill(x, y, w, h, color);
	}

	public function rect(x: Float, y: Float, w: Float, h: Float, color: kha.Color, strength = 1.0) {
		Krom.zui_rect(x, y, w, h, color, strength);
	}

	public function drawRect(g: Graphics, fill: Bool, x: Float, y: Float, w: Float, h: Float, strength = 0.0) {
		Krom.zui_draw_rect(fill, x, y, w, h, strength);
	}

	public function endElement(elementSize: Null<Float> = null) {
		Krom.zui_end_element(elementSize == null ? -1 : elementSize);
	}

	public function startTextEdit(handle: Handle, align = Align.Left) {
		Krom.zui_start_text_edit(handle.handle_, align);
	}

	public function getInputInRect(x: Float, y: Float, w: Float, h: Float): Bool {
		return Krom.zui_input_in_rect(x, y, w, h);
	}

	public function drawString(g: Graphics, text: String, xOffset: Null<Float> = null, yOffset: Float = 0, align = Align.Left, truncation = true) {
		Krom.zui_draw_string(text, xOffset == null ? -1 : xOffset, yOffset, align, truncation);
	}

	public function getHoveredTabName(): String {
		return Krom.zui_get_hovered_tab_name();
	}

	public function setHoveredTabName(name: String) {
		Krom.zui_set_hovered_tab_name(name);
	}

	public function ELEMENT_W(): Float {
		return t.ELEMENT_W * SCALE();
	}

	public function ELEMENT_H(): Float {
		return t.ELEMENT_H * SCALE();
	}

	public function ELEMENT_OFFSET(): Float {
		return t.ELEMENT_OFFSET * SCALE();
	}

	public static var textAreaLineNumbers = false;
	public static var textAreaScrollPastEnd = false;
	public static var textAreaColoring: TTextColoring = null;

	public function floatInput(handle: Handle, label = "", align: Align = Left, precision = 1000.0): Float {
		return Krom.zui_float_input(handle.handle_, label, align, precision);
	}

	public function inlineRadio(handle: Handle, texts: Array<String>, align: Align = Left): Int {
		return Krom.zui_inline_radio(handle.handle_, texts, align);
	}

	public function colorWheel(handle: Handle, alpha = false, w: Null<Float> = null, h: Null<Float> = null, colorPreview = true, picker: Void->Void = null): kha.Color {
		return Krom.zui_color_wheel(handle.handle_, alpha, w != null ? w : 0, h != null ? h : 0, colorPreview, picker);
	}

	public function textArea(handle: Handle, align = Align.Left, editable = true, label = "", wordWrap = false): String {
		return Krom.zui_text_area(handle.handle_, align, editable, label, wordWrap);
	}

	public function beginMenu() {
		Krom.zui_begin_menu();
	}

	public function endMenu() {
		Krom.zui_end_menu();
	}

	public function menuButton(text: String): Bool {
		return Krom.zui_menu_button(text);
	}

	public function MENUBAR_H(): Float {
		// return ui.BUTTON_H() * 1.1 + 2 + ui.buttonOffsetY; ////
		return 30;
	}

	public static var children: Map<String, Handle> = [];

	public static function handle(s: String, ops: HandleOptions = null): Handle {
		var h = children.get(s);
		if (h == null) {
			h = new Handle(ops);
			children.set(s, h);
		}
		return h;
	}
}

typedef HandleOptions = {
	?selected: Bool,
	?position: Int,
	?value: Float,
	?text: String,
	?color: kha.Color,
	?layout: Layout
}

class Handle {
	public var selected(get, set): Bool;
	function get_selected(): Bool { return Krom.zui_handle_get(handle_, "selected"); }
	function set_selected(a: Bool) { Krom.zui_handle_set(handle_, "selected", a); return a; }

	public var position(get, set): Int;
	function get_position(): Int { return Krom.zui_handle_get(handle_, "position"); }
	function set_position(a: Int) { Krom.zui_handle_set(handle_, "position", a); return a; }

	public var color(get, set): kha.Color;
	function get_color(): Int { return Krom.zui_handle_get(handle_, "color"); }
	function set_color(a: Int) { Krom.zui_handle_set(handle_, "color", a); return a; }

	public var value(get, set): Float;
	function get_value(): Float { return Krom.zui_handle_get(handle_, "value"); }
	function set_value(a: Float) { Krom.zui_handle_set(handle_, "value", a); return a; }

	public var text(get, set): String;
	function get_text(): String { return Krom.zui_handle_get(handle_, "text"); }
	function set_text(a: String) { Krom.zui_handle_set(handle_, "text", a); return a; }

	public var redraws(never, set): Int;
	function set_redraws(a: Int) { Krom.zui_handle_set(handle_, "redraws", a); return a; }

	public var scrollOffset(get, never): Float;
	function get_scrollOffset(): Float { return Krom.zui_handle_get(handle_, "scroll_offset"); }

	public var dragX(get, set): Int;
	function get_dragX(): Int { return Krom.zui_handle_get(handle_, "drag_x"); }
	function set_dragX(a: Int) { Krom.zui_handle_set(handle_, "drag_x", a); return a; }

	public var dragY(get, set): Int;
	function get_dragY(): Int { return Krom.zui_handle_get(handle_, "drag_y"); }
	function set_dragY(a: Int) { Krom.zui_handle_set(handle_, "drag_y", a); return a; }

	public var changed(get, set): Bool;
	function get_changed(): Bool { return Krom.zui_handle_get(handle_, "changed"); }
	function set_changed(a: Bool) { Krom.zui_handle_set(handle_, "changed", a); return a; }

	public var children: Map<Int, Handle>;
	public var handle_: Dynamic;

	public function new(ops: HandleOptions = null) {
		if (ops == null) ops = {};
		if (ops.selected == null) ops.selected = false;
		if (ops.position == null) ops.position = 0;
		if (ops.value == null) ops.value = 0.0;
		if (ops.text == null) ops.text = "";
		if (ops.color == null) ops.color = 0xffffffff;
		if (ops.layout == null) ops.layout = Vertical;
		handle_ = Krom.zui_handle(ops);
	}

	public function nest(i: Int, ops: HandleOptions = null): Handle {
		if (children == null) children = [];
		var c = children.get(i);
		if (c == null) {
			c = new Handle(ops);
			children.set(i, c);
		}
		return c;
	}
}

@:enum abstract Layout(Int) from Int to Int {
	var Vertical = 0;
	var Horizontal = 1;
}

@:enum abstract Align(Int) from Int to Int {
	var Left = 0;
	var Center = 1;
	var Right = 2;
}

@:enum abstract State(Int) from Int to Int {
	var Idle = 0;
	var Started = 1;
	var Down = 2;
	var Released = 3;
	var Hovered = 4;
}

typedef TColoring = {
	var color: Int;
	var start: Array<String>;
	var end: String;
	var separated: Bool;
}

typedef TTextColoring = {
	var colorings: Array<TColoring>;
	var default_color: Int;
}

@:keep
class Theme {

	public var theme_: Dynamic;

	public function new() {
		theme_ = Krom.zui_theme_init();
	}

	public var WINDOW_BG_COL(get, set): Int;
	function get_WINDOW_BG_COL(): Int { return Krom.zui_theme_get(theme_, "WINDOW_BG_COL"); }
	function set_WINDOW_BG_COL(a: Int) { Krom.zui_theme_set(theme_, "WINDOW_BG_COL", a); return a; }

	public var WINDOW_TINT_COL(get, set): Int;
	function get_WINDOW_TINT_COL(): Int { return Krom.zui_theme_get(theme_, "WINDOW_TINT_COL"); }
	function set_WINDOW_TINT_COL(a: Int) { Krom.zui_theme_set(theme_, "WINDOW_TINT_COL", a); return a; }

	public var ACCENT_COL(get, set): Int;
	function get_ACCENT_COL(): Int { return Krom.zui_theme_get(theme_, "ACCENT_COL"); }
	function set_ACCENT_COL(a: Int) { Krom.zui_theme_set(theme_, "ACCENT_COL", a); return a; }

	public var ACCENT_HOVER_COL(get, set): Int;
	function get_ACCENT_HOVER_COL(): Int { return Krom.zui_theme_get(theme_, "ACCENT_HOVER_COL"); }
	function set_ACCENT_HOVER_COL(a: Int) { Krom.zui_theme_set(theme_, "ACCENT_HOVER_COL", a); return a; }

	public var ACCENT_SELECT_COL(get, set): Int;
	function get_ACCENT_SELECT_COL(): Int { return Krom.zui_theme_get(theme_, "ACCENT_SELECT_COL"); }
	function set_ACCENT_SELECT_COL(a: Int) { Krom.zui_theme_set(theme_, "ACCENT_SELECT_COL", a); return a; }

	public var BUTTON_COL(get, set): Int;
	function get_BUTTON_COL(): Int { return Krom.zui_theme_get(theme_, "BUTTON_COL"); }
	function set_BUTTON_COL(a: Int) { Krom.zui_theme_set(theme_, "BUTTON_COL", a); return a; }

	public var BUTTON_TEXT_COL(get, set): Int;
	function get_BUTTON_TEXT_COL(): Int { return Krom.zui_theme_get(theme_, "BUTTON_TEXT_COL"); }
	function set_BUTTON_TEXT_COL(a: Int) { Krom.zui_theme_set(theme_, "BUTTON_TEXT_COL", a); return a; }

	public var BUTTON_HOVER_COL(get, set): Int;
	function get_BUTTON_HOVER_COL(): Int { return Krom.zui_theme_get(theme_, "BUTTON_HOVER_COL"); }
	function set_BUTTON_HOVER_COL(a: Int) { Krom.zui_theme_set(theme_, "BUTTON_HOVER_COL", a); return a; }

	public var BUTTON_PRESSED_COL(get, set): Int;
	function get_BUTTON_PRESSED_COL(): Int { return Krom.zui_theme_get(theme_, "BUTTON_PRESSED_COL"); }
	function set_BUTTON_PRESSED_COL(a: Int) { Krom.zui_theme_set(theme_, "BUTTON_PRESSED_COL", a); return a; }

	public var TEXT_COL(get, set): Int;
	function get_TEXT_COL(): Int { return Krom.zui_theme_get(theme_, "TEXT_COL"); }
	function set_TEXT_COL(a: Int) { Krom.zui_theme_set(theme_, "TEXT_COL", a); return a; }

	public var LABEL_COL(get, set): Int;
	function get_LABEL_COL(): Int { return Krom.zui_theme_get(theme_, "LABEL_COL"); }
	function set_LABEL_COL(a: Int) { Krom.zui_theme_set(theme_, "LABEL_COL", a); return a; }

	public var SEPARATOR_COL(get, set): Int;
	function get_SEPARATOR_COL(): Int { return Krom.zui_theme_get(theme_, "SEPARATOR_COL"); }
	function set_SEPARATOR_COL(a: Int) { Krom.zui_theme_set(theme_, "SEPARATOR_COL", a); return a; }

	public var HIGHLIGHT_COL(get, set): Int;
	function get_HIGHLIGHT_COL(): Int { return Krom.zui_theme_get(theme_, "HIGHLIGHT_COL"); }
	function set_HIGHLIGHT_COL(a: Int) { Krom.zui_theme_set(theme_, "HIGHLIGHT_COL", a); return a; }

	public var CONTEXT_COL(get, set): Int;
	function get_CONTEXT_COL(): Int { return Krom.zui_theme_get(theme_, "CONTEXT_COL"); }
	function set_CONTEXT_COL(a: Int) { Krom.zui_theme_set(theme_, "CONTEXT_COL", a); return a; }

	public var PANEL_BG_COL(get, set): Int;
	function get_PANEL_BG_COL(): Int { return Krom.zui_theme_get(theme_, "PANEL_BG_COL"); }
	function set_PANEL_BG_COL(a: Int) { Krom.zui_theme_set(theme_, "PANEL_BG_COL", a); return a; }

	public var FONT_SIZE(get, set): Int;
	function get_FONT_SIZE(): Int { return Krom.zui_theme_get(theme_, "FONT_SIZE"); }
	function set_FONT_SIZE(a: Int) { Krom.zui_theme_set(theme_, "FONT_SIZE", a); return a; }

	public var ELEMENT_W(get, set): Int;
	function get_ELEMENT_W(): Int { return Krom.zui_theme_get(theme_, "ELEMENT_W"); }
	function set_ELEMENT_W(a: Int) { Krom.zui_theme_set(theme_, "ELEMENT_W", a); return a; }

	public var ELEMENT_H(get, set): Int;
	function get_ELEMENT_H(): Int { return Krom.zui_theme_get(theme_, "ELEMENT_H"); }
	function set_ELEMENT_H(a: Int) { Krom.zui_theme_set(theme_, "ELEMENT_H", a); return a; }

	public var ELEMENT_OFFSET(get, set): Int;
	function get_ELEMENT_OFFSET(): Int { return Krom.zui_theme_get(theme_, "ELEMENT_OFFSET"); }
	function set_ELEMENT_OFFSET(a: Int) { Krom.zui_theme_set(theme_, "ELEMENT_OFFSET", a); return a; }

	public var ARROW_SIZE(get, set): Int;
	function get_ARROW_SIZE(): Int { return Krom.zui_theme_get(theme_, "ARROW_SIZE"); }
	function set_ARROW_SIZE(a: Int) { Krom.zui_theme_set(theme_, "ARROW_SIZE", a); return a; }

	public var BUTTON_H(get, set): Int;
	function get_BUTTON_H(): Int { return Krom.zui_theme_get(theme_, "BUTTON_H"); }
	function set_BUTTON_H(a: Int) { Krom.zui_theme_set(theme_, "BUTTON_H", a); return a; }

	public var CHECK_SIZE(get, set): Int;
	function get_CHECK_SIZE(): Int { return Krom.zui_theme_get(theme_, "CHECK_SIZE"); }
	function set_CHECK_SIZE(a: Int) { Krom.zui_theme_set(theme_, "CHECK_SIZE", a); return a; }

	public var CHECK_SELECT_SIZE(get, set): Int;
	function get_CHECK_SELECT_SIZE(): Int { return Krom.zui_theme_get(theme_, "CHECK_SELECT_SIZE"); }
	function set_CHECK_SELECT_SIZE(a: Int) { Krom.zui_theme_set(theme_, "CHECK_SELECT_SIZE", a); return a; }

	public var SCROLL_W(get, set): Int;
	function get_SCROLL_W(): Int { return Krom.zui_theme_get(theme_, "SCROLL_W"); }
	function set_SCROLL_W(a: Int) { Krom.zui_theme_set(theme_, "SCROLL_W", a); return a; }

	public var SCROLL_MINI_W(get, set): Int;
	function get_SCROLL_MINI_W(): Int { return Krom.zui_theme_get(theme_, "SCROLL_MINI_W"); }
	function set_SCROLL_MINI_W(a: Int) { Krom.zui_theme_set(theme_, "SCROLL_MINI_W", a); return a; }

	public var TEXT_OFFSET(get, set): Int;
	function get_TEXT_OFFSET(): Int { return Krom.zui_theme_get(theme_, "TEXT_OFFSET"); }
	function set_TEXT_OFFSET(a: Int) { Krom.zui_theme_set(theme_, "TEXT_OFFSET", a); return a; }

	public var TAB_W(get, set): Int;
	function get_TAB_W(): Int { return Krom.zui_theme_get(theme_, "TAB_W"); }
	function set_TAB_W(a: Int) { Krom.zui_theme_set(theme_, "TAB_W", a); return a; }

	public var FILL_WINDOW_BG(get, set): Bool;
	function get_FILL_WINDOW_BG(): Bool { return Krom.zui_theme_get(theme_, "FILL_WINDOW_BG") > 0; }
	function set_FILL_WINDOW_BG(a: Bool) { Krom.zui_theme_set(theme_, "FILL_WINDOW_BG", a); return a; }

	public var FILL_BUTTON_BG(get, set): Bool;
	function get_FILL_BUTTON_BG(): Bool { return Krom.zui_theme_get(theme_, "FILL_BUTTON_BG") > 0; }
	function set_FILL_BUTTON_BG(a: Bool) { Krom.zui_theme_set(theme_, "FILL_BUTTON_BG", a); return a; }

	public var FILL_ACCENT_BG(get, set): Bool;
	function get_FILL_ACCENT_BG(): Bool { return Krom.zui_theme_get(theme_, "FILL_ACCENT_BG") > 0; }
	function set_FILL_ACCENT_BG(a: Bool) { Krom.zui_theme_set(theme_, "FILL_ACCENT_BG", a); return a; }

	public var LINK_STYLE(get, set): Int;
	function get_LINK_STYLE(): Int { return Krom.zui_theme_get(theme_, "LINK_STYLE"); }
	function set_LINK_STYLE(a: Int) { Krom.zui_theme_set(theme_, "LINK_STYLE", a); return a; }

	public var FULL_TABS(get, set): Bool;
	function get_FULL_TABS(): Bool { return Krom.zui_theme_get(theme_, "FULL_TABS") > 0; }
	function set_FULL_TABS(a: Bool) { Krom.zui_theme_set(theme_, "FULL_TABS", a); return a; }

	public var ROUND_CORNERS(get, set): Bool;
	function get_ROUND_CORNERS(): Bool { return Krom.zui_theme_get(theme_, "ROUND_CORNERS") > 0; }
	function set_ROUND_CORNERS(a: Bool) { Krom.zui_theme_set(theme_, "ROUND_CORNERS", a); return a; }
}

@:enum abstract LinkStyle(Int) from Int {
	var Line = 0;
	var CubicBezier = 1;
}

class Nodes {

	public static var excludeRemove: Array<String> = [];
	public static var onLinkDrag: TNodeLink->Bool->Void = null;
	public static var onSocketReleased: TNodeSocket->Void = null;
	public static var onCanvasReleased: Void->Void = null;
	public static var onNodeRemove: TNode->Void = null;
	public static var onCanvasControl: Void->CanvasControl = null;
	public static var enumTexts: String->Array<String> = null;
	public static var socketReleased = false;
	public static var clipboard = "";

	public var nodesSelected: Array<TNode> = [];
	public var nodesDrag = false;
	public var panX = 0.0;
	public var panY = 0.0;
	public var zoom = 1.0;
	public var _inputStarted = false;

	public var colorPickerCallback: kha.Color->Void = null;
	public var linkDrag: TNodeLink = null;
	public var handle = new Zui.Handle();

	public var nodes_: Dynamic;

	public function new() {
		nodes_ = Krom.zui_nodes_init();
	}

	// public static dynamic function tr(id: String, vars: Map<String, String> = null) {
	// 	return id;
	// }

	public function p(f: Float): Float {
		return f;
	}

	public function getNode(nodes: Array<TNode>, id: Int): TNode {
		for (node in nodes) if (node.id == id) return node;
		return null;
	}

	var nodeId = -1;
	public function getNodeId(nodes: Array<TNode>): Int {
		if (nodeId == -1) for (n in nodes) if (nodeId < n.id) nodeId = n.id;
		return ++nodeId;
	}

	public function getLinkId(links: Array<TNodeLink>): Int {
		var id = 0;
		for (l in links) if (l.id >= id) id = l.id + 1;
		return id;
	}

	public function getSocketId(nodes: Array<TNode>): Int {
		var id = 0;
		for (n in nodes) {
			for (s in n.inputs) if (s.id >= id) id = s.id + 1;
			for (s in n.outputs) if (s.id >= id) id = s.id + 1;
		}
		return id;
	}

	static inline var eps = 0.00001;

	public function nodeCanvas(ui: Zui, canvas: TNodeCanvas) {

		// Fill in optional values
		for (n in canvas.nodes) {
			for (soc in n.inputs) {
				if (soc.min == null) soc.min = 0.0 + eps;
				if (soc.max == null) soc.max = 1.0 + eps;
				if (soc.precision == null) soc.precision = 100.0 + eps;
				if (soc.display == null) soc.display = 0;
				if (!Std.isOfType(soc.default_value, js.lib.Float32Array)) {
					var f32 = new js.lib.Float32Array(1);
					f32[0] = soc.default_value;
					soc.default_value = f32;
				}
			}
			for (soc in n.outputs) {
				if (soc.min == null) soc.min = 0.0 + eps;
				if (soc.max == null) soc.max = 1.0 + eps;
				if (soc.precision == null) soc.precision = 100.0 + eps;
				if (soc.display == null) soc.display = 0;
				if (!Std.isOfType(soc.default_value, js.lib.Float32Array)) {
					var f32 = new js.lib.Float32Array(1);
					f32[0] = soc.default_value;
					soc.default_value = f32;
				}
			}
			for (but in n.buttons) {
				if (but.output == null) but.output = 0;
				if (but.default_value == null) but.default_value = new js.lib.Float32Array(1);
				if (!Std.isOfType(but.default_value, js.lib.Float32Array)) {
					var f32 = new js.lib.Float32Array(1);
					f32[0] = but.default_value;
					but.default_value = f32;
				}
				if (but.data == null) but.data = new js.lib.Float32Array(1);
				if (but.min == null) but.min = 0.0 + eps;
				if (but.max == null) but.max = 1.0 + eps;
				if (but.precision == null) but.precision = 100.0 + eps;
				if (but.height == null) but.height = 0;
			}
			if (n.width == null) n.width = 0;
		}

		var packed = Krom.zui_node_canvas(iron.system.ArmPack.encode(canvas).getData());
		var canvas_: TNodeCanvas = iron.system.ArmPack.decode(haxe.io.Bytes.ofData(packed));
		canvas.name = canvas_.name;
		canvas.nodes = canvas_.nodes;
		canvas.links = canvas_.links;
	}

	public function rgbaPopup(ui: Zui, nhandle: zui.Zui.Handle, val: kha.arrays.Float32Array, x: Int, y: Int) {
		Krom.zui_nodes_rgba_popup(nhandle.handle_, val.buffer, x, y);
	}

	public function removeNode(n: TNode, canvas: TNodeCanvas) {
		if (n == null) return;
		var i = 0;
		while (i < canvas.links.length) {
			var l = canvas.links[i];
			if (l.from_id == n.id || l.to_id == n.id) {
				canvas.links.splice(i, 1);
			}
			else i++;
		}
		canvas.nodes.remove(n);
		if (onNodeRemove != null) {
			onNodeRemove(n);
		}
	}

	public function NODE_H(canvas: TNodeCanvas, node: TNode): Int {
		return 1;
	}
	public function NODE_W(node: TNode): Int {
		return 1;
	}
	public function NODE_X(node: TNode): Float {
		return 1;
	}
	public function NODE_Y(node: TNode): Float {
		return 1;
	}
	public function BUTTONS_H(node: TNode): Int {
		return 1;
	}
	public function OUTPUTS_H(sockets: Array<TNodeSocket>, length: Null<Int> = null): Int {
		return 1;
	}
	public function INPUT_Y(canvas: TNodeCanvas, sockets: Array<TNodeSocket>, pos: Int): Int {
		return 1;
	}
	public function OUTPUT_Y(sockets: Array<TNodeSocket>, pos: Int): Int {
		return 1;
	}
	public function SCALE(): Float {
		return 1;
	}
	public function PAN_X(): Float {
		return 1;
	}
	public function PAN_Y(): Float {
		return 1;
	}
	public function LINE_H(): Int {
		return 1;
	}
}

typedef CanvasControl = {
	var panX: Float;
	var panY: Float;
	var zoom: Float;
}

typedef TNodeCanvas = {
	var name: String;
	var nodes: Array<TNode>;
	var links: Array<TNodeLink>;
}

typedef TNode = {
	var id: Int;
	var name: String;
	var type: String;
	var x: Float;
	var y: Float;
	var color: Int;
	var inputs: Array<TNodeSocket>;
	var outputs: Array<TNodeSocket>;
	var buttons: Array<TNodeButton>;
	@:optional var width: Null<Float>;
}

typedef TNodeSocket = {
	var id: Int;
	var node_id: Int;
	var name: String;
	var type: String;
	var color: Int;
	var default_value: Dynamic;
	@:optional var min: Null<Float>;
	@:optional var max: Null<Float>;
	@:optional var precision: Null<Float>;
	@:optional var display: Null<Int>;
}

typedef TNodeLink = {
	var id: Int;
	var from_id: Int;
	var from_socket: Int;
	var to_id: Int;
	var to_socket: Int;
}

typedef TNodeButton = {
	var name: String;
	var type: String;
	@:optional var output: Null<Int>;
	@:optional var default_value: Dynamic;
	@:optional var data: Dynamic;
	@:optional var min: Null<Float>;
	@:optional var max: Null<Float>;
	@:optional var precision: Null<Float>;
	@:optional var height: Null<Float>;
}
