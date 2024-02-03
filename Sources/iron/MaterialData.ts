
class MaterialData {

	static uidCounter = 0;

	static create(raw: TMaterialData, done: (data: TMaterialData)=>void, file = "") {
		raw._uid = ++MaterialData.uidCounter; // Start from 1

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

		Data.getShader(object_file, data_ref, (b: TShaderData) => {
			raw._shader = b;

			// Contexts have to be in the same order as in raw data for now
			raw._contexts = [];
			while (raw._contexts.length < raw.contexts.length) raw._contexts.push(null);
			let contextsLoaded = 0;

			for (let i = 0; i < raw.contexts.length; ++i) {
				let c = raw.contexts[i];
				MaterialContext.create(c, (self: TMaterialContext) => {
					raw._contexts[i] = self;
					contextsLoaded++;
					if (contextsLoaded == raw.contexts.length) done(raw);
				});
			}
		}, raw.override_context);
	}

	static parse = (file: string, name: string, done: (data: TMaterialData)=>void) => {
		Data.getSceneRaw(file, (format: TSceneFormat) => {
			let raw: TMaterialData = Data.getMaterialRawByName(format.material_datas, name);
			if (raw == null) {
				Krom.log(`Material data "${name}" not found!`);
				done(null);
			}
			MaterialData.create(raw, done, file);
		});
	}

	static getContext = (raw: TMaterialData, name: string): TMaterialContext => {
		for (let c of raw._contexts) {
			// 'mesh' will fetch both 'mesh' and 'meshheight' contexts
			if (c.name.substr(0, name.length) == name) return c;
		}
		return null;
	}
}

class MaterialContext {

	static create(raw: TMaterialContext, done: (context: TMaterialContext)=>void) {

		if (raw.bind_textures != null && raw.bind_textures.length > 0) {

			raw._textures = [];
			let texturesLoaded = 0;

			for (let i = 0; i < raw.bind_textures.length; ++i) {
				let tex = raw.bind_textures[i];

				if (tex.file == "" || tex.source == "movie") { // Empty texture
					texturesLoaded++;
					if (texturesLoaded == raw.bind_textures.length) done(raw);
					continue;
				}

				Data.getImage(tex.file, (image: ImageRaw) => {
					raw._textures[i] = image;
					texturesLoaded++;

					// Set mipmaps
					if (tex.mipmaps != null) {
						let mipmaps: ImageRaw[] = [];
						while (mipmaps.length < tex.mipmaps.length) mipmaps.push(null);
						let mipmapsLoaded = 0;

						for (let j = 0; j < tex.mipmaps.length; ++j) {
							let name = tex.mipmaps[j];

							Data.getImage(name, (mipimg: ImageRaw) => {
								mipmaps[j] = mipimg;
								mipmapsLoaded++;

								if (mipmapsLoaded == tex.mipmaps.length) {
									Image.setMipmaps(image, mipmaps);
									tex.mipmaps = null;
									tex.generate_mipmaps = false;

									if (texturesLoaded == raw.bind_textures.length) done(raw);
								}
							});
						}
					}
					else if (tex.generate_mipmaps == true && image != null) {
						Image.generateMipmaps(image, 1000);
						tex.mipmaps = null;
						tex.generate_mipmaps = false;

						if (texturesLoaded == raw.bind_textures.length) done(raw);
					}
					else if (texturesLoaded == raw.bind_textures.length) done(raw);

				}, false, tex.format != null ? tex.format : "RGBA32");
			}
		}
		else done(raw);
	}

	static setTextureParameters = (raw: TMaterialContext, g: Graphics4Raw, textureIndex: i32, context: TShaderContext, unitIndex: i32) => {
		// This function is called by MeshObject for samplers set using material context
		ShaderContext.setTextureParameters(context, g, unitIndex, raw.bind_textures[textureIndex]);
	}
}
