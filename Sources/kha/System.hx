package kha;

import iron.system.Input;

@:structInit
class SystemOptions {
	public var title: String;
	public var x: Int;
	public var y: Int;
	public var width: Int;
	public var height: Int;
	public var features: WindowFeatures;
	public var mode: WindowMode;
	public var frequency: Int;
	public var vsync: Bool;
}

class System {
	static var renderListeners: Array<Graphics2 -> Graphics4 -> Void> = [];
	static var foregroundListeners: Array<Void -> Void> = [];
	static var resumeListeners: Array<Void -> Void> = [];
	static var pauseListeners: Array<Void -> Void> = [];
	static var backgroundListeners: Array<Void -> Void> = [];
	static var shutdownListeners: Array<Void -> Void> = [];
	static var dropFilesListeners: Array<String -> Void> = [];
	static var cutListener: Void->String = null;
	static var copyListener: Void->String = null;
	static var pasteListener: String->Void = null;

	static var startTime: Float;
	static var g2: Graphics2;
	static var g4: Graphics4;
	static var keyboard: Keyboard;
	static var mouse: Mouse;
	static var surface: Surface;
	static var pen: Pen;
	static var maxGamepads: Int = 4;
	static var gamepads: Array<Gamepad>;
	static var windowTitle: String;

	public static function start(options: SystemOptions, callback: Void -> Void): Void {
		Krom.init(options.title, options.width, options.height, options.vsync, options.mode, options.features, options.x, options.y, options.frequency);

		startTime = Krom.getTime();
		haxe.Log.trace = function(v: Dynamic, ?infos: haxe.PosInfos) {
			Krom.log(haxe.Log.formatOutput(v, infos));
		};

		g4 = new kha.Graphics4();
		g2 = new kha.Graphics2(g4, null);
		Krom.setCallback(renderCallback);
		Krom.setDropFilesCallback(dropFilesCallback);
		Krom.setCutCopyPasteCallback(cutCallback, copyCallback, pasteCallback);
		Krom.setApplicationStateCallback(foregroundCallback, resumeCallback, pauseCallback, backgroundCallback, shutdownCallback);
		Krom.setKeyboardDownCallback(keyboardDownCallback);
		Krom.setKeyboardUpCallback(keyboardUpCallback);
		Krom.setKeyboardPressCallback(keyboardPressCallback);
		Krom.setMouseDownCallback(mouseDownCallback);
		Krom.setMouseUpCallback(mouseUpCallback);
		Krom.setMouseMoveCallback(mouseMoveCallback);
		Krom.setMouseWheelCallback(mouseWheelCallback);
		Krom.setTouchDownCallback(touchDownCallback);
		Krom.setTouchUpCallback(touchUpCallback);
		Krom.setTouchMoveCallback(touchMoveCallback);
		Krom.setPenDownCallback(penDownCallback);
		Krom.setPenUpCallback(penUpCallback);
		Krom.setPenMoveCallback(penMoveCallback);
		Krom.setGamepadAxisCallback(gamepadAxisCallback);
		Krom.setGamepadButtonCallback(gamepadButtonCallback);

		keyboard = iron.system.Input.getKeyboard();
		mouse = iron.system.Input.getMouse();
		surface = iron.system.Input.getSurface();
		pen = iron.system.Input.getPen();
		gamepads = new Array<Gamepad>();
		for (i in 0...maxGamepads) {
			gamepads[i] = iron.system.Input.getGamepad(i);
		}

		callback();
	}

	public static function notifyOnFrames(listener: Graphics2 -> Graphics4 -> Void): Void {
		renderListeners.push(listener);
	}

	public static function notifyOnApplicationState(foregroundListener: Void -> Void, resumeListener: Void -> Void,	pauseListener: Void -> Void, backgroundListener: Void-> Void, shutdownListener: Void -> Void): Void {
		if (foregroundListener != null) foregroundListeners.push(foregroundListener);
		if (resumeListener != null) resumeListeners.push(resumeListener);
		if (pauseListener != null) pauseListeners.push(pauseListener);
		if (backgroundListener != null) backgroundListeners.push(backgroundListener);
		if (shutdownListener != null) shutdownListeners.push(shutdownListener);
	}

	public static function notifyOnDropFiles(dropFilesListener: String -> Void): Void {
		dropFilesListeners.push(dropFilesListener);
	}

	public static function notifyOnCutCopyPaste(cutListener: Void->String, copyListener: Void->String, pasteListener: String->Void): Void {
		System.cutListener = cutListener;
		System.copyListener = copyListener;
		System.pasteListener = pasteListener;
	}

	static function foreground(): Void {
		for (listener in foregroundListeners) {
			listener();
		}
	}

	static function resume(): Void {
		for (listener in resumeListeners) {
			listener();
		}
	}

	static function pause(): Void {
		for (listener in pauseListeners) {
			listener();
		}
	}

	static function background(): Void {
		for (listener in backgroundListeners) {
			listener();
		}
	}

	static function shutdown(): Void {
		for (listener in shutdownListeners) {
			listener();
		}
	}

	static function dropFiles(filePath: String): Void {
		for (listener in dropFilesListeners) {
			listener(filePath);
		}
	}

	public static var time(get, null): Float;
	static function get_time(): Float {
		return Krom.getTime() - startTime;
	}

	public static var systemId(get, null): String;
	static function get_systemId(): String {
		return Krom.systemId();
	}

	public static var language(get, never): String;
	static function get_language(): String {
		return Krom.language();
	}

	public static function stop(): Void {
		Krom.requestShutdown();
	}

	public static function loadUrl(url: String): Void {
		Krom.loadUrl(url);
	}

	static function renderCallback(): Void {
		for (listener in renderListeners) {
			listener(g2, g4);
		}
	}

	static function dropFilesCallback(filePath: String): Void {
		System.dropFiles(filePath);
	}

	static function copyCallback(): String {
		if (System.copyListener != null) {
			return System.copyListener();
		}
		return null;
	}

	static function cutCallback(): String {
		if (System.cutListener != null) {
			return System.cutListener();
		}
		return null;
	}

	static function pasteCallback(data: String): Void {
		if (System.pasteListener != null) {
			System.pasteListener(data);
		}
	}

	static function foregroundCallback(): Void {
		System.foreground();
	}

	static function resumeCallback(): Void {
		System.resume();
	}

	static function pauseCallback(): Void {
		System.pause();
	}

	static function backgroundCallback(): Void {
		System.background();
	}

	static function shutdownCallback(): Void {
		System.shutdown();
	}

	static function keyboardDownCallback(code: Int): Void {
		keyboard.downListener(code);
	}

	static function keyboardUpCallback(code: Int): Void {
		keyboard.upListener(code);
	}

	static function keyboardPressCallback(charCode: Int): Void {
		keyboard.pressListener(String.fromCharCode(charCode));
	}

	static function mouseDownCallback(button: Int, x: Int, y: Int): Void {
		mouse.downListener(button, x, y);
	}

	static function mouseUpCallback(button: Int, x: Int, y: Int): Void {
		mouse.upListener(button, x, y);
	}

	static function mouseMoveCallback(x: Int, y: Int, mx: Int, my: Int): Void {
		mouse.moveListener(x, y, mx, my);
	}

	static function mouseWheelCallback(delta: Int): Void {
		mouse.wheelListener(delta);
	}

	static function touchDownCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchDown(index, x, y);
		#end
	}

	static function touchUpCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchUp(index, x, y);
		#end
	}

	static function touchMoveCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchMove(index, x, y);
		#end
	}

	static function penDownCallback(x: Int, y: Int, pressure: Float): Void {
		pen.downListener(x, y, pressure);
	}

	static function penUpCallback(x: Int, y: Int, pressure: Float): Void {
		pen.upListener(x, y, pressure);
	}

	static function penMoveCallback(x: Int, y: Int, pressure: Float): Void {
		pen.moveListener(x, y, pressure);
	}

	static function gamepadAxisCallback(gamepad: Int, axis: Int, value: Float): Void {
		gamepads[gamepad].axisListener(axis, value);
	}

	static function gamepadButtonCallback(gamepad: Int, button: Int, value: Float): Void {
		gamepads[gamepad].buttonListener(button, value);
	}

	public static function lockMouse(): Void {
		if (!isMouseLocked()){
			Krom.lockMouse();
		}
	}

	public static function unlockMouse(): Void {
		if (isMouseLocked()){
			Krom.unlockMouse();
		}
	}

	public static function canLockMouse(): Bool {
		return Krom.canLockMouse();
	}

	public static function isMouseLocked(): Bool {
		return Krom.isMouseLocked();
	}

	public static function hideSystemCursor(): Void {
		Krom.showMouse(false);
	}

	public static function showSystemCursor(): Void {
		Krom.showMouse(true);
	}

	public static function resize(width: Int, height: Int): Void {
		Krom.resizeWindow(width, height);
	}

	public static function move(x: Int, y: Int): Void {
		Krom.moveWindow(x, y);
	}

	public static var x(get, never): Int;
	static function get_x(): Int {
		return Krom.windowX();
	}

	public static var y(get, never): Int;
	static function get_y(): Int {
		return Krom.windowY();
	}

	public static var width(get, never): Int;
	static function get_width(): Int {
		return Krom.windowWidth();
	}

	public static var height(get, never): Int;
	static function get_height(): Int {
		return Krom.windowHeight();
	}

	public static var mode(get, set): WindowMode;

	static function get_mode(): WindowMode {
		return cast Krom.getWindowMode();
	}

	static function set_mode(mode: WindowMode): WindowMode {
		Krom.setWindowMode(mode);
		return mode;
	}

	public static var title(get, set): String;

	static function get_title(): String {
		return windowTitle;
	}

	static function set_title(value: String): String {
		Krom.setWindowTitle(value);
		windowTitle = value;
		return windowTitle;
	}

	static function displayPrimaryId(): Int {
		for (i in 0...Krom.displayCount()) {
			if (Krom.displayIsPrimary(i)) return i;
		}
		return 0;
	}

	public static function displayWidth(): Int {
		return Krom.displayWidth(displayPrimaryId());
	}

	public static function displayHeight(): Int {
		return Krom.displayHeight(displayPrimaryId());
	}

	public static function displayFrequency(): Int {
		return Krom.displayFrequency(displayPrimaryId());
	}

	public static function bufferToString(b: js.lib.ArrayBuffer): String {
		var str = "";
		var u8 = new js.lib.Uint8Array(b);
		for (i in 0...u8.length) {
			str += String.fromCharCode(u8[i]);
		}
		return str;
	}

	public static function stringToBuffer(str: String): js.lib.ArrayBuffer {
		var u8 = new js.lib.Uint8Array(str.length);
		for (i in 0...str.length) {
			u8[i] = str.charCodeAt(i);
		}
		return u8.buffer;
	}
}

@:enum abstract WindowFeatures(Int) from Int to Int {
    var None = 0;
    var FeatureResizable = 1;
    var FeatureMinimizable = 2;
    var FeatureMaximizable = 4;
}

@:enum abstract WindowMode(Int) from Int to Int {
	var Windowed = 0;            // Use an ordinary window
	var Fullscreen = 1;          // Regular fullscreen mode
}
