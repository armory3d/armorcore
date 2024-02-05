
///if arm_audio

class speaker_object_t {
	base: TBaseObject;
	data: speaker_data_t;
	paused = false;
	sound: sound_t = null;
	channels: audio_channel_t[] = [];
	volume: f32;
}

function speaker_object_create(data: speaker_data_t): speaker_object_t {
	let raw = new speaker_object_t();
	raw.base = BaseObject.create();
	raw.base.ext = raw;
	raw.data = data;

	scene_speakers.push(raw);

	if (data.sound == "") return raw;

	Data.getSound(data.sound, (sound: sound_t) => {
		raw.sound = sound;
		App.notifyOnInit(function() {
			if (raw.base.visible && raw.data.play_on_start) {
				speaker_object_play(raw);
			}
		});
	});
	return raw;
}

function speaker_object_play(raw: speaker_object_t) {
	if (raw.sound == null || raw.data.muted) return;
	if (raw.paused) {
		for (let c of raw.channels) {
			audio_play(c);
		}
		raw.paused = false;
		return;
	}
	let channel = audio_channel(raw.sound, raw.data.loop, raw.data.stream);
	if (channel != null) {
		raw.channels.push(channel);
		if (raw.data.attenuation > 0 && raw.channels.length == 1) {
			function _update() { speaker_object_update(raw); }
			(raw as any).update = _update;
			App.notifyOnUpdate(_update);
		}
	}
}

function speaker_object_pause(raw: speaker_object_t) {
	for (let c of raw.channels) audio_pause(c);
	raw.paused = true;
}

function speaker_object_stop(raw: speaker_object_t) {
	for (let c of raw.channels) audio_stop(c);
	raw.channels.splice(0, raw.channels.length);
}

function speaker_object_update(raw: speaker_object_t) {
	if (raw.paused) return;
	for (let c of raw.channels) if (c.finished) array_remove(raw.channels, c);
	if (raw.channels.length == 0) {
		App.removeUpdate((raw as any).update);
		return;
	}

	if (raw.data.attenuation > 0) {
		let distance = vec4_dist(mat4_get_loc(scene_camera.base.transform.world), mat4_get_loc(raw.base.transform.world));
		raw.volume = 1.0 / (1.0 + raw.data.attenuation * (distance - 1.0));
		raw.volume *= raw.data.volume;
	}
	else {
		raw.volume = raw.data.volume;
	}

	for (let c of raw.channels) c.volume = raw.volume;
}

function speaker_object_remove(raw: speaker_object_t) {
	speaker_object_stop(raw);
	array_remove(scene_speakers, this);

	BaseObject.removeSuper(raw.base);
}

///end
