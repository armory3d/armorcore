
function light_data_parse(name: string, id: string, done: (data: light_data_t)=>void) {
	Data.getSceneRaw(name, function (format: scene_t) {
		let raw: light_data_t = Data.getLightRawByName(format.light_datas, id);
		if (raw == null) {
			Krom.log(`Light data "${id}" not found!`);
		}
		done(raw);
	});
}
