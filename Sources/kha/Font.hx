package kha;

class Font {

	public var font_: Dynamic = null;
	public var blob: js.lib.ArrayBuffer;
	public var fontGlyphs: Array<Int> = null;
	public var fontIndex = 0;

	public function new(blob: js.lib.ArrayBuffer, fontIndex = 0) {
		this.blob = blob;
		this.fontIndex = fontIndex;
	}

	public function height(fontSize: Int): Float {
		init();
		return Krom.g2_font_height(font_, fontSize);
	}

	public function width(fontSize: Int, str: String): Float {
		init();
		return Krom.g2_string_width(font_, fontSize, str);
	}

	public function unload() {
		blob = null;
	}

	public function setFontIndex(fontIndex: Int) {
		this.fontIndex = fontIndex;
		kha.Graphics2.fontGlyphs = kha.Graphics2.fontGlyphs.copy(); // Trigger atlas update
	}

	public function clone(): Font {
		return new Font(blob, fontIndex);
	}

	public function init() {
		if (kha.Graphics2.fontGlyphsLast != kha.Graphics2.fontGlyphs) {
			kha.Graphics2.fontGlyphsLast = kha.Graphics2.fontGlyphs;
			Krom.g2_font_set_glyphs(kha.Graphics2.fontGlyphs);
		}
		if (fontGlyphs != kha.Graphics2.fontGlyphs) {
			fontGlyphs = kha.Graphics2.fontGlyphs;
			font_ = Krom.g2_font_init(blob, fontIndex);
		}
	}
}
