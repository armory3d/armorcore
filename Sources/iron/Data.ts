
// Global data list and asynchronous data loading
class Data {

	static cachedSceneRaws: Map<string, TSceneFormat> = new Map();
	static cachedMeshes: Map<string, MeshData> = new Map();
	static cachedLights: Map<string, TLightData> = new Map();
	static cachedCameras: Map<string, TCameraData> = new Map();
	static cachedMaterials: Map<string, MaterialData> = new Map();
	static cachedParticles: Map<string, TParticleData> = new Map();
	static cachedWorlds: Map<string, WorldData> = new Map();
	static cachedShaders: Map<string, ShaderData> = new Map();

	static cachedBlobs: Map<string, ArrayBuffer> = new Map();
	static cachedImages: Map<string, Image> = new Map();
	static cachedVideos: Map<string, Video> = new Map();
	static cachedFonts: Map<string, Font> = new Map();
	///if arm_audio
	static cachedSounds: Map<string, Sound> = new Map();
	///end

	static assetsLoaded = 0;
	static loadingMeshes: Map<string, ((d: MeshData)=>void)[]> = new Map();
	static loadingLights: Map<string, ((d: TLightData)=>void)[]> = new Map();
	static loadingCameras: Map<string, ((d: TCameraData)=>void)[]> = new Map();
	static loadingMaterials: Map<string, ((d: MaterialData)=>void)[]> = new Map();
	static loadingParticles: Map<string, ((d: TParticleData)=>void)[]> = new Map();
	static loadingWorlds: Map<string, ((d: WorldData)=>void)[]> = new Map();
	static loadingShaders: Map<string, ((d: ShaderData)=>void)[]> = new Map();
	static loadingSceneRaws: Map<string, ((fmt: TSceneFormat)=>void)[]> = new Map();
	static loadingBlobs: Map<string, ((ab: ArrayBuffer)=>void)[]> = new Map();
	static loadingImages: Map<string, ((img: Image)=>void)[]> = new Map();
	static loadingVideos: Map<string, ((vid: Video)=>void)[]> = new Map();
	static loadingFonts: Map<string, ((f: Font)=>void)[]> = new Map();
	///if arm_audio
	static loadingSounds: Map<string, ((snd: Sound)=>void)[]> = new Map();
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

	constructor() {}

	static deleteAll = () => {
		for (let c of Data.cachedMeshes.values()) c.delete();
		Data.cachedMeshes = new Map();
		for (let c of Data.cachedShaders.values()) c.delete();
		Data.cachedShaders = new Map();
		Data.cachedSceneRaws = new Map();
		Data.cachedLights = new Map();
		Data.cachedCameras = new Map();
		Data.cachedMaterials = new Map();
		Data.cachedParticles = new Map();
		Data.cachedWorlds = new Map();
		if (RenderPath.active != null) RenderPath.active.unload();

		Data.cachedBlobs = new Map();
		for (let c of Data.cachedImages.values()) c.unload();
		Data.cachedImages = new Map();
		///if arm_audio
		for (let c of Data.cachedSounds.values()) c.unload();
		Data.cachedSounds = new Map();
		///end
		for (let c of Data.cachedVideos.values()) c.unload();
		Data.cachedVideos = new Map();
		for (let c of Data.cachedFonts.values()) c.unload();
		Data.cachedFonts = new Map();
	}

	static getMesh = (file: string, name: string, done: (md: MeshData)=>void) => {
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

		MeshData.parse(file, name, (b: MeshData) => {
			Data.cachedMeshes.set(handle, b);
			b.handle = handle;
			for (let f of Data.loadingMeshes.get(handle)) f(b);
			Data.loadingMeshes.delete(handle);
		});
	}

	static deleteMesh = (handle: string) => {
		// Remove cached mesh
		let mesh = Data.cachedMeshes.get(handle);
		if (mesh == null) return;
		mesh.delete();
		Data.cachedMeshes.delete(handle);
	}

	static getLight = (file: string, name: string, done: (ld: TLightData)=>void) => {
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

		light_data_parse(file, name, (b: TLightData) => {
			Data.cachedLights.set(handle, b);
			for (let f of Data.loadingLights.get(handle)) f(b);
			Data.loadingLights.delete(handle);
		});
	}

	static getCamera = (file: string, name: string, done: (cd: TCameraData)=>void) => {
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

		camera_data_parse(file, name, (b: TCameraData) => {
			Data.cachedCameras.set(handle, b);
			for (let f of Data.loadingCameras.get(handle)) f(b);
			Data.loadingCameras.delete(handle);
		});
	}

	static getMaterial = (file: string, name: string, done: (md: MaterialData)=>void) => {
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

		MaterialData.parse(file, name, (b: MaterialData) => {
			Data.cachedMaterials.set(handle, b);
			for (let f of Data.loadingMaterials.get(handle)) f(b);
			Data.loadingMaterials.delete(handle);
		});
	}

	static getParticle = (file: string, name: string, done: (pd: TParticleData)=>void) => {
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

		particle_data_parse(file, name, (b: TParticleData) => {
			Data.cachedParticles.set(handle, b);
			for (let f of Data.loadingParticles.get(handle)) f(b);
			Data.loadingParticles.delete(handle);
		});
	}

	static getWorld = (file: string, name: string, done: (wd: WorldData)=>void) => {
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

		WorldData.parse(file, name, (b: WorldData) => {
			Data.cachedWorlds.set(handle, b);
			for (let f of Data.loadingWorlds.get(handle)) f(b);
			Data.loadingWorlds.delete(handle);
		});
	}

	static getShader = (file: string, name: string, done: (sd: ShaderData)=>void, overrideContext: TShaderOverride = null) => {
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

		ShaderData.parse(file, name, (b: ShaderData) => {
			Data.cachedShaders.set(cacheName, b);
			for (let f of Data.loadingShaders.get(cacheName)) f(b);
			Data.loadingShaders.delete(cacheName);
		}, overrideContext);
	}

	static getSceneRaw = (file: string, done: (fmt: TSceneFormat)=>void) => {
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
			let parsed: TSceneFormat = null;
			parsed = ArmPack.decode(b);
			Data.returnSceneRaw(file, parsed);
		});
	}

	static returnSceneRaw = (file: string, parsed: TSceneFormat) => {
		Data.cachedSceneRaws.set(file, parsed);
		for (let f of Data.loadingSceneRaws.get(file)) f(parsed);
		Data.loadingSceneRaws.delete(file);
	}

	static getMeshRawByName = (datas: TMeshData[], name: string): TMeshData => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getLightRawByName = (datas: TLightData[], name: string): TLightData => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getCameraRawByName = (datas: TCameraData[], name: string): TCameraData => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getMaterialRawByName = (datas: TMaterialData[], name: string): TMaterialData => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getParticleRawByName = (datas: TParticleData[], name: string): TParticleData => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getWorldRawByName = (datas: TWorldData[], name: string): TWorldData => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	static getShaderRawByName = (datas: TShaderData[], name: string): TShaderData => {
		if (name == "") return datas[0];
		for (let dat of datas) if (dat.name == name) return dat;
		return null;
	}

	///if arm_audio
	static getSpeakerRawByName = (datas: TSpeakerData[], name: string): TSpeakerData => {
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

	static getImage = (file: string, done: (img: Image)=>void, readable = false, format = "RGBA32") => {
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
			Image.fromEncodedBytes(imageBlob, ".k", (b: Image) => {
				Data.cachedImages.set(file, b);
				for (let f of Data.loadingImages.get(file)) f(b);
				Data.loadingImages.delete(file);
				Data.assetsLoaded++;
			}, null, readable);
			return;
		}
		///end

		// Krom.load_image(resolvePath(file), readable, (b: Image) => {
			let image_ = Krom.loadImage(Data.resolvePath(file), readable);
			if (image_ != null) {
				let b = Image._fromTexture(image_);
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
		image.unload();
		Data.cachedImages.delete(handle);
	}

	///if arm_audio
	static getSound = (file: string, done: (snd: Sound)=>void) => {
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


		// Krom.load_sound(Data.resolvePath(file), (b: Sound) => {
			let b = new Sound(Krom.loadSound(Data.resolvePath(file)));
			Data.cachedSounds.set(file, b);
			for (let f of Data.loadingSounds.get(file)) f(b);
			Data.loadingSounds.delete(file);
			Data.assetsLoaded++;
		// });
	}

	static deleteSound = (handle: string) => {
		let sound = Data.cachedSounds.get(handle);
		if (sound == null) return;
		sound.unload();
		Data.cachedSounds.delete(handle);
	}
	///end

	static getVideo = (file: string, done: (vid: Video)=>void) => {
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

		// Krom.load_video(Data.resolvePath(file), (b: Video) => {
		// 	cachedVideos.set(file, b);
		// 	for (let f of Data.loadingVideos.get(file)) f(b);
		// 	Data.loadingVideos.delete(file);
		// 	assetsLoaded++;
		// });
	}

	static deleteVideo = (handle: string) => {
		let video = Data.cachedVideos.get(handle);
		if (video == null) return;
		video.unload();
		Data.cachedVideos.delete(handle);
	}

	static getFont = (file: string, done: (f: Font)=>void) => {
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
			let b = new Font(blob);
			Data.cachedFonts.set(file, b);
			for (let f of Data.loadingFonts.get(file)) f(b);
			Data.loadingFonts.delete(file);
			Data.assetsLoaded++;
		// });
	}

	static deleteFont = (handle: string) => {
		let font = Data.cachedFonts.get(handle);
		if (font == null) return;
		font.unload();
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
