
///if arm_audio

class Audio {

	static channel = (sound: Sound, loop = false, stream = false): TAudioChannel => {
		let channel = new TAudioChannel();
		channel.sound = sound;
		channel.loop = loop;
		return channel;
	}

	static play = (channel: TAudioChannel) => {
		Krom.playSound(channel.sound.sound_, channel.loop);
	}

	static pause = (channel: TAudioChannel) => {

	}

	static stop = (channel: TAudioChannel) => {
		Krom.stopSound(channel.sound.sound_);
	}
}

class TAudioChannel {
	sound: Sound;
	loop: bool;
	length: f32;
	volume: f32;
	finished: bool;
}

///end
