package iron.system;

import iron.System;

class Time {

	public static var delta(get, never): Float;
	static function get_delta(): Float {
		if (frequency == null) initFrequency();
		return (1 / frequency);
	}

	static var last = 0.0;
	public static var realDelta = 0.0;

	public static inline function time(): Float {
		return System.time;
	}

	static var frequency: Null<Int> = null;

	static function initFrequency() {
		frequency = System.displayFrequency();
	}

	public static function update() {
		realDelta = time() - last;
		last = time();
	}
}
