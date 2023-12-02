package kha;

class DisplayMode {
	public var width: Int;
	public var height: Int;
	public var frequency: Int;
	public var bitsPerPixel: Int;

	public function new(width: Int, height: Int, frequency: Int, bitsPerPixel: Int) {
		this.width = width;
		this.height = height;
		this.frequency = frequency;
		this.bitsPerPixel = bitsPerPixel;
	}
}

class Display {
	static var displays: Array<Display> = [];
	var num: Int;
	var isPrimary: Bool;

	function new(num: Int, isPrimary: Bool) {
		this.num = num;
		this.isPrimary = isPrimary;
	}

	static function init(): Void {
		if (displays.length > 0) return;
		for (i in 0...Krom.displayCount()) {
			displays.push(new Display(i, Krom.displayIsPrimary(i)));
		}
	}

	public static var primary(get, never): Display;

	static function get_primary(): Display {
		init();
		for (display in displays) {
			if (display.isPrimary) {
				return display;
			}
		}
		return null;
	}

	public var width(get, never): Int;

	function get_width(): Int {
		return Krom.displayWidth(num);
	}

	public var height(get, never): Int;

	function get_height(): Int {
		return Krom.displayHeight(num);
	}

	public var frequency(get, never): Int;

	function get_frequency(): Int {
		return Krom.displayFrequency(num);
	}
}
