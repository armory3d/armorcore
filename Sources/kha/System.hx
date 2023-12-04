package kha;

import kha.Window.FramebufferOptions;
import kha.Window.WindowOptions;
import iron.system.Input.Gamepad;
import iron.system.Input.Keyboard;
import iron.system.Input.Surface;
import iron.system.Input.Mouse;
import iron.system.Input.Pen;
import kha.System;

@:structInit
class SystemOptions {
	@:optional public var title: String = "Kha";
	@:optional public var window: WindowOptions = null;
	@:optional public var framebuffer: FramebufferOptions = null;
}

class System {
	static var renderListeners: Array<Framebuffer -> Void> = [];
	static var foregroundListeners: Array<Void -> Void> = [];
	static var resumeListeners: Array<Void -> Void> = [];
	static var pauseListeners: Array<Void -> Void> = [];
	static var backgroundListeners: Array<Void -> Void> = [];
	static var shutdownListeners: Array<Void -> Void> = [];
	static var dropFilesListeners: Array<String -> Void> = [];
	static var cutListener: Void->String = null;
	static var copyListener: Void->String = null;
	static var pasteListener: String->Void = null;

	private static var startTime: Float;
	private static var framebuffer: Framebuffer;
	private static var keyboard: Keyboard;
	private static var mouse: Mouse;
	private static var surface: Surface;
	private static var pen: Pen;
	private static var maxGamepads: Int = 4;
	private static var gamepads: Array<Gamepad>;

	public static function start(options: SystemOptions, callback: Window -> Void): Void {
		Krom.init(options.title, options.window.width, options.window.height, options.framebuffer.samplesPerPixel, options.framebuffer.verticalSync, cast options.window.mode, options.window.windowFeatures, 0, options.window.x, options.window.y, options.framebuffer.frequency);

		startTime = Krom.getTime();

		haxe.Log.trace = function(v: Dynamic, ?infos: haxe.PosInfos) {
			var message = haxe.Log.formatOutput(v, infos);
			Krom.log(message);
		};

		new Window();

		var g4 = new kha.Graphics4();
		framebuffer = new Framebuffer(null, g4);
		framebuffer.init(new kha.Graphics2(framebuffer), g4);
		Krom.setCallback(renderCallback);
		Krom.setDropFilesCallback(dropFilesCallback);
		Krom.setCutCopyPasteCallback(cutCallback, copyCallback, pasteCallback);
		Krom.setApplicationStateCallback(foregroundCallback, resumeCallback, pauseCallback, backgroundCallback, shutdownCallback);

		keyboard = iron.system.Input.getKeyboard();
		mouse = iron.system.Input.getMouse();
		surface = iron.system.Input.getSurface();
		pen = iron.system.Input.getPen();
		gamepads = new Array<Gamepad>();
		for (i in 0...maxGamepads) {
			gamepads[i] = iron.system.Input.getGamepad(i);
		}

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

		callback(Window.get());
	}

	public static function notifyOnFrames(listener: Framebuffer -> Void): Void {
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

	public static function removeDropListerer(listener: String -> Void): Void {
		dropFilesListeners.remove(listener);
	}

	public static function notifyOnCutCopyPaste(cutListener: Void->String, copyListener: Void->String, pasteListener: String->Void): Void {
		System.cutListener = cutListener;
		System.copyListener = copyListener;
		System.pasteListener = pasteListener;
	}

	static function render(framebuffer: Framebuffer): Void {
		for (listener in renderListeners) {
			listener(framebuffer);
		}
	}

	private static function foreground(): Void {
		for (listener in foregroundListeners) {
			listener();
		}
	}

	private static function resume(): Void {
		for (listener in resumeListeners) {
			listener();
		}
	}

	private static function pause(): Void {
		for (listener in pauseListeners) {
			listener();
		}
	}

	private static function background(): Void {
		for (listener in backgroundListeners) {
			listener();
		}
	}

	private static function shutdown(): Void {
		for (listener in shutdownListeners) {
			listener();
		}
	}

	private static function dropFiles(filePath: String): Void {
		for (listener in dropFilesListeners) {
			listener(filePath);
		}
	}

	public static var time(get, null): Float;
	private static function get_time(): Float {
		return Krom.getTime() - startTime;
	}

	public static function windowWidth(): Int {
		return Window.get().width;
	}

	public static function windowHeight(): Int {
		return Window.get().height;
	}

	public static var systemId(get, null): String;
	static function get_systemId(): String {
		return Krom.systemId();
	}

	public static var language(get, never): String;
	private static function get_language(): String {
		return Krom.language();
	}

	public static function stop(): Void {
		Krom.requestShutdown();
	}

	public static function loadUrl(url: String): Void {
		Krom.loadUrl(url);
	}

	private static function renderCallback(): Void {
		System.render(framebuffer);
	}

	private static function dropFilesCallback(filePath: String): Void {
		System.dropFiles(filePath);
	}

	private static function copyCallback(): String {
		if (System.copyListener != null) {
			return System.copyListener();
		}
		return null;
	}

	private static function cutCallback(): String {
		if (System.cutListener != null) {
			return System.cutListener();
		}
		return null;
	}

	private static function pasteCallback(data: String): Void {
		if (System.pasteListener != null) {
			System.pasteListener(data);
		}
	}

	private static function foregroundCallback(): Void {
		System.foreground();
	}

	private static function resumeCallback(): Void {
		System.resume();
	}

	private static function pauseCallback(): Void {
		System.pause();
	}

	private static function backgroundCallback(): Void {
		System.background();
	}

	private static function shutdownCallback(): Void {
		System.shutdown();
	}

	private static function keyboardDownCallback(code: Int): Void {
		keyboard.downListener(code);
	}

	private static function keyboardUpCallback(code: Int): Void {
		keyboard.upListener(code);
	}

	private static function keyboardPressCallback(charCode: Int): Void {
		keyboard.pressListener(String.fromCharCode(charCode));
	}

	private static function mouseDownCallback(button: Int, x: Int, y: Int): Void {
		mouse.downListener(button, x, y);
	}

	private static function mouseUpCallback(button: Int, x: Int, y: Int): Void {
		mouse.upListener(button, x, y);
	}

	private static function mouseMoveCallback(x: Int, y: Int, mx: Int, my: Int): Void {
		mouse.moveListener(x, y, mx, my);
	}

	private static function mouseWheelCallback(delta: Int): Void {
		mouse.wheelListener(delta);
	}

	private static function touchDownCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchDown(index, x, y);
		#end
	}

	private static function touchUpCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchUp(index, x, y);
		#end
	}

	private static function touchMoveCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchMove(index, x, y);
		#end
	}

	private static function penDownCallback(x: Int, y: Int, pressure: Float): Void {
		pen.downListener(x, y, pressure);
	}

	private static function penUpCallback(x: Int, y: Int, pressure: Float): Void {
		pen.upListener(x, y, pressure);
	}

	private static function penMoveCallback(x: Int, y: Int, pressure: Float): Void {
		pen.moveListener(x, y, pressure);
	}

	private static function gamepadAxisCallback(gamepad: Int, axis: Int, value: Float): Void {
		gamepads[gamepad].axisListener(axis, value);
	}

	private static function gamepadButtonCallback(gamepad: Int, button: Int, value: Float): Void {
		gamepads[gamepad].buttonListener(button, value);
	}

	public static function lockMouse(): Void {
		if(!isMouseLocked()){
			Krom.lockMouse();
		}
	}

	public static function unlockMouse(): Void {
		if(isMouseLocked()){
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
}
