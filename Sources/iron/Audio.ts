
///if arm_audio

class Audio {
	static play = (sound: Sound, loop = false, stream = false): AudioChannel => {
		let channel = new AudioChannel(sound, loop);
		channel.play();
		return channel;
	}
}

class AudioChannel {
	sound: Sound;
	loop: bool;

	constructor(sound: Sound, loop: bool) {
		this.sound = sound;
		this.loop = loop;
	}

	play = () => {
		Krom.playSound(this.sound.sound_, this.loop);
	}

	pause = () => {}

	stop = () => {
		Krom.stopSound(this.sound.sound_);
	}

	length: f32;
	volume: f32;
	finished: bool;
}

///end
