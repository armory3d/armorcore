
// Global data list and asynchronous data loading
let data_cached_scene_raws: Map<string, scene_t> = new Map();
let data_cached_meshes: Map<string, mesh_data_t> = new Map();
let data_cached_lights: Map<string, light_data_t> = new Map();
let data_cached_cameras: Map<string, camera_data_t> = new Map();
let data_cached_materials: Map<string, material_data_t> = new Map();
let data_cached_particles: Map<string, particle_data_t> = new Map();
let data_cached_worlds: Map<string, world_data_t> = new Map();
let data_cached_shaders: Map<string, shader_data_t> = new Map();

let data_cached_blobs: Map<string, ArrayBuffer> = new Map();
let data_cached_images: Map<string, image_t> = new Map();
let data_cached_videos: Map<string, video_t> = new Map();
let data_cached_fonts: Map<string, font_t> = new Map();
///if arm_audio
let data_cached_sounds: Map<string, sound_t> = new Map();
///end

let data_assets_loaded = 0;
let data_loading_meshes: Map<string, ((d: mesh_data_t)=>void)[]> = new Map();
let data_loading_lights: Map<string, ((d: light_data_t)=>void)[]> = new Map();
let data_loading_cameras: Map<string, ((d: camera_data_t)=>void)[]> = new Map();
let data_loading_materials: Map<string, ((d: material_data_t)=>void)[]> = new Map();
let data_loading_particles: Map<string, ((d: particle_data_t)=>void)[]> = new Map();
let data_loading_worlds: Map<string, ((d: world_data_t)=>void)[]> = new Map();
let data_loading_shaders: Map<string, ((d: shader_data_t)=>void)[]> = new Map();
let data_loading_scene_raws: Map<string, ((fmt: scene_t)=>void)[]> = new Map();
let data_loading_blobs: Map<string, ((ab: ArrayBuffer)=>void)[]> = new Map();
let data_loading_images: Map<string, ((img: image_t)=>void)[]> = new Map();
let data_loading_videos: Map<string, ((vid: video_t)=>void)[]> = new Map();
let data_loading_fonts: Map<string, ((f: font_t)=>void)[]> = new Map();
///if arm_audio
let data_loading_sounds: Map<string, ((snd: sound_t)=>void)[]> = new Map();
///end

function data_sep(): string {
	///if krom_windows
	return "\\";
	///else
	return "/";
	///end
}

function data_data_path(): string {
	///if krom_android
	return "data" + data_sep();
	///else
	return "." + data_sep() + "data" + data_sep();
	///end
}

function data_delete_all() {
	for (let c of data_cached_meshes.values()) mesh_data_delete(c);
	data_cached_meshes = new Map();
	for (let c of data_cached_shaders.values()) shader_data_delete(c);
	data_cached_shaders = new Map();
	data_cached_scene_raws = new Map();
	data_cached_lights = new Map();
	data_cached_cameras = new Map();
	data_cached_materials = new Map();
	data_cached_particles = new Map();
	data_cached_worlds = new Map();
	render_path_unload();

	data_cached_blobs = new Map();
	for (let c of data_cached_images.values()) image_unload(c);
	data_cached_images = new Map();
	///if arm_audio
	for (let c of data_cached_sounds.values()) sound_unload(c);
	data_cached_sounds = new Map();
	///end
	for (let c of data_cached_videos.values()) video_unload(c);
	data_cached_videos = new Map();
	for (let c of data_cached_fonts.values()) font_unload(c);
	data_cached_fonts = new Map();
}

function data_get_mesh(file: string, name: string, done: (md: mesh_data_t)=>void) {
	let handle = file + name;
	let cached = data_cached_meshes.get(handle);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_meshes.get(handle);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_meshes.set(handle, [done]);

	mesh_data_parse(file, name, (b: mesh_data_t) => {
		data_cached_meshes.set(handle, b);
		b._handle = handle;
		for (let f of data_loading_meshes.get(handle)) f(b);
		data_loading_meshes.delete(handle);
	});
}

function data_delete_mesh(handle: string) {
	// Remove cached mesh
	let mesh = data_cached_meshes.get(handle);
	if (mesh == null) return;
	mesh_data_delete(mesh);
	data_cached_meshes.delete(handle);
}

function data_get_light(file: string, name: string, done: (ld: light_data_t)=>void) {
	let handle = file + name;
	let cached = data_cached_lights.get(handle);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_lights.get(handle);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_lights.set(handle, [done]);

	light_data_parse(file, name, (b: light_data_t) => {
		data_cached_lights.set(handle, b);
		for (let f of data_loading_lights.get(handle)) f(b);
		data_loading_lights.delete(handle);
	});
}

function data_get_camera(file: string, name: string, done: (cd: camera_data_t)=>void) {
	let handle = file + name;
	let cached = data_cached_cameras.get(handle);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_cameras.get(handle);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_cameras.set(handle, [done]);

	camera_data_parse(file, name, (b: camera_data_t) => {
		data_cached_cameras.set(handle, b);
		for (let f of data_loading_cameras.get(handle)) f(b);
		data_loading_cameras.delete(handle);
	});
}

function data_get_material(file: string, name: string, done: (md: material_data_t)=>void) {
	let handle = file + name;
	let cached = data_cached_materials.get(handle);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_materials.get(handle);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_materials.set(handle, [done]);

	material_data_parse(file, name, (b: material_data_t) => {
		data_cached_materials.set(handle, b);
		for (let f of data_loading_materials.get(handle)) f(b);
		data_loading_materials.delete(handle);
	});
}

function data_get_particle(file: string, name: string, done: (pd: particle_data_t)=>void) {
	let handle = file + name;
	let cached = data_cached_particles.get(handle);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_particles.get(handle);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_particles.set(handle, [done]);

	particle_data_parse(file, name, (b: particle_data_t) => {
		data_cached_particles.set(handle, b);
		for (let f of data_loading_particles.get(handle)) f(b);
		data_loading_particles.delete(handle);
	});
}

function data_get_world(file: string, name: string, done: (wd: world_data_t)=>void) {
	if (name == null) { // No world defined in scene
		done(null);
		return;
	}

	let handle = file + name;
	let cached = data_cached_worlds.get(handle);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_worlds.get(handle);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_worlds.set(handle, [done]);

	world_data_parse(file, name, (b: world_data_t) => {
		data_cached_worlds.set(handle, b);
		for (let f of data_loading_worlds.get(handle)) f(b);
		data_loading_worlds.delete(handle);
	});
}

function data_get_shader(file: string, name: string, done: (sd: shader_data_t)=>void, overrideContext: shader_override_t = null) {
	// Only one context override per shader data for now
	let cacheName = name;
	if (overrideContext != null) cacheName += "2";
	let cached = data_cached_shaders.get(cacheName); // Shader must have unique name
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_shaders.get(cacheName);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_shaders.set(cacheName, [done]);

	shader_data_parse(file, name, (b: shader_data_t) => {
		data_cached_shaders.set(cacheName, b);
		for (let f of data_loading_shaders.get(cacheName)) f(b);
		data_loading_shaders.delete(cacheName);
	}, overrideContext);
}

function data_get_scene_raw(file: string, done: (fmt: scene_t)=>void) {
	let cached = data_cached_scene_raws.get(file);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_scene_raws.get(file);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_scene_raws.set(file, [done]);

	// If no extension specified, set to .arm
	let ext = file.endsWith(".arm") ? "" : ".arm";

	data_get_blob(file + ext, (b: ArrayBuffer) => {
		let parsed: scene_t = null;
		parsed = armpack_decode(b);
		data_return_scene_raw(file, parsed);
	});
}

function data_return_scene_raw(file: string, parsed: scene_t) {
	data_cached_scene_raws.set(file, parsed);
	for (let f of data_loading_scene_raws.get(file)) f(parsed);
	data_loading_scene_raws.delete(file);
}

function data_get_mesh_raw_by_name(datas: mesh_data_t[], name: string): mesh_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}

function data_get_light_raw_by_name(datas: light_data_t[], name: string): light_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}

function data_get_camera_raw_by_name(datas: camera_data_t[], name: string): camera_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}

function data_get_material_raw_by_name(datas: material_data_t[], name: string): material_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}

function data_get_particle_raw_by_name(datas: particle_data_t[], name: string): particle_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}

function data_get_world_raw_by_name(datas: world_data_t[], name: string): world_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}

function data_get_shader_raw_by_name(datas: shader_data_t[], name: string): shader_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}

///if arm_audio
function data_get_speaker_raw_by_name(datas: speaker_data_t[], name: string): speaker_data_t {
	if (name == "") return datas[0];
	for (let dat of datas) if (dat.name == name) return dat;
	return null;
}
///end

// Raw assets
function data_get_blob(file: string, done: (ab: ArrayBuffer)=>void) {
	let cached = data_cached_blobs.get(file); // Is already cached
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_blobs.get(file); // Is already being loaded
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_blobs.set(file, [done]); // Start loading

	// Krom.load_blob(resolvePath(file), (b: ArrayBuffer) => {
		let b = Krom.loadBlob(data_resolve_path(file));
		data_cached_blobs.set(file, b);
		for (let f of data_loading_blobs.get(file)) f(b);
		data_loading_blobs.delete(file);
		data_assets_loaded++;
	// });
}

function data_delete_blob(handle: string) {
	let blob = data_cached_blobs.get(handle);
	if (blob == null) return;
	data_cached_blobs.delete(handle);
}

function data_get_image(file: string, done: (img: image_t)=>void, readable = false, format = "RGBA32") {
	let cached = data_cached_images.get(file);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_images.get(file);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_images.set(file, [done]);

	///if arm_image_embed
	let imageBlob = data_cached_blobs.get(file);
	if (imageBlob != null) {
		image_from_encoded_bytes(imageBlob, ".k", (b: image_t) => {
			data_cached_images.set(file, b);
			for (let f of data_loading_images.get(file)) f(b);
			data_loading_images.delete(file);
			data_assets_loaded++;
		}, null, readable);
		return;
	}
	///end

	// Krom.load_image(resolvePath(file), readable, (b: ImageRaw) => {
		let image_ = Krom.loadImage(data_resolve_path(file), readable);
		if (image_ != null) {
			let b = image_from_texture(image_);
			data_cached_images.set(file, b);
			for (let f of data_loading_images.get(file)) f(b);
			data_loading_images.delete(file);
			data_assets_loaded++;
		}
	// });
}

function data_delete_image(handle: string) {
	let image = data_cached_images.get(handle);
	if (image == null) return;
	image_unload(image);
	data_cached_images.delete(handle);
}

///if arm_audio
function data_get_sound(file: string, done: (snd: sound_t)=>void) {
	let cached = data_cached_sounds.get(file);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_sounds.get(file);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_sounds.set(file, [done]);

	// Krom.load_sound(data_resolve_path(file), (b: sound_t) => {
		let b = sound_create(Krom.loadSound(data_resolve_path(file)));
		data_cached_sounds.set(file, b);
		for (let f of data_loading_sounds.get(file)) f(b);
		data_loading_sounds.delete(file);
		data_assets_loaded++;
	// });
}

function data_delete_sound(handle: string) {
	let sound = data_cached_sounds.get(handle);
	if (sound == null) return;
	sound_unload(sound);
	data_cached_sounds.delete(handle);
}
///end

function data_get_video(file: string, done: (vid: video_t)=>void) {
	file = file.substring(0, file.length - 4) + ".webm";
	let cached = data_cached_videos.get(file);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_videos.get(file);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_videos.set(file, [done]);

	// Krom.load_video(data_resolve_path(file), (b: video_t) => {
	// 	cachedVideos.set(file, b);
	// 	for (let f of data_loading_videos.get(file)) f(b);
	// 	data_loading_videos.delete(file);
	// 	assetsLoaded++;
	// });
}

function data_delete_video(handle: string) {
	let video = data_cached_videos.get(handle);
	if (video == null) return;
	video_unload(video);
	data_cached_videos.delete(handle);
}

function data_get_font(file: string, done: (f: font_t)=>void) {
	let cached = data_cached_fonts.get(file);
	if (cached != null) {
		done(cached);
		return;
	}

	let loading = data_loading_fonts.get(file);
	if (loading != null) {
		loading.push(done);
		return;
	}

	data_loading_fonts.set(file, [done]);

	// Krom.load_blob(resolvePath(file), (blob: ArrayBuffer) => {
		let blob = Krom.loadBlob(data_resolve_path(file));
		let b = font_create(blob);
		data_cached_fonts.set(file, b);
		for (let f of data_loading_fonts.get(file)) f(b);
		data_loading_fonts.delete(file);
		data_assets_loaded++;
	// });
}

function data_delete_font(handle: string) {
	let font = data_cached_fonts.get(handle);
	if (font == null) return;
	font_unload(font);
	data_cached_fonts.delete(handle);
}

function data_is_abs(file: string): bool {
	return file.charAt(0) == "/" || file.charAt(1) == ":" || (file.charAt(0) == "\\" && file.charAt(1) == "\\");
}

function data_is_up(file: string): bool {
	return file.charAt(0) == "." && file.charAt(1) == ".";
}

function data_base_name(path: string): string {
	let slash = path.lastIndexOf(data_sep());
	return slash >= 0 ? path.substr(slash + 1) : path;
}

function data_resolve_path(file: string): string {
	if (data_is_abs(file) || data_is_up(file)) return file;
	return data_data_path() + file;
}
