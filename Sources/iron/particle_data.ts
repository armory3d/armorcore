
function particle_data_parse(name: string, id: string, done: (pd: particle_data_t)=>void) {
	Data.getSceneRaw(name, function (format: scene_t) {
		let raw: particle_data_t = Data.getParticleRawByName(format.particle_datas, id);
		if (raw == null) {
			Krom.log(`Particle data "${id}" not found!`);
		}
		done(raw);
	});
}
