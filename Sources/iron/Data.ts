
// Global data list and asynchronous data loading
class Data {

	static cachedSceneRaws: Map<string, scene_t> = new Map();
	static cachedMeshes: Map<string, mesh_data_t> = new Map();
	static cachedLights: Map<string, light_data_t> = new Map();
	static cachedCameras: Map<string, camera_data_t> = new Map();
	static cachedMaterials: Map<string, material_data_t> = new Map();
	static cachedParticles: Map<string, particle_data_t> = new Map();
	static cachedWorlds: Map<string, world_data_t> = new Map();
	static cachedShaders: Map<string, shader_data_t> = new Map();

	static cachedBlobs: Map<string, ArrayBuffer> = new Map();
	static cachedImages: Map<string, image_t> = new Map();
	static cachedVideos: Map<string, video_t> = new Map();
	static cachedFonts: Map<string, font_t> = new Map();
	///if arm_audio
	static cachedSounds: Map<string, sound_t> = new Map();
	///end

	static assetsLoaded = 0;
	static loadingMeshes: Map<string, ((d: mesh_data_t)=>void)[]> = new Map();
	static loadingLights: Map<string, ((d: light_data_t)=>void)[]> = new Map();
	static loadingCameras: Map<string, ((d: camera_data_t)=>void)[]> = new Map();
	static loadingMaterials: Map<string, ((d: material_data_t)=>void)[]> = new Map();
	static loadingParticles: Map<string, ((d: particle_data_t)=>void)[]> = new Map();
	static loadingWorlds: Map<string, ((d: world_data_t)=>void)[]> = new Map();
	static loadingShaders: Map<string, ((d: shader_data_t)=>void)[]> = new Map();
	static loadingSceneRaws: Map<string, ((fmt: scene_t)=>void)[]> = new Map();
	static loadingBlobs: Map<string, ((ab: ArrayBuffer)=>void)[]> = new Map();
	static loadingImages: Map<string, ((img: image_t)=>void)[]> = new Map();
	static loadingVideos: Map<string, ((vid: video_t)=>void)[]> = new Map();
	static loadingFonts: Map<string, ((f: font_t)=>void)[]> = new Map();
	///if arm_audio
	static loadingSounds: Map<string, ((snd: sound_t)=>void)[]> = new Map();
	///end

	static get sep(): string {
		///if krom_windows
		return "\\";
		///else
		return "/";
		///end
	}

	static get dataPath(): string {
		///if krom_android
		return "data" + Data.sep;
		///else
		return "." + Data.sep + "data" + Data.sep;
		///end
	}

	static deleteAll = () => {
		for (let c of Data.cachedMeshes.values()) MeshData.delete(c);
		Data.cachedMeshes = new Map();
		for (let c of Data.cachedShaders.values()) shader_data_delete(c);
		Data.cachedShaders = new Map();
		Data.cachedSceneRaws = new Map();
		Data.cachedLights = new Map();
		Data.cachedCameras = new Map();
		Data.cachedMaterials = new Map();
		Data.cachedParticles = new Map();
		Data.cachedWorlds = new Map();
		render_path_unload();

		Data.cachedBlobs = new Map();
		for (let c of Data.cachedImages.values()) image_unload(c);
		Data.cachedImages = new Map();
		///if arm_audio
		for (let c of Data.cachedSounds.values()) sound_unload(c);
		Data.cachedSounds = new Map();
		///end
		for (let c of Data.cachedVideos.values()) video_unload(c);
		Data.cachedVideos = new Map();
		for (let c of Data.cachedFonts.values()) font_unload(c);
		Data.cachedFonts = new Map();
	}

	static getMesh = (file: string, name: string, done: (md: mesh_data_t)=>void) => {
		let handle = file + name;
		let cached = Data.cachedMeshes.get(handle);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingMeshes.get(handle);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingMeshes.set(handle, [done]);

		MeshData.parse(file, name, (b: mesh_data_t) => {
			Data.cachedMeshes.set(handle, b);
			b._handle = handle;
			for (let f of Data.loadingMeshes.get(handle)) f(b);
			Data.loadingMeshes.delete(handle);
		});
	}

	static deleteMesh = (handle: string) => {
		// Remove cached mesh
		let mesh = Data.cachedMeshes.get(handle);
		if (mesh == null) return;
		MeshData.delete(mesh);
		Data.cachedMeshes.delete(handle);
	}

	static getLight = (file: string, name: string, done: (ld: light_data_t)=>void) => {
		let handle = file + name;
		let cached = Data.cachedLights.get(handle);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingLights.get(handle);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingLights.set(handle, [done]);

		light_data_parse(file, name, (b: light_data_t) => {
			Data.cachedLights.set(handle, b);
			for (let f of Data.loadingLights.get(handle)) f(b);
			Data.loadingLights.delete(handle);
		});
	}

	static getCamera = (file: string, name: string, done: (cd: camera_data_t)=>void) => {
		let handle = file + name;
		let cached = Data.cachedCameras.get(handle);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingCameras.get(handle);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingCameras.set(handle, [done]);

		camera_data_parse(file, name, (b: camera_data_t) => {
			Data.cachedCameras.set(handle, b);
			for (let f of Data.loadingCameras.get(handle)) f(b);
			Data.loadingCameras.delete(handle);
		});
	}

	static getMaterial = (file: string, name: string, done: (md: material_data_t)=>void) => {
		let handle = file + name;
		let cached = Data.cachedMaterials.get(handle);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingMaterials.get(handle);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingMaterials.set(handle, [done]);

		MaterialData.parse(file, name, (b: material_data_t) => {
			Data.cachedMaterials.set(handle, b);
			for (let f of Data.loadingMaterials.get(handle)) f(b);
			Data.loadingMaterials.delete(handle);
		});
	}

	static getParticle = (file: string, name: string, done: (pd: particle_data_t)=>void) => {
		let handle = file + name;
		let cached = Data.cachedParticles.get(handle);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingParticles.get(handle);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingParticles.set(handle, [done]);

		particle_data_parse(file, name, (b: particle_data_t) => {
			Data.cachedParticles.set(handle, b);
			for (let f of Data.loadingParticles.get(handle)) f(b);
			Data.loadingParticles.delete(handle);
		});
	}

	static getWorld = (file: string, name: string, done: (wd: world_data_t)=>void) => {
		if (name == null) { // No world defined in scene
			done(null);
			return;
		}

		let handle = file + name;
		let cached = Data.cachedWorlds.get(handle);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingWorlds.get(handle);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingWorlds.set(handle, [done]);

		world_data_parse(file, name, (b: world_data_t) => {
			Data.cachedWorlds.set(handle, b);
			for (let f of Data.loadingWorlds.get(handle)) f(b);
			Data.loadingWorlds.delete(handle);
		});
	}

	static getShader = (file: string, name: string, done: (sd: shader_data_t)=>void, overrideContext: shader_override_t = null) => {
		// Only one context override per shader data for now
		let cacheName = name;
		if (overrideContext != null) cacheName += "2";
		let cached = Data.cachedShaders.get(cacheName); // Shader must have unique name
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingShaders.get(cacheName);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingShaders.set(cacheName, [done]);

		shader_data_parse(file, name, (b: shader_data_t) => {
			Data.cachedShaders.set(cacheName, b);
			for (let f of Data.loadingShaders.get(cacheName)) f(b);
			Data.loadingShaders.delete(cacheName);
		}, overrideContext);
	}

	static getSceneRaw = (file: string, done: (fmt: scene_t)=>void) => {
		let cached = Data.cachedSceneRaws.get(file);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingSceneRaws.get(file);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingSceneRaws.set(file, [done]);

		// If no extension specified, set to .arm
		let ext = file.endsWith(".arm") ? "" : ".arm";

		Data.getBlob(file + ext, (b: ArrayBuffer) => {
			let parsed: scene_t = null;
			parsed = armpack_decode(b);
			Data.returnSceneRaw(file, parsed);
		});
	}

	static returnSceneRaw = (file: string, parsed: scene_t) => {
		Data.cachedSceneRaws.set(file, parsed);
		for (let f of Data.loadingSceneRaws.get(file)) f(parsed);
		Data.loadingSceneRaws.delete(file);
	}

	static getMeshRawByName = (datas: mesh_data_t[], name: string): mesh_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getLightRawByName = (datas: light_data_t[], name: string): light_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getCameraRawByName = (datas: camera_data_t[], name: string): camera_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getMaterialRawByName = (datas: material_data_t[], name: string): material_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getParticleRawByName = (datas: particle_data_t[], name: string): particle_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getWorldRawByName = (datas: world_data_t[], name: string): world_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getShaderRawByName = (datas: shader_data_t[], name: string): shader_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	///if arm_audio
	static getSpeakerRawByName = (datas: speaker_data_t[], name: string): speaker_data_t => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}
	///end

	// Raw assets
	static getBlob = (file: string, done: (ab: ArrayBuffer)=>void) => {
		let cached = Data.cachedBlobs.get(file); // Is already cached
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingBlobs.get(file); // Is already being loaded
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingBlobs.set(file, [done]); // Start loading

		// Krom.load_blob(resolvePath(file), (b: ArrayBuffer) => {
			let b = Krom.loadBlob(Data.resolvePath(file));
			Data.cachedBlobs.set(file, b);
			for (let f of Data.loadingBlobs.get(file)) f(b);
			Data.loadingBlobs.delete(file);
			Data.assetsLoaded++;
		// });
	}

	static deleteBlob = (handle: string) => {
		let blob = Data.cachedBlobs.get(handle);
		if (blob == null) return;
		Data.cachedBlobs.delete(handle);
	}

	static getImage = (file: string, done: (img: image_t)=>void, readable = false, format = "RGBA32") => {
		let cached = Data.cachedImages.get(file);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingImages.get(file);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingImages.set(file, [done]);

		///if arm_image_embed
		let imageBlob = Data.cachedBlobs.get(file);
		if (imageBlob != null) {
			image_from_encoded_bytes(imageBlob, ".k", (b: image_t) => {
				Data.cachedImages.set(file, b);
				for (let f of Data.loadingImages.get(file)) f(b);
				Data.loadingImages.delete(file);
				Data.assetsLoaded++;
			}, null, readable);
			return;
		}
		///end

		// Krom.load_image(resolvePath(file), readable, (b: ImageRaw) => {
			let image_ = Krom.loadImage(Data.resolvePath(file), readable);
			if (image_ != null) {
				let b = image_from_texture(image_);
				Data.cachedImages.set(file, b);
				for (let f of Data.loadingImages.get(file)) f(b);
				Data.loadingImages.delete(file);
				Data.assetsLoaded++;
			}
		// });
	}

	static deleteImage = (handle: string) => {
		let image = Data.cachedImages.get(handle);
		if (image == null) return;
		image_unload(image);
		Data.cachedImages.delete(handle);
	}

	///if arm_audio
	static getSound = (file: string, done: (snd: sound_t)=>void) => {
		let cached = Data.cachedSounds.get(file);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingSounds.get(file);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingSounds.set(file, [done]);

		// Krom.load_sound(Data.resolvePath(file), (b: SoundRaw) => {
			let b = sound_create(Krom.loadSound(Data.resolvePath(file)));
			Data.cachedSounds.set(file, b);
			for (let f of Data.loadingSounds.get(file)) f(b);
			Data.loadingSounds.delete(file);
			Data.assetsLoaded++;
		// });
	}

	static deleteSound = (handle: string) => {
		let sound = Data.cachedSounds.get(handle);
		if (sound == null) return;
		sound_unload(sound);
		Data.cachedSounds.delete(handle);
	}
	///end

	static getVideo = (file: string, done: (vid: video_t)=>void) => {
		file = file.substring(0, file.length - 4) + ".webm";
		let cached = Data.cachedVideos.get(file);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingVideos.get(file);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingVideos.set(file, [done]);

		// Krom.load_video(Data.resolvePath(file), (b: video_t) => {
		// 	cachedVideos.set(file, b);
		// 	for (let f of Data.loadingVideos.get(file)) f(b);
		// 	Data.loadingVideos.delete(file);
		// 	assetsLoaded++;
		// });
	}

	static deleteVideo = (handle: string) => {
		let video = Data.cachedVideos.get(handle);
		if (video == null) return;
		video_unload(video);
		Data.cachedVideos.delete(handle);
	}

	static getFont = (file: string, done: (f: font_t)=>void) => {
		let cached = Data.cachedFonts.get(file);
		if (cached != null) {
			done(cached);
			return;
		}

		let loading = Data.loadingFonts.get(file);
		if (loading != null) {
			loading.push(done);
			return;
		}

		Data.loadingFonts.set(file, [done]);

		// Krom.load_blob(resolvePath(file), (blob: ArrayBuffer) => {
			let blob = Krom.loadBlob(Data.resolvePath(file));
			let b = font_create(blob);
			Data.cachedFonts.set(file, b);
			for (let f of Data.loadingFonts.get(file)) f(b);
			Data.loadingFonts.delete(file);
			Data.assetsLoaded++;
		// });
	}

	static deleteFont = (handle: string) => {
		let font = Data.cachedFonts.get(handle);
		if (font == null) return;
		font_unload(font);
		Data.cachedFonts.delete(handle);
	}

	static isAbsolute = (file: string): bool => {
		return file.charAt(0) == "/" || file.charAt(1) == ":" || (file.charAt(0) == "\\" && file.charAt(1) == "\\");
	}

	static isUp = (file: string): bool => {
		return file.charAt(0) == "." && file.charAt(1) == ".";
	}

	static baseName = (path: string): string => {
		let slash = path.lastIndexOf(Data.sep);
		return slash >= 0 ? path.substr(slash + 1) : path;
	}

	static resolvePath = (file: string): string => {
		if (Data.isAbsolute(file) || Data.isUp(file)) return file;
		return Data.dataPath + file;
	}
}
