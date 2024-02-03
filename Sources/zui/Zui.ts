// Bindings to Zui in C

type ZuiOptions = {
	font: FontRaw;
	theme: Theme;
	scaleFactor: f32;
	color_wheel: ImageRaw;
	black_white_gradient: ImageRaw;
}

class Zui {
	static current: Zui = null;

	static set onBorderHover(f: (handle_ptr: any, side: i32)=>void) { Krom.zui_set_on_border_hover(f); }

	static set onTextHover(f: ()=>void) { Krom.zui_set_on_text_hover(f); }

	static set onDeselectText(f: ()=>void) { Krom.zui_set_on_deselect_text(f); }

	static set onTabDrop(f: (to_ptr: any, to_pos: i32, from_ptr: any, from_pos: i32)=>void) { Krom.zui_set_on_tab_drop(f); }

	static set textAreaLineNumbers(a: bool) { Krom.zui_set(null, "zui_text_area_line_numbers", a); }

	static set textAreaScrollPastEnd(a: bool) { Krom.zui_set(null, "zui_text_area_scroll_past_end", a); }

	static set textAreaColoring(coloring: TTextColoring) {
		Krom.zui_text_area_coloring(coloring == null ? null : ArmPack.encode(coloring));
	}

	static get alwaysRedrawWindow(): bool { return Krom.zui_get(null, "zui_always_redraw_window"); }
	static set alwaysRedrawWindow(a: bool) { Krom.zui_set(null, "zui_always_redraw_window", a); }

	static get touchScroll(): bool { return Krom.zui_get(null, "zui_touch_scroll"); }
	static set touchScroll(a: bool) { Krom.zui_set(null, "zui_touch_scroll", a); }

	static get touchHold(): bool { return Krom.zui_get(null, "zui_touch_hold"); }
	static set touchHold(a: bool) { Krom.zui_set(null, "zui_touch_hold", a); }

	static get touchTooltip(): bool { return Krom.zui_get(null, "zui_touch_tooltip"); }
	static set touchTooltip(a: bool) { Krom.zui_set(null, "zui_touch_tooltip", a); }

	static set isCut(a: bool) { Krom.zui_set(null, "zui_is_cut", a); }

	static set isCopy(a: bool) { Krom.zui_set(null, "zui_is_copy", a); }

	static get isPaste(): bool { return Krom.zui_get(null, "zui_is_paste"); }
	static set isPaste(a: bool) { Krom.zui_set(null, "zui_is_paste", a); }

	get isScrolling(): bool { return Krom.zui_get(this.zui_, "is_scrolling"); }

	get isTyping(): bool { return Krom.zui_get(this.zui_, "is_typing"); }

	get enabled(): bool { return Krom.zui_get(this.zui_, "enabled"); }
	set enabled(a: bool) { Krom.zui_set(this.zui_, "enabled", a); }

	get isHovered(): bool { return Krom.zui_get(this.zui_, "is_hovered"); }
	set isHovered(a: bool) { Krom.zui_set(this.zui_, "is_hovered", a); }

	get isReleased(): bool { return Krom.zui_get(this.zui_, "is_released"); }

	get changed(): bool { return Krom.zui_get(this.zui_, "changed"); }
	set changed(a: bool) { Krom.zui_set(this.zui_, "changed", a); }

	set imageInvertY(a: bool) { Krom.zui_set(this.zui_, "image_invert_y", a); }

	set scrollEnabled(a: bool) { Krom.zui_set(this.zui_, "scroll_enabled", a); }

	set windowBorderTop(a: i32) { Krom.zui_set(this.zui_, "window_border_top", a); }

	set windowBorderBottom(a: i32) { Krom.zui_set(this.zui_, "window_border_bottom", a); }

	set windowBorderRight(a: i32) { Krom.zui_set(this.zui_, "window_border_right", a); }

	get inputEnabled(): bool { return Krom.zui_get(this.zui_, "input_enabled"); }
	set inputEnabled(a: bool) { Krom.zui_set(this.zui_, "input_enabled", a); }

	get inputX(): f32 { return Krom.zui_get(this.zui_, "input_x"); }
	set inputX(a: f32) { Krom.zui_set(this.zui_, "input_x", a); }

	get inputY(): f32 { return Krom.zui_get(this.zui_, "input_y"); }
	set inputY(a: f32) { Krom.zui_set(this.zui_, "input_y", a); }

	get inputStartedX(): f32 { return Krom.zui_get(this.zui_, "input_started_x"); }

	get inputStartedY(): f32 { return Krom.zui_get(this.zui_, "input_started_y"); }

	get inputDX(): f32 { return Krom.zui_get(this.zui_, "input_dx"); }

	get inputDY(): f32 { return Krom.zui_get(this.zui_, "input_dy"); }

	get inputWheelDelta(): f32 { return Krom.zui_get(this.zui_, "input_wheel_delta"); }

	get inputStarted(): bool { return Krom.zui_get(this.zui_, "input_started"); }
	set inputStarted(a: bool) { Krom.zui_set(this.zui_, "input_started", a); }

	get inputStartedR(): bool { return Krom.zui_get(this.zui_, "input_started_r"); }

	get inputReleased(): bool { return Krom.zui_get(this.zui_, "input_released"); }

	get inputReleasedR(): bool { return Krom.zui_get(this.zui_, "input_released_r"); }

	get inputDown(): bool { return Krom.zui_get(this.zui_, "input_down"); }

	get inputDownR(): bool { return Krom.zui_get(this.zui_, "input_down_r"); }

	get isKeyPressed(): bool { return Krom.zui_get(this.zui_, "is_key_pressed"); }

	get isCtrlDown(): bool { return Krom.zui_get(this.zui_, "is_ctrl_down"); }

	get isDeleteDown(): bool { return Krom.zui_get(this.zui_, "is_delete_down"); }
	set isDeleteDown(a: bool) { Krom.zui_set(this.zui_, "is_delete_down", a); }

	get isEscapeDown(): bool { return Krom.zui_get(this.zui_, "is_escape_down"); }

	get isReturnDown(): bool { return Krom.zui_get(this.zui_, "is_return_down"); }

	get key(): KeyCode { return Krom.zui_get(this.zui_, "key_code"); }

	get curRatio(): i32 { return Krom.zui_get(this.zui_, "current_ratio"); }
	set curRatio(a: i32) { Krom.zui_set(this.zui_, "current_ratio", a); }

	get fontSize(): i32 { return Krom.zui_get(this.zui_, "font_size"); }
	set fontSize(a: i32) { Krom.zui_set(this.zui_, "font_size", a); }

	get fontOffsetY(): f32 { return Krom.zui_get(this.zui_, "font_offset_y"); }
	set fontOffsetY(a: f32) { Krom.zui_set(this.zui_, "font_offset_y", a); }

	set imageScrollAlign(a: bool) { Krom.zui_set(this.zui_, "image_scroll_align", a); }

	get _x(): f32 { return Krom.zui_get(this.zui_, "_x"); }
	set _x(a: f32) { Krom.zui_set(this.zui_, "_x", a); }

	get _y(): f32 { return Krom.zui_get(this.zui_, "_y"); }
	set _y(a: f32) { Krom.zui_set(this.zui_, "_y", a); }

	get _w(): i32 { return Krom.zui_get(this.zui_, "_w"); }
	set _w(a: i32) { Krom.zui_set(this.zui_, "_w", a); }

	get _windowX(): f32 { return Krom.zui_get(this.zui_, "_window_x"); }

	get _windowY(): f32 { return Krom.zui_get(this.zui_, "_window_y"); }

	get _windowW(): f32 { return Krom.zui_get(this.zui_, "_window_w"); }

	get _windowH(): f32 { return Krom.zui_get(this.zui_, "_window_h"); }

	get scissor(): bool { return Krom.zui_get(this.zui_, "scissor"); }
	set scissor(a: bool) { Krom.zui_set(this.zui_, "scissor", a); }

	set elementsBaked(a: bool) { Krom.zui_set(this.zui_, "elements_baked", a); }

	get textSelectedHandle_ptr(): Null<i32> { let h = Krom.zui_get(this.zui_, "text_selected_handle"); return h == 0 ? null : h; }

	get submitTextHandle_ptr(): Null<i32> { let h = Krom.zui_get(this.zui_, "submit_text_handle"); return h == 0 ? null : h; }

	get comboSelectedHandle_ptr(): Null<i32> { let h = Krom.zui_get(this.zui_, "combo_selected_handle"); return h == 0 ? null : h; }

	_t: Theme = null;
	get t(): Theme {
		return this._t;
	}
	set t(theme: Theme) {
		if (this.t != null) {
			for (let key of Object.getOwnPropertyNames(Theme.prototype)) {
				if (key == "constructor") continue;
				let t_: any = this.t;
				let theme_: any = theme;
				t_[key] = theme_[key];
			}
			theme.theme_ = this.t.theme_;
		}
		this._t = theme;
	}

	g: Graphics2Raw;
	font: FontRaw;
	zui_: any;

	constructor(ops: ZuiOptions) {
		this.zui_ = Krom.zui_init(
			{
				font: ops.font.font_,
				theme: ops.theme.theme_,
				scale_factor: ops.scaleFactor,
				color_wheel: ops.color_wheel != null ? ops.color_wheel.texture_ : null,
				black_white_gradient: ops.black_white_gradient != null ? ops.black_white_gradient.texture_ : null
			}
		);
		Zui.current = this;
		this.t = ops.theme;
		this.font = ops.font;
	}

	setFont(font: FontRaw) {
		Font.init(font); // Make sure font_ is ready
		this.font = font;
		Krom.zui_set_font(this.zui_, font.font_);
	}

	SCALE(): f32 {
		return Krom.zui_get_scale(this.zui_);
	}

	setScale(factor: f32) {
		Krom.zui_set_scale(this.zui_, factor);
	}

	begin(g: Graphics2Raw) {
		Zui.current = this;
		Krom.zui_begin(this.zui_);
		Graphics2.current = g;
	}

	end(last = true) {
		Krom.zui_end(last);
		Graphics2.current = null;
	}

	beginRegion(g: Graphics2Raw, x: i32, y: i32, w: i32) {
		Zui.current = this;
		this.g = g;
		Krom.zui_begin_region(this.zui_, x, y, w);
	}

	endRegion(last = true) {
		Krom.zui_end_region(last);
	}

	beginSticky() {
		Krom.zui_begin_sticky();
	}

	endSticky() {
		Krom.zui_end_sticky();
	}

	endInput() {
		Krom.zui_end_input();
	}

	window(handle: Handle, x: i32, y: i32, w: i32, h: i32, drag = false): bool {
		let img = Image._create(null);
		img.renderTarget_ = handle.texture;
		Graphics2.current = this.g = img.g2;
		return Krom.zui_window(handle.handle_, x, y, w, h, drag);
	}

	endWindow(bindGlobalG = true) {
		Krom.zui_end_window(bindGlobalG);
	}

	tab(handle: Handle, text: string, vertical = false, color: i32 = -1): bool {
		return Krom.zui_tab(handle.handle_, text, vertical, color);
	}

	panel(handle: Handle, text: string, isTree = false, filled = true, pack = true): bool {
		return Krom.zui_panel(handle.handle_, text, isTree, filled, pack);
	}

	image(image: ImageRaw, tint = 0xffffffff, h: Null<f32> = null, sx = 0, sy = 0, sw = 0, sh = 0): State {
		return Krom.zui_image(image, tint, h == null ? -1 : Math.floor(h), sx, sy, sw, sh);
	}

	text(text: string, align = Align.Left, bg = 0x00000000): State {
		return Krom.zui_text(text, align, bg);
	}

	textInput(handle: Handle, label = "", align = Align.Left, editable = true, liveUpdate = false): string {
		return Krom.zui_text_input(handle.handle_, label, align, editable, liveUpdate);
	}

	button(text: string, align = Align.Center, label = "", icon: ImageRaw = null, sx = 0, sy = 0, sw = 0, sh = 0): bool {
		return Krom.zui_button(text, align, label);
	}

	check(handle: Handle, text: string, label: string = ""): bool {
		return Krom.zui_check(handle.handle_, text, label);
	}

	radio(handle: Handle, position: i32, text: string, label: string = ""): bool {
		return Krom.zui_radio(handle.handle_, position, text, label);
	}

	combo(handle: Handle, texts: string[], label = "", showLabel = false, align = Align.Left, searchBar = true): i32 {
		return Krom.zui_combo(handle.handle_, texts, label, showLabel, align, searchBar);
	}

	slider(handle: Handle, text: string, from = 0.0, to = 1.0, filled = false, precision = 100.0, displayValue = true, align = Align.Right, textEdit = true): f32 {
		return Krom.zui_slider(handle.handle_, text, from, to, filled, precision, displayValue, align, textEdit);
	}

	separator(h = 4, fill = true) {
		Krom.zui_separator(h, fill);
	}

	tooltip(text: string) {
		Krom.zui_tooltip(text);
	}

	tooltipImage(image: ImageRaw, maxWidth: Null<i32> = null) {
		Krom.zui_tooltip_image(image, maxWidth == null ? 0 : maxWidth);
	}

	row(ratios: f32[]) {
		Krom.zui_row(ratios);
	}

	fill(x: f32, y: f32, w: f32, h: f32, color: Color) {
		Krom.zui_fill(x, y, w, h, color);
	}

	rect(x: f32, y: f32, w: f32, h: f32, color: Color, strength = 1.0) {
		Krom.zui_rect(x, y, w, h, color, strength);
	}

	drawRect(g: Graphics2Raw, fill: bool, x: f32, y: f32, w: f32, h: f32, strength = 0.0) {
		Krom.zui_draw_rect(fill, x, y, w, h, strength);
	}

	endElement(elementSize: Null<f32> = null) {
		Krom.zui_end_element(elementSize == null ? -1 : elementSize);
	}

	startTextEdit(handle: Handle, align = Align.Left) {
		Krom.zui_start_text_edit(handle.handle_, align);
	}

	getInputInRect(x: f32, y: f32, w: f32, h: f32): bool {
		return Krom.zui_input_in_rect(x, y, w, h);
	}

	drawString(g: Graphics2Raw, text: string, xOffset: Null<f32> = null, yOffset: f32 = 0, align = Align.Left, truncation = true) {
		Krom.zui_draw_string(text, xOffset == null ? -1 : xOffset, yOffset, align, truncation);
	}

	getHoveredTabName(): string {
		return Krom.zui_get_hovered_tab_name();
	}

	setHoveredTabName(name: string) {
		Krom.zui_set_hovered_tab_name(name);
	}

	ELEMENT_W(): f32 {
		return this.t.ELEMENT_W * this.SCALE();
	}

	ELEMENT_H(): f32 {
		return this.t.ELEMENT_H * this.SCALE();
	}

	ELEMENT_OFFSET(): f32 {
		return this.t.ELEMENT_OFFSET * this.SCALE();
	}

	floatInput(handle: Handle, label = "", align = Align.Left, precision = 1000.0): f32 {
		return Krom.zui_float_input(handle.handle_, label, align, precision);
	}

	inlineRadio(handle: Handle, texts: string[], align = Align.Left): i32 {
		return Krom.zui_inline_radio(handle.handle_, texts, align);
	}

	colorWheel(handle: Handle, alpha = false, w: Null<f32> = null, h: Null<f32> = null, colorPreview = true, picker: ()=>void = null): Color {
		return Krom.zui_color_wheel(handle.handle_, alpha, w != null ? w : -1, h != null ? h : -1, colorPreview, picker);
	}

	textArea(handle: Handle, align = Align.Left, editable = true, label = "", wordWrap = false): string {
		return Krom.zui_text_area(handle.handle_, align, editable, label, wordWrap);
	}

	beginMenu() {
		Krom.zui_begin_menu();
	}

	endMenu() {
		Krom.zui_end_menu();
	}

	menuButton(text: string): bool {
		return Krom.zui_menu_button(text);
	}

	MENUBAR_H(): f32 {
		let buttonOffsetY = (this.t.ELEMENT_H * this.SCALE() - this.t.BUTTON_H * this.SCALE()) / 2;
		return this.t.BUTTON_H * this.SCALE() * 1.1 + 2 + buttonOffsetY;
	}

	static children: Map<string, Handle> = new Map();

	static handle(s: string, ops: HandleOptions = null): Handle {
		let h = Zui.children.get(s);
		if (h == null) {
			h = new Handle(ops);
			Zui.children.set(s, h);
		}
		return h;
	}
}

type HandleOptions = {
	selected?: boolean,
	position?: i32,
	value?: f32,
	text?: string,
	color?: Color,
	layout?: Layout
}

class Handle {
	get selected(): bool { return Krom.zui_handle_get(this.handle_, "selected"); }
	set selected(a: bool) { Krom.zui_handle_set(this.handle_, "selected", a); }

	get position(): i32 { return Krom.zui_handle_get(this.handle_, "position"); }
	set position(a: i32) { Krom.zui_handle_set(this.handle_, "position", a); }

	get color(): i32 { return Krom.zui_handle_get(this.handle_, "color"); }
	set color(a: i32) { Krom.zui_handle_set(this.handle_, "color", a); }

	get value(): f32 { return Krom.zui_handle_get(this.handle_, "value"); }
	set value(a: f32) { Krom.zui_handle_set(this.handle_, "value", a); }

	get text(): string { return Krom.zui_handle_get(this.handle_, "text"); }
	set text(a: string) { Krom.zui_handle_set(this.handle_, "text", a); }

	set redraws(a: i32) { Krom.zui_handle_set(this.handle_, "redraws", a); }

	get scrollOffset(): f32 { return Krom.zui_handle_get(this.handle_, "scroll_offset"); }

	get dragX(): i32 { return Krom.zui_handle_get(this.handle_, "drag_x"); }
	set dragX(a: i32) { Krom.zui_handle_set(this.handle_, "drag_x", a); }

	get dragY(): i32 { return Krom.zui_handle_get(this.handle_, "drag_y"); }
	set dragY(a: i32) { Krom.zui_handle_set(this.handle_, "drag_y", a); }

	get changed(): bool { return Krom.zui_handle_get(this.handle_, "changed"); }
	set changed(a: bool) { Krom.zui_handle_set(this.handle_, "changed", a); }

	get texture(): any { return Krom.zui_handle_get(this.handle_, "texture"); }

	get ptr(): Null<i32> { return Krom.zui_handle_ptr(this.handle_); }

	ops: HandleOptions;
	children: Map<i32, Handle>;

	handle__: any = null;
	get handle_(): any { if (this.handle__ == null) this.handle__ = Krom.zui_handle(this.ops); return this.handle__; }

	constructor(ops: HandleOptions = null) {
		if (ops == null) ops = {};
		if (ops.selected == null) ops.selected = false;
		if (ops.position == null) ops.position = 0;
		if (ops.value == null) ops.value = 0.0;
		if (ops.text == null) ops.text = "";
		if (ops.color == null) ops.color = 0xffffffff;
		if (ops.layout == null) ops.layout = Layout.Vertical;
		this.ops = ops;
	}

	nest(i: i32, ops: HandleOptions = null): Handle {
		if (this.children == null) this.children = new Map();
		let c = this.children.get(i);
		if (c == null) {
			c = new Handle(ops);
			this.children.set(i, c);
		}
		return c;
	}
}

enum Layout {
	Vertical,
	Horizontal,
}

enum Align {
	Left,
	Center,
	Right,
}

enum State {
	Idle,
	Started,
	Down,
	Released,
	Hovered,
}

type TColoring = {
	color: i32;
	start: string[];
	end: string;
	separated: boolean;
}

type TTextColoring = {
	colorings: TColoring[];
	default_color: i32;
}

class Theme {
	theme_: any;

	constructor() {
		this.theme_ = Krom.zui_theme_init();
	}

	get WINDOW_BG_COL(): i32 { return Krom.zui_theme_get(this.theme_, "WINDOW_BG_COL"); }
	set WINDOW_BG_COL(a: i32) { Krom.zui_theme_set(this.theme_, "WINDOW_BG_COL", a); }

	get WINDOW_TINT_COL(): i32 { return Krom.zui_theme_get(this.theme_, "WINDOW_TINT_COL"); }
	set WINDOW_TINT_COL(a: i32) { Krom.zui_theme_set(this.theme_, "WINDOW_TINT_COL", a); }

	get ACCENT_COL(): i32 { return Krom.zui_theme_get(this.theme_, "ACCENT_COL"); }
	set ACCENT_COL(a: i32) { Krom.zui_theme_set(this.theme_, "ACCENT_COL", a); }

	get ACCENT_HOVER_COL(): i32 { return Krom.zui_theme_get(this.theme_, "ACCENT_HOVER_COL"); }
	set ACCENT_HOVER_COL(a: i32) { Krom.zui_theme_set(this.theme_, "ACCENT_HOVER_COL", a); }

	get ACCENT_SELECT_COL(): i32 { return Krom.zui_theme_get(this.theme_, "ACCENT_SELECT_COL"); }
	set ACCENT_SELECT_COL(a: i32) { Krom.zui_theme_set(this.theme_, "ACCENT_SELECT_COL", a); }

	get BUTTON_COL(): i32 { return Krom.zui_theme_get(this.theme_, "BUTTON_COL"); }
	set BUTTON_COL(a: i32) { Krom.zui_theme_set(this.theme_, "BUTTON_COL", a); }

	get BUTTON_TEXT_COL(): i32 { return Krom.zui_theme_get(this.theme_, "BUTTON_TEXT_COL"); }
	set BUTTON_TEXT_COL(a: i32) { Krom.zui_theme_set(this.theme_, "BUTTON_TEXT_COL", a); }

	get BUTTON_HOVER_COL(): i32 { return Krom.zui_theme_get(this.theme_, "BUTTON_HOVER_COL"); }
	set BUTTON_HOVER_COL(a: i32) { Krom.zui_theme_set(this.theme_, "BUTTON_HOVER_COL", a); }

	get BUTTON_PRESSED_COL(): i32 { return Krom.zui_theme_get(this.theme_, "BUTTON_PRESSED_COL"); }
	set BUTTON_PRESSED_COL(a: i32) { Krom.zui_theme_set(this.theme_, "BUTTON_PRESSED_COL", a); }

	get TEXT_COL(): i32 { return Krom.zui_theme_get(this.theme_, "TEXT_COL"); }
	set TEXT_COL(a: i32) { Krom.zui_theme_set(this.theme_, "TEXT_COL", a); }

	get LABEL_COL(): i32 { return Krom.zui_theme_get(this.theme_, "LABEL_COL"); }
	set LABEL_COL(a: i32) { Krom.zui_theme_set(this.theme_, "LABEL_COL", a); }

	get SEPARATOR_COL(): i32 { return Krom.zui_theme_get(this.theme_, "SEPARATOR_COL"); }
	set SEPARATOR_COL(a: i32) { Krom.zui_theme_set(this.theme_, "SEPARATOR_COL", a); }

	get HIGHLIGHT_COL(): i32 { return Krom.zui_theme_get(this.theme_, "HIGHLIGHT_COL"); }
	set HIGHLIGHT_COL(a: i32) { Krom.zui_theme_set(this.theme_, "HIGHLIGHT_COL", a); }

	get CONTEXT_COL(): i32 { return Krom.zui_theme_get(this.theme_, "CONTEXT_COL"); }
	set CONTEXT_COL(a: i32) { Krom.zui_theme_set(this.theme_, "CONTEXT_COL", a); }

	get PANEL_BG_COL(): i32 { return Krom.zui_theme_get(this.theme_, "PANEL_BG_COL"); }
	set PANEL_BG_COL(a: i32) { Krom.zui_theme_set(this.theme_, "PANEL_BG_COL", a); }

	get FONT_SIZE(): i32 { return Krom.zui_theme_get(this.theme_, "FONT_SIZE"); }
	set FONT_SIZE(a: i32) { Krom.zui_theme_set(this.theme_, "FONT_SIZE", a); }

	get ELEMENT_W(): i32 { return Krom.zui_theme_get(this.theme_, "ELEMENT_W"); }
	set ELEMENT_W(a: i32) { Krom.zui_theme_set(this.theme_, "ELEMENT_W", a); }

	get ELEMENT_H(): i32 { return Krom.zui_theme_get(this.theme_, "ELEMENT_H"); }
	set ELEMENT_H(a: i32) { Krom.zui_theme_set(this.theme_, "ELEMENT_H", a); }

	get ELEMENT_OFFSET(): i32 { return Krom.zui_theme_get(this.theme_, "ELEMENT_OFFSET"); }
	set ELEMENT_OFFSET(a: i32) { Krom.zui_theme_set(this.theme_, "ELEMENT_OFFSET", a); }

	get ARROW_SIZE(): i32 { return Krom.zui_theme_get(this.theme_, "ARROW_SIZE"); }
	set ARROW_SIZE(a: i32) { Krom.zui_theme_set(this.theme_, "ARROW_SIZE", a); }

	get BUTTON_H(): i32 { return Krom.zui_theme_get(this.theme_, "BUTTON_H"); }
	set BUTTON_H(a: i32) { Krom.zui_theme_set(this.theme_, "BUTTON_H", a); }

	get CHECK_SIZE(): i32 { return Krom.zui_theme_get(this.theme_, "CHECK_SIZE"); }
	set CHECK_SIZE(a: i32) { Krom.zui_theme_set(this.theme_, "CHECK_SIZE", a); }

	get CHECK_SELECT_SIZE(): i32 { return Krom.zui_theme_get(this.theme_, "CHECK_SELECT_SIZE"); }
	set CHECK_SELECT_SIZE(a: i32) { Krom.zui_theme_set(this.theme_, "CHECK_SELECT_SIZE", a); }

	get SCROLL_W(): i32 { return Krom.zui_theme_get(this.theme_, "SCROLL_W"); }
	set SCROLL_W(a: i32) { Krom.zui_theme_set(this.theme_, "SCROLL_W", a); }

	get SCROLL_MINI_W(): i32 { return Krom.zui_theme_get(this.theme_, "SCROLL_MINI_W"); }
	set SCROLL_MINI_W(a: i32) { Krom.zui_theme_set(this.theme_, "SCROLL_MINI_W", a); }

	get TEXT_OFFSET(): i32 { return Krom.zui_theme_get(this.theme_, "TEXT_OFFSET"); }
	set TEXT_OFFSET(a: i32) { Krom.zui_theme_set(this.theme_, "TEXT_OFFSET", a); }

	get TAB_W(): i32 { return Krom.zui_theme_get(this.theme_, "TAB_W"); }
	set TAB_W(a: i32) { Krom.zui_theme_set(this.theme_, "TAB_W", a); }

	get FILL_WINDOW_BG(): bool { return Krom.zui_theme_get(this.theme_, "FILL_WINDOW_BG") > 0; }
	set FILL_WINDOW_BG(a: bool) { Krom.zui_theme_set(this.theme_, "FILL_WINDOW_BG", a); }

	get FILL_BUTTON_BG(): bool { return Krom.zui_theme_get(this.theme_, "FILL_BUTTON_BG") > 0; }
	set FILL_BUTTON_BG(a: bool) { Krom.zui_theme_set(this.theme_, "FILL_BUTTON_BG", a); }

	get FILL_ACCENT_BG(): bool { return Krom.zui_theme_get(this.theme_, "FILL_ACCENT_BG") > 0; }
	set FILL_ACCENT_BG(a: bool) { Krom.zui_theme_set(this.theme_, "FILL_ACCENT_BG", a); }

	get LINK_STYLE(): i32 { return Krom.zui_theme_get(this.theme_, "LINK_STYLE"); }
	set LINK_STYLE(a: i32) { Krom.zui_theme_set(this.theme_, "LINK_STYLE", a); }

	get FULL_TABS(): bool { return Krom.zui_theme_get(this.theme_, "FULL_TABS") > 0; }
	set FULL_TABS(a: bool) { Krom.zui_theme_set(this.theme_, "FULL_TABS", a); }

	get ROUND_CORNERS(): bool { return Krom.zui_theme_get(this.theme_, "ROUND_CORNERS") > 0; }
	set ROUND_CORNERS(a: bool) { Krom.zui_theme_set(this.theme_, "ROUND_CORNERS", a); }
}

class Nodes {
	static current: Nodes;
	static currentCanvas: TNodeCanvas;
	static tr: (id: string, vars?: Map<string, string>)=>string;

	static clipboard = "";

	static excludeRemove: string[] = ["OUTPUT_MATERIAL_PBR", "GROUP_OUTPUT", "GROUP_INPUT", "BrushOutputNode"];

	static set onLinkDrag(f: (link_drag_id: i32, is_new_link: bool)=>void) { Krom.zui_nodes_set_on_link_drag(f); }

	static set onSocketReleased(f: (socket_id: i32)=>void) { Krom.zui_nodes_set_on_socket_released(f); }

	static set onCanvasReleased(f: ()=>void) { Krom.zui_nodes_set_on_canvas_released(f); }

	// static onNodeRemove: (node_id: i32)=>void;

	static set onCanvasControl(f: ()=>CanvasControl) { Krom.zui_nodes_set_on_canvas_control(f); }

	static get socketReleased(): bool { return Krom.zui_nodes_get(null, "socket_released"); }

	static enumTextsHaxe: (node_type: string)=>string[] = null;
	static set enumTexts(f: (node_type: string)=>string[]) { Nodes.enumTextsHaxe = f; Krom.zui_nodes_set_enum_texts(f); }

	colorPickerCallback: (col: Color)=>void = null;

	get nodesSelectedId(): i32[] { return Krom.zui_nodes_get(this.nodes_, "nodes_selected_id"); }
	set nodesSelectedId(a: i32[]) { Krom.zui_nodes_set(this.nodes_, "nodes_selected_id", a); }

	set _inputStarted(a: bool) { Krom.zui_nodes_set(this.nodes_, "_input_started", a); }

	set nodesDrag(a: bool) { Krom.zui_nodes_set(this.nodes_, "nodes_drag", a); }

	get panX(): f32 { return Krom.zui_nodes_get(this.nodes_, "pan_x"); }
	set panX(a: f32) { Krom.zui_nodes_set(this.nodes_, "pan_x", a); }

	get panY(): f32 { return Krom.zui_nodes_get(this.nodes_, "pan_y"); }
	set panY(a: f32) { Krom.zui_nodes_set(this.nodes_, "pan_y", a); }

	set zoom(a: f32) { Krom.zui_nodes_set(this.nodes_, "zoom", a); }

	get linkDragId(): i32 { return Krom.zui_nodes_get(this.nodes_, "link_drag_id"); }
	set linkDragId(a: i32) { Krom.zui_nodes_set(this.nodes_, "link_drag_id", a); }

	handle = new Handle();
	ELEMENT_H = 25;
	nodes_: any;

	constructor() {
		this.nodes_ = Krom.zui_nodes_init();
		Krom.zui_nodes_set_on_custom_button(Nodes.on_custom_button);
	}

	static on_custom_button(node_id: i32, button_name: string) {
		eval(button_name + "(Zui.current, Nodes.current, Nodes.current.getNode(currentCanvas.nodes, node_id))");
	}

	getNode(nodes: TNode[], id: i32): TNode {
		for (let node of nodes) if (node.id == id) return node;
		return null;
	}

	nodeId = -1;
	getNodeId(nodes: TNode[]): i32 {
		if (this.nodeId == -1) for (let n of nodes) if (this.nodeId < n.id) this.nodeId = n.id;
		return ++this.nodeId;
	}

	getLink(links: TNodeLink[], id: i32): TNodeLink {
		for (let link of links) if (link.id == id) return link;
		return null;
	}

	getLinkId(links: TNodeLink[]): i32 {
		let id = 0;
		for (let l of links) if (l.id >= id) id = l.id + 1;
		return id;
	}

	getSocket(nodes: TNode[], id: i32): TNodeSocket {
		for (let n of nodes) {
			for (let s of n.inputs) if (s.id == id) return s;
			for (let s of n.outputs) if (s.id == id) return s;
		}
		return null;
	}

	getSocketId(nodes: TNode[]): i32 {
		let id = 0;
		for (let n of nodes) {
			for (let s of n.inputs) if (s.id >= id) id = s.id + 1;
			for (let s of n.outputs) if (s.id >= id) id = s.id + 1;
		}
		return id;
	}

	static eps = 0.00001;

	static jsToC(type: string, d: any): Uint8Array {
		if (type == "RGBA") {
			if (d == null) return new Uint8Array(16);
			else {
				let f32a = new Float32Array(4);
				f32a[0] = d[0];
				f32a[1] = d[1];
				f32a[2] = d[2];
				f32a[3] = d[3];
				d = new Uint8Array(f32a.buffer);
			}
			return new Uint8Array(d.buffer);
		}
		if (type == "VECTOR") {
			if (d == null) return new Uint8Array(12);
			else {
				let f32a = new Float32Array(4);
				f32a[0] = d[0];
				f32a[1] = d[1];
				f32a[2] = d[2];
				d = new Uint8Array(f32a.buffer);
			}
			return new Uint8Array(d.buffer);
		}
		if (type == "VALUE") {
			if (d == null) return new Uint8Array(4);
			let f32a = new Float32Array([d]);
			return new Uint8Array(f32a.buffer);
		}
		if (type == "STRING") {
			if (d == null) return new Uint8Array(1);
			let s: string = d;
			let u8a = new Uint8Array(s.length + 1);
			for (let i = 0; i < s.length; ++i) u8a[i] = s.charCodeAt(i);
			return u8a;
		}
		if (type == "ENUM") {
			if (d == null) return new Uint8Array(1);
			let u32a = new Uint32Array([d]);
			return new Uint8Array(u32a.buffer);
		}
		if (type == "BOOL") {
			if (d == null) return new Uint8Array(1);
			let u8a = new Uint8Array(1);
			u8a[0] = d == true ? 1 : 0;
			return u8a;
		}
		if (type == "CUSTOM") {
			return new Uint8Array(1);
		}
		return new Uint8Array(1);
	}

	static jsToCData(type: string, d: any): Uint8Array {
		if (type == "ENUM") {
			if (d == null) return new Uint8Array(1);
			let a: string[] = d;
			let length = 0;
			for (let s of a) {
				length += s.length + 1;
			}
			if (length == 0) return new Uint8Array(1);
			let u8a = new Uint8Array(length);
			let pos = 0;
			for (let s of a) {
				for (let i = 0; i < s.length; ++i) u8a[pos++] = s.charCodeAt(i);
				u8a[pos++] = 0; // '\0'
			}
			return u8a;
		}
		if (type == "CUSTOM") {
			return new Uint8Array(1);
		}
		return new Uint8Array(1);
	}

	static cToJs(type: string, u8a: Uint8Array): any {
		if (type == "RGBA") {
			return new Float32Array(u8a.buffer);
		}
		if (type == "VECTOR") {
			return new Float32Array(u8a.buffer);
		}
		if (type == "VALUE") {
			let f32a = new Float32Array(u8a.buffer);
			return f32a[0];
		}
		if (type == "STRING") {
			let s = "";
			for (let i = 0; i < u8a.length - 1; ++i) s += String.fromCharCode(u8a[i]);
			return s;
		}
		if (type == "ENUM") {
			let u32a = new Uint32Array(u8a.buffer);
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

	static cToJsData(type: string, u8a: Uint8Array): any {
		if (type == "ENUM") {
			let a: string[] = [];
			let s = "";
			for (let i = 0; i < u8a.length; ++i) {
				if (u8a[i] == 0) {
					a.push(s);
					s = "";
				}
				else {
					s += String.fromCharCode(u8a[i]);
				}
			}
			return a;
		}
		if (type == "CUSTOM") {
			return 0;
		}
		return null;
	}

	static updateCanvasFormat(canvas: TNodeCanvas) {
		for (let n of canvas.nodes) {
			for (let soc of n.inputs) {
				if (soc.min == null) soc.min = 0.0;
				if (soc.max == null) soc.max = 1.0;
				if (soc.precision == null) soc.precision = 100.0;
				if (soc.display == null) soc.display = 0;
				if (soc.min - Math.floor(soc.min) == 0.0) soc.min += Nodes.eps;
				if (soc.max - Math.floor(soc.max) == 0.0) soc.max += Nodes.eps;
				if (soc.precision - Math.floor(soc.precision) == 0.0) soc.precision += Nodes.eps;
			}
			for (let soc of n.outputs) {
				if (soc.min == null) soc.min = 0.0;
				if (soc.max == null) soc.max = 1.0;
				if (soc.precision == null) soc.precision = 100.0;
				if (soc.display == null) soc.display = 0;
				if (soc.min - Math.floor(soc.min) == 0.0) soc.min += Nodes.eps;
				if (soc.max - Math.floor(soc.max) == 0.0) soc.max += Nodes.eps;
				if (soc.precision - Math.floor(soc.precision) == 0.0) soc.precision += Nodes.eps;
			}
			for (let but of n.buttons) {
				if (but.output == null) but.output = -1;
				if (but.min == null) but.min = 0.0;
				if (but.max == null) but.max = 1.0;
				if (but.precision == null) but.precision = 100.0;
				if (but.height == null) but.height = 0.0;
				if (but.height - Math.floor(but.height) == 0.0) but.height += Nodes.eps;
				if (but.min - Math.floor(but.min) == 0.0) but.min += Nodes.eps;
				if (but.max - Math.floor(but.max) == 0.0) but.max += Nodes.eps;
				if (but.precision - Math.floor(but.precision) == 0.0) but.precision += Nodes.eps;
			}
			if (n.width == null) n.width = 0;
		}
	}

	static node_replace: TNode[] = [];

	nodeCanvas(ui: Zui, canvas: TNodeCanvas) {
		Nodes.current = this;
		Nodes.currentCanvas = canvas;

		// Fill in optional values
		Nodes.updateCanvasFormat(canvas);

		// Ensure properties order
		let canvas_: TNodeCanvas = {
			name: canvas.name,
			nodes: canvas.nodes.slice(),
			nodes_count: canvas.nodes.length,
			links: canvas.links.slice(),
			links_count: canvas.links.length,
		}

		// Convert default data
		for (let n of canvas_.nodes) {
			for (let soc of n.inputs) {
				soc.default_value = Nodes.jsToC(soc.type, soc.default_value);
			}
			for (let soc of n.outputs) {
				soc.default_value = Nodes.jsToC(soc.type, soc.default_value);
			}
			for (let but of n.buttons) {
				but.default_value = Nodes.jsToC(but.type, but.default_value);
				but.data = Nodes.jsToCData(but.type, but.data);
			}
		}

		// Ensure properties order
		for (let n of canvas_.nodes) {
			n.name = Nodes.tr(n.name);
			for (let i = 0; i < n.inputs.length; ++i) {
				n.inputs[i] = {
					id: n.inputs[i].id,
					node_id: n.inputs[i].node_id,
					name: Nodes.tr(n.inputs[i].name),
					type: n.inputs[i].type,
					color: n.inputs[i].color,
					default_value: n.inputs[i].default_value,
					min: n.inputs[i].min,
					max: n.inputs[i].max,
					precision: n.inputs[i].precision,
					display: n.inputs[i].display,
				};
			}
			for (let i = 0; i < n.outputs.length; ++i) {
				n.outputs[i] = {
					id: n.outputs[i].id,
					node_id: n.outputs[i].node_id,
					name: Nodes.tr(n.outputs[i].name),
					type: n.outputs[i].type,
					color: n.outputs[i].color,
					default_value: n.outputs[i].default_value,
					min: n.outputs[i].min,
					max: n.outputs[i].max,
					precision: n.outputs[i].precision,
					display: n.outputs[i].display,
				};
			}
			for (let i = 0; i < n.buttons.length; ++i) {
				n.buttons[i] = {
					name: Nodes.tr(n.buttons[i].name),
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

		let packed = Krom.zui_node_canvas(this.nodes_, ArmPack.encode(canvas_));
		canvas_ = ArmPack.decode(packed);
		if (canvas_.nodes == null) canvas_.nodes = [];
		if (canvas_.links == null) canvas_.links = [];

		// Convert default data
		for (let n of canvas_.nodes) {
			for (let soc of n.inputs) {
				soc.default_value = Nodes.cToJs(soc.type, soc.default_value);
			}
			for (let soc of n.outputs) {
				soc.default_value = Nodes.cToJs(soc.type, soc.default_value);
			}
			for (let but of n.buttons) {
				but.default_value = Nodes.cToJs(but.type, but.default_value);
				but.data = Nodes.cToJsData(but.type, but.data);
			}
		}

		canvas.name = canvas_.name;
		canvas.nodes = canvas_.nodes;
		canvas.links = canvas_.links;

		// Restore nodes modified in js while Krom.zui_node_canvas was running
		for (let n of Nodes.node_replace) {
			for (let i = 0; i < canvas.nodes.length; ++i) {
				if (canvas.nodes[i].id == n.id) {
					canvas.nodes[i] = n;
					break;
				}
			}
		}
		Nodes.node_replace = [];

		this.ELEMENT_H = ui.t.ELEMENT_H + 2;
	}

	rgbaPopup(ui: Zui, nhandle: Handle, val: Float32Array, x: i32, y: i32) {
		Krom.zui_nodes_rgba_popup(nhandle.handle_, val.buffer, x, y);
	}

	removeNode(n: TNode, canvas: TNodeCanvas) {
		if (n == null) return;
		let i = 0;
		while (i < canvas.links.length) {
			let l = canvas.links[i];
			if (l.from_id == n.id || l.to_id == n.id) {
				canvas.links.splice(i, 1);
			}
			else i++;
		}
		array_remove(canvas.nodes, n);
		// if (onNodeRemove != null) {
		// 	onNodeRemove(n);
		// }
	}

	SCALE(): f32 {
		return Krom.zui_nodes_scale();
	}

	PAN_X(): f32 {
		return Krom.zui_nodes_pan_x();
	}

	PAN_Y(): f32 {
		return Krom.zui_nodes_pan_y();
	}

	NODE_H(canvas: TNodeCanvas, node: TNode): i32 {
		return Math.floor(this.LINE_H() * 1.2 + this.INPUTS_H(canvas, node.inputs) + this.OUTPUTS_H(node.outputs) + this.BUTTONS_H(node));
	}

	NODE_W(node: TNode): i32 {
		return Math.floor((node.width != 0 ? node.width : 140) * this.SCALE());
	}

	NODE_X(node: TNode): f32 {
		return node.x * this.SCALE() + this.PAN_X();
	}

	NODE_Y(node: TNode): f32 {
		return node.y * this.SCALE() + this.PAN_Y();
	}

	BUTTONS_H(node: TNode): i32 {
		let h = 0.0;
		for (let but of node.buttons) {
			if (but.type == "RGBA") h += 102 * this.SCALE() + this.LINE_H() * 5; // Color wheel + controls
			else if (but.type == "VECTOR") h += this.LINE_H() * 4;
			else if (but.type == "CUSTOM") h += this.LINE_H() * but.height;
			else h += this.LINE_H();
		}
		return Math.floor(h);
	}

	OUTPUTS_H(sockets: TNodeSocket[], length: Null<i32> = null): i32 {
		let h = 0.0;
		for (let i = 0; i < (length == null ? sockets.length : length); ++i) {
			h += this.LINE_H();
		}
		return Math.floor(h);
	}

	inputLinked(canvas: TNodeCanvas, node_id: i32, i: i32): bool {
		for (let l of canvas.links) if (l.to_id == node_id && l.to_socket == i) return true;
		return false;
	}

	INPUTS_H(canvas: TNodeCanvas, sockets: TNodeSocket[], length: Null<i32> = null): i32 {
		let h = 0.0;
		for (let i = 0; i < (length == null ? sockets.length : length); ++i) {
			if (sockets[i].type == "VECTOR" && sockets[i].display == 1 && !this.inputLinked(canvas, sockets[i].node_id, i)) h += this.LINE_H() * 4;
			else h += this.LINE_H();
		}
		return Math.floor(h);
	}

	INPUT_Y(canvas: TNodeCanvas, sockets: TNodeSocket[], pos: i32): i32 {
		return Math.floor(this.LINE_H() * 1.62) + this.INPUTS_H(canvas, sockets, pos);
	}

	OUTPUT_Y(sockets: TNodeSocket[], pos: i32): i32 {
		return Math.floor(this.LINE_H() * 1.62) + this.OUTPUTS_H(sockets, pos);
	}

	LINE_H(): i32 {
		return Math.floor(this.ELEMENT_H * this.SCALE());
	}

	p(f: f32): f32 {
		return f * this.SCALE();
	}
}

type CanvasControl = {
	panX: f32;
	panY: f32;
	zoom: f32;
}

type TNodeCanvas = {
	name: string;
	nodes: TNode[];
	nodes_count?: i32;
	links: TNodeLink[];
	links_count?: i32;
}

type TNode = {
	id: i32;
	name: string;
	type: string;
	x: f32;
	y: f32;
	color: i32;
	inputs: TNodeSocket[];
	outputs: TNodeSocket[];
	buttons: TNodeButton[];
	width?: f32;
}

type TNodeSocket = {
	id: i32;
	node_id: i32;
	name: string;
	type: string;
	color: i32;
	default_value: any;
	min?: f32;
	max?: f32;
	precision?: f32;
	display?: i32;
}

type TNodeLink = {
	id: i32;
	from_id: i32;
	from_socket: i32;
	to_id: i32;
	to_socket: i32;
}

type TNodeButton = {
	name: string;
	type: string;
	output?: i32;
	default_value?: any;
	data?: any;
	min?: f32;
	max?: f32;
	precision?: f32;
	height?: f32;
}
