
class CameraData {

	name: string;
	raw: TCameraData;

	constructor(raw: TCameraData, done: (data: CameraData)=>void) {
		this.raw = raw;
		this.name = raw.name;
		done(this);
	}

	static parse = (name: string, id: string, done: (data: CameraData)=>void) => {
		Data.getSceneRaw(name, (format: TSceneFormat) => {
			let raw: TCameraData = Data.getCameraRawByName(format.camera_datas, id);
			if (raw == null) {
				Krom.log(`Camera data "${id}" not found!`);
				done(null);
			}
			new CameraData(raw, done);
		});
	}
}
