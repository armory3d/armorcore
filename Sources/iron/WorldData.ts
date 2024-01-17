
class WorldData {

	name: string;
	raw: TWorldData;
	envmap: Image;
	probe: Probe;

	static emptyIrr: Float32Array = null;

	constructor(raw: TWorldData, done: (wd: WorldData)=>void) {
		this.raw = raw;
		this.name = raw.name;

		// Parse probes
		if (raw.probe != null) {
			new Probe(raw.probe, (self: Probe) => {
				this.probe = self;
				done(this);
			});
		}
		else {
			done(this);
		}
	}

	loadEnvmap = (done: (wd: WorldData)=>void) => {
		if (this.raw.envmap != null) {
			Data.getImage(this.raw.envmap, (image: Image) => {
				this.envmap = image;
				done(this);
			});
		}
		else done(this);
	}

	static parse = (name: string, id: string, done: (wd: WorldData)=>void) => {
		Data.getSceneRaw(name, (format: TSceneFormat) => {
			let raw: TWorldData = Data.getWorldRawByName(format.world_datas, id);
			if (raw == null) {
				Krom.log(`World data "${id}" not found!`);
				done(null);
			}
			new WorldData(raw, done);
		});
	}

	static getEmptyIrradiance = (): Float32Array => {
		if (WorldData.emptyIrr == null) {
			WorldData.emptyIrr = new Float32Array(28);
			for (let i = 0; i < WorldData.emptyIrr.length; ++i) WorldData.emptyIrr[i] = 0.0;
		}
		return WorldData.emptyIrr;
	}
}

class Probe {

	raw: TProbeData;
	radiance: Image;
	radianceMipmaps: Image[] = [];
	irradiance: Float32Array;

	constructor(raw: TProbeData, done: (p: Probe)=>void) {
		this.raw = raw;

		this.setIrradiance((irr: Float32Array) => {
			this.irradiance = irr;
			if (raw.radiance != null) {
				Data.getImage(raw.radiance, (rad: Image) => {
					this.radiance = rad;
					while (this.radianceMipmaps.length < raw.radiance_mipmaps) this.radianceMipmaps.push(null);
					let dot = raw.radiance.lastIndexOf(".");
					let ext = raw.radiance.substring(dot);
					let base = raw.radiance.substring(0, dot);

					let mipsLoaded = 0;
					for (let i = 0; i < raw.radiance_mipmaps; ++i) {
						Data.getImage(base + "_" + i + ext, (mipimg: Image) => {
							this.radianceMipmaps[i] = mipimg;
							mipsLoaded++;

							if (mipsLoaded == raw.radiance_mipmaps) {
								this.radiance.setMipmaps(this.radianceMipmaps);
								done(this);
							}
						}, true); // Readable
					}
				});
			}
			else done(this);
		});
	}

	setIrradiance = (done: (ar: Float32Array)=>void) => {
		// Parse probe data
		if (this.raw.irradiance == null) {
			done(WorldData.getEmptyIrradiance());
		}
		else {
			let ext = this.raw.irradiance.endsWith(".json") ? "" : ".arm";
			Data.getBlob(this.raw.irradiance + ext, (b: ArrayBuffer) => {
				let irradianceParsed: TSceneFormat = ext == "" ?
					JSON.parse(System.bufferToString(b)) :
					ArmPack.decode(b);
				let irr = new Float32Array(28); // Align to mult of 4 - 27->28
				for (let i = 0; i < 27; ++i) irr[i] = irradianceParsed.irradiance[i];
				done(irr);
			});
		}
	}
}
