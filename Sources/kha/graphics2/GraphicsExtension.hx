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

	/**
	 * Draws a cubic bezier using 4 pairs of points. If the x and y arrays have a length bigger then 4, the additional
	 * points will be ignored. With a length smaller of 4 a error will occur, there is no check for this.
	 * You can construct the curves visually in Inkscape with a path using default nodes.
	 * Provide x and y in the following order: startPoint, controlPoint1, controlPoint2, endPoint
	 * Reference: http://devmag.org.za/2011/04/05/bzier-curves-a-tutorial/
	 */
	public static function drawCubicBezier(g2:Graphics, x:Array<Float>, y:Array<Float>, segments:Int = 20, strength:Float = 1.0):Void {
		var t:Float;

		var q0 = calculateCubicBezierPoint(0, x, y);
		var q1:Array<Float>;

		for (i in 1...(segments + 1)) {
			t = i / segments;
			q1 = calculateCubicBezierPoint(t, x, y);
			g2.drawLine(q0[0], q0[1], q1[0], q1[1], strength);
			q0 = q1;
		}
	}

	static function calculateCubicBezierPoint(t:Float, x:Array<Float>, y:Array<Float>):Array<Float> {
		var u:Float = 1 - t;
		var tt:Float = t * t;
		var uu:Float = u * u;
		var uuu:Float = uu * u;
		var ttt:Float = tt * t;

		// first term
		var p:Array<Float> = [uuu * x[0], uuu * y[0]];

		// second term
		p[0] += 3 * uu * t * x[1];
		p[1] += 3 * uu * t * y[1];

		// third term
		p[0] += 3 * u * tt * x[2];
		p[1] += 3 * u * tt * y[2];

		// fourth term
		p[0] += ttt * x[3];
		p[1] += ttt * y[3];

		return p;
	}
}
