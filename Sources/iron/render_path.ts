
class render_target_t {
	name: string;
	width: i32;
	height: i32;
	// Optional:
	format: string;
	scale: f32;
	depth_buffer: string; // 2D texture
	mipmaps: bool;
	depth: i32; // 3D texture
	is_image: bool; // Image
	// Runtime:
	depth_format: depth_format_t;
	depth_from: string;
	image: image_t; // RT or image
	has_depth: bool;
}

class cached_shader_context_t {
	context: shader_context_t;
}

enum draw_order_t {
	DIST, // Early-z
	SHADER, // Less state changes
}

let render_path_commands: ()=>void = null;
let render_path_render_targets: Map<string, render_target_t> = new Map();
let render_path_current_w: i32;
let render_path_current_h: i32;
let render_path_current_d: i32;
let _render_path_frame_time = 0.0;
let _render_path_frame = 0;
let _render_path_current_target: render_target_t = null;
let _render_path_light: light_object_t = null;
let _render_path_sun: light_object_t = null;
let _render_path_point: light_object_t = null;
let _render_path_current_image: image_t = null;
let _render_path_draw_order = draw_order_t.DIST;
let _render_path_paused = false;
let _render_path_depth_to_render_target: Map<string, render_target_t> = new Map();
let _render_path_last_w = 0;
let _render_path_last_h = 0;
let _render_path_bind_params: string[];
let _render_path_meshes_sorted: bool;
let _render_path_scissor_set = false;
let _render_path_last_frame_time = 0.0;
let _render_path_loading = 0;
let _render_path_cached_shader_contexts: Map<string, cached_shader_context_t> = new Map();
let _render_path_depth_buffers: { name: string, format: string }[] = [];

function render_path_ready(): bool {
	return _render_path_loading == 0;
}

///if arm_voxels
let _render_path_voxelized = 0;
function render_path_voxelize() { // Returns true if scene should be voxelized
	return ++_render_path_voxelized > 2 ? false : true;
}
///end

function render_path_render_frame() {
	if (!render_path_ready() ||_render_path_paused || app_w() == 0 || app_h() == 0) {
		return;
	}

	if (_render_path_last_w > 0 && (_render_path_last_w != app_w() ||_render_path_last_h != app_h())) {
		render_path_resize();
	}
	_render_path_last_w = app_w();
	_render_path_last_h = app_h();

	_render_path_frame_time = time_time() -_render_path_last_frame_time;
	_render_path_last_frame_time = time_time();

	render_path_current_w = app_w();
	render_path_current_h = app_h();
	render_path_current_d = 1;
	_render_path_meshes_sorted = false;

	for (let l of scene_lights) {
		if (l.base.visible) {
			light_object_build_mat(l, scene_camera);
		}
		if (l.data.type == "sun") {
			_render_path_sun = l;
		}
		else {
			_render_path_point = l;
		}
	}
	_render_path_light = scene_lights[0];

	render_path_commands();

	_render_path_frame++;
}

function render_path_set_target(target: string, additional: string[] = null) {
	if (target == "") { // Framebuffer
		render_path_current_d = 1;
		_render_path_current_target = null;
		render_path_current_w = app_w();
		render_path_current_h = app_h();
		render_path_begin();
		render_path_set_current_viewport(app_w(), app_h());
		render_path_set_current_scissor(app_w(), app_h());
	}
	else { // Render target
		let rt = render_path_render_targets.get(target);
		_render_path_current_target = rt;
		let additional_images: image_t[] = null;
		if (additional != null) {
			additional_images = [];
			for (let s of additional) {
				let t = render_path_render_targets.get(s);
				additional_images.push(t.image);
			}
		}
		render_path_current_w = rt.image.width;
		render_path_current_h = rt.image.height;
		if (rt.depth > 1) {
			render_path_current_d = rt.image.depth;
		}
		render_path_begin(rt.image, additional_images);
	}
	_render_path_bind_params = null;
}

function render_path_set_depth_from(target: string, from: string) {
	let rt = render_path_render_targets.get(target);
	image_set_depth_from(rt.image,render_path_render_targets.get(from).image);
}

function render_path_begin(render_target: image_t = null, additional_targets: image_t[] = null) {
	if (_render_path_current_image != null) {
		render_path_end();
	}
	_render_path_current_image = render_target;
	g4_begin(render_target, additional_targets);
}

function render_path_end() {
	if (_render_path_scissor_set) {
		g4_disable_scissor();
		_render_path_scissor_set = false;
	}
	g4_end();
	_render_path_current_image = null;
	_render_path_bind_params = null;
}

function render_path_set_current_viewport(view_w: i32, view_h: i32) {
	g4_viewport(app_x(),render_path_current_h - (view_h - app_y()), view_w, view_h);
}

function render_path_set_current_scissor(view_w: i32, view_h: i32) {
	g4_scissor(app_x(),render_path_current_h - (view_h - app_y()), view_w, view_h);
	_render_path_scissor_set = true;
}

function render_path_set_viewport(view_w: i32, view_h: i32) {
	render_path_set_current_viewport(view_w, view_h);
	render_path_set_current_scissor(view_w, view_h);
}

function render_path_clear_target(color_flag?: i32, depth_flag?: f32) {
	if (color_flag == -1) { // -1 == 0xffffffff
		if (scene_world != null) {
			color_flag = scene_world.background_color;
		}
		else if (scene_camera != null) {
			let cc = scene_camera.data.clear_color;
			if (cc != null) {
				color_flag = color_from_floats(cc[0], cc[1], cc[2]);
			}
		}
	}
	g4_clear(color_flag, depth_flag);
}

function render_path_clear_image(target: string, color: i32) {
	let rt = render_path_render_targets.get(target);
	image_clear(rt.image, 0, 0, 0, rt.image.width, rt.image.height, rt.image.depth, color);
}

function render_path_gen_mipmaps(target: string) {
	let rt = render_path_render_targets.get(target);
	image_gen_mipmaps(rt.image, 1000);
}

function render_path_sort_meshes_dist(meshes: mesh_object_t[]) {
	meshes.sort(function (a: mesh_object_t, b: mesh_object_t): i32 {
		return a.camera_dist >= b.camera_dist ? 1 : -1;
	});
}

function render_path_sort_meshes_shader(meshes: mesh_object_t[]) {
	meshes.sort(function (a: mesh_object_t, b: mesh_object_t): i32 {
		return a.materials[0].name >= b.materials[0].name ? 1 : -1;
	});
}

function render_path_draw_meshes(context: string) {
	render_path_submit_draw(context);
	render_path_end();
}

function render_path_submit_draw(context: string) {
	let camera = scene_camera;
	let meshes = scene_meshes;
	_mesh_object_last_pipeline = null;

	if (!_render_path_meshes_sorted && camera != null) { // Order max once per frame for now
		let cam_x = transform_world_x(camera.base.transform);
		let cam_y = transform_world_y(camera.base.transform);
		let cam_z = transform_world_z(camera.base.transform);
		for (let mesh of meshes) {
			mesh_object_compute_camera_dist(mesh, cam_x, cam_y, cam_z);
		}
		_render_path_draw_order == draw_order_t.SHADER ? render_path_sort_meshes_shader(meshes) : render_path_sort_meshes_dist(meshes);
		_render_path_meshes_sorted = true;
	}

	for (let m of meshes) {
		mesh_object_render(m, context, _render_path_bind_params);
	}
}

function render_path_draw_skydome(handle: string) {
	if (const_data_skydome_vb == null) {
		const_data_create_skydome_data();
	}
	let cc: cached_shader_context_t = _render_path_cached_shader_contexts.get(handle);
	if (cc.context == null) {
		return; // World data not specified
	}
	g4_set_pipeline(cc.context._pipe_state);
	uniforms_set_context_consts(cc.context, _render_path_bind_params);
	uniforms_set_obj_consts(cc.context, null); // External hosek
	g4_set_vertex_buffer(const_data_skydome_vb);
	g4_set_index_buffer(const_data_skydome_ib);
	g4_draw();
	render_path_end();
}

function render_path_bind_target(target: string, uniform: string) {
	if (_render_path_bind_params != null) {
		_render_path_bind_params.push(target);
		_render_path_bind_params.push(uniform);
	}
	else {
		_render_path_bind_params = [target, uniform];
	}
}

// Full-screen triangle
function render_path_draw_shader(handle: string) {
	// file/data_name/context
	let cc: cached_shader_context_t = _render_path_cached_shader_contexts.get(handle);
	if (const_data_screen_aligned_vb == null) {
		const_data_create_screen_aligned_data();
	}
	g4_set_pipeline(cc.context._pipe_state);
	uniforms_set_context_consts(cc.context,_render_path_bind_params);
	uniforms_set_obj_consts(cc.context, null);
	g4_set_vertex_buffer(const_data_screen_aligned_vb);
	g4_set_index_buffer(const_data_screen_aligned_ib);
	g4_draw();

	render_path_end();
}

function render_path_load_shader(handle: string) {
	_render_path_loading++;
	let cc: cached_shader_context_t = _render_path_cached_shader_contexts.get(handle);
	if (cc != null) {
		_render_path_loading--;
		return;
	}

	cc = new cached_shader_context_t();
	_render_path_cached_shader_contexts.set(handle, cc);

	// file/data_name/context
	let shader_path = handle.split("/");

	data_get_shader(shader_path[0], shader_path[1], function (res: shader_data_t) {
		cc.context = shader_data_get_context(res, shader_path[2]);
		_render_path_loading--;
	});
}

function render_path_unload_shader(handle: string) {
	_render_path_cached_shader_contexts.delete(handle);

	// file/data_name/context
	let shader_path = handle.split("/");
	// Todo: Handle context overrides (see data_get_shader())
	data_cached_shaders.delete(shader_path[1]);
}

function render_path_unload() {
	for (let rt of render_path_render_targets.values()) {
		render_target_unload(rt);
	}
}

function render_path_resize() {
	if (sys_width() == 0 || sys_height() == 0) {
		return;
	}

	// Make sure depth buffer is attached to single target only and gets released once
	for (let rt of render_path_render_targets.values()) {
		if (rt == null || rt.width > 0 || rt.depth_from == "" || rt == _render_path_depth_to_render_target.get(rt.depth_from)) {
			continue;
		}

		let nodepth: render_target_t = null;
		for (let rt2 of render_path_render_targets.values()) {
			if (rt2 == null || rt2.width > 0 || rt2.depth_from != "" || _render_path_depth_to_render_target.get(rt2.depth_buffer) != null) {
				continue;
			}

			nodepth = rt2;
			break;
		}

		if (nodepth != null) {
			image_set_depth_from(rt.image, nodepth.image);
		}
	}

	// Resize textures
	for (let rt of render_path_render_targets.values()) {
		if (rt != null && rt.width == 0) {
			let _image = rt.image;
			app_notify_on_init(function () {
				image_unload(_image);
			});
			rt.image = render_path_create_image(rt, rt.depth_format);
		}
	}

	// Attach depth buffers
	for (let rt of render_path_render_targets.values()) {
		if (rt != null && rt.depth_from != "") {
			image_set_depth_from(rt.image,_render_path_depth_to_render_target.get(rt.depth_from).image);
		}
	}
}

function render_path_create_render_target(t: render_target_t): render_target_t {
	render_path_create_target(t);
	render_path_render_targets.set(t.name, t);
	return t;
}

function render_path_create_depth_buffer(name: string, format: string = null) {
	_render_path_depth_buffers.push({ name: name, format: format });
}

function render_path_create_target(t: render_target_t): render_target_t {
	// With depth buffer
	if (t.depth_buffer != null) {
		t.has_depth = true;
		let depth_target = _render_path_depth_to_render_target.get(t.depth_buffer);

		if (depth_target == null) { // Create new one
			for (let db of _render_path_depth_buffers) {
				if (db.name == t.depth_buffer) {
					_render_path_depth_to_render_target.set(db.name, t);
					t.depth_format = render_path_get_depth_format(db.format);
					t.image = render_path_create_image(t, t.depth_format);
					break;
				}
			}
		}
		else { // Reuse
			t.depth_format = depth_format_t.NO_DEPTH;
			t.depth_from = t.depth_buffer;
			t.image = render_path_create_image(t, t.depth_format);
			image_set_depth_from(t.image, depth_target.image);
		}
	}
	else { // No depth buffer
		t.has_depth = false;
		t.depth_format = depth_format_t.NO_DEPTH;
		t.image = render_path_create_image(t, t.depth_format);
	}
	return t;
}

function render_path_create_image(t: render_target_t, depth_format: depth_format_t): image_t {
	let width = t.width == 0 ? app_w() : t.width;
	let height = t.height == 0 ? app_h() : t.height;
	let depth = t.depth;
	width = Math.floor(width * t.scale);
	height = Math.floor(height * t.scale);
	depth = Math.floor(depth * t.scale);
	if (width < 1) {
		width = 1;
	}
	if (height < 1) {
		height = 1;
	}
	if (t.depth > 1) { // 3D texture
		// Image only
		let img = image_create_3d(width, height, depth,
			t.format != null ?render_path_get_tex_format(t.format) : tex_format_t.RGBA32);
		if (t.mipmaps) {
			image_gen_mipmaps(img, 1000); // Allocate mipmaps
		}
		return img;
	}
	else { // 2D texture
		if (t.is_image) { // Image
			return image_create(width, height,
				t.format != null ?render_path_get_tex_format(t.format) : tex_format_t.RGBA32);
		}
		else { // Render target
			return image_create_render_target(width, height,
				t.format != null ?render_path_get_tex_format(t.format) : tex_format_t.RGBA32,
				depth_format);
		}
	}
}

function render_path_get_tex_format(s: string): tex_format_t {
	if (s == "RGBA32") {
		return tex_format_t.RGBA32;
	}
	if (s == "RGBA64") {
		return tex_format_t.RGBA64;
	}
	if (s == "RGBA128") {
		return tex_format_t.RGBA128;
	}
	if (s == "DEPTH16") {
		return tex_format_t.DEPTH16;
	}
	if (s == "R32") {
		return tex_format_t.R32;
	}
	if (s == "R16") {
		return tex_format_t.R16;
	}
	if (s == "R8") {
		return tex_format_t.R8;
	}
	return tex_format_t.RGBA32;
}

function render_path_get_depth_format(s: string): depth_format_t {
	if (s == null || s == "") {
		return depth_format_t.DEPTH24;
	}
	if (s == "DEPTH24") {
		return depth_format_t.DEPTH24;
	}
	if (s == "DEPTH16") {
		return depth_format_t.DEPTH16;
	}
	return depth_format_t.DEPTH24;
}

function render_target_create(): render_target_t {
	let raw = new render_target_t();
	raw.scale = 1.0;
	raw.mipmaps = false;
	raw.depth = 1;
	raw.is_image = false;
	raw.depth_from = "";
	raw.has_depth = false;
	return raw;
}

function render_target_unload(raw: render_target_t) {
	if (raw.image != null) {
		image_unload(raw.image);
	}
}
