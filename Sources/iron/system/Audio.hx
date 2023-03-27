package iron.system;

class Audio {

#if arm_audio

	public static function play(sound: kha.Sound, loop = false, stream = false): kha.audio1.AudioChannel {
		return stream ? kha.audio1.Audio.stream(sound, loop) : kha.audio1.Audio.play(sound, loop);
	}

#end
}
