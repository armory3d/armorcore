
function camera_data_parse(name: string, id: string, done: (data: camera_data_t)=>void) {
	data_get_scene_raw(name, function (format: scene_t) {
		let raw: camera_data_t = data_get_camera_raw_by_name(format.camera_datas, id);
		if (raw == null) {
			krom_log(`Camera data "${id}" not found!`);
		}
		done(raw);
	});
}
