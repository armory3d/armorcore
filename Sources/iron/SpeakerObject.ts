
///if arm_audio

class TSpeakerObject {
	base: TBaseObject;
	data: TSpeakerData;
	paused = false;
	sound: Sound = null;
	channels: TAudioChannel[] = [];
	volume: f32;
}

class SpeakerObject {

	static create(data: TSpeakerData): TSpeakerObject {
		let raw = new TSpeakerObject();
		raw.base = BaseObject.create();
		raw.base.ext = raw;
		raw.data = data;

		Scene.speakers.push(raw);

		if (data.sound == "") return raw;

		Data.getSound(data.sound, (sound: Sound) => {
			raw.sound = sound;
			App.notifyOnInit(function() {
				if (raw.base.visible && raw.data.play_on_start) {
					SpeakerObject.play(raw);
				}
			});
		});
		return raw;
	}

	static play = (raw: TSpeakerObject) => {
		if (raw.sound == null || raw.data.muted) return;
		if (raw.paused) {
			for (let c of raw.channels) {
				Audio.play(c);
			}
			raw.paused = false;
			return;
		}
		let channel = Audio.channel(raw.sound, raw.data.loop, raw.data.stream);
		if (channel != null) {
			raw.channels.push(channel);
			if (raw.data.attenuation > 0 && raw.channels.length == 1) {
				function _update() { SpeakerObject.update(raw); }
				(raw as any).update = _update;
				App.notifyOnUpdate(_update);
			}
		}
	}

	static pause = (raw: TSpeakerObject) => {
		for (let c of raw.channels) Audio.pause(c);
		raw.paused = true;
	}

	static stop = (raw: TSpeakerObject) => {
		for (let c of raw.channels) Audio.stop(c);
		raw.channels.splice(0, raw.channels.length);
	}

	static update = (raw: TSpeakerObject) => {
		if (raw.paused) return;
		for (let c of raw.channels) if (c.finished) array_remove(raw.channels, c);
		if (raw.channels.length == 0) {
			App.removeUpdate((raw as any).update);
			return;
		}

		if (raw.data.attenuation > 0) {
			let distance = Vec4.distance(Mat4.getLoc(Scene.camera.base.transform.world), Mat4.getLoc(raw.base.transform.world));
			raw.volume = 1.0 / (1.0 + raw.data.attenuation * (distance - 1.0));
			raw.volume *= raw.data.volume;
		}
		else {
			raw.volume = raw.data.volume;
		}

		for (let c of raw.channels) c.volume = raw.volume;
	}

	static remove = (raw: TSpeakerObject) => {
		SpeakerObject.stop(raw);
		array_remove(Scene.speakers, this);

		BaseObject.removeSuper(raw.base);
	}
}

///end
