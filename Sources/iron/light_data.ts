
function light_data_parse(name: string, id: string, done: (data: light_data_t)=>void) {
	data_get_scene_raw(name, function (format: scene_t) {
		let raw: light_data_t = data_get_light_raw_by_name(format.light_datas, id);
		if (raw == null) {
			Krom.log(`Light data "${id}" not found!`);
		}
		done(raw);
	});
}
