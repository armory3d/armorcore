
class MaterialData {

	static uidCounter = 0;
	uid: f32;
	name: string;
	raw: TMaterialData;
	shader: ShaderData;
	contexts: MaterialContext[] = null;

	constructor(raw: TMaterialData, done: (data: MaterialData)=>void, file = "") {
		this.uid = ++MaterialData.uidCounter; // Start from 1
		this.raw = raw;
		this.name = raw.name;

		let ref = raw.shader.split("/");
		let object_file = "";
		let data_ref = "";
		if (ref.length == 2) { // File reference
			object_file = ref[0];
			data_ref = ref[1];
		}
		else { // Local data
			object_file = file;
			data_ref = raw.shader;
		}

		Data.getShader(object_file, data_ref, (b: ShaderData) => {
			this.shader = b;

			// Contexts have to be in the same order as in raw data for now
			this.contexts = [];
			while (this.contexts.length < raw.contexts.length) this.contexts.push(null);
			let contextsLoaded = 0;

			for (let i = 0; i < raw.contexts.length; ++i) {
				let c = raw.contexts[i];
				new MaterialContext(c, (self: MaterialContext) => {
					this.contexts[i] = self;
					contextsLoaded++;
					if (contextsLoaded == raw.contexts.length) done(this);
				});
			}
		}, raw.override_context);
	}

	static parse = (file: string, name: string, done: (data: MaterialData)=>void) => {
		Data.getSceneRaw(file, (format: TSceneFormat) => {
			let raw: TMaterialData = Data.getMaterialRawByName(format.material_datas, name);
			if (raw == null) {
				Krom.log(`Material data "${name}" not found!`);
				done(null);
			}
			new MaterialData(raw, done, file);
		});
	}

	getContext = (name: string): MaterialContext => {
		for (let c of this.contexts) {
			// 'mesh' will fetch both 'mesh' and 'meshheight' contexts
			if (c.raw.name.substr(0, name.length) == name) return c;
		}
		return null;
	}
}

class MaterialContext {
	raw: TMaterialContext;
	textures: Image[] = null;
	id = 0;
	static num = 0;

	constructor(raw: TMaterialContext, done: (context: MaterialContext)=>void) {
		this.raw = raw;
		this.id = MaterialContext.num++;

		if (raw.bind_textures != null && raw.bind_textures.length > 0) {

			this.textures = [];
			let texturesLoaded = 0;

			for (let i = 0; i < raw.bind_textures.length; ++i) {
				let tex = raw.bind_textures[i];

				if (tex.file == "" || tex.source == "movie") { // Empty texture
					texturesLoaded++;
					if (texturesLoaded == raw.bind_textures.length) done(this);
					continue;
				}

				Data.getImage(tex.file, (image: Image) => {
					this.textures[i] = image;
					texturesLoaded++;

					// Set mipmaps
					if (tex.mipmaps != null) {
						let mipmaps: Image[] = [];
						while (mipmaps.length < tex.mipmaps.length) mipmaps.push(null);
						let mipmapsLoaded = 0;

						for (let j = 0; j < tex.mipmaps.length; ++j) {
							let name = tex.mipmaps[j];

							Data.getImage(name, (mipimg: Image) => {
								mipmaps[j] = mipimg;
								mipmapsLoaded++;

								if (mipmapsLoaded == tex.mipmaps.length) {
									image.setMipmaps(mipmaps);
									tex.mipmaps = null;
									tex.generate_mipmaps = false;

									if (texturesLoaded == raw.bind_textures.length) done(this);
								}
							});
						}
					}
					else if (tex.generate_mipmaps == true && image != null) {
						image.generateMipmaps(1000);
						tex.mipmaps = null;
						tex.generate_mipmaps = false;

						if (texturesLoaded == raw.bind_textures.length) done(this);
					}
					else if (texturesLoaded == raw.bind_textures.length) done(this);

				}, false, tex.format != null ? tex.format : "RGBA32");
			}
		}
		else done(this);
	}

	setTextureParameters = (g: Graphics4, textureIndex: i32, context: ShaderContext, unitIndex: i32) => {
		// This function is called by MeshObject for samplers set using material context
		context.setTextureParameters(g, unitIndex, this.raw.bind_textures[textureIndex]);
	}
}
