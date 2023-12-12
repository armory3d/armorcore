package iron;

import iron.System;

class Input {

	public static var occupied = false;
	static var mouse: Mouse = null;
	static var pen: Pen = null;
	static var keyboard: Keyboard = null;
	static var gamepads: Array<Gamepad> = [];
	static var sensor: Sensor = null;
	static var registered = false;

	public static function reset() {
		occupied = false;
		if (mouse != null) mouse.reset();
		if (pen != null) pen.reset();
		if (keyboard != null) keyboard.reset();
		for (gamepad in gamepads) gamepad.reset();
	}

	public static function endFrame() {
		if (mouse != null) mouse.endFrame();
		if (pen != null) pen.endFrame();
		if (keyboard != null) keyboard.endFrame();
		for (gamepad in gamepads) gamepad.endFrame();
	}

	public static function getMouse(): Mouse {
		if (!registered) register();
		if (mouse == null) mouse = new Mouse();
		return mouse;
	}

	public static function getPen(): Pen {
		if (!registered) register();
		if (pen == null) pen = new Pen();
		return pen;
	}

	public static function getSurface(): Surface {
		if (!registered) register();
		// Map to mouse for now..
		return getMouse();
	}

	public static function getKeyboard(): Keyboard {
		if (!registered) register();
		if (keyboard == null) keyboard = new Keyboard();
		return keyboard;
	}

	public static function getGamepad(i = 0): Gamepad {
		if (i >= 4) return null;
		if (!registered) register();
		while (gamepads.length <= i) gamepads.push(new Gamepad(gamepads.length));
		return gamepads[i];
	}

	public static function getSensor(): Sensor {
		if (!registered) register();
		if (sensor == null) sensor = new Sensor();
		return sensor;
	}

	static inline function register() {
		registered = true;
		App.notifyOnEndFrame(endFrame);
		App.notifyOnReset(reset);
		// Reset mouse delta on foreground
		System.notifyOnApplicationState(function() { getMouse().reset(); }, null, null, null, null);
	}
}

typedef Surface = Mouse;

class Mouse {

	public static var buttons = ["left", "right", "middle", "side1", "side2"];
	var buttonsDown = [false, false, false, false, false];
	var buttonsStarted = [false, false, false, false, false];
	var buttonsReleased = [false, false, false, false, false];

	public var x = 0.0;
	public var y = 0.0;
	public var viewX(get, null) = 0.0;
	public var viewY(get, null) = 0.0;
	public var moved = false;
	public var movementX = 0.0;
	public var movementY = 0.0;
	public var wheelDelta = 0.0;
	public var locked = false;
	public var hidden = false;
	public var lastX = -1.0;
	public var lastY = -1.0;

	public function new() {}

	public function endFrame() {
		buttonsStarted[0] = buttonsStarted[1] = buttonsStarted[2] = buttonsStarted[3] = buttonsStarted[4] = false;
		buttonsReleased[0] = buttonsReleased[1] = buttonsReleased[2] = buttonsReleased[3] = buttonsReleased[4] = false;
		moved = false;
		movementX = 0;
		movementY = 0;
		wheelDelta = 0;
	}

	public function reset() {
		buttonsDown[0] = buttonsDown[1] = buttonsDown[2] = buttonsDown[3] = buttonsDown[4] = false;
		endFrame();
	}

	function buttonIndex(button: String): Int {
		for (i in 0...buttons.length) if (buttons[i] == button) return i;
		return 0;
	}

	public function down(button = "left"): Bool {
		return buttonsDown[buttonIndex(button)];
	}

	public function downAny(): Bool {
		return buttonsDown[0] || buttonsDown[1] || buttonsDown[2] || buttonsDown[3] || buttonsDown[4];
	}

	public function started(button = "left"): Bool {
		return buttonsStarted[buttonIndex(button)];
	}

	public function startedAny(): Bool {
		return buttonsStarted[0] || buttonsStarted[1] || buttonsStarted[2] || buttonsStarted[3] || buttonsStarted[4];
	}

	public function released(button = "left"): Bool {
		return buttonsReleased[buttonIndex(button)];
	}

	public function lock() {
		if (System.canLockMouse()) {
			System.lockMouse();
			locked = true;
			hidden = true;
		}
	}
	public function unlock() {
		if (System.canLockMouse()) {
			System.unlockMouse();
			locked = false;
			hidden = false;
		}
	}

	public function hide() {
		System.hideSystemCursor();
		hidden = true;
	}

	public function show() {
		System.showSystemCursor();
		hidden = false;
	}

	public function downListener(index: Int, x: Int, y: Int) {
		if (Input.getPen().inUse) return;

		buttonsDown[index] = true;
		buttonsStarted[index] = true;
		this.x = x;
		this.y = y;
		#if (krom_android || krom_ios) // For movement delta using touch
		if (index == 0) {
			lastX = x;
			lastY = y;
		}
		#end
	}

	public function upListener(index: Int, x: Int, y: Int) {
		if (Input.getPen().inUse) return;

		buttonsDown[index] = false;
		buttonsReleased[index] = true;
		this.x = x;
		this.y = y;
	}

	public function moveListener(x: Int, y: Int, movementX: Int, movementY: Int) {
		if (lastX == -1.0 && lastY == -1.0) { // First frame init
			lastX = x;
			lastY = y;
		}
		if (locked) {
			// Can be called multiple times per frame
			this.movementX += movementX;
			this.movementY += movementY;
		}
		else {
			this.movementX += x - lastX;
			this.movementY += y - lastY;
		}
		lastX = x;
		lastY = y;
		this.x = x;
		this.y = y;
		moved = true;
	}

	public function wheelListener(delta: Int) {
		wheelDelta = delta;
	}

	#if (krom_android || krom_ios)
	public function onTouchDown(index: Int, x: Int, y: Int) {
		if (index == 1) { // Two fingers down - right mouse button
			buttonsDown[0] = false;
			downListener(1, Std.int(this.x), Std.int(this.y));
			pinchStarted = true;
			pinchTotal = 0.0;
			pinchDistance = 0.0;
		}
		else if (index == 2) { // Three fingers down - middle mouse button
			buttonsDown[1] = false;
			downListener(2, Std.int(this.x), Std.int(this.y));
		}
	}

	public function onTouchUp(index: Int, x: Int, y: Int) {
		if (index == 1) upListener(1, Std.int(this.x), Std.int(this.y));
		else if (index == 2) upListener(2, Std.int(this.x), Std.int(this.y));
	}

	var pinchDistance = 0.0;
	var pinchTotal = 0.0;
	var pinchStarted = false;

	public function onTouchMove(index: Int, x: Int, y: Int) {
		// Pinch to zoom - mouse wheel
		if (index == 1) {
			var lastDistance = pinchDistance;
			var dx = this.x - x;
			var dy = this.y - y;
			pinchDistance = Math.sqrt(dx * dx + dy * dy);
			pinchTotal += lastDistance != 0 ? lastDistance - pinchDistance : 0;
			if (!pinchStarted) {
				wheelDelta = pinchTotal / 10;
				if (wheelDelta != 0) {
					pinchTotal = 0.0;
				}
			}
			pinchStarted = false;
		}
	}
	#end

	inline function get_viewX(): Float {
		return x - App.x();
	}

	inline function get_viewY(): Float {
		return y - App.y();
	}
}

class Pen {

	static var buttons = ["tip"];
	var buttonsDown = [false];
	var buttonsStarted = [false];
	var buttonsReleased = [false];

	public var x = 0.0;
	public var y = 0.0;
	public var viewX(get, null) = 0.0;
	public var viewY(get, null) = 0.0;
	public var moved = false;
	public var movementX = 0.0;
	public var movementY = 0.0;
	public var pressure = 0.0;
	public var connected = false;
	public var inUse = false;
	var lastX = -1.0;
	var lastY = -1.0;

	public function new() {}

	public function endFrame() {
		buttonsStarted[0] = false;
		buttonsReleased[0] = false;
		moved = false;
		movementX = 0;
		movementY = 0;
		inUse = false;
	}

	public function reset() {
		buttonsDown[0] = false;
		endFrame();
	}

	function buttonIndex(button: String): Int {
		return 0;
	}

	public function down(button = "tip"): Bool {
		return buttonsDown[buttonIndex(button)];
	}

	public function started(button = "tip"): Bool {
		return buttonsStarted[buttonIndex(button)];
	}

	public function released(button = "tip"): Bool {
		return buttonsReleased[buttonIndex(button)];
	}

	public function downListener(x: Int, y: Int, pressure: Float) {
		buttonsDown[0] = true;
		buttonsStarted[0] = true;
		this.x = x;
		this.y = y;
		this.pressure = pressure;

		#if (!krom_android && !krom_ios)
		Input.getMouse().downListener(0, x, y);
		#end
	}

	public function upListener(x: Int, y: Int, pressure: Float) {
		#if (!krom_android && !krom_ios)
		if (buttonsStarted[0]) { buttonsStarted[0] = false; inUse = true; return; }
		#end

		buttonsDown[0] = false;
		buttonsReleased[0] = true;
		this.x = x;
		this.y = y;
		this.pressure = pressure;

		#if (!krom_android && !krom_ios)
		Input.getMouse().upListener(0, x, y);
		inUse = true; // On pen release, additional mouse down & up events are fired at once - filter those out
		#end
	}

	public function moveListener(x: Int, y: Int, pressure: Float) {
		#if krom_ios
		// Listen to pen hover if no other input is active
		if (!buttonsDown[0] && pressure == 0.0) {
			if (!Input.getMouse().downAny()) {
				Input.getMouse().moveListener(x, y, 0, 0);
			}
			return;
		}
		#end

		if (lastX == -1.0 && lastY == -1.0) { // First frame init
			lastX = x;
			lastY = y;
		}
		this.movementX = x - lastX;
		this.movementY = y - lastY;
		lastX = x;
		lastY = y;
		this.x = x;
		this.y = y;
		moved = true;
		this.pressure = pressure;
		connected = true;
	}

	inline function get_viewX(): Float {
		return x - App.x();
	}

	inline function get_viewY(): Float {
		return y - App.y();
	}
}

class Keyboard {

	public static var keys = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "space", "backspace", "tab", "enter", "shift", "control", "alt", "win", "escape", "delete", "up", "down", "left", "right", "back", ",", ".", ":", ";", "<", "=", ">", "?", "!", '"', "#", "$", "%", "&", "_", "(", ")", "*", "|", "{", "}", "[", "]", "~", "`", "/", "\\", "@", "+", "-", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12"];
	var keysDown = new Map<String, Bool>();
	var keysStarted = new Map<String, Bool>();
	var keysReleased = new Map<String, Bool>();
	var keysFrame: Array<String> = [];
	var repeatKey = false;
	var repeatTime = 0.0;

	public function new() {
		reset();
	}

	public function endFrame() {
		if (keysFrame.length > 0) {
			for (s in keysFrame) {
				keysStarted.set(s, false);
				keysReleased.set(s, false);
			}
			keysFrame.splice(0, keysFrame.length);
		}

		if (Time.time() - repeatTime > 0.05) {
			repeatTime = Time.time();
			repeatKey = true;
		}
		else repeatKey = false;
	}

	public function reset() {
		// Use Map for now..
		for (s in keys) {
			keysDown.set(s, false);
			keysStarted.set(s, false);
			keysReleased.set(s, false);
		}
		endFrame();
	}

	public function down(key: String): Bool {
		return keysDown.get(key);
	}

	public function started(key: String): Bool {
		return keysStarted.get(key);
	}

	public function startedAny(): Bool {
		return keysFrame.length > 0;
	}

	public function released(key: String): Bool {
		return keysReleased.get(key);
	}

	public function repeat(key: String): Bool {
		return keysStarted.get(key) || (repeatKey && keysDown.get(key));
	}

	public static function keyCode(key: KeyCode): String {
		return switch(key) {
			case KeyCode.Space: "space";
			case KeyCode.Backspace: "backspace";
			case KeyCode.Tab: "tab";
			case KeyCode.Return: "enter";
			case KeyCode.Shift: "shift";
			case KeyCode.Control: "control";
			#if krom_darwin
			case KeyCode.Meta: "control";
			#end
			case KeyCode.Alt: "alt";
			case KeyCode.Win: "win";
			case KeyCode.Escape: "escape";
			case KeyCode.Delete: "delete";
			case KeyCode.Up: "up";
			case KeyCode.Down: "down";
			case KeyCode.Left: "left";
			case KeyCode.Right: "right";
			case KeyCode.Back: "back";
			case KeyCode.Comma: ",";
			case KeyCode.Period: ".";
			case KeyCode.Colon: ":";
			case KeyCode.Semicolon: ";";
			case KeyCode.LessThan: "<";
			case KeyCode.Equals: "=";
			case KeyCode.GreaterThan: ">";
			case KeyCode.QuestionMark: "?";
			case KeyCode.Exclamation: "!";
			case KeyCode.DoubleQuote: '"';
			case KeyCode.Hash: "#";
			case KeyCode.Dollar: "$";
			case KeyCode.Percent: "%";
			case KeyCode.Ampersand: "&";
			case KeyCode.Underscore: "_";
			case KeyCode.OpenParen: "(";
			case KeyCode.CloseParen: ")";
			case KeyCode.Asterisk: "*";
			case KeyCode.Pipe: "|";
			case KeyCode.OpenCurlyBracket: "{";
			case KeyCode.CloseCurlyBracket: "}";
			case KeyCode.OpenBracket: "[";
			case KeyCode.CloseBracket: "]";
			case KeyCode.Tilde: "~";
			case KeyCode.BackQuote: "`";
			case KeyCode.Slash: "/";
			case KeyCode.BackSlash: "\\";
			case KeyCode.At: "@";
			case KeyCode.Add: "+";
			case KeyCode.Plus: "+";
			case KeyCode.Subtract: "-";
			case KeyCode.HyphenMinus: "-";
			case KeyCode.Multiply: "*";
			case KeyCode.Divide: "/";
			case KeyCode.Decimal: ".";
			case KeyCode.Zero: "0";
			case KeyCode.Numpad0: "0";
			case KeyCode.One: "1";
			case KeyCode.Numpad1: "1";
			case KeyCode.Two: "2";
			case KeyCode.Numpad2: "2";
			case KeyCode.Three: "3";
			case KeyCode.Numpad3: "3";
			case KeyCode.Four: "4";
			case KeyCode.Numpad4: "4";
			case KeyCode.Five: "5";
			case KeyCode.Numpad5: "5";
			case KeyCode.Six: "6";
			case KeyCode.Numpad6: "6";
			case KeyCode.Seven: "7";
			case KeyCode.Numpad7: "7";
			case KeyCode.Eight: "8";
			case KeyCode.Numpad8: "8";
			case KeyCode.Nine: "9";
			case KeyCode.Numpad9: "9";
			case KeyCode.F1: "f1";
			case KeyCode.F2: "f2";
			case KeyCode.F3: "f3";
			case KeyCode.F4: "f4";
			case KeyCode.F5: "f5";
			case KeyCode.F6: "f6";
			case KeyCode.F7: "f7";
			case KeyCode.F8: "f8";
			case KeyCode.F9: "f9";
			case KeyCode.F10: "f10";
			case KeyCode.F11: "f11";
			case KeyCode.F12: "f12";
			default: String.fromCharCode(cast key).toLowerCase();
		}
	}

	public function downListener(code: KeyCode) {
		var s = keyCode(code);
		keysFrame.push(s);
		keysStarted.set(s, true);
		keysDown.set(s, true);
		repeatTime = Time.time() + 0.4;

		#if krom_android_rmb // Detect right mouse button on Android..
		if (code == KeyCode.Back) {
			var m = Input.getMouse();
			if (!m.buttonsDown[1]) m.downListener(1, Std.int(m.x), Std.int(m.y));
		}
		#end
	}

	public function upListener(code: KeyCode) {
		var s = keyCode(code);
		keysFrame.push(s);
		keysReleased.set(s, true);
		keysDown.set(s, false);

		#if krom_android_rmb
		if (code == KeyCode.Back) {
			var m = Input.getMouse();
			m.upListener(1, Std.int(m.x), Std.int(m.y));
		}
		#end
	}

	public function pressListener(char: String) {}
}

class GamepadStick {
	public var x = 0.0;
	public var y = 0.0;
	public var lastX = 0.0;
	public var lastY = 0.0;
	public var moved = false;
	public var movementX = 0.0;
	public var movementY = 0.0;
	public function new() {}
}

class Gamepad {

	public static var buttonsPS = ["cross", "circle", "square", "triangle", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
	public static var buttonsXBOX = ["a", "b", "x", "y", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
	public static var buttons = buttonsPS;

	var buttonsDown: Array<Float> = []; // Intensity 0 - 1
	var buttonsStarted: Array<Bool> = [];
	var buttonsReleased: Array<Bool> = [];

	var buttonsFrame: Array<Int> = [];

	public var leftStick = new GamepadStick();
	public var rightStick = new GamepadStick();

	var num = 0;

	public function new(i: Int) {
		for (s in buttons) {
			buttonsDown.push(0.0);
			buttonsStarted.push(false);
			buttonsReleased.push(false);
		}
		num = i;
		reset();
	}

	public function endFrame() {
		if (buttonsFrame.length > 0) {
			for (i in buttonsFrame) {
				buttonsStarted[i] = false;
				buttonsReleased[i] = false;
			}
			buttonsFrame.splice(0, buttonsFrame.length);
		}
		leftStick.moved = false;
		leftStick.movementX = 0;
		leftStick.movementY = 0;
		rightStick.moved = false;
		rightStick.movementX = 0;
		rightStick.movementY = 0;
	}

	public function reset() {
		for (i in 0...buttonsDown.length) {
			buttonsDown[i] = 0.0;
			buttonsStarted[i] = false;
			buttonsReleased[i] = false;
		}
		endFrame();
	}

	public static function keyCode(button: Int): String {
		return buttons[button];
	}

	function buttonIndex(button: String): Int {
		for (i in 0...buttons.length) if (buttons[i] == button) return i;
		return 0;
	}

	public function down(button: String): Float {
		return buttonsDown[buttonIndex(button)];
	}

	public function started(button: String): Bool {
		return buttonsStarted[buttonIndex(button)];
	}

	public function released(button: String): Bool {
		return buttonsReleased[buttonIndex(button)];
	}

	public function axisListener(axis: Int, value: Float) {
		var stick = axis <= 1 ? leftStick : rightStick;

		if (axis == 0 || axis == 2) { // X
			stick.lastX = stick.x;
			stick.x = value;
			stick.movementX = stick.x - stick.lastX;
		}
		else if (axis == 1 || axis == 3) { // Y
			stick.lastY = stick.y;
			stick.y = value;
			stick.movementY = stick.y - stick.lastY;
		}
		stick.moved = true;
	}

	public function buttonListener(button: Int, value: Float) {
		buttonsFrame.push(button);

		buttonsDown[button] = value;
		if (value > 0) buttonsStarted[button] = true; // Will trigger L2/R2 multiple times..
		else buttonsReleased[button] = true;
	}
}

class Sensor {
	public var x = 0.0;
	public var y = 0.0;
	public var z = 0.0;

	public function new() {
		// System.getSensor(Accelerometer).notify(listener);
	}

	function listener(x: Float, y: Float, z: Float) {
		this.x = x;
		this.y = y;
		this.z = z;
	}
}

@:enum abstract SensorType(Int) from Int to Int {
	var Accelerometer = 0;
	var Gyroscope = 1;
}

@:enum abstract KeyCode(Int) from Int to Int {
	var Unknown = 0;
	var Back = 1; // Android
	var Cancel = 3;
	var Help = 6;
	var Backspace = 8;
	var Tab = 9;
	var Clear = 12;
	var Return = 13;
	var Shift = 16;
	var Control = 17;
	var Alt = 18;
	var Pause = 19;
	var CapsLock = 20;
	var Kana = 21;
	var Hangul = 21;
	var Eisu = 22;
	var Junja = 23;
	var Final = 24;
	var Hanja = 25;
	var Kanji = 25;
	var Escape = 27;
	var Convert = 28;
	var NonConvert = 29;
	var Accept = 30;
	var ModeChange = 31;
	var Space = 32;
	var PageUp = 33;
	var PageDown = 34;
	var End = 35;
	var Home = 36;
	var Left = 37;
	var Up = 38;
	var Right = 39;
	var Down = 40;
	var Select = 41;
	var Print = 42;
	var Execute = 43;
	var PrintScreen = 44;
	var Insert = 45;
	var Delete = 46;
	var Zero = 48;
	var One = 49;
	var Two = 50;
	var Three = 51;
	var Four = 52;
	var Five = 53;
	var Six = 54;
	var Seven = 55;
	var Eight = 56;
	var Nine = 57;
	var Colon = 58;
	var Semicolon = 59;
	var LessThan = 60;
	var Equals = 61;
	var GreaterThan = 62;
	var QuestionMark = 63;
	var At = 64;
	var A = 65;
	var B = 66;
	var C = 67;
	var D = 68;
	var E = 69;
	var F = 70;
	var G = 71;
	var H = 72;
	var I = 73;
	var J = 74;
	var K = 75;
	var L = 76;
	var M = 77;
	var N = 78;
	var O = 79;
	var P = 80;
	var Q = 81;
	var R = 82;
	var S = 83;
	var T = 84;
	var U = 85;
	var V = 86;
	var W = 87;
	var X = 88;
	var Y = 89;
	var Z = 90;
	var Win = 91;
	var ContextMenu = 93;
	var Sleep = 95;
	var Numpad0 = 96;
	var Numpad1 = 97;
	var Numpad2 = 98;
	var Numpad3 = 99;
	var Numpad4 = 100;
	var Numpad5 = 101;
	var Numpad6 = 102;
	var Numpad7 = 103;
	var Numpad8 = 104;
	var Numpad9 = 105;
	var Multiply = 106;
	var Add = 107;
	var Separator = 108;
	var Subtract = 109;
	var Decimal = 110;
	var Divide = 111;
	var F1 = 112;
	var F2 = 113;
	var F3 = 114;
	var F4 = 115;
	var F5 = 116;
	var F6 = 117;
	var F7 = 118;
	var F8 = 119;
	var F9 = 120;
	var F10 = 121;
	var F11 = 122;
	var F12 = 123;
	var F13 = 124;
	var F14 = 125;
	var F15 = 126;
	var F16 = 127;
	var F17 = 128;
	var F18 = 129;
	var F19 = 130;
	var F20 = 131;
	var F21 = 132;
	var F22 = 133;
	var F23 = 134;
	var F24 = 135;
	var NumLock = 144;
	var ScrollLock = 145;
	var WinOemFjJisho = 146;
	var WinOemFjMasshou = 147;
	var WinOemFjTouroku = 148;
	var WinOemFjLoya = 149;
	var WinOemFjRoya = 150;
	var Circumflex = 160;
	var Exclamation = 161;
	var DoubleQuote = 162;
	var Hash = 163;
	var Dollar = 164;
	var Percent = 165;
	var Ampersand = 166;
	var Underscore = 167;
	var OpenParen = 168;
	var CloseParen = 169;
	var Asterisk = 170;
	var Plus = 171;
	var Pipe = 172;
	var HyphenMinus = 173;
	var OpenCurlyBracket = 174;
	var CloseCurlyBracket = 175;
	var Tilde = 176;
	var VolumeMute = 181;
	var VolumeDown = 182;
	var VolumeUp = 183;
	var Comma = 188;
	var Period = 190;
	var Slash = 191;
	var BackQuote = 192;
	var OpenBracket = 219;
	var BackSlash = 220;
	var CloseBracket = 221;
	var Quote = 222;
	var Meta = 224;
	var AltGr = 225;
	var WinIcoHelp = 227;
	var WinIco00 = 228;
	var WinIcoClear = 230;
	var WinOemReset = 233;
	var WinOemJump = 234;
	var WinOemPA1 = 235;
	var WinOemPA2 = 236;
	var WinOemPA3 = 237;
	var WinOemWSCTRL = 238;
	var WinOemCUSEL = 239;
	var WinOemATTN = 240;
	var WinOemFinish = 241;
	var WinOemCopy = 242;
	var WinOemAuto = 243;
	var WinOemENLW = 244;
	var WinOemBackTab = 245;
	var ATTN = 246;
	var CRSEL = 247;
	var EXSEL = 248;
	var EREOF = 249;
	var Play = 250;
	var Zoom = 251;
	var PA1 = 253;
	var WinOemClear = 254;
}
