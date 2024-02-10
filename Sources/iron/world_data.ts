
let _world_data_empty_irr: Float32Array = null;

function world_data_parse(name: string, id: string, done: (wd: world_data_t)=>void) {
	data_get_scene_raw(name, function (format: scene_t) {
		let raw: world_data_t = data_get_world_raw_by_name(format.world_datas, id);
		if (raw == null) {
			krom_log(`World data "${id}" not found!`);
			done(null);
		}

		raw._radiance_mipmaps = [];

		world_data_set_irradiance(raw, function (irr: Float32Array) {
			raw._irradiance = irr;
			if (raw.radiance != null) {
				data_get_image(raw.radiance, function (rad: image_t) {
					raw._radiance = rad;
					while (raw._radiance_mipmaps.length < raw.radiance_mipmaps) {
						raw._radiance_mipmaps.push(null);
					}
					let dot = raw.radiance.lastIndexOf(".");
					let ext = raw.radiance.substring(dot);
					let base = raw.radiance.substring(0, dot);

					let mips_loaded = 0;
					for (let i = 0; i < raw.radiance_mipmaps; ++i) {
						data_get_image(base + "_" + i + ext, function (mipimg: image_t) {
							raw._radiance_mipmaps[i] = mipimg;
							mips_loaded++;

							if (mips_loaded == raw.radiance_mipmaps) {
								image_set_mipmaps(raw._radiance, raw._radiance_mipmaps);
								done(raw);
							}
						}, true); // Readable
					}
				});
			}
			else done(raw);
		});
	});
}

function world_data_get_empty_irradiance(): Float32Array {
	if (_world_data_empty_irr == null) {
		_world_data_empty_irr = new Float32Array(28);
		for (let i = 0; i < _world_data_empty_irr.length; ++i) {
			_world_data_empty_irr[i] = 0.0;
		}
	}
	return _world_data_empty_irr;
}

function world_data_set_irradiance(raw: world_data_t, done: (ar: Float32Array)=>void) {
	if (raw.irradiance == null) {
		done(world_data_get_empty_irradiance());
	}
	else {
		data_get_blob(raw.irradiance + ".arm", function (b: ArrayBuffer) {
			let irradiance_parsed: irradiance_t = armpack_decode(b);
			let irr = new Float32Array(28); // Align to mult of 4 - 27->28
			for (let i = 0; i < 27; ++i) {
				irr[i] = irradiance_parsed.irradiance[i];
			}
			done(irr);
		});
	}
}

function world_data_load_envmap(raw: world_data_t, done: (wd: world_data_t)=>void) {
	if (raw.envmap != null) {
		data_get_image(raw.envmap, function (image: image_t) {
			raw._envmap = image;
			done(raw);
		});
	}
	else {
		done(raw);
	}
}
