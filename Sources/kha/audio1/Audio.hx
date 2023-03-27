package kha.audio1;

class Audio {

	public static function play(sound: Sound, loop: Bool = false): kha.audio1.AudioChannel {
		var channel = new AudioChannel(sound, loop);
		channel.play();
		return channel;
	}

	public static function stream(sound: Sound, loop: Bool = false): kha.audio1.AudioChannel {
		return play(sound, loop);
	}
}
