package kha.graphics2;

import kha.graphics2.Graphics;

/**
 * Static extension functions for Graphics2.
 * Usage: "using kha.graphics2.GraphicsExtension;"
 */
class GraphicsExtension {

	/**
	 * Draws a filled circle.
	 * @param	segments (optional) The amount of lines that should be used to draw the circle.
	 */
	public static function fillCircle(g2: Graphics, cx: Float, cy: Float, radius: Float, segments: Int = 0): Void {
		if (segments <= 0) {
			segments = Math.floor(10 * Math.sqrt(radius));
		}

		var theta = 2 * Math.PI / segments;
		var c = Math.cos(theta);
		var s = Math.sin(theta);

		var x = radius;
		var y = 0.0;

		for (n in 0...segments) {
			var px = x + cx;
			var py = y + cy;

			var t = x;
			x = c * x - s * y;
			y = c * y + s * t;

			g2.fillTriangle(px, py, x + cx, y + cy, cx, cy);
		}
	}
}
