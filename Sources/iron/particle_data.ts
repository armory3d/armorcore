
function particle_data_parse(name: string, id: string, done: (pd: TParticleData)=>void) {
	Data.getSceneRaw(name, function (format: TSceneFormat) {
		let raw: TParticleData = Data.getParticleRawByName(format.particle_datas, id);
		if (raw == null) {
			Krom.log(`Particle data "${id}" not found!`);
		}
		done(raw);
	});
}
