package kha;

import kha.Window.FramebufferOptions;
import kha.Window.WindowOptions;
import kha.input.Gamepad;
import kha.input.Keyboard;
import kha.input.Surface;
import kha.input.Mouse;
import kha.input.Pen;
import kha.input.Surface;
import kha.System;
import haxe.ds.Vector;

@:structInit
class SystemOptions {
	@:optional public var title: String = "Kha";
	@:optional public var width: Int = -1;
	@:optional public var height: Int = -1;
	@:optional public var window: WindowOptions = null;
	@:optional public var framebuffer: FramebufferOptions = null;

	public function new(title: String = "Kha", ?width: Int = -1, ?height: Int = -1, window: WindowOptions = null, framebuffer: FramebufferOptions = null) {
		this.title = title;
		this.window = window == null ? {} : window;

		if (width > 0) {
			this.window.width = width;
			this.width = width;
		}
		else {
			this.width = this.window.width;
		}

		if (height > 0) {
			this.window.height = height;
			this.height = height;
		}
		else {
			this.height = this.window.height;
		}

		if (this.window.title == null) {
			this.window.title = title;
		}

		this.framebuffer = framebuffer == null ? {} : framebuffer;
	}
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
	private static var mouseLockListeners: Array<Void->Void> = [];

	public static function start(options: SystemOptions, callback: Window -> Void): Void {
		Krom.init(options.title, options.width, options.height, options.framebuffer.samplesPerPixel, options.framebuffer.verticalSync, cast options.window.mode, options.window.windowFeatures, 0, options.window.x, options.window.y, options.framebuffer.frequency);

		startTime = Krom.getTime();

		haxe.Log.trace = function(v: Dynamic, ?infos: haxe.PosInfos) {
			var message = haxe.Log.formatOutput(v, infos);
			Krom.log(message);
		};

		new Window(0);

		var g4 = new kha.graphics4.Graphics();
		framebuffer = new Framebuffer(0, null, g4);
		framebuffer.init(new kha.graphics2.Graphics(framebuffer), g4);
		Krom.setCallback(renderCallback);
		Krom.setDropFilesCallback(dropFilesCallback);
		Krom.setCutCopyPasteCallback(cutCallback, copyCallback, pasteCallback);
		Krom.setApplicationStateCallback(foregroundCallback, resumeCallback, pauseCallback, backgroundCallback, shutdownCallback);

		keyboard = new Keyboard();
		mouse = new Mouse();
		surface = new Surface();
		pen = new Pen();
		gamepads = new Array<Gamepad>();
		for (i in 0...maxGamepads) {
			gamepads[i] = new Gamepad(i);
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

		callback(Window.get(0));
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

	public static function windowWidth(window: Int = 0): Int {
		return Window.get(window).width;
	}

	public static function windowHeight(window: Int = 0): Int {
		return Window.get(window).height;
	}

	public static var systemId(get, null): String;
	static function get_systemId(): String {
		return Krom.systemId();
	}

	public static var language(get, never): String;
	private static function get_language(): String {
		return Krom.language();
	}

	public static function stop(): Bool {
		Krom.requestShutdown();
		return true;
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
		else {
			return null;
		}
	}

	private static function cutCallback(): String {
		if (System.cutListener != null) {
			return System.cutListener();
		}
		else {
			return null;
		}
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
		keyboard.sendDownEvent(cast code);
	}

	private static function keyboardUpCallback(code: Int): Void {
		keyboard.sendUpEvent(cast code);
	}

	private static function keyboardPressCallback(charCode: Int): Void {
		keyboard.sendPressEvent(String.fromCharCode(charCode));
	}

	private static function mouseDownCallback(button: Int, x: Int, y: Int): Void {
		mouse.sendDownEvent(button, x, y);
	}

	private static function mouseUpCallback(button: Int, x: Int, y: Int): Void {
		mouse.sendUpEvent(button, x, y);
	}

	private static function mouseMoveCallback(x: Int, y: Int, mx: Int, my: Int): Void {
		mouse.sendMoveEvent(x, y, mx, my);
	}

	private static function mouseWheelCallback(delta: Int): Void {
		mouse.sendWheelEvent(delta);
	}

	private static function touchDownCallback(index: Int, x: Int, y: Int): Void {
		surface.sendTouchStartEvent(index, x, y);
	}

	private static function touchUpCallback(index: Int, x: Int, y: Int): Void {
		surface.sendTouchEndEvent(index, x, y);
	}

	private static function touchMoveCallback(index: Int, x: Int, y: Int): Void {
		surface.sendMoveEvent(index, x, y);
	}

	private static function penDownCallback(x: Int, y: Int, pressure: Float): Void {
		pen.sendDownEvent(x, y, pressure);
	}

	private static function penUpCallback(x: Int, y: Int, pressure: Float): Void {
		pen.sendUpEvent(x, y, pressure);
	}

	private static function penMoveCallback(x: Int, y: Int, pressure: Float): Void {
		pen.sendMoveEvent(x, y, pressure);
	}

	private static function gamepadAxisCallback(gamepad: Int, axis: Int, value: Float): Void {
		gamepads[gamepad].sendAxisEvent(axis, value);
	}

	private static function gamepadButtonCallback(gamepad: Int, button: Int, value: Float): Void {
		gamepads[gamepad].sendButtonEvent(button, value);
	}

	public static function getMouse(): Mouse {
		return mouse;
	}

	public static function getSurface(): Surface {
		return surface;
	}

	public static function getPen(): Pen {
		return pen;
	}

	public static function getKeyboard(): Keyboard {
		return keyboard;
	}

	public static function lockMouse(): Void {
		if(!isMouseLocked()){
			Krom.lockMouse();
			for (listener in mouseLockListeners) {
				listener();
			}
		}
	}

	public static function unlockMouse(): Void {
		if(isMouseLocked()){
			Krom.unlockMouse();
			for (listener in mouseLockListeners) {
				listener();
			}
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

	public static function getGamepadId(index: Int): String {
		return "unkown";
	}
}
