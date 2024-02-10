
let _material_data_uid_counter = 0;

function material_data_create(raw: material_data_t, done: (data: material_data_t)=>void, file = "") {
	raw._uid = ++_material_data_uid_counter; // Start from 1

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

	data_get_shader(object_file, data_ref, function (b: shader_data_t) {
		raw._shader = b;

		// Contexts have to be in the same order as in raw data for now
		raw._contexts = [];
		while (raw._contexts.length < raw.contexts.length) {
			raw._contexts.push(null);
		}
		let contexts_loaded = 0;

		for (let i = 0; i < raw.contexts.length; ++i) {
			let c = raw.contexts[i];
			material_context_create(c, function (self: material_context_t) {
				raw._contexts[i] = self;
				contexts_loaded++;
				if (contexts_loaded == raw.contexts.length) {
					done(raw);
				}
			});
		}
	}, raw.override_context);
}

function material_data_parse(file: string, name: string, done: (data: material_data_t)=>void) {
	data_get_scene_raw(file, function (format: scene_t) {
		let raw: material_data_t = data_get_material_raw_by_name(format.material_datas, name);
		if (raw == null) {
			krom_log(`Material data "${name}" not found!`);
			done(null);
		}
		material_data_create(raw, done, file);
	});
}

function material_data_get_context(raw: material_data_t, name: string): material_context_t {
	for (let c of raw._contexts) {
		// 'mesh' will fetch both 'mesh' and 'meshheight' contexts
		if (c.name.substring(0, name.length) == name) {
			return c;
		}
	}
	return null;
}

function material_context_create(raw: material_context_t, done: (context: material_context_t)=>void) {

	if (raw.bind_textures != null && raw.bind_textures.length > 0) {

		raw._textures = [];
		let textures_loaded = 0;

		for (let i = 0; i < raw.bind_textures.length; ++i) {
			let tex = raw.bind_textures[i];

			if (tex.file == "" || tex.source == "movie") { // Empty texture
				textures_loaded++;
				if (textures_loaded == raw.bind_textures.length) {
					done(raw);
				}
				continue;
			}

			data_get_image(tex.file, function (image: image_t) {
				raw._textures[i] = image;
				textures_loaded++;

				// Set mipmaps
				if (tex.mipmaps != null) {
					let mipmaps: image_t[] = [];
					while (mipmaps.length < tex.mipmaps.length) {
						mipmaps.push(null);
					}
					let mipmaps_loaded = 0;

					for (let j = 0; j < tex.mipmaps.length; ++j) {
						let name = tex.mipmaps[j];

						data_get_image(name, function (mipimg: image_t) {
							mipmaps[j] = mipimg;
							mipmaps_loaded++;

							if (mipmaps_loaded == tex.mipmaps.length) {
								image_set_mipmaps(image, mipmaps);
								tex.mipmaps = null;
								tex.generate_mipmaps = false;

								if (textures_loaded == raw.bind_textures.length) {
									done(raw);
								}
							}
						});
					}
				}
				else if (tex.generate_mipmaps == true && image != null) {
					image_gen_mipmaps(image, 1000);
					tex.mipmaps = null;
					tex.generate_mipmaps = false;

					if (textures_loaded == raw.bind_textures.length) {
						done(raw);
					}
				}
				else if (textures_loaded == raw.bind_textures.length) {
					done(raw);
				}

			}, false, tex.format != null ? tex.format : "RGBA32");
		}
	}
	else {
		done(raw);
	}
}

function material_context_set_tex_params(raw: material_context_t, texture_index: i32, context: shader_context_t, unit_index: i32) {
	// This function is called by mesh_object_t for samplers set using material context
	shader_context_set_tex_params(context, unit_index, raw.bind_textures[texture_index]);
}
