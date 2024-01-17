
class LightData {

	name: string;
	raw: TLightData;

	constructor(raw: TLightData, done: (data: LightData)=>void) {
		this.raw = raw;
		this.name = raw.name;
		done(this);
	}

	static typeToInt = (s: string): i32 => {
		switch (s) {
			case "sun": return 0;
			case "point": return 1;
			case "spot": return 2;
			case "area": return 3;
			default: return 0;
		}
	}

	static parse = (name: string, id: string, done: (data: LightData)=>void) => {
		Data.getSceneRaw(name, (format: TSceneFormat) => {
			let raw: TLightData = Data.getLightRawByName(format.light_datas, id);
			if (raw == null) {
				Krom.log(`Light data "${id}" not found!`);
				done(null);
			}
			new LightData(raw, done);
		});
	}
}
