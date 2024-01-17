
class ParticleData {

	name: string;
	raw: TParticleData;

	constructor(raw: TParticleData, done: (pd: ParticleData)=>void) {
		this.raw = raw;
		this.name = raw.name;

		done(this);
	}

	static parse = (name: string, id: string, done: (pd: ParticleData)=>void) => {
		Data.getSceneRaw(name, (format: TSceneFormat) => {
			let raw: TParticleData = Data.getParticleRawByName(format.particle_datas, id);
			if (raw == null) {
				Krom.log(`Particle data "${id}" not found!`);
				done(null);
			}
			new ParticleData(raw, done);
		});
	}
}
