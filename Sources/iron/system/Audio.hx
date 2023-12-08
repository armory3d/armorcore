package iron.system;

#if arm_audio

class Audio {
	public static function play(sound: kha.System.Sound, loop = false, stream = false): AudioChannel {
		var channel = new AudioChannel(sound, loop);
		channel.play();
		return channel;
	}
}

class AudioChannel {
	var sound: kha.System.Sound;
	var loop: Bool;

	public function new(sound: kha.System.Sound, loop: Bool) {
		this.sound = sound;
		this.loop = loop;
	}

	public function play(): Void {
		Krom.playSound(sound.sound_, loop);
	}

	public function pause(): Void {}

	public function stop(): Void {
		Krom.stopSound(sound.sound_);
	}

	public var length: Float;
	public var volume: Float;
	public var finished: Bool;
}

#end
