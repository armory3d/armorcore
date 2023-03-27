package kha.audio1;

class AudioChannel {
	var sound: kha.Sound;
	var loop: Bool;

	public function new(sound: kha.Sound, loop: Bool) {
		this.sound = sound;
		this.loop = loop;
	}

	public function play(): Void {
		Krom.playSound(sound.sound_, loop);
	}

	public function pause(): Void {

	}

	public function stop(): Void {
		Krom.stopSound(sound.sound_);
	}

	public var length(get, null): Float; // Seconds

	private function get_length(): Float {
		return 0.0;
	}

	public var position(get, set): Float; // Seconds

	private function get_position(): Float {
		return 0.0;
	}

	private function set_position(value: Float): Float {
		return 0.0;
	}

	public var volume(get, set): Float;

	private function get_volume(): Float {
		return 1.0;
	}

	private function set_volume(value: Float): Float {
		return 1.0;
	}

	public var finished(get, null): Bool;

	private function get_finished(): Bool {
		return false;
	}
}
