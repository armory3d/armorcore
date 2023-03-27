package kha;

class Sound {
	public var sound_: Dynamic;

	public function new(sound_: Dynamic) {
		this.sound_ = sound_;
	}

	public function uncompress(done: Void->Void): Void {
		done();
	}

	public function unload() {
		Krom.unloadSound(sound_);
	}
}
