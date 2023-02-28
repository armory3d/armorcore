#include "g2_ext.h"

#include <math.h>
#include "g2.h"

#define MATH_PI 3.14159265358979323846

void g2_fill_circle(float cx, float cy, float radius, int segments/* = 0*/) {
	if (segments <= 0) {
		segments = (int)floor(10 * sqrt(radius));
	}

	float theta = 2 * (float)MATH_PI / segments;
	float c = (float)cos(theta);
	float s = (float)sin(theta);

	float x = radius;
	float y = 0.0;

	for (int n = 0; n < segments; ++n) {
		float px = x + cx;
		float py = y + cy;

		float t = x;
		x = c * x - s * y;
		y = c * y + s * t;

		g2_fill_triangle(px, py, x + cx, y + cy, cx, cy);
	}
}

void g2_calculate_cubic_bezier_point(float t, float *x, float *y, float *out) {
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	// first term
	out[0] = uuu * x[0];
	out[1] = uuu * y[0];

	// second term
	out[0] += 3 * uu * t * x[1];
	out[1] += 3 * uu * t * y[1];

	// third term
	out[0] += 3 * u * tt * x[2];
	out[1] += 3 * u * tt * y[2];

	// fourth term
	out[0] += ttt * x[3];
	out[1] += ttt * y[3];
}

/**
 * Draws a cubic bezier using 4 pairs of points. If the x and y arrays have a length bigger then 4, the additional
 * points will be ignored. With a length smaller of 4 a error will occur, there is no check for this.
 * You can construct the curves visually in Inkscape with a path using default nodes.
 * Provide x and y in the following order: startPoint, controlPoint1, controlPoint2, endPoint
 * Reference: http://devmag.org.za/2011/04/05/bzier-curves-a-tutorial/
 */
void g2_draw_cubic_bezier(float *x, float *y, int segments/* = 20*/, float strength/* = 1.0*/) {
	float q0[2];
	float q1[2];
	g2_calculate_cubic_bezier_point(0, x, y, q0);

	for (int i = 0; i < (segments + 1); ++i) {
		float t = (float)(i / segments);
		g2_calculate_cubic_bezier_point(t, x, y, q1);
		g2_draw_line(q0[0], q0[1], q1[0], q1[1], strength);
		q0[0] = q1[0];
		q0[1] = q1[1];
	}
}
