
function particle_data_parse(name: string, id: string, done: (pd: particle_data_t)=>void) {
	data_get_scene_raw(name, function (format: scene_t) {
		let raw: particle_data_t = data_get_particle_raw_by_name(format.particle_datas, id);
		if (raw == null) {
			krom_log(`Particle data "${id}" not found!`);
		}
		done(raw);
	});
}
