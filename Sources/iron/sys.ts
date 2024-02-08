
let _sys_render_listeners: (()=>void)[] = [];
let _sys_foreground_listeners: (()=>void)[] = [];
let _sys_resume_listeners: (()=>void)[] = [];
let _sys_pause_listeners: (()=>void)[] = [];
let _sys_background_listeners: (()=>void)[] = [];
let _sys_shutdown_listeners: (()=>void)[] = [];
let _sys_drop_files_listeners: ((s: string)=>void)[] = [];
let _sys_cut_listener: ()=>string = null;
let _sys_copy_listener: ()=>string = null;
let _sys_paste_listener: (data: string)=>void = null;

let _sys_start_time: f32;
let _sys_window_title: string;
let _sys_shaders: Map<string, shader_t> = new Map();

function sys_start(ops: kinc_sys_ops_t, callback: ()=>void) {
	Krom.init(ops.title, ops.width, ops.height, ops.vsync, ops.mode, ops.features, ops.x, ops.y, ops.frequency);

	_sys_start_time = Krom.getTime();
	g2_init();
	Krom.setCallback(sys_render_callback);
	Krom.setDropFilesCallback(sys_drop_files_callback);
	Krom.setCutCopyPasteCallback(sys_cut_callback, sys_copy_callback, sys_paste_callback);
	Krom.setApplicationStateCallback(sys_foreground_callback, sys_resume_callback, sys_pause_callback, sys_background_callback, sys_shutdown_callback);
	Krom.setKeyboardDownCallback(sys_keyboard_down_callback);
	Krom.setKeyboardUpCallback(sys_keyboard_up_callback);
	Krom.setKeyboardPressCallback(sys_keyboard_press_callback);
	Krom.setMouseDownCallback(sys_mouse_down_callback);
	Krom.setMouseUpCallback(sys_mouse_up_callback);
	Krom.setMouseMoveCallback(sys_mouse_move_callback);
	Krom.setMouseWheelCallback(sys_mouse_wheel_callback);
	Krom.setTouchDownCallback(sys_touch_down_callback);
	Krom.setTouchUpCallback(sys_touch_up_callback);
	Krom.setTouchMoveCallback(sys_touch_move_callback);
	Krom.setPenDownCallback(sys_pen_down_callback);
	Krom.setPenUpCallback(sys_pen_up_callback);
	Krom.setPenMoveCallback(sys_pen_move_callback);
	Krom.setGamepadAxisCallback(sys_gamepad_axis_callback);
	Krom.setGamepadButtonCallback(sys_gamepad_button_callback);
	input_register();

	callback();
}

function sys_notify_on_frames(listener: ()=>void) {
	_sys_render_listeners.push(listener);
}

function sys_notify_on_app_state(on_foreground: ()=>void, on_resume: ()=>void, on_pause: ()=>void, on_background: ()=>void, on_shutdown: ()=>void) {
	if (on_foreground != null) {
		_sys_foreground_listeners.push(on_foreground);
	}
	if (on_resume != null) {
		_sys_resume_listeners.push(on_resume);
	}
	if (on_pause != null) {
		_sys_pause_listeners.push(on_pause);
	}
	if (on_background != null) {
		_sys_background_listeners.push(on_background);
	}
	if (on_shutdown != null) {
		_sys_shutdown_listeners.push(on_shutdown);
	}
}

function sys_notify_on_drop_files(dropFilesListener: (s: string)=>void) {
	_sys_drop_files_listeners.push(dropFilesListener);
}

function sys_notify_on_cut_copy_paste(on_cut: ()=>string, on_copy: ()=>string, on_paste: (data: string)=>void) {
	_sys_cut_listener = on_cut;
	_sys_copy_listener = on_copy;
	_sys_paste_listener = on_paste;
}

function sys_foreground() {
	for (let listener of _sys_foreground_listeners) {
		listener();
	}
}

function sys_resume() {
	for (let listener of _sys_resume_listeners) {
		listener();
	}
}

function sys_pause() {
	for (let listener of _sys_pause_listeners) {
		listener();
	}
}

function sys_background() {
	for (let listener of _sys_background_listeners) {
		listener();
	}
}

function sys_shutdown() {
	for (let listener of _sys_shutdown_listeners) {
		listener();
	}
}

function sys_drop_files(file_path: string) {
	for (let listener of _sys_drop_files_listeners) {
		listener(file_path);
	}
}

function sys_time(): f32 {
	return Krom.getTime() - _sys_start_time;
}

function sys_system_id(): string {
	return Krom.systemId();
}

function sys_language(): string {
	return Krom.language();
}

function sys_stop() {
	Krom.requestShutdown();
}

function sys_load_url(url: string) {
	Krom.loadUrl(url);
}

function sys_render_callback() {
	for (let listener of _sys_render_listeners) {
		listener();
	}
}

function sys_drop_files_callback(file_path: string) {
	sys_drop_files(file_path);
}

function sys_copy_callback(): string {
	if (_sys_copy_listener != null) {
		return _sys_copy_listener();
	}
	return null;
}

function sys_cut_callback(): string {
	if (_sys_cut_listener != null) {
		return _sys_cut_listener();
	}
	return null;
}

function sys_paste_callback(data: string) {
	if (_sys_paste_listener != null) {
		_sys_paste_listener(data);
	}
}

function sys_foreground_callback() {
	sys_foreground();
}

function sys_resume_callback() {
	sys_resume();
}

function sys_pause_callback() {
	sys_pause();
}

function sys_background_callback() {
	sys_background();
}

function sys_shutdown_callback() {
	sys_shutdown();
}

function sys_keyboard_down_callback(code: i32) {
	keyboard_down_listener(code);
}

function sys_keyboard_up_callback(code: i32) {
	keyboard_up_listener(code);
}

function sys_keyboard_press_callback(charCode: i32) {
	keyboard_press_listener(String.fromCharCode(charCode));
}

function sys_mouse_down_callback(button: i32, x: i32, y: i32) {
	mouse_down_listener(button, x, y);
}

function sys_mouse_up_callback(button: i32, x: i32, y: i32) {
	mouse_up_listener(button, x, y);
}

function sys_mouse_move_callback(x: i32, y: i32, mx: i32, my: i32) {
	mouse_move_listener(x, y, mx, my);
}

function sys_mouse_wheel_callback(delta: i32) {
	mouse_wheel_listener(delta);
}

function sys_touch_down_callback(index: i32, x: i32, y: i32) {
	///if (krom_android || krom_ios)
	mouse_on_touch_down(index, x, y);
	///end
}

function sys_touch_up_callback(index: i32, x: i32, y: i32) {
	///if (krom_android || krom_ios)
	mouse_on_touch_up(index, x, y);
	///end
}

function sys_touch_move_callback(index: i32, x: i32, y: i32) {
	///if (krom_android || krom_ios)
	mouse_on_touch_move(index, x, y);
	///end
}

function sys_pen_down_callback(x: i32, y: i32, pressure: f32) {
	pen_down_listener(x, y, pressure);
}

function sys_pen_up_callback(x: i32, y: i32, pressure: f32) {
	pen_up_listener(x, y, pressure);
}

function sys_pen_move_callback(x: i32, y: i32, pressure: f32) {
	pen_move_listener(x, y, pressure);
}

function sys_gamepad_axis_callback(gamepad: i32, axis: i32, value: f32) {
	gamepad_axis_listener(gamepad, axis, value);
}

function sys_gamepad_button_callback(gamepad: i32, button: i32, value: f32) {
	gamepad_button_listener(gamepad, button, value);
}

function sys_lock_mouse() {
	if (!sys_is_mouse_locked()){
		Krom.lockMouse();
	}
}

function sys_unlock_mouse() {
	if (sys_is_mouse_locked()){
		Krom.unlockMouse();
	}
}

function sys_can_lock_mouse(): bool {
	return Krom.canLockMouse();
}

function sys_is_mouse_locked(): bool {
	return Krom.isMouseLocked();
}

function sys_hide_system_cursor() {
	Krom.showMouse(false);
}

function sys_show_system_cursor() {
	Krom.showMouse(true);
}

function sys_resize(width: i32, height: i32) {
	Krom.resizeWindow(width, height);
}

function sys_move(x: i32, y: i32) {
	Krom.moveWindow(x, y);
}

function sys_x(): i32 {
	return Krom.windowX();
}

function sys_y(): i32 {
	return Krom.windowY();
}

function sys_width(): i32 {
	return Krom.windowWidth();
}

function sys_height(): i32 {
	return Krom.windowHeight();
}

function sys_mode(): window_mode_t {
	return Krom.getWindowMode();
}

function sys_mode_set(mode: window_mode_t) {
	Krom.setWindowMode(mode);
}

function sys_title(): string {
	return _sys_window_title;
}

function sys_title_set(value: string) {
	Krom.setWindowTitle(value);
	_sys_window_title = value;
}

function sys_display_primary_id(): i32 {
	for (let i = 0; i < Krom.displayCount(); ++i) {
		if (Krom.displayIsPrimary(i)) {
			return i;
		}
	}
	return 0;
}

function sys_display_width(): i32 {
	return Krom.displayWidth(sys_display_primary_id());
}

function sys_display_height(): i32 {
	return Krom.displayHeight(sys_display_primary_id());
}

function sys_display_frequency(): i32 {
	return Krom.displayFrequency(sys_display_primary_id());
}

function sys_buffer_to_string(b: ArrayBuffer): string {
	let str = "";
	let u8a = new Uint8Array(b);
	for (let i = 0; i < u8a.length; ++i) {
		str += String.fromCharCode(u8a[i]);
	}
	return str;
}

function sys_string_to_buffer(str: string): ArrayBuffer {
	let u8a = new Uint8Array(str.length);
	for (let i = 0; i < str.length; ++i) {
		u8a[i] = str.charCodeAt(i);
	}
	return u8a.buffer;
}

function sys_shader_ext(): string {
	///if krom_vulkan
	return ".spirv";
	///elseif (krom_android || krom_wasm)
	return ".essl";
	///elseif krom_opengl
	return ".glsl";
	///elseif krom_metal
	return ".metal";
	///else
	return ".d3d11";
	///end
}

function sys_get_shader_buffer(name: string): ArrayBuffer {
	///if arm_shader_embed
	let global: any = globalThis;
	return global["data/" + name + sys_shader_ext()];
	///else
	return Krom.loadBlob("data/" + name + sys_shader_ext());
	///end
}

function sys_get_shader(name: string): shader_t {
	let shader = _sys_shaders.get(name);
	if (shader == null) {
		shader = shader_create(
			sys_get_shader_buffer(name),
			name.endsWith(".frag") ? shader_type_t.FRAGMENT : name.endsWith(".vert") ? shader_type_t.VERTEX : shader_type_t.GEOMETRY);
		_sys_shaders.set(name, shader);
	}
	return shader;
}

function video_unload(self: video_t) {}

function sound_create(sound_: any): sound_t {
	let raw = new sound_t();
	raw.sound_ = sound_;
	return raw;
}

function sound_unload(raw: sound_t) {
	Krom.unloadSound(raw.sound_);
}

function shader_create(buffer: ArrayBuffer, type: shader_type_t): shader_t {
	let raw = new shader_t();
	if (buffer != null) {
		raw.shader_ = Krom.createShader(buffer, type);
	}
	return raw;
}

function shader_from_source(source: string, type: shader_type_t): shader_t {
	let shader = shader_create(null, 0);
	if (type == shader_type_t.VERTEX) {
		shader.shader_ = Krom.createVertexShaderFromSource(source);
	}
	else if (type == shader_type_t.FRAGMENT) {
		shader.shader_ = Krom.createFragmentShaderFromSource(source);
	}
	return shader;
}

function shader_delete(raw: shader_t) {
	Krom.deleteShader(raw.shader_);
}

function pipeline_create(): pipeline_t {
	let raw = new pipeline_t();
	raw.cull_mode = cull_mode_t.NONE;
	raw.depth_write = false;
	raw.depth_mode = compare_mode_t.ALWAYS;

	raw.blend_source = blend_factor_t.BLEND_ONE;
	raw.blend_dest = blend_factor_t.BLEND_ZERO;
	raw.alpha_blend_source = blend_factor_t.BLEND_ONE;
	raw.alpha_blend_dest = blend_factor_t.BLEND_ZERO;

	raw.color_write_masks_red = [];
	raw.color_write_masks_green = [];
	raw.color_write_masks_blue = [];
	raw.color_write_masks_alpha = [];
	for (let i = 0; i < 8; ++i) {
		raw.color_write_masks_red.push(true);
	}
	for (let i = 0; i < 8; ++i) {
		raw.color_write_masks_green.push(true);
	}
	for (let i = 0; i < 8; ++i) {
		raw.color_write_masks_blue.push(true);
	}
	for (let i = 0; i < 8; ++i) {
		raw.color_write_masks_alpha.push(true);
	}

	raw.color_attachment_count = 1;
	raw.color_attachments = [];
	for (let i = 0; i < 8; ++i) {
		raw.color_attachments.push(tex_format_t.RGBA32);
	}
	raw.depth_attachment = depth_format_t.NO_DEPTH;

	raw.pipeline_ = Krom.createPipeline();
	return raw;
}

function pipeline_get_depth_buffer_bits(format: depth_format_t): i32 {
	if (format == depth_format_t.NO_DEPTH) return 0;
	if (format == depth_format_t.DEPTH24) return 24;
	if (format == depth_format_t.DEPTH16) return 16;
	return 0;
}

function pipeline_delete(raw: pipeline_t) {
	Krom.deletePipeline(raw.pipeline_);
}

function pipeline_compile(raw: pipeline_t) {
	let structure0 = raw.input_layout.length > 0 ? raw.input_layout[0] : null;
	let structure1 = raw.input_layout.length > 1 ? raw.input_layout[1] : null;
	let structure2 = raw.input_layout.length > 2 ? raw.input_layout[2] : null;
	let structure3 = raw.input_layout.length > 3 ? raw.input_layout[3] : null;
	let gs = raw.geometry_shader != null ? raw.geometry_shader.shader_ : null;
	let color_attachments: i32[] = [];
	for (let i = 0; i < 8; ++i) {
		color_attachments.push(raw.color_attachments[i]);
	}
	Krom.compilePipeline(raw.pipeline_, structure0, structure1, structure2, structure3, raw.input_layout.length, raw.vertex_shader.shader_, raw.fragment_shader.shader_, gs, {
		cullMode: raw.cull_mode,
		depthWrite: raw.depth_write,
		depthMode: raw.depth_mode,
		blendSource: raw.blend_source,
		blendDestination: raw.blend_dest,
		alphaBlendSource: raw.alpha_blend_source,
		alphaBlendDestination: raw.alpha_blend_dest,
		colorWriteMaskRed: raw.color_write_masks_red,
		colorWriteMaskGreen: raw.color_write_masks_green,
		colorWriteMaskBlue: raw.color_write_masks_blue,
		colorWriteMaskAlpha: raw.color_write_masks_alpha,
		colorAttachmentCount: raw.color_attachment_count,
		colorAttachments: raw.color_attachments,
		depthAttachmentBits: pipeline_get_depth_buffer_bits(raw.depth_attachment),
		stencilAttachmentBits: 0
	});
}

function pipeline_set(raw: pipeline_t) {
	Krom.setPipeline(raw.pipeline_);
}

function pipeline_get_const_loc(raw: pipeline_t, name: string): kinc_const_loc_t {
	return Krom.getConstantLocation(raw.pipeline_, name);
}

function pipeline_get_tex_unit(raw: pipeline_t, name: string): kinc_tex_unit_t {
	return Krom.getTextureUnit(raw.pipeline_, name);
}

function vertex_buffer_create(vertex_count: i32, structure: vertex_struct_t, usage: usage_t, inst_data_step_rate: i32 = 0): vertex_buffer_t {
	let raw = new vertex_buffer_t();
	raw.vertex_count = vertex_count;
	raw.buffer_ = Krom.createVertexBuffer(vertex_count, structure.elements, usage, inst_data_step_rate);
	return raw;
}

function vertex_buffer_delete(raw: vertex_buffer_t) {
	Krom.deleteVertexBuffer(raw.buffer_);
}

function vertex_buffer_lock(raw: vertex_buffer_t): DataView {
	return new DataView(Krom.lockVertexBuffer(raw.buffer_, 0, raw.vertex_count));
}

function vertex_buffer_unlock(raw: vertex_buffer_t) {
	Krom.unlockVertexBuffer(raw.buffer_, raw.vertex_count);
}

function vertex_buffer_set(raw: vertex_buffer_t) {
	Krom.setVertexBuffer(raw.buffer_);
}

function vertex_struct_create(): vertex_struct_t {
	return new vertex_struct_t();
}

function vertex_struct_add(raw: vertex_struct_t, name: string, data: vertex_data_t) {
	raw.elements.push({ name: name, data: data });
}

function vertex_struct_byte_size(raw: vertex_struct_t): i32 {
	let byte_size = 0;
	for (let i = 0; i < raw.elements.length; ++i) {
		byte_size += vertex_struct_data_byte_size(raw.elements[i].data);
	}
	return byte_size;
}

function vertex_struct_data_byte_size(data: vertex_data_t): i32 {
	if (data == vertex_data_t.F32_1X) return 1 * 4;
	if (data == vertex_data_t.F32_2X) return 2 * 4;
	if (data == vertex_data_t.F32_3X) return 3 * 4;
	if (data == vertex_data_t.F32_4X) return 4 * 4;
	if (data == vertex_data_t.U8_4X_NORM) return 4 * 1;
	if (data == vertex_data_t.I16_2X_NORM) return 2 * 2;
	if (data == vertex_data_t.I16_4X_NORM) return 4 * 2;
	return 0;
}

function index_buffer_create(index_count: i32): index_buffer_t {
	let raw = new index_buffer_t();
	raw.buffer_ = Krom.createIndexBuffer(index_count);
	return raw;
}

function index_buffer_delete(raw: index_buffer_t) {
	Krom.deleteIndexBuffer(raw.buffer_);
}

function index_buffer_lock(raw: index_buffer_t): Uint32Array {
	return Krom.lockIndexBuffer(raw.buffer_);
}

function index_buffer_unlock(raw: index_buffer_t) {
	Krom.unlockIndexBuffer(raw.buffer_);
}

function index_buffer_set(raw: index_buffer_t) {
	Krom.setIndexBuffer(raw.buffer_);
}

function font_create(blob: ArrayBuffer, index = 0): font_t {
	let raw = new font_t();
	raw.blob = blob;
	raw.index = index;
	return raw;
}

function font_height(raw: font_t, size: i32): f32 {
	g2_font_init(raw);
	return Krom.g2_font_height(raw.font_, size);
}

function font_width(raw: font_t, size: i32, str: string): f32 {
	g2_font_init(raw);
	return Krom.g2_string_width(raw.font_, size, str);
}

function font_unload(raw: font_t) {
	raw.blob = null;
}

function font_set_font_index(raw: font_t, index: i32) {
	raw.index = index;
	_g2_font_glyphs = _g2_font_glyphs.slice(); // Trigger atlas update
}

function font_clone(raw: font_t): font_t {
	return font_create(raw.blob, raw.index);
}

function g2_font_init(raw: font_t) {
	if (_g2_font_glyphs_last != _g2_font_glyphs) {
		_g2_font_glyphs_last = _g2_font_glyphs;
		Krom.g2_font_set_glyphs(_g2_font_glyphs);
	}
	if (raw.glyphs != _g2_font_glyphs) {
		raw.glyphs = _g2_font_glyphs;
		raw.font_ = Krom.g2_font_init(raw.blob, raw.index);
	}
}

function _image_create(tex: any): image_t {
	let raw = new image_t();
	raw.texture_ = tex;
	return raw;
}

function image_get_depth_buffer_bits(format: depth_format_t): i32 {
	if (format == depth_format_t.NO_DEPTH) return -1;
	if (format == depth_format_t.DEPTH24) return 24;
	if (format == depth_format_t.DEPTH16) return 16;
	return 0;
}

function image_get_tex_format(format: tex_format_t): i32 {
	if (format == tex_format_t.RGBA32) return 0;
	if (format == tex_format_t.RGBA128) return 3;
	if (format == tex_format_t.RGBA64) return 4;
	if (format == tex_format_t.R32) return 5;
	if (format == tex_format_t.R16) return 7;
	return 1; // R8
}

function image_from_texture(tex: any): image_t {
	return _image_create(tex);
}

function image_from_bytes(buffer: ArrayBuffer, width: i32, height: i32, format: tex_format_t = null, usage: usage_t = null): image_t {
	if (format == null) {
		format = tex_format_t.RGBA32;
	}
	let readable = true;
	let image = _image_create(null);
	image.format = format;
	image.texture_ = Krom.createTextureFromBytes(buffer, width, height, image_get_tex_format(format), readable);
	return image;
}

function image_from_bytes_3d(buffer: ArrayBuffer, width: i32, height: i32, depth: i32, format: tex_format_t = null, usage: usage_t = null): image_t {
	if (format == null) {
		format = tex_format_t.RGBA32;
	}
	let readable = true;
	let image = _image_create(null);
	image.format = format;
	image.texture_ = Krom.createTextureFromBytes3D(buffer, width, height, depth, image_get_tex_format(format), readable);
	return image;
}

function image_from_encoded_bytes(buffer: ArrayBuffer, format: string, done: (img: image_t)=>void, readable: bool = false) {
	let image = _image_create(null);
	image.texture_ = Krom.createTextureFromEncodedBytes(buffer, format, readable);
	done(image);
}

function image_create(width: i32, height: i32, format: tex_format_t = null, usage: usage_t = null): image_t {
	if (format == null) {
		format = tex_format_t.RGBA32;
	}
	let image = _image_create(null);
	image.format = format;
	image.texture_ = Krom.createTexture(width, height, image_get_tex_format(format));
	return image;
}

function image_create_3d(width: i32, height: i32, depth: i32, format: tex_format_t = null, usage: usage_t = null): image_t {
	if (format == null) {
		format = tex_format_t.RGBA32;
	}
	let image = _image_create(null);
	image.format = format;
	image.texture_ = Krom.createTexture3D(width, height, depth, image_get_tex_format(format));
	return image;
}

function image_create_render_target(width: i32, height: i32, format: tex_format_t = null, depth_format: depth_format_t = depth_format_t.NO_DEPTH): image_t {
	if (format == null) {
		format = tex_format_t.RGBA32;
	}
	let image = _image_create(null);
	image.format = format;
	image.render_target_ = Krom.createRenderTarget(width, height, format, image_get_depth_buffer_bits(depth_format), 0);
	return image;
}

function image_render_targets_inv_y(): bool {
	return Krom.renderTargetsInvertedY();
}

function image_format_byte_size(format: tex_format_t): i32 {
	if (format == tex_format_t.RGBA32) return 4;
	if (format == tex_format_t.R8) return 1;
	if (format == tex_format_t.RGBA128) return 16;
	if (format == tex_format_t.DEPTH16) return 2;
	if (format == tex_format_t.RGBA64) return 8;
	if (format == tex_format_t.R32) return 4;
	if (format == tex_format_t.R16) return 2;
	return 4;
}

function image_unload(raw: image_t) {
	Krom.unloadImage(raw);
	raw.texture_ = null;
	raw.render_target_ = null;
}

function image_lock(raw: image_t, level: i32 = 0): ArrayBuffer {
	return Krom.lockTexture(raw.texture_, level);
}

function image_unlock(raw: image_t) {
	Krom.unlockTexture(raw.texture_);
}

function image_get_pixels(raw: image_t): ArrayBuffer {
	if (raw.render_target_ != null) {
		// Minimum size of 32x32 required after https://github.com/Kode/Kinc/commit/3797ebce5f6d7d360db3331eba28a17d1be87833
		let pixels_w = raw.width < 32 ? 32 : raw.width;
		let pixels_h = raw.height < 32 ? 32 : raw.height;
		if (raw.pixels == null) {
			raw.pixels = new ArrayBuffer(image_format_byte_size(raw.format) * pixels_w * pixels_h);
		}
		Krom.getRenderTargetPixels(raw.render_target_, raw.pixels);
		return raw.pixels;
	}
	else {
		return Krom.getTexturePixels(raw.texture_);
	}
}

function image_gen_mipmaps(raw: image_t, levels: i32) {
	raw.texture_ == null ? Krom.generateRenderTargetMipmaps(raw.render_target_, levels) : Krom.generateTextureMipmaps(raw.texture_, levels);
}

function image_set_mipmaps(raw: image_t, mipmaps: image_t[]) {
	Krom.setMipmaps(raw.texture_, mipmaps);
}

function image_set_depth_from(raw: image_t, image: image_t) {
	Krom.setDepthStencilFrom(raw.render_target_, image.render_target_);
}

function image_clear(raw: image_t, x: i32, y: i32, z: i32, width: i32, height: i32, depth: i32, color: Color) {
	Krom.clearTexture(raw.texture_, x, y, z, width, height, depth, color);
}

let _g2_color: Color;
let _g2_font: font_t;
let _g2_font_size: i32 = 0;
let _g2_pipeline: pipeline_t;
let _g2_transformation: mat3_t = null;
let _g2_render_target: image_t;

function g2_set_color(c: Color) {
	Krom.g2_set_color(c);
	_g2_color = c;
}

function g2_set_font_and_size(font: font_t, font_size: i32) {
	g2_font_init(font);
	Krom.g2_set_font(font.font_, font_size);
}

function g2_set_font(f: font_t) {
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
	Krom.g2_set_pipeline(p == null ? null : p.pipeline_);
	_g2_pipeline = p;
}

function g2_set_bilinear_filter(bilinear: bool) {
	Krom.g2_set_bilinear_filter(bilinear);
}

function g2_set_transformation(m: mat3_t) {
	if (m == null) {
		Krom.g2_set_transform(null);
	}
	else {
		_g2_mat[0] = m._00; _g2_mat[1] = m._01; _g2_mat[2] = m._02;
		_g2_mat[3] = m._10; _g2_mat[4] = m._11; _g2_mat[5] = m._12;
		_g2_mat[6] = m._20; _g2_mat[7] = m._21; _g2_mat[8] = m._22;
		Krom.g2_set_transform(_g2_mat.buffer);
	}
}

function _g2_make_glyphs(start: i32, end: i32): i32[] {
	let ar: i32[] = [];
	for (let i = start; i < end; ++i) {
		ar.push(i);
	}
	return ar;
}

let _g2_current: image_t = null;
let _g2_font_glyphs: i32[] = _g2_make_glyphs(32, 127);
let _g2_font_glyphs_last: i32[] = _g2_font_glyphs;
let _g2_thrown = false;
let _g2_mat = new Float32Array(9);
let _g2_initialized = false;

function g2_init() {
	if (!_g2_initialized) {
		Krom.g2_init(
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
	Krom.g2_draw_scaled_sub_image(img, sx, sy, sw, sh, dx, dy, dw, dh);
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
	Krom.g2_draw_rect(x, y, width, height, strength);
}

function g2_fill_rect(x: f32, y: f32, width: f32, height: f32) {
	Krom.g2_fill_rect(x, y, width, height);
}

function g2_draw_string(text: string, x: f32, y: f32) {
	Krom.g2_draw_string(text, x, y);
}

function g2_draw_line(x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0) {
	Krom.g2_draw_line(x0, y0, x1, y1, strength);
}

function g2_fill_triangle(x0: f32, y0: f32, x1: f32, y1: f32, x2: f32, y2: f32) {
	Krom.g2_fill_triangle(x0, y0, x1, y1, x2, y2);
}

function g2_scissor(x: i32, y: i32, width: i32, height: i32) {
	Krom.g2_end(); // flush
	g4_scissor(x, y, width, height);
}

function g2_disable_scissor() {
	Krom.g2_end(); // flush
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

	Krom.g2_begin();

	if (render_target != null) {
		Krom.g2_set_render_target(render_target.render_target_);
	}
	else {
		Krom.g2_restore_render_target();
	}

	if (clear) {
		g2_clear(clear_color);
	}
}

function g2_clear(color = 0x00000000) {
	g4_clear(color);
}

function g2_end() {
	Krom.g2_end();

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
	Krom.g2_fill_circle(cx, cy, radius, segments);
}

function g2_draw_circle(cx: f32, cy: f32, radius: f32, segments: i32 = 0, strength: f32 = 1.0) {
	Krom.g2_draw_circle(cx, cy, radius, segments, strength);
}

function g2_draw_cubic_bezier(x: f32[], y: f32[], segments: i32 = 20, strength: f32 = 1.0) {
	Krom.g2_draw_cubic_bezier(x, y, segments, strength);
}

function g4_begin(render_target: image_t, additional_targets: image_t[] = null) {
	Krom.begin(render_target, additional_targets);
}

function g4_end() {
	Krom.end();
}

function g4_clear(color?: Color, depth?: f32) {
	let flags: i32 = 0;
	if (color != null) {
		flags |= 1;
	}
	if (depth != null) {
		flags |= 2;
	}
	Krom.clear(flags, color == null ? 0 : color, depth, 0);
}

function g4_viewport(x: i32, y: i32, width: i32, height: i32) {
	Krom.viewport(x, y, width, height);
}

function g4_set_vertex_buffer(vb: vertex_buffer_t) {
	vertex_buffer_set(vb);
}

function g4_set_vertex_buffers(vbs: vertex_buffer_t[]) {
	Krom.setVertexBuffers(vbs);
}

function g4_set_index_buffer(ib: index_buffer_t) {
	index_buffer_set(ib);
}

function g4_set_tex(unit: kinc_tex_unit_t, tex: image_t) {
	if (tex == null) {
		return;
	}
	tex.texture_ != null ? Krom.setTexture(unit, tex.texture_) : Krom.setRenderTarget(unit, tex.render_target_);
}

function g4_set_tex_depth(unit: kinc_tex_unit_t, tex: image_t) {
	if (tex == null) {
		return;
	}
	Krom.setTextureDepth(unit, tex.render_target_);
}

function g4_set_image_tex(unit: kinc_tex_unit_t, tex: image_t) {
	if (tex == null) {
		return;
	}
	Krom.setImageTexture(unit, tex.texture_);
}

function g4_set_tex_params(tex_unit: kinc_tex_unit_t, u_addressing: tex_addressing, v_addressing: tex_addressing, minification_filter: tex_filter_t, magnification_filter: tex_filter_t, mipmap_filter: mip_map_filter_t) {
	Krom.setTextureParameters(tex_unit, u_addressing, v_addressing, minification_filter, magnification_filter, mipmap_filter);
}

function g4_set_tex_3d_params(tex_unit: kinc_tex_unit_t, u_addressing: tex_addressing, v_addressing: tex_addressing, w_addressing: tex_addressing, minification_filter: tex_filter_t, magnification_filter: tex_filter_t, mipmap_filter: mip_map_filter_t) {
	Krom.setTexture3DParameters(tex_unit, u_addressing, v_addressing, w_addressing, minification_filter, magnification_filter, mipmap_filter);
}

function g4_set_pipeline(pipe: pipeline_t) {
	pipeline_set(pipe);
}

function g4_set_bool(loc: kinc_const_loc_t, value: bool) {
	Krom.setBool(loc, value);
}

function g4_set_int(loc: kinc_const_loc_t, value: i32) {
	Krom.setInt(loc, value);
}

function g4_set_float(loc: kinc_const_loc_t, value: f32) {
	Krom.setFloat(loc, value);
}

function g4_set_float2(loc: kinc_const_loc_t, value1: f32, value2: f32) {
	Krom.setFloat2(loc, value1, value2);
}

function g4_set_float3(loc: kinc_const_loc_t, value1: f32, value2: f32, value3: f32) {
	Krom.setFloat3(loc, value1, value2, value3);
}

function g4_set_float4(loc: kinc_const_loc_t, value1: f32, value2: f32, value3: f32, value4: f32) {
	Krom.setFloat4(loc, value1, value2, value3, value4);
}

function g4_set_floats(loc: kinc_const_loc_t, values: Float32Array) {
	Krom.setFloats(loc, values.buffer);
}

function g4_set_vec2(loc: kinc_const_loc_t, v: vec2_t) {
	Krom.setFloat2(loc, v.x, v.y);
}

function g4_set_vec3(loc: kinc_const_loc_t, v: vec3_t) {
	Krom.setFloat3(loc, v.x, v.y, v.z);
}

function g4_set_vec4(loc: kinc_const_loc_t, v: vec4_t) {
	Krom.setFloat4(loc, v.x, v.y, v.z, v.w);
}

function g4_set_mat(loc: kinc_const_loc_t, mat: mat4_t) {
	Krom.setMatrix(loc, mat.buffer.buffer);
}

function g4_set_mat3(loc: kinc_const_loc_t, mat: mat3_t) {
	Krom.setMatrix3(loc, mat.buffer.buffer);
}

function g4_draw(start: i32 = 0, count: i32 = -1) {
	Krom.drawIndexedVertices(start, count);
}

function g4_draw_inst(inst_count: i32, start: i32 = 0, count: i32 = -1) {
	Krom.drawIndexedVerticesInstanced(inst_count, start, count);
}

function g4_scissor(x: i32, y: i32, width: i32, height: i32) {
	Krom.scissor(x, y, width, height);
}

function g4_disable_scissor() {
	Krom.disableScissor();
}

type Color = i32;

class video_t {
	video_: any;
}

class sound_t {
	sound_: any;
}

class shader_t {
	shader_: any;
}

class pipeline_t {
	pipeline_: any;
	input_layout: vertex_struct_t[];
	vertex_shader: shader_t;
	fragment_shader: shader_t;
	geometry_shader: shader_t;
	cull_mode: cull_mode_t;
	depth_write: bool;
	depth_mode: compare_mode_t;
	blend_source: blend_factor_t;
	blend_dest: blend_factor_t;
	alpha_blend_source: blend_factor_t;
	alpha_blend_dest: blend_factor_t;
	color_write_masks_red: bool[];
	color_write_masks_green: bool[];
	color_write_masks_blue: bool[];
	color_write_masks_alpha: bool[];
	color_attachment_count: i32;
	color_attachments: tex_format_t[];
	depth_attachment: depth_format_t;
}

class vertex_buffer_t {
	buffer_: any;
	vertex_count: i32;
}

class vertex_struct_t {
	elements: kinc_vertex_elem_t[] = [];
	instanced: bool = false;
}

class index_buffer_t {
	buffer_: any;
}

class font_t {
	font_: any;
	blob: ArrayBuffer;
	glyphs: i32[];
	index = 0;
}

class image_t {
	texture_: any;
	render_target_: any;
	format: tex_format_t;
	readable: bool;
	pixels: ArrayBuffer;

	get width(): i32 { return this.texture_ == null ? this.render_target_.width : this.texture_.width; }
	get height(): i32 { return this.texture_ == null ? this.render_target_.height : this.texture_.height; }
	get depth(): i32 { return this.texture_ != null ? this.texture_.depth : 1; }
}

type kinc_sys_ops_t = {
	title: string;
	x: i32;
	y: i32;
	width: i32;
	height: i32;
	features: window_features_t;
	mode: window_mode_t;
	frequency: i32;
	vsync: bool;
}

type kinc_vertex_elem_t = {
	name: string;
	data: vertex_data_t;
}

type kinc_const_loc_t = any;
type kinc_tex_unit_t = any;

enum tex_filter_t {
	POINT,
	LINEAR,
	ANISOTROPIC,
}

enum mip_map_filter_t {
	NONE,
	POINT,
	LINEAR,
}

enum tex_addressing {
	REPEAT,
	MIRROR,
	CLAMP,
}

enum usage_t {
	STATIC,
	DYNAMIC,
	READABLE,
}

enum tex_format_t {
	RGBA32,
	RGBA64,
	R32,
	RGBA128,
	DEPTH16,
	R8,
	R16,
}

enum depth_format_t {
	NO_DEPTH,
	DEPTH16,
	DEPTH24,
}

enum vertex_data_t {
	F32_1X = 1,
	F32_2X = 2,
	F32_3X = 3,
	F32_4X = 4,
	U8_4X_NORM = 17,
	I16_2X_NORM = 24,
	I16_4X_NORM = 28,
}

enum blend_factor_t {
	BLEND_ONE,
	BLEND_ZERO,
	SOURCE_ALPHA,
	DEST_ALPHA,
	INV_SOURCE_ALPHA,
	INV_DEST_ALPHA,
	SOURCE_COLOR,
	DEST_COLOR,
	INV_SOURCE_COLOR,
	INV_DEST_COLOR,
}

enum compare_mode_t {
	ALWAYS,
	NEVER,
	EQUAL,
	NOT_EQUAL,
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
}

enum cull_mode_t {
	CLOCKWISE,
	COUNTER_CLOCKWISE,
	NONE,
}

enum shader_type_t {
	FRAGMENT = 0,
	VERTEX = 1,
	GEOMETRY = 3,
}

enum window_features_t {
    NONE = 0,
    RESIZABLE = 1,
    MINIMIZABLE = 2,
    MAXIMIZABLE = 4,
}

enum window_mode_t {
	WINDOWED,
	FULLSCREEN,
}
