
///if arm_audio

type speaker_object_t = {
	base?: object_t;
	data?: speaker_data_t;
	paused?: bool;
	sound?: sound_t;
	channels?: audio_channel_t[];
	volume?: f32;
};

function speaker_object_create(data: speaker_data_t): speaker_object_t {
	let raw: speaker_object_t = {};
	raw.paused = false;
	raw.channels = [];
	raw.base = object_create(false);
	raw.base.ext = raw;
	raw.base.ext_type = "speaker_object_t";
	raw.data = data;

	scene_speakers.push(raw);

	if (data.sound == "") {
		return raw;
	}

	data_get_sound(data.sound, function (sound: sound_t) {
		raw.sound = sound;
		app_notify_on_init(function () {
			if (raw.base.visible && raw.data.play_on_start) {
				speaker_object_play(raw);
			}
		});
	});
	return raw;
}

function speaker_object_play(raw: speaker_object_t) {
	if (raw.sound == null || raw.data.muted) {
		return;
	}
	if (raw.paused) {
		for (let i: i32 = 0; i < raw.channels.length; ++i) {
			let c: audio_channel_t = raw.channels[i];
			audio_play(c);
		}
		raw.paused = false;
		return;
	}
	let channel: audio_channel_t = audio_channel(raw.sound, raw.data.loop, raw.data.stream);
	if (channel != null) {
		raw.channels.push(channel);
		if (raw.data.attenuation > 0 && raw.channels.length == 1) {
			function _update() {
				speaker_object_update(raw);
			}
			(raw as any).update = _update;
			app_notify_on_update(_update);
		}
	}
}

function speaker_object_pause(raw: speaker_object_t) {
	for (let i: i32 = 0; i < raw.channels.length; ++i) {
		let c: audio_channel_t = raw.channels[i];
		audio_pause(c);
	}
	raw.paused = true;
}

function speaker_object_stop(raw: speaker_object_t) {
	for (let i: i32 = 0; i < raw.channels.length; ++i) {
		let c: audio_channel_t = raw.channels[i];
		audio_stop(c);
	}
	raw.channels.splice(0, raw.channels.length);
}

function speaker_object_update(raw: speaker_object_t) {
	if (raw.paused) {
		return;
	}
	for (let i: i32 = 0; i < raw.channels.length; ++i) {
		let c: audio_channel_t = raw.channels[i];
		if (c.finished) {
			array_remove(raw.channels, c);
		}
	}
	if (raw.channels.length == 0) {
		app_remove_update((raw as any).update);
		return;
	}

	if (raw.data.attenuation > 0) {
		let distance: f32 = vec4_dist(mat4_get_loc(scene_camera.base.transform.world), mat4_get_loc(raw.base.transform.world));
		raw.volume = 1.0 / (1.0 + raw.data.attenuation * (distance - 1.0));
		raw.volume *= raw.data.volume;
	}
	else {
		raw.volume = raw.data.volume;
	}

	for (let i: i32 = 0; i < raw.channels.length; ++i) {
		let c: audio_channel_t = raw.channels[i];
		c.volume = raw.volume;
	}
}

function speaker_object_remove(raw: speaker_object_t) {
	speaker_object_stop(raw);
	array_remove(scene_speakers, raw);
	object_remove_super(raw.base);
}

///end
