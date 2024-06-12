
///if arm_audio

type audio_channel_t = {
	sound?: sound_t;
	loop?: bool;
	length?: f32;
	volume?: f32;
	finished?: bool;
};

function audio_channel(sound: sound_t, loop: bool = false, stream: bool = false): audio_channel_t {
	let channel: audio_channel_t = {};
	channel.sound = sound;
	channel.loop = loop;
	return channel;
}

function audio_play(channel: audio_channel_t) {
	krom_play_sound(channel.sound.sound_, channel.loop);
}

function audio_pause(channel: audio_channel_t) {

}

function audio_stop(channel: audio_channel_t) {
	krom_stop_sound(channel.sound.sound_);
}

///end
