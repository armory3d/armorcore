
class MaterialData {

	static uidCounter = 0;

	static create(raw: material_data_t, done: (data: material_data_t)=>void, file = "") {
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

		Data.getShader(object_file, data_ref, (b: shader_data_t) => {
			raw._shader = b;

			// Contexts have to be in the same order as in raw data for now
			raw._contexts = [];
			while (raw._contexts.length < raw.contexts.length) raw._contexts.push(null);
			let contextsLoaded = 0;

			for (let i = 0; i < raw.contexts.length; ++i) {
				let c = raw.contexts[i];
				MaterialContext.create(c, (self: material_context_t) => {
					raw._contexts[i] = self;
					contextsLoaded++;
					if (contextsLoaded == raw.contexts.length) done(raw);
				});
			}
		}, raw.override_context);
	}

	static parse = (file: string, name: string, done: (data: material_data_t)=>void) => {
		Data.getSceneRaw(file, (format: scene_t) => {
			let raw: material_data_t = Data.getMaterialRawByName(format.material_datas, name);
			if (raw == null) {
				Krom.log(`Material data "${name}" not found!`);
				done(null);
			}
			MaterialData.create(raw, done, file);
		});
	}

	static getContext = (raw: material_data_t, name: string): material_context_t => {
		for (let c of raw._contexts) {
			// 'mesh' will fetch both 'mesh' and 'meshheight' contexts
			if (c.name.substr(0, name.length) == name) return c;
		}
		return null;
	}
}

class MaterialContext {

	static create(raw: material_context_t, done: (context: material_context_t)=>void) {

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

				Data.getImage(tex.file, (image: image_t) => {
					raw._textures[i] = image;
					texturesLoaded++;

					// Set mipmaps
					if (tex.mipmaps != null) {
						let mipmaps: image_t[] = [];
						while (mipmaps.length < tex.mipmaps.length) mipmaps.push(null);
						let mipmapsLoaded = 0;

						for (let j = 0; j < tex.mipmaps.length; ++j) {
							let name = tex.mipmaps[j];

							Data.getImage(name, (mipimg: image_t) => {
								mipmaps[j] = mipimg;
								mipmapsLoaded++;

								if (mipmapsLoaded == tex.mipmaps.length) {
									image_set_mipmaps(image, mipmaps);
									tex.mipmaps = null;
									tex.generate_mipmaps = false;

									if (texturesLoaded == raw.bind_textures.length) done(raw);
								}
							});
						}
					}
					else if (tex.generate_mipmaps == true && image != null) {
						image_gen_mipmaps(image, 1000);
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

	static setTextureParameters = (raw: material_context_t, g: g4_t, textureIndex: i32, context: shader_context_t, unitIndex: i32) => {
		// This function is called by MeshObject for samplers set using material context
		shader_context_set_tex_params(context, g, unitIndex, raw.bind_textures[textureIndex]);
	}
}
