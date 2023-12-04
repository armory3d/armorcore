package kha;

@:enum abstract WindowMode(Int) to Int {
	var Windowed = 0;            // Use an ordinary window
	var Fullscreen = 1;          // Regular fullscreen mode
}

@:enum abstract WindowFeatures(Int) from Int to Int {
    var None = 0;
    var FeatureResizable = 1;
    var FeatureMinimizable = 2;
    var FeatureMaximizable = 4;

}

@:structInit
class WindowOptions {
	@:optional public var title: String = null;
	@:optional public var x: Int = -1;
	@:optional public var y: Int = -1;
	@:optional public var width: Int = 800;
	@:optional public var height: Int = 600;
	@:optional public var display: Int = -1;
	@:optional public var visible: Bool = true;
	@:optional public var windowFeatures:WindowFeatures = FeatureResizable | FeatureMaximizable | FeatureMinimizable;
	@:optional public var mode: WindowMode = Windowed;
}

@:structInit
class FramebufferOptions {
	@:optional public var frequency: Int = 60;
	@:optional public var verticalSync: Bool = true;
	@:optional public var colorBufferBits: Int = 32;
	@:optional public var samplesPerPixel: Int = 1;
}

class Window {
	static var inst: Window;
	var windowTitle: String;

	public function new() {
		inst = this;
	}

	public static function get(): Window {
		return inst;
	}

	public function resize(width: Int, height: Int): Void {
		Krom.resizeWindow(0, width, height);
	}

	public function move(x: Int, y: Int): Void {
		Krom.moveWindow(0, x, y);
	}

	public var x(get, never): Int;
	function get_x(): Int {
		return Krom.windowX(0);
	}

	public var y(get, never): Int;
	function get_y(): Int {
		return Krom.windowY(0);
	}

	public var width(get, never): Int;
	function get_width(): Int {
		return Krom.windowWidth(0);
	}

	public var height(get, never): Int;
	function get_height(): Int {
		return Krom.windowHeight(0);
	}

	public var mode(get, set): WindowMode;

	function get_mode(): WindowMode {
		return cast Krom.getWindowMode(0);
	}

	function set_mode(mode: WindowMode): WindowMode {
		Krom.setWindowMode(0, cast mode);
		return mode;
	}

	public var title(get, set): String;

	function get_title(): String {
		return windowTitle;
	}

	function set_title(value: String): String {
		Krom.setWindowTitle(0, value);
		windowTitle = value;
		return windowTitle;
	}
}
