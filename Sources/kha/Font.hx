package kha;

class Font {

	public var font_: Dynamic = null;
	public var blob: Blob;
	public var fontGlyphs: Array<Int> = null;
	public var fontIndex = 0;

	public function new(blob: Blob, fontIndex = 0) {
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
		kha.graphics2.Graphics.fontGlyphs = kha.graphics2.Graphics.fontGlyphs.copy(); // Trigger atlas update
	}

	public function clone(): Font {
		return new Font(blob, fontIndex);
	}

	public function init() {
		if (fontGlyphs != kha.graphics2.Graphics.fontGlyphs) {
			fontGlyphs = kha.graphics2.Graphics.fontGlyphs;
			font_ = Krom.g2_font_init(blob.bytes.getData(), fontIndex);
		}
	}
}
