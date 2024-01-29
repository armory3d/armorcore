
function camera_data_parse(name: string, id: string, done: (data: TCameraData)=>void) {
	Data.getSceneRaw(name, function (format: TSceneFormat) {
		let raw: TCameraData = Data.getCameraRawByName(format.camera_datas, id);
		if (raw == null) {
			Krom.log(`Camera data "${id}" not found!`);
		}
		done(raw);
	});
}
