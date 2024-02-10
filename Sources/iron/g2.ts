
let _g2_color: Color;
let _g2_font: g2_font_t;
let _g2_font_size: i32 = 0;
let _g2_pipeline: pipeline_t;
let _g2_transformation: mat3_t = null;
let _g2_render_target: image_t;

let _g2_current: image_t = null;
let _g2_font_glyphs: i32[] = _g2_make_glyphs(32, 127);
let _g2_font_glyphs_last: i32[] = _g2_font_glyphs;
let _g2_thrown = false;
let _g2_mat = new Float32Array(9);
let _g2_initialized = false;

function g2_set_color(c: Color) {
	krom_g2_set_color(c);
	_g2_color = c;
}

function g2_set_font_and_size(font: g2_font_t, font_size: i32) {
	g2_font_init(font);
	krom_g2_set_font(font.font_, font_size);
}

function g2_set_font(f: g2_font_t) {
	if (_g2_font_size != 0) {
		g2_set_font_and_size(f, _g2_font_size);
	}
	_g2_font = f;
}

function g2_set_font_size(i: i32) {
	if (_g2_font.font_ != null) {
		g2_set_font_and_size(_g2_font, i);
	}
	_g2_font_size = i;
}

function g2_set_pipeline(p: pipeline_t) {
	krom_g2_set_pipeline(p == null ? null : p.pipeline_);
	_g2_pipeline = p;
}

function g2_set_bilinear_filter(bilinear: bool) {
	krom_g2_set_bilinear_filter(bilinear);
}

function g2_set_transformation(m: mat3_t) {
	if (m == null) {
		krom_g2_set_transform(null);
	}
	else {
		_g2_mat[0] = m.m[0];
		_g2_mat[1] = m.m[1];
		_g2_mat[2] = m.m[2];
		_g2_mat[3] = m.m[3];
		_g2_mat[4] = m.m[4];
		_g2_mat[5] = m.m[5];
		_g2_mat[6] = m.m[6];
		_g2_mat[7] = m.m[7];
		_g2_mat[8] = m.m[8];
		krom_g2_set_transform(_g2_mat.buffer);
	}
}

function _g2_make_glyphs(start: i32, end: i32): i32[] {
	let ar: i32[] = [];
	for (let i = start; i < end; ++i) {
		ar.push(i);
	}
	return ar;
}

function g2_init() {
	if (!_g2_initialized) {
		krom_g2_init(
			sys_get_shader_buffer("painter-image.vert"),
			sys_get_shader_buffer("painter-image.frag"),
			sys_get_shader_buffer("painter-colored.vert"),
			sys_get_shader_buffer("painter-colored.frag"),
			sys_get_shader_buffer("painter-text.vert"),
			sys_get_shader_buffer("painter-text.frag")
		);
		_g2_initialized = true;
	}
}

function g2_draw_scaled_sub_image(img: image_t, sx: f32, sy: f32, sw: f32, sh: f32, dx: f32, dy: f32, dw: f32, dh: f32) {
	krom_g2_draw_scaled_sub_image(img, sx, sy, sw, sh, dx, dy, dw, dh);
}

function g2_draw_sub_image(img: image_t, x: f32, y: f32, sx: f32, sy: f32, sw: f32, sh: f32) {
	g2_draw_scaled_sub_image(img, sx, sy, sw, sh, x, y, sw, sh);
}

function g2_draw_scaled_image(img: image_t, dx: f32, dy: f32, dw: f32, dh: f32) {
	g2_draw_scaled_sub_image(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
}

function g2_draw_image(img: image_t, x: f32, y: f32) {
	g2_draw_scaled_sub_image(img, 0, 0, img.width, img.height, x, y, img.width, img.height);
}

function g2_draw_rect(x: f32, y: f32, width: f32, height: f32, strength: f32 = 1.0) {
	krom_g2_draw_rect(x, y, width, height, strength);
}

function g2_fill_rect(x: f32, y: f32, width: f32, height: f32) {
	krom_g2_fill_rect(x, y, width, height);
}

function g2_draw_string(text: string, x: f32, y: f32) {
	krom_g2_draw_string(text, x, y);
}

function g2_draw_line(x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0) {
	krom_g2_draw_line(x0, y0, x1, y1, strength);
}

function g2_fill_triangle(x0: f32, y0: f32, x1: f32, y1: f32, x2: f32, y2: f32) {
	krom_g2_fill_triangle(x0, y0, x1, y1, x2, y2);
}

function g2_scissor(x: i32, y: i32, width: i32, height: i32) {
	krom_g2_end(); // flush
	g4_scissor(x, y, width, height);
}

function g2_disable_scissor() {
	krom_g2_end(); // flush
	g4_disable_scissor();
}

function g2_begin(render_target: image_t = null, clear = true, clear_color: Color = null) {
	if (_g2_current == null) {
		_g2_current = render_target;
	}
	else {
		if (!_g2_thrown) {
			_g2_thrown = true;
			throw "End before you begin";
		}
	}

	krom_g2_begin();

	if (render_target != null) {
		krom_g2_set_render_target(render_target.render_target_);
	}
	else {
		krom_g2_restore_render_target();
	}

	if (clear) {
		g2_clear(clear_color);
	}
}

function g2_clear(color = 0x00000000) {
	g4_clear(color);
}

function g2_end() {
	krom_g2_end();

	if (_g2_current != null) {
		_g2_current = null;
	}
	else {
		if (!_g2_thrown) {
			_g2_thrown = true;
			throw "Begin before you end";
		}
	}
}

function g2_fill_circle(cx: f32, cy: f32, radius: f32, segments: i32 = 0) {
	krom_g2_fill_circle(cx, cy, radius, segments);
}

function g2_draw_circle(cx: f32, cy: f32, radius: f32, segments: i32 = 0, strength: f32 = 1.0) {
	krom_g2_draw_circle(cx, cy, radius, segments, strength);
}

function g2_draw_cubic_bezier(x: f32[], y: f32[], segments: i32 = 20, strength: f32 = 1.0) {
	krom_g2_draw_cubic_bezier(x, y, segments, strength);
}

function g2_font_init(raw: g2_font_t) {
	if (_g2_font_glyphs_last != _g2_font_glyphs) {
		_g2_font_glyphs_last = _g2_font_glyphs;
		krom_g2_font_set_glyphs(_g2_font_glyphs);
	}
	if (raw.glyphs != _g2_font_glyphs) {
		raw.glyphs = _g2_font_glyphs;
		raw.font_ = krom_g2_font_init(raw.blob, raw.index);
	}
}

function g2_font_create(blob: ArrayBuffer, index = 0): g2_font_t {
	let raw = new g2_font_t();
	raw.blob = blob;
	raw.index = index;
	return raw;
}

function g2_font_height(raw: g2_font_t, size: i32): f32 {
	g2_font_init(raw);
	return krom_g2_font_height(raw.font_, size);
}

function g2_font_width(raw: g2_font_t, size: i32, str: string): f32 {
	g2_font_init(raw);
	return krom_g2_string_width(raw.font_, size, str);
}

function g2_font_unload(raw: g2_font_t) {
	raw.blob = null;
}

function g2_font_set_font_index(raw: g2_font_t, index: i32) {
	raw.index = index;
	_g2_font_glyphs = _g2_font_glyphs.slice(); // Trigger atlas update
}

function g2_font_clone(raw: g2_font_t): g2_font_t {
	return g2_font_create(raw.blob, raw.index);
}

class g2_font_t {
	font_: any;
	blob: ArrayBuffer;
	glyphs: i32[];
	index: i32;
}
