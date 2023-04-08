package kha.graphics2;

import kha.graphics4.PipelineState;
import kha.math.FastMatrix3;
import kha.Canvas;
import kha.Color;
import kha.Font;
import kha.Image;
import kha.Shaders;

class Graphics {

	public static var current: Graphics;
	public static var fontGlyphs: Array<Int> = [for (i in 32...127) i];
	public static var fontGlyphsLast: Array<Int> = fontGlyphs;
	static var thrown = false;
	static var mat = new kha.arrays.Float32Array(9);
	static var initialized = false;

	public var color(default, set): Color;
	public var font(default, set): Font;
	public var fontSize(default, set): Int = 0;
	public var pipeline(default, set): PipelineState;
	public var imageScaleQuality(default, set): ImageScaleQuality;
	public var transformation(default, set): FastMatrix3 = null;
	var canvas: Canvas;

	public function new(canvas: Canvas) {
		if (!initialized) {
			Krom.g2_init(Shaders.getBuffer("painter-image.vert"), Shaders.getBuffer("painter-image.frag"), Shaders.getBuffer("painter-colored.vert"), Shaders.getBuffer("painter-colored.frag"), Shaders.getBuffer("painter-text.vert"), Shaders.getBuffer("painter-text.frag"));
			initialized = true;
		}
		this.canvas = canvas;
	}

	function set_color(c: Color): Color {
		Krom.g2_set_color(c);
		return color = c;
	}

	function set_font_and_size(font: Font, fontSize: Int) {
		font.init();
		Krom.g2_set_font(font.font_, fontSize);
	}

	function set_font(f: Font): Font {
		if (fontSize != 0) set_font_and_size(f, fontSize);
		return font = f;
	}

	function set_fontSize(i: Int): Int {
		if (font.font_ != null) set_font_and_size(font, i);
		return fontSize = i;
	}

	function set_pipeline(p: PipelineState): PipelineState {
		Krom.g2_set_pipeline(p == null ? null : p.pipeline);
		return pipeline = p;
	}

	function set_imageScaleQuality(q: ImageScaleQuality): ImageScaleQuality {
		Krom.g2_set_bilinear_filter(q == High);
		return imageScaleQuality = q;
	}

	function set_transformation(m: FastMatrix3): FastMatrix3 {
		if (m == null) {
			Krom.g2_set_transform(null);
		}
		else {
			mat[0] = m._00; mat[1] = m._01; mat[2] = m._02;
			mat[3] = m._10; mat[4] = m._11; mat[5] = m._12;
			mat[6] = m._20; mat[7] = m._21; mat[8] = m._22;
			Krom.g2_set_transform(mat.buffer);
		}
		return transformation = m;
	}

	public function drawScaledSubImage(img: kha.Image, sx: Float, sy: Float, sw: Float, sh: Float, dx: Float, dy: Float, dw: Float, dh: Float): Void {
		Krom.g2_draw_scaled_sub_image(img, sx, sy, sw, sh, dx, dy, dw, dh);
	}

	public function drawSubImage(img: Image, x: Float, y: Float, sx: Float, sy: Float, sw: Float, sh: Float): Void {
		drawScaledSubImage(img, sx, sy, sw, sh, x, y, sw, sh);
	}

	public function drawScaledImage(img: Image, dx: Float, dy: Float, dw: Float, dh: Float): Void {
		drawScaledSubImage(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
	}

	public function drawImage(img: kha.Image, x: Float, y: Float): Void {
		drawScaledSubImage(img, 0, 0, img.width, img.height, x, y, img.width, img.height);
	}

	public function drawRect(x: Float, y: Float, width: Float, height: Float, strength: Float = 1.0): Void {
		Krom.g2_draw_rect(x, y, width, height, strength);
	}

	public function fillRect(x: Float, y: Float, width: Float, height: Float): Void {
		Krom.g2_fill_rect(x, y, width, height);
	}

	public function drawString(text: String, x: Float, y: Float): Void {
		Krom.g2_draw_string(text, x, y);
	}

	public function drawLine(x0: Float, y0: Float, x1: Float, y1: Float, strength: Float = 1.0): Void {
		Krom.g2_draw_line(x0, y0, x1, y1, strength);
	}

	public function fillTriangle(x0: Float, y0: Float, x1: Float, y1: Float, x2: Float, y2: Float) {
		Krom.g2_fill_triangle(x0, y0, x1, y1, x2, y2);
	}

	public function scissor(x: Int, y: Int, width: Int, height: Int): Void {
		Krom.g2_end(); // flush
		canvas.g4.scissor(x, y, width, height);
	}

	public function disableScissor(): Void {
		Krom.g2_end(); // flush
		canvas.g4.disableScissor();
	}

	public function begin(clear = true, clearColor: Color = null): Void {
		if (current == null) {
			current = this;
		}
		else {
			if (!thrown) { thrown = true; throw "End before you begin"; }
		}

		Krom.g2_begin();

		if (Std.isOfType(canvas, Image)) {
			Krom.g2_set_render_target(untyped canvas.renderTarget_);
		}
		else {
			Krom.g2_restore_render_target();
		}

		if (clear) this.clear(clearColor);
	}

	public function clear(color = Color.Black): Void {
		canvas.g4.clear(color);
	}

	public function end(): Void {
		Krom.g2_end();

		if (current == this) {
			current = null;
		}
		else {
			if (!thrown) { thrown = true; throw "Begin before you end"; }
		}
	}

	public static function createTextVertexStructure(): kha.graphics4.VertexStructure {
		return null;
	}

	public static function createTextPipeline(structure: kha.graphics4.VertexStructure): Dynamic {
		return { compile: function() {} };
	}
}
