
function camera_data_parse(name: string, id: string, done: (data: camera_data_t)=>void) {
	Data.getSceneRaw(name, function (format: scene_t) {
		let raw: camera_data_t = Data.getCameraRawByName(format.camera_datas, id);
		if (raw == null) {
			Krom.log(`Camera data "${id}" not found!`);
		}
		done(raw);
	});
}
