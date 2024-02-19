
// Global data list
let data_cached_scene_raws: map_t<string, scene_t> = new map_t();
let data_cached_meshes: map_t<string, mesh_data_t> = new map_t();
let data_cached_lights: map_t<string, light_data_t> = new map_t();
let data_cached_cameras: map_t<string, camera_data_t> = new map_t();
let data_cached_materials: map_t<string, material_data_t> = new map_t();
let data_cached_particles: map_t<string, particle_data_t> = new map_t();
let data_cached_worlds: map_t<string, world_data_t> = new map_t();
let data_cached_shaders: map_t<string, shader_data_t> = new map_t();

let data_cached_blobs: map_t<string, buffer_t> = new map_t();
let data_cached_images: map_t<string, image_t> = new map_t();
let data_cached_videos: map_t<string, video_t> = new map_t();
let data_cached_fonts: map_t<string, g2_font_t> = new map_t();
///if arm_audio
let data_cached_sounds: map_t<string, sound_t> = new map_t();
///end
let data_assets_loaded: i32 = 0;

function data_get_mesh(file: string, name: string): mesh_data_t {
	let handle: string = file + name;
	let cached: mesh_data_t = data_cached_meshes.get(handle);
	if (cached != null) {
		return cached;
	}

	let b: mesh_data_t = mesh_data_parse(file, name);
	data_cached_meshes.set(handle, b);
	b._handle = handle;
	return b;
}

function data_get_light(file: string, name: string): light_data_t {
	let handle: string = file + name;
	let cached: light_data_t = data_cached_lights.get(handle);
	if (cached != null) {
		return cached;
	}

	let b: light_data_t = light_data_parse(file, name);
	data_cached_lights.set(handle, b);
	return b;
}

function data_get_camera(file: string, name: string): camera_data_t {
	let handle: string = file + name;
	let cached: camera_data_t = data_cached_cameras.get(handle);
	if (cached != null) {
		return cached;
	}

	let b: camera_data_t = camera_data_parse(file, name);
	data_cached_cameras.set(handle, b);
	return b;
}

function data_get_material(file: string, name: string): material_data_t {
	let handle: string = file + name;
	let cached: material_data_t = data_cached_materials.get(handle);
	if (cached != null) {
		return cached;
	}

	let b: material_data_t = material_data_parse(file, name);
	data_cached_materials.set(handle, b);
	return b;
}

function data_get_particle(file: string, name: string): particle_data_t {
	let handle: string = file + name;
	let cached: particle_data_t = data_cached_particles.get(handle);
	if (cached != null) {
		return cached;
	}

	let b: particle_data_t = particle_data_parse(file, name);
	data_cached_particles.set(handle, b);
	return b;
}

function data_get_world(file: string, name: string): world_data_t {
	if (name == null) { // No world defined in scene
		return null;
	}

	let handle: string = file + name;
	let cached: world_data_t = data_cached_worlds.get(handle);
	if (cached != null) {
		return cached;
	}

	let b: world_data_t = world_data_parse(file, name);
	data_cached_worlds.set(handle, b);
	return b;
}

function data_get_shader(file: string, name: string, override_context: shader_override_t = null): shader_data_t {
	// Only one context override per shader data for now
	let handle: string = name;
	if (override_context != null) {
		handle += "2";
	}
	let cached: shader_data_t = data_cached_shaders.get(handle); // Shader must have unique name
	if (cached != null) {
		return cached;
	}

	let b: shader_data_t = shader_data_parse(file, name, override_context);
	data_cached_shaders.set(handle, b);
	return b;
}

function data_get_scene_raw(file: string): scene_t {
	let cached: scene_t = data_cached_scene_raws.get(file);
	if (cached != null) {
		return cached;
	}

	// If no extension specified, set to .arm
	let ext: string = ends_with(file, ".arm") ? "" : ".arm";
	let b: buffer_t = data_get_blob(file + ext);
	let parsed: scene_t = null;
	parsed = armpack_decode(b);
	data_cached_scene_raws.set(file, parsed);
	return parsed;
}

// Raw assets
function data_get_blob(file: string): buffer_t {
	let cached: buffer_t = data_cached_blobs.get(file);
	if (cached != null) {
		return cached;
	}

	let b: buffer_t = krom_load_blob(data_resolve_path(file));
	data_cached_blobs.set(file, b);
	data_assets_loaded++;
	return b;
}

function data_get_image(file: string, readable: bool = false): image_t {
	let cached: image_t = data_cached_images.get(file);
	if (cached != null) {
		return cached;
	}

	///if arm_image_embed
	let image_blob: buffer_t = data_cached_blobs.get(file);
	if (image_blob != null) {
		let b: image_t = image_from_encoded_bytes(image_blob, ".k", readable);
		data_cached_images.set(file, b);
		data_assets_loaded++;
		return b;
	}
	///end

	let image_: any = krom_load_image(data_resolve_path(file), readable);
	if (image_ == null) {
		return null;
	}

	let b: image_t = image_from_texture(image_);
	data_cached_images.set(file, b);
	data_assets_loaded++;
	return b;
}

function data_get_video(file: string): video_t {
	file = substring(file, 0, file.length - 4) + ".webm";
	let cached: video_t = data_cached_videos.get(file);
	if (cached != null) {
		return cached;
	}

	// let b: video_t = krom_load_video(data_resolve_path(file));
	// data_cached_videos.set(file, b);
	// data_assets_loaded++;
	// return b;
	return null;
}

function data_get_font(file: string): g2_font_t {
	let cached: g2_font_t = data_cached_fonts.get(file);
	if (cached != null) {
		return cached;
	}

	let blob: buffer_t = krom_load_blob(data_resolve_path(file));
	let b: g2_font_t = g2_font_create(blob);
	data_cached_fonts.set(file, b);
	data_assets_loaded++;
	return b;
}

///if arm_audio
function data_get_sound(file: string): sound_t {
	let cached: sound_t = data_cached_sounds.get(file);
	if (cached != null) {
		return cached;
	}

	let b: sound_t = sound_create(krom_load_sound(data_resolve_path(file)));
	data_cached_sounds.set(file, b);
	data_assets_loaded++;
	return b;
}
///end

function data_delete_mesh(handle: string) {
	let mesh: mesh_data_t = data_cached_meshes.get(handle);
	if (mesh == null) {
		return;
	}
	mesh_data_delete(mesh);
	data_cached_meshes.delete(handle);
}

function data_delete_blob(handle: string) {
	let blob: buffer_t = data_cached_blobs.get(handle);
	if (blob == null) {
		return;
	}
	data_cached_blobs.delete(handle);
}

function data_delete_image(handle: string) {
	let image: image_t = data_cached_images.get(handle);
	if (image == null) {
		return;
	}
	image_unload(image);
	data_cached_images.delete(handle);
}

function data_delete_video(handle: string) {
	let video: video_t = data_cached_videos.get(handle);
	if (video == null) {
		return;
	}
	video_unload(video);
	data_cached_videos.delete(handle);
}

function data_delete_font(handle: string) {
	let font: g2_font_t = data_cached_fonts.get(handle);
	if (font == null) {
		return;
	}
	g2_font_unload(font);
	data_cached_fonts.delete(handle);
}

///if arm_audio
function data_delete_sound(handle: string) {
	let sound: sound_t = data_cached_sounds.get(handle);
	if (sound == null) {
		return;
	}
	sound_unload(sound);
	data_cached_sounds.delete(handle);
}
///end

function data_delete_all() {
	let cached_meshes: mesh_data_t[] = map_to_array(data_cached_meshes);
	for (let i: i32 = 0; i < cached_meshes.length; ++i) {
		let c: mesh_data_t = cached_meshes[i];
		mesh_data_delete(c);
	}
	data_cached_meshes = new map_t();

	let cached_shaders: shader_data_t[] = map_to_array(data_cached_shaders);
	for (let i: i32 = 0; i < cached_shaders.length; ++i) {
		let c: shader_data_t = cached_shaders[i];
		shader_data_delete(c);
	}
	data_cached_shaders = new map_t();

	data_cached_scene_raws = new map_t();
	data_cached_lights = new map_t();
	data_cached_cameras = new map_t();
	data_cached_materials = new map_t();
	data_cached_particles = new map_t();
	data_cached_worlds = new map_t();
	render_path_unload();
	data_cached_blobs = new map_t();

	let cached_images: image_t[] = map_to_array(data_cached_images);
	for (let i: i32 = 0; i < cached_images.length; ++i) {
		let c: image_t = cached_images[i];
		image_unload(c);
	}
	data_cached_images = new map_t();

	///if arm_audio
	let cached_sounds: sound_t[] = map_to_array(data_cached_sounds);
	for (let i: i32 = 0; i < cached_sounds.length; ++i) {
		let c = cached_sounds[i];
		sound_unload(c);
	}
	data_cached_sounds = new map_t();
	///end

	let cached_videos: video_t[] = map_to_array(data_cached_videos);
	for (let i: i32 = 0; i < cached_videos.length; ++i) {
		let c: video_t = cached_videos[i];
		video_unload(c);
	}
	data_cached_videos = new map_t();

	let cached_fonts: g2_font_t[] = map_to_array(data_cached_fonts);
	for (let i: i32 = 0; i < cached_fonts.length; ++i) {
		let c: g2_font_t = cached_fonts[i];
		g2_font_unload(c);
	}
	data_cached_fonts = new map_t();
}

function data_sep(): string {
	///if krom_windows
	return "\\";
	///else
	return "/";
	///end
}

function data_path(): string {
	///if krom_android
	return "data" + data_sep();
	///else
	return "." + data_sep() + "data" + data_sep();
	///end
}

function data_is_abs(file: string): bool {
	return char_at(file, 0) == "/" || char_at(file, 1) == ":" || (char_at(file, 0) == "\\" && char_at(file, 1) == "\\");
}

function data_is_up(file: string): bool {
	return char_at(file, 0) == "." && char_at(file, 1) == ".";
}

function data_base_name(path: string): string {
	let slash: i32 = string_last_index_of(path, data_sep());
	return slash >= 0 ? substring(path, slash + 1, path.length) : path;
}

function data_resolve_path(file: string): string {
	if (data_is_abs(file) || data_is_up(file)) {
		return file;
	}
	return data_path() + file;
}
