
class WorldData {

	static emptyIrr: Float32Array = null;

	static parse = (name: string, id: string, done: (wd: TWorldData)=>void) => {
		Data.getSceneRaw(name, (format: TSceneFormat) => {
			let raw: TWorldData = Data.getWorldRawByName(format.world_datas, id);
			if (raw == null) {
				Krom.log(`World data "${id}" not found!`);
				done(null);
			}

			raw._radianceMipmaps = [];

			WorldData.setIrradiance(raw, (irr: Float32Array) => {
				raw._irradiance = irr;
				if (raw.radiance != null) {
					Data.getImage(raw.radiance, (rad: ImageRaw) => {
						raw._radiance = rad;
						while (raw._radianceMipmaps.length < raw.radiance_mipmaps) {
							raw._radianceMipmaps.push(null);
						}
						let dot = raw.radiance.lastIndexOf(".");
						let ext = raw.radiance.substring(dot);
						let base = raw.radiance.substring(0, dot);

						let mipsLoaded = 0;
						for (let i = 0; i < raw.radiance_mipmaps; ++i) {
							Data.getImage(base + "_" + i + ext, (mipimg: ImageRaw) => {
								raw._radianceMipmaps[i] = mipimg;
								mipsLoaded++;

								if (mipsLoaded == raw.radiance_mipmaps) {
									Image.setMipmaps(raw._radiance, raw._radianceMipmaps);
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

	static getEmptyIrradiance = (): Float32Array => {
		if (WorldData.emptyIrr == null) {
			WorldData.emptyIrr = new Float32Array(28);
			for (let i = 0; i < WorldData.emptyIrr.length; ++i) WorldData.emptyIrr[i] = 0.0;
		}
		return WorldData.emptyIrr;
	}

	static setIrradiance = (raw: TWorldData, done: (ar: Float32Array)=>void) => {
		if (raw.irradiance == null) {
			done(WorldData.getEmptyIrradiance());
		}
		else {
			Data.getBlob(raw.irradiance + ".arm", (b: ArrayBuffer) => {
				let irradianceParsed: TIrradiance = ArmPack.decode(b);
				let irr = new Float32Array(28); // Align to mult of 4 - 27->28
				for (let i = 0; i < 27; ++i) {
					irr[i] = irradianceParsed.irradiance[i];
				}
				done(irr);
			});
		}
	}

	static loadEnvmap = (raw:TWorldData, done: (wd: TWorldData)=>void) => {
		if (raw.envmap != null) {
			Data.getImage(raw.envmap, (image: ImageRaw) => {
				raw._envmap = image;
				done(raw);
			});
		}
		else done(raw);
	}
}
