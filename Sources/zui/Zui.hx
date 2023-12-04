// Bindings to Zui in C

package zui;

import iron.system.Input.KeyCode;
import kha.Graphics2;

typedef ZuiOptions = {
	font: kha.Font,
	theme: zui.Theme,
	scaleFactor: Float,
	color_wheel: kha.Image,
	black_white_gradient: kha.Image,
}

class Zui {
	public static var current: Zui = null;

	public static var onBorderHover(never, set): Dynamic->Int->Void;
	static function set_onBorderHover(f: Dynamic->Int->Void) { Krom.zui_set_on_border_hover(f); return f; }

	public static var onTextHover(never, set): Void->Void ;
	static function set_onTextHover(f: Void->Void) { Krom.zui_set_on_text_hover(f); return f; }

	public static var onDeselectText(never, set): Void->Void ;
	static function set_onDeselectText(f: Void->Void) { Krom.zui_set_on_deselect_text(f); return f; }

	public static var onTabDrop(never, set): Dynamic->Int->Dynamic->Int->Void ;
	static function set_onTabDrop(f: Dynamic->Int->Dynamic->Int->Void) { Krom.zui_set_on_tab_drop(f); return f; }

	public static var textAreaLineNumbers(never, set): Bool;
	static function set_textAreaLineNumbers(a: Bool): Bool { Krom.zui_set(null, "zui_text_area_line_numbers", a); return a; }

	public static var textAreaScrollPastEnd(never, set): Bool;
	static function set_textAreaScrollPastEnd(a: Bool): Bool { Krom.zui_set(null, "zui_text_area_scroll_past_end", a); return a; }

	public static var textAreaColoring(never, set): TTextColoring;
	static function set_textAreaColoring(coloring: TTextColoring): TTextColoring {
		Krom.zui_text_area_coloring(coloring == null ? null : iron.system.ArmPack.encode(coloring).getData());
		return coloring;
	}

	public static var alwaysRedrawWindow(get, set): Bool;
	static function get_alwaysRedrawWindow(): Bool { return Krom.zui_get(null, "zui_always_redraw_window"); }
	static function set_alwaysRedrawWindow(a: Bool): Bool { Krom.zui_set(null, "zui_always_redraw_window", a); return a; }

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

	public var textSelectedHandle_ptr(get, never): Null<Int>;
	function get_textSelectedHandle_ptr(): Null<Int> { var h = Krom.zui_get(zui_, "text_selected_handle"); return h == 0 ? null : h; }

	public var submitTextHandle_ptr(get, never): Null<Int>;
	function get_submitTextHandle_ptr(): Null<Int> { var h = Krom.zui_get(zui_, "submit_text_handle"); return h == 0 ? null : h; }

	public var comboSelectedHandle_ptr(get, never): Null<Int>;
	function get_comboSelectedHandle_ptr(): Null<Int> { var h = Krom.zui_get(zui_, "combo_selected_handle"); return h == 0 ? null : h; }

	public var t(default, set): zui.Theme = null;
	function set_t(theme: zui.Theme) {
		if (t != null) {
			for (key in Type.getInstanceFields(zui.Theme)) {
				if (key == "theme_") continue;
				if (StringTools.startsWith(key, "set_")) continue;
				if (StringTools.startsWith(key, "get_")) key = key.substr(4);
				Reflect.setProperty(t, key, Reflect.getProperty(theme, key));
			}
			theme.theme_ = t.theme_;
		}
		return t = theme;
	}

	public var g: Graphics2;
	public var font: kha.Font;
	public var zui_: Dynamic;

	public function new(ops: ZuiOptions) {
		zui_ = Krom.zui_init(
			{
				font: ops.font.font_,
				theme: ops.theme.theme_,
				scale_factor: ops.scaleFactor,
				color_wheel: ops.color_wheel != null ? ops.color_wheel.texture_ : null,
				black_white_gradient: ops.black_white_gradient != null ? ops.black_white_gradient.texture_ : null
			}
		);
		current = this;
		t = ops.theme;
		font = ops.font;
	}

	public function setFont(font: kha.Font) {
		font.init(); // Make sure font_ is ready
		this.font = font;
		Krom.zui_set_font(zui_, font.font_);
	}

	public function SCALE(): Float {
		return Krom.zui_get_scale(zui_);
	}

	public function setScale(factor: Float) {
		Krom.zui_set_scale(zui_, factor);
	}

	public function begin(g: Graphics2) {
		current = this;
		Krom.zui_begin(zui_);
		Graphics2.current = g;
	}

	public function end(last = true) {
		Krom.zui_end(last);
		Graphics2.current = null;
	}

	public function beginRegion(g: Graphics2, x: Int, y: Int, w: Int) {
		current = this;
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
		var img = @:privateAccess new kha.Image(null);
		img.renderTarget_ = handle.texture;
		Graphics2.current = g = img.g2;
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

	public function drawRect(g: Graphics2, fill: Bool, x: Float, y: Float, w: Float, h: Float, strength = 0.0) {
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

	public function drawString(g: Graphics2, text: String, xOffset: Null<Float> = null, yOffset: Float = 0, align = Align.Left, truncation = true) {
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

	public function floatInput(handle: Handle, label = "", align: Align = Left, precision = 1000.0): Float {
		return Krom.zui_float_input(handle.handle_, label, align, precision);
	}

	public function inlineRadio(handle: Handle, texts: Array<String>, align: Align = Left): Int {
		return Krom.zui_inline_radio(handle.handle_, texts, align);
	}

	public function colorWheel(handle: Handle, alpha = false, w: Null<Float> = null, h: Null<Float> = null, colorPreview = true, picker: Void->Void = null): kha.Color {
		return Krom.zui_color_wheel(handle.handle_, alpha, w != null ? w : -1, h != null ? h : -1, colorPreview, picker);
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
		var buttonOffsetY = (t.ELEMENT_H * SCALE() - t.BUTTON_H * SCALE()) / 2;
		return t.BUTTON_H * SCALE() * 1.1 + 2 + buttonOffsetY;
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

	public var texture(get, never): Dynamic;
	function get_texture(): Dynamic { return Krom.zui_handle_get(handle_, "texture"); }

	public var ptr(get, never): Null<Int>;
	function get_ptr(): Null<Int> { return Krom.zui_handle_ptr(handle_); }

	public var ops: HandleOptions;
	public var children: Map<Int, Handle>;

	public var handle__: Dynamic = null;
	public var handle_(get, never): Dynamic;
	function get_handle_(): Dynamic { if (handle__ == null) handle__ = Krom.zui_handle(ops); return handle__; }

	public function new(ops: HandleOptions = null) {
		if (ops == null) ops = {};
		if (ops.selected == null) ops.selected = false;
		if (ops.position == null) ops.position = 0;
		if (ops.value == null) ops.value = 0.0;
		if (ops.text == null) ops.text = "";
		if (ops.color == null) ops.color = 0xffffffff;
		if (ops.layout == null) ops.layout = Vertical;
		this.ops = ops;
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

class Nodes {
	public static var current: Nodes;
	public static var currentCanvas: TNodeCanvas;
	public static var tr: String->?Map<String, String>->String;

	public static var clipboard = "";

	public static var excludeRemove: Array<String> = ["OUTPUT_MATERIAL_PBR", "GROUP_OUTPUT", "GROUP_INPUT", "BrushOutputNode"];

	public static var onLinkDrag(never, set): Int->Bool->Void;
	static function set_onLinkDrag(f: Int->Bool->Void) { Krom.zui_nodes_set_on_link_drag(f); return f; }

	public static var onSocketReleased(never, set): Int->Void;
	static function set_onSocketReleased(f: Int->Void) { Krom.zui_nodes_set_on_socket_released(f); return f; }

	public static var onCanvasReleased(never, set): Void->Void;
	static function set_onCanvasReleased(f: Void->Void) { Krom.zui_nodes_set_on_canvas_released(f); return f; }

	// public static var onNodeRemove(never, set): Int->Void;

	public static var onCanvasControl(never, set): Void->CanvasControl;
	static function set_onCanvasControl(f: Void->CanvasControl) { Krom.zui_nodes_set_on_canvas_control(f); return f; }

	public static var socketReleased(get, never): Bool;
	static function get_socketReleased(): Bool { return Krom.zui_nodes_get(null, "socket_released"); }

	public static var enumTextsHaxe: String->Array<String> = null;
	public static var enumTexts(never, set): String->Array<String>;
	static function set_enumTexts(f: String->Array<String>) { enumTextsHaxe = f; Krom.zui_nodes_set_enum_texts(f); return f; }

	public var colorPickerCallback: kha.Color->Void = null;

	public var nodesSelectedId(get ,set): Array<Int>;
	function get_nodesSelectedId(): Array<Int> { return Krom.zui_nodes_get(nodes_, "nodes_selected_id"); }
	function set_nodesSelectedId(a: Array<Int>) { Krom.zui_nodes_set(nodes_, "nodes_selected_id", a); return a; }

	public var _inputStarted(never, set): Bool;
	function set__inputStarted(a: Bool) { Krom.zui_nodes_set(nodes_, "_input_started", a); return a; }

	public var nodesDrag(never, set): Bool;
	function set_nodesDrag(a: Bool) { Krom.zui_nodes_set(nodes_, "nodes_drag", a); return a; }

	public var panX(get, set): Float;
	function get_panX(): Float { return Krom.zui_nodes_get(nodes_, "pan_x"); }
	function set_panX(a: Float) { Krom.zui_nodes_set(nodes_, "pan_x", a); return a; }

	public var panY(get, set): Float;
	function get_panY(): Float { return Krom.zui_nodes_get(nodes_, "pan_y"); }
	function set_panY(a: Float) { Krom.zui_nodes_set(nodes_, "pan_y", a); return a; }

	public var zoom(never, set): Float;
	function set_zoom(a: Float) { Krom.zui_nodes_set(nodes_, "zoom", a); return a; }

	public var linkDragId(get, set): Int;
	function get_linkDragId(): Int { return Krom.zui_nodes_get(nodes_, "link_drag_id"); }
	function set_linkDragId(a: Int) { Krom.zui_nodes_set(nodes_, "link_drag_id", a); return a; }

	public var handle = new Zui.Handle();
	public var ELEMENT_H = 25;
	public var nodes_: Dynamic;

	public function new() {
		nodes_ = Krom.zui_nodes_init();
		Krom.zui_nodes_set_on_custom_button(on_custom_button);
	}

	public static function on_custom_button(node_id: Int, button_name: String) {
		var dot = button_name.lastIndexOf("."); // button_name specifies external function path
		var fn = Reflect.field(Type.resolveClass(button_name.substr(0, dot)), button_name.substr(dot + 1));
		fn(Zui.current, current, current.getNode(currentCanvas.nodes, node_id));
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

	public function getLink(links: Array<TNodeLink>, id: Int): TNodeLink {
		for (link in links) if (link.id == id) return link;
		return null;
	}

	public function getLinkId(links: Array<TNodeLink>): Int {
		var id = 0;
		for (l in links) if (l.id >= id) id = l.id + 1;
		return id;
	}

	public function getSocket(nodes: Array<TNode>, id: Int): TNodeSocket {
		for (n in nodes) {
			for (s in n.inputs) if (s.id == id) return s;
			for (s in n.outputs) if (s.id == id) return s;
		}
		return null;
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

	static function jsToC(type: String, d: Dynamic): js.lib.Uint8Array {
		if (type == "RGBA") {
			if (d == null) return new js.lib.Uint8Array(16);
			if (Type.typeof(d) == TObject) {
				var f32 = new js.lib.Float32Array(4);
				f32[0] = d[0];
				f32[1] = d[1];
				f32[2] = d[2];
				f32[3] = d[3];
				d = new js.lib.Uint8Array(f32.buffer);
			}
			return new js.lib.Uint8Array(d.buffer);
		}
		if (type == "VECTOR") {
			if (d == null) return new js.lib.Uint8Array(12);
			if (Type.typeof(d) == TObject) {
				var f32 = new js.lib.Float32Array(4);
				f32[0] = d[0];
				f32[1] = d[1];
				f32[2] = d[2];
				d = new js.lib.Uint8Array(f32.buffer);
			}
			return new js.lib.Uint8Array(d.buffer);
		}
		if (type == "VALUE") {
			if (d == null) return new js.lib.Uint8Array(4);
			var f32 = new js.lib.Float32Array([d]);
			return new js.lib.Uint8Array(f32.buffer);
		}
		if (type == "STRING") {
			if (d == null) return new js.lib.Uint8Array(1);
			var s: String = cast d;
			var u8 = new js.lib.Uint8Array(s.length + 1);
			for (i in 0...s.length) u8[i] = s.charCodeAt(i);
			return u8;
		}
		if (type == "ENUM") {
			if (d == null) return new js.lib.Uint8Array(1);
			var u32 = new js.lib.Uint32Array([d]);
			return new js.lib.Uint8Array(u32.buffer);
		}
		if (type == "BOOL") {
			if (d == null) return new js.lib.Uint8Array(1);
			var u8 = new js.lib.Uint8Array(1);
			u8[0] = d == true ? 1 : 0;
			return u8;
		}
		if (type == "CUSTOM") {
			return new js.lib.Uint8Array(1);
		}
		return new js.lib.Uint8Array(1);
	}

	static function jsToCData(type: String, d: Dynamic): js.lib.Uint8Array {
		if (type == "ENUM") {
			if (d == null) return new js.lib.Uint8Array(1);
			var a: Array<String> = cast d;
			var length = 0;
			for (s in a) {
				length += s.length + 1;
			}
			if (length == 0) return new js.lib.Uint8Array(1);
			var u8 = new js.lib.Uint8Array(length);
			var pos = 0;
			for (s in a) {
				for (i in 0...s.length) u8[pos++] = s.charCodeAt(i);
				u8[pos++] = 0; // '\0'
			}
			return u8;
		}
		if (type == "CUSTOM") {
			return new js.lib.Uint8Array(1);
		}
		return new js.lib.Uint8Array(1);
	}

	static function cToJs(type: String, u8: js.lib.Uint8Array): Dynamic {
		if (type == "RGBA") {
			return new js.lib.Float32Array(u8.buffer);
		}
		if (type == "VECTOR") {
			return new js.lib.Float32Array(u8.buffer);
		}
		if (type == "VALUE") {
			var f32 = new js.lib.Float32Array(u8.buffer);
			return f32[0];
		}
		if (type == "STRING") {
			var s = "";
			for (i in 0...u8.length - 1) s += String.fromCharCode(u8[i]);
			return s;
		}
		if (type == "ENUM") {
			var u32 = new js.lib.Uint32Array(u8.buffer);
			return u32[0];
		}
		if (type == "BOOL") {
			return u8[0] > 0 ? true : false;
		}
		if (type == "CUSTOM") {
			return 0;
		}
		return null;
	}

	static function cToJsData(type: String, u8: js.lib.Uint8Array): Dynamic {
		if (type == "ENUM") {
			var a: Array<String> = [];
			var s = "";
			for (i in 0...u8.length) {
				if (u8[i] == 0) {
					a.push(s);
					s = "";
				}
				else {
					s += String.fromCharCode(u8[i]);
				}
			}
			return a;
		}
		if (type == "CUSTOM") {
			return 0;
		}
		return null;
	}

	public static function updateCanvasFormat(canvas: TNodeCanvas) {
		for (n in canvas.nodes) {
			for (soc in n.inputs) {
				if (soc.min == null) soc.min = 0.0;
				if (soc.max == null) soc.max = 1.0;
				if (soc.precision == null) soc.precision = 100.0;
				if (soc.display == null) soc.display = 0;
				if (soc.min - Std.int(soc.min) == 0.0) soc.min += eps;
				if (soc.max - Std.int(soc.max) == 0.0) soc.max += eps;
				if (soc.precision - Std.int(soc.precision) == 0.0) soc.precision += eps;
			}
			for (soc in n.outputs) {
				if (soc.min == null) soc.min = 0.0;
				if (soc.max == null) soc.max = 1.0;
				if (soc.precision == null) soc.precision = 100.0;
				if (soc.display == null) soc.display = 0;
				if (soc.min - Std.int(soc.min) == 0.0) soc.min += eps;
				if (soc.max - Std.int(soc.max) == 0.0) soc.max += eps;
				if (soc.precision - Std.int(soc.precision) == 0.0) soc.precision += eps;
			}
			for (but in n.buttons) {
				if (but.output == null) but.output = -1;
				if (but.min == null) but.min = 0.0;
				if (but.max == null) but.max = 1.0;
				if (but.precision == null) but.precision = 100.0;
				if (but.height == null) but.height = 0.0;
				if (but.height - Std.int(but.height) == 0.0) but.height += eps;
				if (but.min - Std.int(but.min) == 0.0) but.min += eps;
				if (but.max - Std.int(but.max) == 0.0) but.max += eps;
				if (but.precision - Std.int(but.precision) == 0.0) but.precision += eps;
			}
			if (n.width == null) n.width = 0;
		}
	}

	public static var node_replace: Array<TNode> = [];

	public function nodeCanvas(ui: Zui, canvas: TNodeCanvas) {
		current = this;
		currentCanvas = canvas;

		// Fill in optional values
		updateCanvasFormat(canvas);

		// Ensure properties order
		var canvas_: TNodeCanvas = {
			name: canvas.name,
			nodes: canvas.nodes.copy(),
			nodes_count: canvas.nodes.length,
			links: canvas.links.copy(),
			links_count: canvas.links.length,
		}

		// Convert default data
		for (n in canvas_.nodes) {
			for (soc in n.inputs) {
				soc.default_value = jsToC(soc.type, soc.default_value);
			}
			for (soc in n.outputs) {
				soc.default_value = jsToC(soc.type, soc.default_value);
			}
			for (but in n.buttons) {
				but.default_value = jsToC(but.type, but.default_value);
				but.data = jsToCData(but.type, but.data);
			}
		}

		// Ensure properties order
		for (n in canvas_.nodes) {
			n.name = tr(n.name);
			for (i in 0...n.inputs.length) {
				n.inputs[i] = {
					id: n.inputs[i].id,
					node_id: n.inputs[i].node_id,
					name: tr(n.inputs[i].name),
					type: n.inputs[i].type,
					color: n.inputs[i].color,
					default_value: n.inputs[i].default_value,
					min: n.inputs[i].min,
					max: n.inputs[i].max,
					precision: n.inputs[i].precision,
					display: n.inputs[i].display,
				};
			}
			for (i in 0...n.outputs.length) {
				n.outputs[i] = {
					id: n.outputs[i].id,
					node_id: n.outputs[i].node_id,
					name: tr(n.outputs[i].name),
					type: n.outputs[i].type,
					color: n.outputs[i].color,
					default_value: n.outputs[i].default_value,
					min: n.outputs[i].min,
					max: n.outputs[i].max,
					precision: n.outputs[i].precision,
					display: n.outputs[i].display,
				};
			}
			for (i in 0...n.buttons.length) {
				n.buttons[i] = {
					name: tr(n.buttons[i].name),
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
			canvas_.nodes.push({ id: -1, name: "", type: "", x: 0, y: 0, color: 0, inputs: [], outputs: [], buttons: [], width: 0 });
		}
		while (canvas_.links.length < 256) {
			canvas_.links.push({ id: -1, from_id: 0, from_socket: 0, to_id: 0, to_socket: 0 });
		}

		var packed = Krom.zui_node_canvas(nodes_, iron.system.ArmPack.encode(canvas_).getData());
		var canvas_: TNodeCanvas = iron.system.ArmPack.decode(haxe.io.Bytes.ofData(packed));
		if (canvas_.nodes == null) canvas_.nodes = [];
		if (canvas_.links == null) canvas_.links = [];

		// Convert default data
		for (n in canvas_.nodes) {
			for (soc in n.inputs) {
				soc.default_value = cToJs(soc.type, soc.default_value);
			}
			for (soc in n.outputs) {
				soc.default_value = cToJs(soc.type, soc.default_value);
			}
			for (but in n.buttons) {
				but.default_value = cToJs(but.type, but.default_value);
				but.data = cToJsData(but.type, but.data);
			}
		}

		canvas.name = canvas_.name;
		canvas.nodes = canvas_.nodes;
		canvas.links = canvas_.links;

		// Restore nodes modified in js while Krom.zui_node_canvas was running
		for (n in node_replace) {
			for (i in 0...canvas.nodes.length) {
				if (canvas.nodes[i].id == n.id) {
					canvas.nodes[i] = n;
					break;
				}
			}
		}
		node_replace = [];

		ELEMENT_H = ui.t.ELEMENT_H + 2;
	}

	public function rgbaPopup(ui: Zui, nhandle: zui.Zui.Handle, val: js.lib.Float32Array, x: Int, y: Int) {
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
		// if (onNodeRemove != null) {
		// 	onNodeRemove(n);
		// }
	}

	public function SCALE(): Float {
		return Krom.zui_nodes_scale();
	}

	public function PAN_X(): Float {
		return Krom.zui_nodes_pan_x();
	}

	public function PAN_Y(): Float {
		return Krom.zui_nodes_pan_y();
	}

	public function NODE_H(canvas: TNodeCanvas, node: TNode): Int {
		return Std.int(LINE_H() * 1.2 + INPUTS_H(canvas, node.inputs) + OUTPUTS_H(node.outputs) + BUTTONS_H(node));
	}

	public function NODE_W(node: TNode): Int {
		return Std.int((node.width != 0 ? node.width : 140) * SCALE());
	}

	public function NODE_X(node: TNode): Float {
		return node.x * SCALE() + PAN_X();
	}

	public function NODE_Y(node: TNode): Float {
		return node.y * SCALE() + PAN_Y();
	}

	public function BUTTONS_H(node: TNode): Int {
		var h = 0.0;
		for (but in node.buttons) {
			if (but.type == "RGBA") h += 102 * SCALE() + LINE_H() * 5; // Color wheel + controls
			else if (but.type == "VECTOR") h += LINE_H() * 4;
			else if (but.type == "CUSTOM") h += LINE_H() * but.height;
			else h += LINE_H();
		}
		return Std.int(h);
	}

	public function OUTPUTS_H(sockets: Array<TNodeSocket>, length: Null<Int> = null): Int {
		var h = 0.0;
		for (i in 0...(length == null ? sockets.length : length)) {
			h += LINE_H();
		}
		return Std.int(h);
	}

	function inputLinked(canvas: TNodeCanvas, node_id: Int, i: Int): Bool {
		for (l in canvas.links) if (l.to_id == node_id && l.to_socket == i) return true;
		return false;
	}

	public function INPUTS_H(canvas: TNodeCanvas, sockets: Array<TNodeSocket>, length: Null<Int> = null): Int {
		var h = 0.0;
		for (i in 0...(length == null ? sockets.length : length)) {
			if (sockets[i].type == "VECTOR" && sockets[i].display == 1 && !inputLinked(canvas, sockets[i].node_id, i)) h += LINE_H() * 4;
			else h += LINE_H();
		}
		return Std.int(h);
	}

	public function INPUT_Y(canvas: TNodeCanvas, sockets: Array<TNodeSocket>, pos: Int): Int {
		return Std.int(LINE_H() * 1.62) + INPUTS_H(canvas, sockets, pos);
	}

	public function OUTPUT_Y(sockets: Array<TNodeSocket>, pos: Int): Int {
		return Std.int(LINE_H() * 1.62) + OUTPUTS_H(sockets, pos);
	}

	public function LINE_H(): Int {
		return Std.int(ELEMENT_H * SCALE());
	}

	public function p(f: Float): Float {
		return f * SCALE();
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
	@:optional var nodes_count: Int;
	var links: Array<TNodeLink>;
	@:optional var links_count: Int;
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
