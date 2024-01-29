
class Input {

	static occupied = false;
	static registered = false;

	static reset = () => {
		Input.occupied = false;
		Mouse.reset();
		Pen.reset();
		Keyboard.reset();
		Gamepad.reset();
	}

	static endFrame = () => {
		Mouse.endFrame();
		Pen.endFrame();
		Keyboard.endFrame();
		Gamepad.endFrame();
	}

	static register = () => {
		if (Input.registered) return;
		Input.registered = true;
		App.notifyOnEndFrame(Input.endFrame);
		App.notifyOnReset(Input.reset);
		// Reset mouse delta on foreground
		System.notifyOnApplicationState(() => { Mouse.reset(); }, null, null, null, null);
		Keyboard.reset();
		Gamepad.reset();
	}
}

class Mouse {

	static buttons = ["left", "right", "middle", "side1", "side2"];
	static buttonsDown = [false, false, false, false, false];
	static buttonsStarted = [false, false, false, false, false];
	static buttonsReleased = [false, false, false, false, false];

	static x = 0.0;
	static y = 0.0;
	static moved = false;
	static movementX = 0.0;
	static movementY = 0.0;
	static wheelDelta = 0.0;
	static locked = false;
	static hidden = false;
	static lastX = -1.0;
	static lastY = -1.0;

	static endFrame = () => {
		Mouse.buttonsStarted[0] = Mouse.buttonsStarted[1] = Mouse.buttonsStarted[2] = Mouse.buttonsStarted[3] = Mouse.buttonsStarted[4] = false;
		Mouse.buttonsReleased[0] = Mouse.buttonsReleased[1] = Mouse.buttonsReleased[2] = Mouse.buttonsReleased[3] = Mouse.buttonsReleased[4] = false;
		Mouse.moved = false;
		Mouse.movementX = 0;
		Mouse.movementY = 0;
		Mouse.wheelDelta = 0;
	}

	static reset = () => {
		Mouse.buttonsDown[0] = Mouse.buttonsDown[1] = Mouse.buttonsDown[2] = Mouse.buttonsDown[3] = Mouse.buttonsDown[4] = false;
		Mouse.endFrame();
	}

	static buttonIndex = (button: string): i32 => {
		for (let i = 0; i < Mouse.buttons.length; ++i) if (Mouse.buttons[i] == button) return i;
		return 0;
	}

	static down = (button = "left"): bool => {
		return Mouse.buttonsDown[Mouse.buttonIndex(button)];
	}

	static downAny = (): bool => {
		return Mouse.buttonsDown[0] || Mouse.buttonsDown[1] || Mouse.buttonsDown[2] || Mouse.buttonsDown[3] || Mouse.buttonsDown[4];
	}

	static started = (button = "left"): bool => {
		return Mouse.buttonsStarted[Mouse.buttonIndex(button)];
	}

	static startedAny = (): bool => {
		return Mouse.buttonsStarted[0] || Mouse.buttonsStarted[1] || Mouse.buttonsStarted[2] || Mouse.buttonsStarted[3] || Mouse.buttonsStarted[4];
	}

	static released = (button = "left"): bool => {
		return Mouse.buttonsReleased[Mouse.buttonIndex(button)];
	}

	static lock = () => {
		if (System.canLockMouse()) {
			System.lockMouse();
			Mouse.locked = true;
			Mouse.hidden = true;
		}
	}

	static unlock = () => {
		if (System.canLockMouse()) {
			System.unlockMouse();
			Mouse.locked = false;
			Mouse.hidden = false;
		}
	}

	static hide = () => {
		System.hideSystemCursor();
		Mouse.hidden = true;
	}

	static show = () => {
		System.showSystemCursor();
		Mouse.hidden = false;
	}

	static downListener = (index: i32, x: i32, y: i32) => {
		if (Pen.inUse) return;

		Mouse.buttonsDown[index] = true;
		Mouse.buttonsStarted[index] = true;
		Mouse.x = x;
		Mouse.y = y;
		///if (krom_android || krom_ios) // For movement delta using touch
		if (index == 0) {
			Mouse.lastX = x;
			Mouse.lastY = y;
		}
		///end
	}

	static upListener = (index: i32, x: i32, y: i32) => {
		if (Pen.inUse) return;

		Mouse.buttonsDown[index] = false;
		Mouse.buttonsReleased[index] = true;
		Mouse.x = x;
		Mouse.y = y;
	}

	static moveListener = (x: i32, y: i32, movementX: i32, movementY: i32) => {
		if (Mouse.lastX == -1.0 && Mouse.lastY == -1.0) { // First frame init
			Mouse.lastX = x;
			Mouse.lastY = y;
		}
		if (Mouse.locked) {
			// Can be called multiple times per frame
			Mouse.movementX += movementX;
			Mouse.movementY += movementY;
		}
		else {
			Mouse.movementX += x - Mouse.lastX;
			Mouse.movementY += y - Mouse.lastY;
		}
		Mouse.lastX = x;
		Mouse.lastY = y;
		Mouse.x = x;
		Mouse.y = y;
		Mouse.moved = true;
	}

	static wheelListener = (delta: i32) => {
		Mouse.wheelDelta = delta;
	}

	///if (krom_android || krom_ios)
	static onTouchDown = (index: i32, x: i32, y: i32) => {
		if (index == 1) { // Two fingers down - right mouse button
			Mouse.buttonsDown[0] = false;
			Mouse.downListener(1, Math.floor(Mouse.x), Math.floor(Mouse.y));
			Mouse.pinchStarted = true;
			Mouse.pinchTotal = 0.0;
			Mouse.pinchDistance = 0.0;
		}
		else if (index == 2) { // Three fingers down - middle mouse button
			Mouse.buttonsDown[1] = false;
			Mouse.downListener(2, Math.floor(Mouse.x), Math.floor(Mouse.y));
		}
	}

	static onTouchUp = (index: i32, x: i32, y: i32) => {
		if (index == 1) Mouse.upListener(1, Math.floor(Mouse.x), Math.floor(Mouse.y));
		else if (index == 2) Mouse.upListener(2, Math.floor(Mouse.x), Math.floor(Mouse.y));
	}

	static pinchDistance = 0.0;
	static pinchTotal = 0.0;
	static pinchStarted = false;

	static onTouchMove = (index: i32, x: i32, y: i32) => {
		// Pinch to zoom - mouse wheel
		if (index == 1) {
			let lastDistance = Mouse.pinchDistance;
			let dx = Mouse.x - x;
			let dy = Mouse.y - y;
			Mouse.pinchDistance = Math.sqrt(dx * dx + dy * dy);
			Mouse.pinchTotal += lastDistance != 0 ? lastDistance - Mouse.pinchDistance : 0;
			if (!Mouse.pinchStarted) {
				Mouse.wheelDelta = Mouse.pinchTotal / 10;
				if (Mouse.wheelDelta != 0) {
					Mouse.pinchTotal = 0.0;
				}
			}
			Mouse.pinchStarted = false;
		}
	}
	///end

	static get viewX(): f32 {
		return Mouse.x - App.x();
	}

	static get viewY(): f32 {
		return Mouse.y - App.y();
	}
}

let Surface = Mouse;

class Pen {

	static buttons = ["tip"];
	static buttonsDown = [false];
	static buttonsStarted = [false];
	static buttonsReleased = [false];

	static x = 0.0;
	static y = 0.0;
	static moved = false;
	static movementX = 0.0;
	static movementY = 0.0;
	static pressure = 0.0;
	static connected = false;
	static inUse = false;
	static lastX = -1.0;
	static lastY = -1.0;

	static endFrame = () => {
		Pen.buttonsStarted[0] = false;
		Pen.buttonsReleased[0] = false;
		Pen.moved = false;
		Pen.movementX = 0;
		Pen.movementY = 0;
		Pen.inUse = false;
	}

	static reset = () => {
		Pen.buttonsDown[0] = false;
		Pen.endFrame();
	}

	static buttonIndex = (button: string): i32 => {
		return 0;
	}

	static down = (button = "tip"): bool => {
		return Pen.buttonsDown[Pen.buttonIndex(button)];
	}

	static started = (button = "tip"): bool => {
		return Pen.buttonsStarted[Pen.buttonIndex(button)];
	}

	static released = (button = "tip"): bool => {
		return Pen.buttonsReleased[Pen.buttonIndex(button)];
	}

	static downListener = (x: i32, y: i32, pressure: f32) => {
		Pen.buttonsDown[0] = true;
		Pen.buttonsStarted[0] = true;
		Pen.x = x;
		Pen.y = y;
		Pen.pressure = pressure;

		///if (!krom_android && !krom_ios)
		Mouse.downListener(0, x, y);
		///end
	}

	static upListener = (x: i32, y: i32, pressure: f32) => {
		///if (!krom_android && !krom_ios)
		if (Pen.buttonsStarted[0]) { Pen.buttonsStarted[0] = false; Pen.inUse = true; return; }
		///end

		Pen.buttonsDown[0] = false;
		Pen.buttonsReleased[0] = true;
		Pen.x = x;
		Pen.y = y;
		Pen.pressure = pressure;

		///if (!krom_android && !krom_ios)
		Mouse.upListener(0, x, y);
		Pen.inUse = true; // On pen release, additional mouse down & up events are fired at once - filter those out
		///end
	}

	static moveListener = (x: i32, y: i32, pressure: f32) => {
		///if krom_ios
		// Listen to pen hover if no other input is active
		if (!Pen.buttonsDown[0] && pressure == 0.0) {
			if (!Mouse.downAny()) {
				Mouse.moveListener(x, y, 0, 0);
			}
			return;
		}
		///end

		if (Pen.lastX == -1.0 && Pen.lastY == -1.0) { // First frame init
			Pen.lastX = x;
			Pen.lastY = y;
		}
		Pen.movementX = x - Pen.lastX;
		Pen.movementY = y - Pen.lastY;
		Pen.lastX = x;
		Pen.lastY = y;
		Pen.x = x;
		Pen.y = y;
		Pen.moved = true;
		Pen.pressure = pressure;
		Pen.connected = true;
	}

	static get viewX(): f32 {
		return Pen.x - App.x();
	}

	static get viewY(): f32 {
		return Pen.y - App.y();
	}
}

class Keyboard {

	static keys = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "space", "backspace", "tab", "enter", "shift", "control", "alt", "win", "escape", "delete", "up", "down", "left", "right", "back", ",", ".", ":", ";", "<", "=", ">", "?", "!", '"', "#", "$", "%", "&", "_", "(", ")", "*", "|", "{", "}", "[", "]", "~", "`", "/", "\\", "@", "+", "-", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12"];
	static keysDown = new Map<string, bool>();
	static keysStarted = new Map<string, bool>();
	static keysReleased = new Map<string, bool>();
	static keysFrame: string[] = [];
	static repeatKey = false;
	static repeatTime = 0.0;

	static endFrame = () => {
		if (Keyboard.keysFrame.length > 0) {
			for (let s of Keyboard.keysFrame) {
				Keyboard.keysStarted.set(s, false);
				Keyboard.keysReleased.set(s, false);
			}
			Keyboard.keysFrame.splice(0, Keyboard.keysFrame.length);
		}

		if (Time.time() - Keyboard.repeatTime > 0.05) {
			Keyboard.repeatTime = Time.time();
			Keyboard.repeatKey = true;
		}
		else Keyboard.repeatKey = false;
	}

	static reset = () => {
		// Use Map for now..
		for (let s of Keyboard.keys) {
			Keyboard.keysDown.set(s, false);
			Keyboard.keysStarted.set(s, false);
			Keyboard.keysReleased.set(s, false);
		}
		Keyboard.endFrame();
	}

	static down = (key: string): bool => {
		return Keyboard.keysDown.get(key);
	}

	static started = (key: string): bool => {
		return Keyboard.keysStarted.get(key);
	}

	static startedAny = (): bool => {
		return Keyboard.keysFrame.length > 0;
	}

	static released = (key: string): bool => {
		return Keyboard.keysReleased.get(key);
	}

	static repeat = (key: string): bool => {
		return Keyboard.keysStarted.get(key) || (Keyboard.repeatKey && Keyboard.keysDown.get(key));
	}

	static keyCode = (key: KeyCode): string => {
		switch(key) {
			case KeyCode.Space: return "space";
			case KeyCode.Backspace: return "backspace";
			case KeyCode.Tab: return "tab";
			case KeyCode.Return: return "enter";
			case KeyCode.Shift: return "shift";
			case KeyCode.Control: return "control";
			///if krom_darwin
			case KeyCode.Meta: return "control";
			///end
			case KeyCode.Alt: return "alt";
			case KeyCode.Win: return "win";
			case KeyCode.Escape: return "escape";
			case KeyCode.Delete: return "delete";
			case KeyCode.Up: return "up";
			case KeyCode.Down: return "down";
			case KeyCode.Left: return "left";
			case KeyCode.Right: return "right";
			case KeyCode.Back: return "back";
			case KeyCode.Comma: return ",";
			case KeyCode.Period: return ".";
			case KeyCode.Colon: return ":";
			case KeyCode.Semicolon: return ";";
			case KeyCode.LessThan: return "<";
			case KeyCode.Equals: return "=";
			case KeyCode.GreaterThan: return ">";
			case KeyCode.QuestionMark: return "?";
			case KeyCode.Exclamation: return "!";
			case KeyCode.DoubleQuote: return '"';
			case KeyCode.Hash: return "#";
			case KeyCode.Dollar: return "$";
			case KeyCode.Percent: return "%";
			case KeyCode.Ampersand: return "&";
			case KeyCode.Underscore: return "_";
			case KeyCode.OpenParen: return "(";
			case KeyCode.CloseParen: return ")";
			case KeyCode.Asterisk: return "*";
			case KeyCode.Pipe: return "|";
			case KeyCode.OpenCurlyBracket: return "{";
			case KeyCode.CloseCurlyBracket: return "}";
			case KeyCode.OpenBracket: return "[";
			case KeyCode.CloseBracket: return "]";
			case KeyCode.Tilde: return "~";
			case KeyCode.BackQuote: return "`";
			case KeyCode.Slash: return "/";
			case KeyCode.BackSlash: return "\\";
			case KeyCode.At: return "@";
			case KeyCode.Add: return "+";
			case KeyCode.Plus: return "+";
			case KeyCode.Subtract: return "-";
			case KeyCode.HyphenMinus: return "-";
			case KeyCode.Multiply: return "*";
			case KeyCode.Divide: return "/";
			case KeyCode.Decimal: return ".";
			case KeyCode.Zero: return "0";
			case KeyCode.Numpad0: return "0";
			case KeyCode.One: return "1";
			case KeyCode.Numpad1: return "1";
			case KeyCode.Two: return "2";
			case KeyCode.Numpad2: return "2";
			case KeyCode.Three: return "3";
			case KeyCode.Numpad3: return "3";
			case KeyCode.Four: return "4";
			case KeyCode.Numpad4: return "4";
			case KeyCode.Five: return "5";
			case KeyCode.Numpad5: return "5";
			case KeyCode.Six: return "6";
			case KeyCode.Numpad6: return "6";
			case KeyCode.Seven: return "7";
			case KeyCode.Numpad7: return "7";
			case KeyCode.Eight: return "8";
			case KeyCode.Numpad8: return "8";
			case KeyCode.Nine: return "9";
			case KeyCode.Numpad9: return "9";
			case KeyCode.F1: return "f1";
			case KeyCode.F2: return "f2";
			case KeyCode.F3: return "f3";
			case KeyCode.F4: return "f4";
			case KeyCode.F5: return "f5";
			case KeyCode.F6: return "f6";
			case KeyCode.F7: return "f7";
			case KeyCode.F8: return "f8";
			case KeyCode.F9: return "f9";
			case KeyCode.F10: return "f10";
			case KeyCode.F11: return "f11";
			case KeyCode.F12: return "f12";
			default: return String.fromCharCode(key).toLowerCase();
		}
	}

	static downListener = (code: KeyCode) => {
		let s = Keyboard.keyCode(code);
		Keyboard.keysFrame.push(s);
		Keyboard.keysStarted.set(s, true);
		Keyboard.keysDown.set(s, true);
		Keyboard.repeatTime = Time.time() + 0.4;

		///if krom_android_rmb // Detect right mouse button on Android..
		if (code == KeyCode.Back) {
			if (!Mouse.buttonsDown[1]) Mouse.downListener(1, Math.floor(Mouse.x), Math.floor(Mouse.y));
		}
		///end
	}

	static upListener = (code: KeyCode) => {
		let s = Keyboard.keyCode(code);
		Keyboard.keysFrame.push(s);
		Keyboard.keysReleased.set(s, true);
		Keyboard.keysDown.set(s, false);

		///if krom_android_rmb
		if (code == KeyCode.Back) {
			Mouse.upListener(1, Math.floor(Mouse.x), Math.floor(Mouse.y));
		}
		///end
	}

	static pressListener(char: string) {}
}

class GamepadStick {
	x = 0.0;
	y = 0.0;
	lastX = 0.0;
	lastY = 0.0;
	moved = false;
	movementX = 0.0;
	movementY = 0.0;
}

class TGamepad {
	buttonsDown: f32[] = []; // Intensity 0 - 1
	buttonsStarted: bool[] = [];
	buttonsReleased: bool[] = [];
	buttonsFrame: i32[] = [];
	leftStick = new GamepadStick();
	rightStick = new GamepadStick();
}

class Gamepad {

	static buttonsPS = ["cross", "circle", "square", "triangle", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
	static buttonsXBOX = ["a", "b", "x", "y", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
	static buttons = Gamepad.buttonsPS;
	static raws: TGamepad[];

	static endFrame = () => {
		for (let g of Gamepad.raws) {
			if (g.buttonsFrame.length > 0) {
				for (let i of g.buttonsFrame) {
					g.buttonsStarted[i] = false;
					g.buttonsReleased[i] = false;
				}
				g.buttonsFrame.splice(0, g.buttonsFrame.length);
			}
			g.leftStick.moved = false;
			g.leftStick.movementX = 0;
			g.leftStick.movementY = 0;
			g.rightStick.moved = false;
			g.rightStick.movementX = 0;
			g.rightStick.movementY = 0;
		}
	}

	static reset = () => {
		Gamepad.raws = [];
		for (let i = 0; i < 4; ++i) {
			let g = new TGamepad();
			Gamepad.raws.push(g);
			for (let s of Gamepad.buttons) {
				g.buttonsDown.push(0.0);
				g.buttonsStarted.push(false);
				g.buttonsReleased.push(false);
			}
		}

		Gamepad.endFrame();
	}

	static keyCode = (button: i32): string => {
		return Gamepad.buttons[button];
	}

	static buttonIndex = (button: string): i32 => {
		for (let i = 0; i < Gamepad.buttons.length; ++i) if (Gamepad.buttons[i] == button) return i;
		return 0;
	}

	static down = (i: i32, button: string): f32 => {
		return Gamepad.raws[i].buttonsDown[Gamepad.buttonIndex(button)];
	}

	static started = (i: i32, button: string): bool => {
		return Gamepad.raws[i].buttonsStarted[Gamepad.buttonIndex(button)];
	}

	static released = (i: i32, button: string): bool => {
		return Gamepad.raws[i].buttonsReleased[Gamepad.buttonIndex(button)];
	}

	static axisListener = (i: i32, axis: i32, value: f32) => {
		let stick = axis <= 1 ? Gamepad.raws[i].leftStick : Gamepad.raws[i].rightStick;

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

	static buttonListener = (i: i32, button: i32, value: f32) => {
		Gamepad.raws[i].buttonsFrame.push(button);

		Gamepad.raws[i].buttonsDown[button] = value;
		if (value > 0) Gamepad.raws[i].buttonsStarted[button] = true; // Will trigger L2/R2 multiple times..
		else Gamepad.raws[i].buttonsReleased[button] = true;
	}
}

// class Sensor {
// 	static x = 0.0;
// 	static y = 0.0;
// 	static z = 0.0;

// 	constructor() {
// 		// System.getSensor(Accelerometer).notify(listener);
// 	}

// 	static listener = (x: f32, y: f32, z: f32) => {
// 		Sensor.x = x;
// 		Sensor.y = y;
// 		Sensor.z = z;
// 	}
// }

// enum SensorType {
// 	Accelerometer,
// 	Gyroscope,
// }

enum KeyCode {
	Unknown = 0,
	Back = 1, // Android
	Cancel = 3,
	Help = 6,
	Backspace = 8,
	Tab = 9,
	Clear = 12,
	Return = 13,
	Shift = 16,
	Control = 17,
	Alt = 18,
	Pause = 19,
	CapsLock = 20,
	Kana = 21,
	Hangul = 21,
	Eisu = 22,
	Junja = 23,
	Final = 24,
	Hanja = 25,
	Kanji = 25,
	Escape = 27,
	Convert = 28,
	NonConvert = 29,
	Accept = 30,
	ModeChange = 31,
	Space = 32,
	PageUp = 33,
	PageDown = 34,
	End = 35,
	Home = 36,
	Left = 37,
	Up = 38,
	Right = 39,
	Down = 40,
	Select = 41,
	Print = 42,
	Execute = 43,
	PrintScreen = 44,
	Insert = 45,
	Delete = 46,
	Zero = 48,
	One = 49,
	Two = 50,
	Three = 51,
	Four = 52,
	Five = 53,
	Six = 54,
	Seven = 55,
	Eight = 56,
	Nine = 57,
	Colon = 58,
	Semicolon = 59,
	LessThan = 60,
	Equals = 61,
	GreaterThan = 62,
	QuestionMark = 63,
	At = 64,
	A = 65,
	B = 66,
	C = 67,
	D = 68,
	E = 69,
	F = 70,
	G = 71,
	H = 72,
	I = 73,
	J = 74,
	K = 75,
	L = 76,
	M = 77,
	N = 78,
	O = 79,
	P = 80,
	Q = 81,
	R = 82,
	S = 83,
	T = 84,
	U = 85,
	V = 86,
	W = 87,
	X = 88,
	Y = 89,
	Z = 90,
	Win = 91,
	ContextMenu = 93,
	Sleep = 95,
	Numpad0 = 96,
	Numpad1 = 97,
	Numpad2 = 98,
	Numpad3 = 99,
	Numpad4 = 100,
	Numpad5 = 101,
	Numpad6 = 102,
	Numpad7 = 103,
	Numpad8 = 104,
	Numpad9 = 105,
	Multiply = 106,
	Add = 107,
	Separator = 108,
	Subtract = 109,
	Decimal = 110,
	Divide = 111,
	F1 = 112,
	F2 = 113,
	F3 = 114,
	F4 = 115,
	F5 = 116,
	F6 = 117,
	F7 = 118,
	F8 = 119,
	F9 = 120,
	F10 = 121,
	F11 = 122,
	F12 = 123,
	F13 = 124,
	F14 = 125,
	F15 = 126,
	F16 = 127,
	F17 = 128,
	F18 = 129,
	F19 = 130,
	F20 = 131,
	F21 = 132,
	F22 = 133,
	F23 = 134,
	F24 = 135,
	NumLock = 144,
	ScrollLock = 145,
	WinOemFjJisho = 146,
	WinOemFjMasshou = 147,
	WinOemFjTouroku = 148,
	WinOemFjLoya = 149,
	WinOemFjRoya = 150,
	Circumflex = 160,
	Exclamation = 161,
	DoubleQuote = 162,
	Hash = 163,
	Dollar = 164,
	Percent = 165,
	Ampersand = 166,
	Underscore = 167,
	OpenParen = 168,
	CloseParen = 169,
	Asterisk = 170,
	Plus = 171,
	Pipe = 172,
	HyphenMinus = 173,
	OpenCurlyBracket = 174,
	CloseCurlyBracket = 175,
	Tilde = 176,
	VolumeMute = 181,
	VolumeDown = 182,
	VolumeUp = 183,
	Comma = 188,
	Period = 190,
	Slash = 191,
	BackQuote = 192,
	OpenBracket = 219,
	BackSlash = 220,
	CloseBracket = 221,
	Quote = 222,
	Meta = 224,
	AltGr = 225,
	WinIcoHelp = 227,
	WinIco00 = 228,
	WinIcoClear = 230,
	WinOemReset = 233,
	WinOemJump = 234,
	WinOemPA1 = 235,
	WinOemPA2 = 236,
	WinOemPA3 = 237,
	WinOemWSCTRL = 238,
	WinOemCUSEL = 239,
	WinOemATTN = 240,
	WinOemFinish = 241,
	WinOemCopy = 242,
	WinOemAuto = 243,
	WinOemENLW = 244,
	WinOemBackTab = 245,
	ATTN = 246,
	CRSEL = 247,
	EXSEL = 248,
	EREOF = 249,
	Play = 250,
	Zoom = 251,
	PA1 = 253,
	WinOemClear = 254,
}
