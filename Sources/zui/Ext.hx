package zui;

import zui.Zui;

class Ext {

	public static var textAreaLineNumbers = false;
	public static var textAreaScrollPastEnd = false;
	public static var textAreaColoring: TTextColoring = null;

	public static function floatInput(ui: Zui, handle: Handle, label = "", align: Align = Left, precision = 1000.0): Float {
		return Krom.zui_float_input(handle.handle_, label, align, precision);
	}

	public static function inlineRadio(ui: Zui, handle: Handle, texts: Array<String>, align: Align = Left): Int {
		return Krom.zui_inline_radio(handle.handle_, texts, align);
	}

	public static function colorWheel(ui: Zui, handle: Handle, alpha = false, w: Null<Float> = null, h: Null<Float> = null, colorPreview = true, picker: Void->Void = null): kha.Color {
		return Krom.zui_color_wheel(handle.handle_, alpha, w != null ? w : 0, h != null ? h : 0, colorPreview, picker);
	}

	public static function textArea(ui: Zui, handle: Handle, align = Align.Left, editable = true, label = "", wordWrap = false): String {
		return Krom.zui_text_area(handle.handle_, align, editable, label, wordWrap);
	}

	public static function beginMenu(ui: Zui) {
		Krom.zui_begin_menu();
	}

	public static function endMenu(ui: Zui) {
		Krom.zui_end_menu();
	}

	public static function menuButton(ui: Zui, text: String): Bool {
		return Krom.zui_menu_button(text);
	}

	public static function MENUBAR_H(ui: Zui): Float {
		// return ui.BUTTON_H() * 1.1 + 2 + ui.buttonOffsetY; ////
		return 30;
	}
}
