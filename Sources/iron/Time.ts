
class Time {

	static get delta(): f32 {
		if (Time.frequency == null) Time.initFrequency();
		return (1 / Time.frequency);
	}

	static last = 0.0;
	static realDelta = 0.0;

	static time = (): f32 => {
		return System.time;
	}

	static frequency: Null<i32> = null;

	static initFrequency = () => {
		Time.frequency = System.displayFrequency();
	}

	static update = () => {
		Time.realDelta = Time.time() - Time.last;
		Time.last = Time.time();
	}
}
