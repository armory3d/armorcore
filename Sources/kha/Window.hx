package kha;

@:enum
abstract WindowMode(Int) {
	var Windowed = 0;            // Use an ordinary window
	var Fullscreen = 1;          // Regular fullscreen mode
	var ExclusiveFullscreen = 2; // Exclusive fullscreen mode (switches monitor resolution, Windows only)
}

@:enum abstract WindowFeatures(Int) to Int {
    var None = 0;
    var FeatureResizable = 1;
    var FeatureMinimizable = 2;
    var FeatureMaximizable = 4;

    function new (value:Int) {
        this = value;
    }

    @:op(A | B) static function or( a:WindowFeatures, b:WindowFeatures) : WindowFeatures;
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

	public function new(title: String = null, ?x: Int = -1, ?y: Int = -1, ?width: Int = 800, ?height: Int = 600, ?display: Int = -1,
	?visible: Bool = true, ?windowFeatures:WindowFeatures, ?mode: WindowMode = WindowMode.Windowed) {
		this.title = title;
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
		this.display = display;
		this.visible = visible;
		this.windowFeatures = (windowFeatures == null) ? WindowFeatures.FeatureResizable | WindowFeatures.FeatureMaximizable | WindowFeatures.FeatureMinimizable : windowFeatures;
		this.mode = mode;
	}
}

@:structInit
class FramebufferOptions {
	@:optional public var frequency: Int = 60;
	@:optional public var verticalSync: Bool = true;
	@:optional public var colorBufferBits: Int = 32;
	@:optional public var samplesPerPixel: Int = 1;

	public function new(?frequency: Int = 60, ?verticalSync: Bool = true, ?colorBufferBits: Int = 32, ?depthBufferBits: Int = 16, ?stencilBufferBits: Int = 8, ?samplesPerPixel: Int = 1) {
		this.frequency = frequency;
		this.verticalSync = verticalSync;
		this.colorBufferBits = colorBufferBits;
		this.samplesPerPixel = samplesPerPixel;
	}
}

class Window {
	static var windows: Array<Window> = [];
	var num: Int;
	var windowTitle: String;

	public function new(num: Int) {
		this.num = num;
		windows.push(this);
	}

	public static function get(index: Int): Window {
		return windows[index];
	}

	public function resize(width: Int, height: Int): Void {
		Krom.resizeWindow(num, width, height);
	}

	public function move(x: Int, y: Int): Void {
		Krom.moveWindow(num, x, y);
	}

	public var x(get, never): Int;
	function get_x(): Int {
		return Krom.windowX(num);
	}

	public var y(get, never): Int;
	function get_y(): Int {
		return Krom.windowY(num);
	}

	public var width(get, never): Int;
	function get_width(): Int {
		return Krom.windowWidth(num);
	}

	public var height(get, never): Int;
	function get_height(): Int {
		return Krom.windowHeight(num);
	}

	public var mode(get, set): WindowMode;

	function get_mode(): WindowMode {
		return cast Krom.getWindowMode(num);
	}

	function set_mode(mode: WindowMode): WindowMode {
		Krom.setWindowMode(num, cast mode);
		return mode;
	}

	public var title(get, set): String;

	function get_title(): String {
		return windowTitle;
	}

	function set_title(value: String): String {
		Krom.setWindowTitle(num, value);
		windowTitle = value;
		return windowTitle;
	}
}
