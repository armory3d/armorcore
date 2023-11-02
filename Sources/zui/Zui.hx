package zui;

import kha.input.KeyCode;
import kha.graphics2.Graphics;

@:structInit
typedef ZuiOptions = {
	font: kha.Font,
	?theme: zui.Themes.TTheme,
	?scaleFactor: Float,
	?color_wheel: kha.Image,
	?black_white_gradient: kha.Image,
}

class Zui {
	public static var current: Zui = null;
	public static var onBorderHover: Handle->Int->Void = null;
	public static var onTextHover: Void->Void = null;
	public static var onDeselectText: Void->Void = null;
	public static var onTabDrop: Handle->Int->Handle->Int->Void = null;
	public static var alwaysRedrawWindow = true;
	public static var touchScroll = false;
	public static var touchHold = false;
	public static var touchTooltip = false;
	public static var isCut = false;
	public static var isCopy = false;
	public static var isPaste = false;
	public var isScrolling = false;
	public var isTyping = false;
	public var enabled = true;
	public var isHovered = false;
	public var isReleased = false;
	public var changed = false;
	public var imageInvertY = false;
	public var scrollEnabled = true;
	public var windowBorderTop = 0;
	public var windowBorderBottom = 0;
	public var windowBorderRight = 0;
	public var inputEnabled = true;
	public var inputX: Float;
	public var inputY: Float;
	public var inputStartedX: Float;
	public var inputStartedY: Float;
	public var inputDX: Float;
	public var inputDY: Float;
	public var inputWheelDelta = 0.0;
	public var inputStarted: Bool;
	public var inputStartedR: Bool;
	public var inputReleased: Bool;
	public var inputReleasedR: Bool;
	public var inputDown: Bool;
	public var inputDownR: Bool;
	public var isKeyPressed = false;
	public var isCtrlDown = false;
	public var isDeleteDown = false;
	public var isEscapeDown = false;
	public var isReturnDown = false;
	public var key: Null<KeyCode> = null;
	public var g: Graphics;
	public var t: zui.Themes.TTheme;
	public var ops: ZuiOptions;
	public var curRatio = -1;
	public var fontSize: Int;
	public var fontOffsetY: Float;
	public var imageScrollAlign = true;
	public var _x: Float;
	public var _y: Float;
	public var _w: Int;
	public var _windowX = 0.0;
	public var _windowY = 0.0;
	public var _windowW: Float;
	public var _windowH: Float;
	public var currentWindow: Handle;
	public var textSelectedHandle: Handle = null;
	public var submitTextHandle: Handle = null;
	public var comboSelectedHandle: Handle = null;
	public var scissor = false;
	public var elementsBaked = false;

	var zui_: Dynamic;

	public function new(ops: ZuiOptions) {
		zui_ = Krom.zui_init({ font: ops.font.font_, theme: ops.theme, scaleFactor: ops.scaleFactor, color_wheel: ops.color_wheel, black_white_gradient: ops.black_white_gradient });
		t = ops.theme; ////
	}

	public function setScale(factor: Float) {
		Krom.zui_set_scale(factor);
	}

	public function begin(g: Graphics) {
		this.g = g;
		Krom.zui_begin(zui_);
	}

	public function end(last = true) {
		Krom.zui_end(last);
	}

	public function beginRegion(g: Graphics, x: Int, y: Int, w: Int) {
		Krom.zui_begin_region(x, y, w);
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
		return "";
	}

	public function setHoveredTabName(name: String) {

	}

	public function ELEMENT_W(): Float {
		return t.ELEMENT_W;
	}

	public function ELEMENT_H(): Float {
		return t.ELEMENT_H;
	}

	public function ELEMENT_OFFSET(): Float {
		return t.ELEMENT_OFFSET;
	}

	public function SCALE(): Float {
		return 1.0;
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
	public var selected = false;
	public var position = 0;
	public var color: kha.Color = 0xffffffff;
	public var value = 0.0;
	public var text = "";
	public var texture: kha.Image = null;
	public var redraws = 2;
	public var scrollOffset = 0.0;
	public var scrollEnabled = false;
	public var layout: Layout = 0;
	public var lastMaxX = 0.0;
	public var lastMaxY = 0.0;
	public var dragEnabled = false;
	public var dragX = 0;
	public var dragY = 0;
	public var changed = false;
	var children: Map<Int, Handle>;

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
	@:optional var separated: Null<Bool>;
}

typedef TTextColoring = {
	var colorings: Array<TColoring>;
	var default_color: Int;
}
