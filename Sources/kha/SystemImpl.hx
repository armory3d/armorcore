package kha;

import kha.graphics4.TextureFormat;
import kha.input.Gamepad;
import kha.input.Keyboard;
import kha.input.Surface;
import kha.input.Mouse;
import kha.input.Pen;
import kha.input.Surface;
import kha.System;
import haxe.ds.Vector;

class SystemImpl {
	private static var start: Float;
	private static var framebuffer: Framebuffer;
	private static var keyboard: Keyboard;
	private static var mouse: Mouse;
	private static var surface: Surface;
	private static var pen: Pen;
	private static var maxGamepads: Int = 4;
	private static var gamepads: Array<Gamepad>;
	private static var mouseLockListeners: Array<Void->Void> = [];

	private static function renderCallback(): Void {
		Scheduler.executeFrame();
		System.render([framebuffer]);
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
		mouse.sendDownEvent(0, button, x, y);
	}

	private static function mouseUpCallback(button: Int, x: Int, y: Int): Void {
		mouse.sendUpEvent(0, button, x, y);
	}

	private static function mouseMoveCallback(x: Int, y: Int, mx: Int, my: Int): Void {
		mouse.sendMoveEvent(0, x, y, mx, my);
	}

	private static function mouseWheelCallback(delta: Int): Void {
		mouse.sendWheelEvent(0, delta);
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
		pen.sendDownEvent(0, x, y, pressure);
	}

	private static function penUpCallback(x: Int, y: Int, pressure: Float): Void {
		pen.sendUpEvent(0, x, y, pressure);
	}

	private static function penMoveCallback(x: Int, y: Int, pressure: Float): Void {
		pen.sendMoveEvent(0, x, y, pressure);
	}

	private static function gamepadAxisCallback(gamepad: Int, axis: Int, value: Float): Void {
		gamepads[gamepad].sendAxisEvent(axis, value);
	}

	private static function gamepadButtonCallback(gamepad: Int, button: Int, value: Float): Void {
		gamepads[gamepad].sendButtonEvent(button, value);
	}

	#if arm_audio
	private static function audioCallback(samples: Int) : Void {
		kha.audio2.Audio._callCallback(samples);
		var buffer = @:privateAccess kha.audio2.Audio.buffer;
		Krom.writeAudioBuffer(buffer.data.buffer, samples);
	}
	#end

	public static function init(options: SystemOptions, callback: Window -> Void): Void {
		Krom.init(options.title, options.width, options.height, options.framebuffer.samplesPerPixel, options.framebuffer.verticalSync, cast options.window.mode, options.window.windowFeatures, Krom.KROM_API, options.window.x, options.window.y);

		start = Krom.getTime();

		haxe.Log.trace = function(v: Dynamic, ?infos: haxe.PosInfos) {
			var message = haxe.Log.formatOutput(v, infos);
			Krom.log(message.substr(0, 2048 - 1));
		};

		new Window(0);
		Scheduler.init();
		Shaders.init();

		var g4 = new kha.graphics4.Graphics();
		framebuffer = new Framebuffer(0, null, g4);
		framebuffer.init(new kha.graphics4.Graphics2(framebuffer), g4);
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

		#if arm_audio
		kha.audio2.Audio._init();
		kha.audio1.Audio._init();
		Krom.setAudioCallback(audioCallback);
		#end

		Scheduler.start();

		callback(Window.get(0));
	}

	public static function getTime(): Float {
		return Krom.getTime() - start;
	}

	public static function getVsync(): Bool {
		return true;
	}

	public static function getRefreshRate(): Int {
		return 60;
	}

	public static function getSystemId(): String {
		return Krom.systemId();
	}

	public static function vibrate(ms:Int): Void {
		//TODO: Implement
	}

	public static function getLanguage(): String {
		return "en"; //TODO: Implement
	}

	public static function requestShutdown(): Bool {
		Krom.requestShutdown();
		return true;
	}

	public static function getMouse(num: Int): Mouse {
		return mouse;
	}

	public static function getSurface(num: Int): Surface {
		return surface;
	}

	public static function getPen(num: Int): Pen {
		return pen;
	}

	public static function getKeyboard(num: Int): Keyboard {
		return keyboard;
	}

	public static function showKeyboard() {
		Krom.showKeyboard(true);
	}

	public static function hideKeyboard() {
		Krom.showKeyboard(false);
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

	public static function notifyOfMouseLockChange(func: Void -> Void, error: Void -> Void): Void {
		if (canLockMouse() && func != null) {
			mouseLockListeners.push(func);
		}
	}

	public static function removeFromMouseLockChange(func: Void -> Void, error: Void -> Void): Void {
		if (canLockMouse() && func != null) {
			mouseLockListeners.remove(func);
		}
	}

	public static function hideSystemCursor(): Void {
		Krom.showMouse(false);
	}

	public static function showSystemCursor(): Void {
		Krom.showMouse(true);
	}

	static function unload(): Void {

	}

	public static function canSwitchFullscreen(): Bool {
		return false;
	}

	public static function isFullscreen(): Bool {
		return false;
	}

	public static function requestFullscreen(): Void {

	}

	public static function exitFullscreen(): Void {

	}

	public static function notifyOfFullscreenChange(func: Void -> Void, error: Void -> Void): Void {

	}


	public static function removeFromFullscreenChange(func: Void -> Void, error: Void -> Void): Void {

	}

	public static function changeResolution(width: Int, height: Int): Void {

	}

	public static function setKeepScreenOn(on: Bool): Void {

	}

	public static function loadUrl(url: String): Void {
		Krom.loadUrl(url);
	}

	public static function getGamepadId(index: Int): String {
		return "unkown";
	}

	public static function safeZone(): Float {
		return 1.0;
	}
}
