
function light_data_parse(name: string, id: string, done: (data: TLightData)=>void) {
	Data.getSceneRaw(name, function (format: TSceneFormat) {
		let raw: TLightData = Data.getLightRawByName(format.light_datas, id);
		if (raw == null) {
			Krom.log(`Light data "${id}" not found!`);
		}
		done(raw);
	});
}
