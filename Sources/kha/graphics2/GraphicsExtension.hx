package kha.graphics2;

class GraphicsExtension {

	public static function fillCircle(g2: Graphics, cx: Float, cy: Float, radius: Float, segments: Int = 0): Void {
		Krom.g2_fill_circle(cx, cy, radius, segments);
	}

	public static function drawCubicBezier(g2: Graphics, x: Array<Float>, y: Array<Float>, segments: Int = 20, strength: Float = 1.0): Void {
		Krom.g2_draw_cubic_bezier(x, y, segments, strength);
	}
}
