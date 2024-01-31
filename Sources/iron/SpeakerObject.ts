
///if arm_audio

class SpeakerObject {

	base: BaseObject;
	data: TSpeakerData;
	paused = false;
	sound: Sound = null;
	channels: TAudioChannel[] = [];
	volume: f32;

	constructor(data: TSpeakerData) {
		this.base = new BaseObject();
		this.base.ext = this;
		this.base.remove = this.remove;
		this.data = data;

		Scene.speakers.push(this);

		if (data.sound == "") return;

		Data.getSound(data.sound, (sound: Sound) => {
			this.sound = sound;
			App.notifyOnInit(this.init);
		});
	}

	init = () => {
		if (this.base.visible && this.data.play_on_start) this.play();
	}

	play = () => {
		if (this.sound == null || this.data.muted) return;
		if (this.paused) {
			for (let c of this.channels) Audio.play(c);
			this.paused = false;
			return;
		}
		let channel = Audio.channel(this.sound, this.data.loop, this.data.stream);
		if (channel != null) {
			this.channels.push(channel);
			if (this.data.attenuation > 0 && this.channels.length == 1) App.notifyOnUpdate(this.update);
		}
	}

	pause = () => {
		for (let c of this.channels) Audio.pause(c);
		this.paused = true;
	}

	stop = () => {
		for (let c of this.channels) Audio.stop(c);
		this.channels.splice(0, this.channels.length);
	}

	update = () => {
		if (this.paused) return;
		for (let c of this.channels) if (c.finished) array_remove(this.channels, c);
		if (this.channels.length == 0) {
			App.removeUpdate(this.update);
			return;
		}

		if (this.data.attenuation > 0) {
			let distance = Vec4.distance(Scene.camera.base.transform.world.getLoc(), this.base.transform.world.getLoc());
			this.volume = 1.0 / (1.0 + this.data.attenuation * (distance - 1.0));
			this.volume *= this.data.volume;
		}
		else {
			this.volume = this.data.volume;
		}

		for (let c of this.channels) c.volume = this.volume;
	}

	remove = () => {
		this.stop();
		array_remove(Scene.speakers, this);
		this.base.removeSuper();
	}
}

///end
