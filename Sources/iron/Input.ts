
class Input {

	static occupied = false;
	static mouse: Mouse = null;
	static pen: Pen = null;
	static keyboard: Keyboard = null;
	static gamepads: Gamepad[] = [];
	static sensor: Sensor = null;
	static registered = false;

	static reset = () => {
		Input.occupied = false;
		if (Input.mouse != null) Input.mouse.reset();
		if (Input.pen != null) Input.pen.reset();
		if (Input.keyboard != null) Input.keyboard.reset();
		for (let gamepad of Input.gamepads) gamepad.reset();
	}

	static endFrame = () => {
		if (Input.mouse != null) Input.mouse.endFrame();
		if (Input.pen != null) Input.pen.endFrame();
		if (Input.keyboard != null) Input.keyboard.endFrame();
		for (let gamepad of Input.gamepads) gamepad.endFrame();
	}

	static getMouse = (): Mouse => {
		if (!Input.registered) Input.register();
		if (Input.mouse == null) Input.mouse = new Mouse();
		return Input.mouse;
	}

	static getPen = (): Pen => {
		if (!Input.registered) Input.register();
		if (Input.pen == null) Input.pen = new Pen();
		return Input.pen;
	}

	static getSurface = (): Surface => {
		if (!Input.registered) Input.register();
		// Map to mouse for now..
		return Input.getMouse();
	}

	static getKeyboard = (): Keyboard => {
		if (!Input.registered) Input.register();
		if (Input.keyboard == null) Input.keyboard = new Keyboard();
		return Input.keyboard;
	}

	static getGamepad = (i = 0): Gamepad => {
		if (i >= 4) return null;
		if (!Input.registered) Input.register();
		while (Input.gamepads.length <= i) Input.gamepads.push(new Gamepad(Input.gamepads.length));
		return Input.gamepads[i];
	}

	static getSensor = (): Sensor => {
		if (!Input.registered) Input.register();
		if (Input.sensor == null) Input.sensor = new Sensor();
		return Input.sensor;
	}

	static register = () => {
		Input.registered = true;
		App.notifyOnEndFrame(Input.endFrame);
		App.notifyOnReset(Input.reset);
		// Reset mouse delta on foreground
		System.notifyOnApplicationState(() => { Input.getMouse().reset(); }, null, null, null, null);
	}
}

type Surface = Mouse;

class Mouse {

	static buttons = ["left", "right", "middle", "side1", "side2"];
	buttonsDown = [false, false, false, false, false];
	buttonsStarted = [false, false, false, false, false];
	buttonsReleased = [false, false, false, false, false];

	x = 0.0;
	y = 0.0;
	moved = false;
	movementX = 0.0;
	movementY = 0.0;
	wheelDelta = 0.0;
	locked = false;
	hidden = false;
	lastX = -1.0;
	lastY = -1.0;

	constructor() {}

	endFrame = () => {
		this.buttonsStarted[0] = this.buttonsStarted[1] = this.buttonsStarted[2] = this.buttonsStarted[3] = this.buttonsStarted[4] = false;
		this.buttonsReleased[0] = this.buttonsReleased[1] = this.buttonsReleased[2] = this.buttonsReleased[3] = this.buttonsReleased[4] = false;
		this.moved = false;
		this.movementX = 0;
		this.movementY = 0;
		this.wheelDelta = 0;
	}

	reset = () => {
		this.buttonsDown[0] = this.buttonsDown[1] = this.buttonsDown[2] = this.buttonsDown[3] = this.buttonsDown[4] = false;
		this.endFrame();
	}

	buttonIndex = (button: string): i32 => {
		for (let i = 0; i < Mouse.buttons.length; ++i) if (Mouse.buttons[i] == button) return i;
		return 0;
	}

	down = (button = "left"): bool => {
		return this.buttonsDown[this.buttonIndex(button)];
	}

	downAny = (): bool => {
		return this.buttonsDown[0] || this.buttonsDown[1] || this.buttonsDown[2] || this.buttonsDown[3] || this.buttonsDown[4];
	}

	started = (button = "left"): bool => {
		return this.buttonsStarted[this.buttonIndex(button)];
	}

	startedAny = (): bool => {
		return this.buttonsStarted[0] || this.buttonsStarted[1] || this.buttonsStarted[2] || this.buttonsStarted[3] || this.buttonsStarted[4];
	}

	released = (button = "left"): bool => {
		return this.buttonsReleased[this.buttonIndex(button)];
	}

	lock = () => {
		if (System.canLockMouse()) {
			System.lockMouse();
			this.locked = true;
			this.hidden = true;
		}
	}
	unlock = () => {
		if (System.canLockMouse()) {
			System.unlockMouse();
			this.locked = false;
			this.hidden = false;
		}
	}

	hide = () => {
		System.hideSystemCursor();
		this.hidden = true;
	}

	show = () => {
		System.showSystemCursor();
		this.hidden = false;
	}

	downListener = (index: i32, x: i32, y: i32) => {
		if (Input.getPen().inUse) return;

		this.buttonsDown[index] = true;
		this.buttonsStarted[index] = true;
		this.x = x;
		this.y = y;
		///if (krom_android || krom_ios) // For movement delta using touch
		if (index == 0) {
			this.lastX = x;
			this.lastY = y;
		}
		///end
	}

	upListener = (index: i32, x: i32, y: i32) => {
		if (Input.getPen().inUse) return;

		this.buttonsDown[index] = false;
		this.buttonsReleased[index] = true;
		this.x = x;
		this.y = y;
	}

	moveListener = (x: i32, y: i32, movementX: i32, movementY: i32) => {
		if (this.lastX == -1.0 && this.lastY == -1.0) { // First frame init
			this.lastX = x;
			this.lastY = y;
		}
		if (this.locked) {
			// Can be called multiple times per frame
			this.movementX += movementX;
			this.movementY += movementY;
		}
		else {
			this.movementX += x - this.lastX;
			this.movementY += y - this.lastY;
		}
		this.lastX = x;
		this.lastY = y;
		this.x = x;
		this.y = y;
		this.moved = true;
	}

	wheelListener = (delta: i32) => {
		this.wheelDelta = delta;
	}

	///if (krom_android || krom_ios)
	onTouchDown = (index: i32, x: i32, y: i32) => {
		if (index == 1) { // Two fingers down - right mouse button
			this.buttonsDown[0] = false;
			this.downListener(1, Math.floor(this.x), Math.floor(this.y));
			this.pinchStarted = true;
			this.pinchTotal = 0.0;
			this.pinchDistance = 0.0;
		}
		else if (index == 2) { // Three fingers down - middle mouse button
			this.buttonsDown[1] = false;
			this.downListener(2, Math.floor(this.x), Math.floor(this.y));
		}
	}

	onTouchUp = (index: i32, x: i32, y: i32) => {
		if (index == 1) this.upListener(1, Math.floor(this.x), Math.floor(this.y));
		else if (index == 2) this.upListener(2, Math.floor(this.x), Math.floor(this.y));
	}

	pinchDistance = 0.0;
	pinchTotal = 0.0;
	pinchStarted = false;

	onTouchMove = (index: i32, x: i32, y: i32) => {
		// Pinch to zoom - mouse wheel
		if (index == 1) {
			let lastDistance = this.pinchDistance;
			let dx = this.x - x;
			let dy = this.y - y;
			this.pinchDistance = Math.sqrt(dx * dx + dy * dy);
			this.pinchTotal += lastDistance != 0 ? lastDistance - this.pinchDistance : 0;
			if (!this.pinchStarted) {
				this.wheelDelta = this.pinchTotal / 10;
				if (this.wheelDelta != 0) {
					this.pinchTotal = 0.0;
				}
			}
			this.pinchStarted = false;
		}
	}
	///end

	get viewX(): f32 {
		return this.x - App.x();
	}

	get viewY(): f32 {
		return this.y - App.y();
	}
}

class Pen {

	static buttons = ["tip"];
	buttonsDown = [false];
	buttonsStarted = [false];
	buttonsReleased = [false];

	x = 0.0;
	y = 0.0;
	moved = false;
	movementX = 0.0;
	movementY = 0.0;
	pressure = 0.0;
	connected = false;
	inUse = false;
	lastX = -1.0;
	lastY = -1.0;

	constructor() {}

	endFrame = () => {
		this.buttonsStarted[0] = false;
		this.buttonsReleased[0] = false;
		this.moved = false;
		this.movementX = 0;
		this.movementY = 0;
		this.inUse = false;
	}

	reset = () => {
		this.buttonsDown[0] = false;
		this.endFrame();
	}

	buttonIndex = (button: string): i32 => {
		return 0;
	}

	down = (button = "tip"): bool => {
		return this.buttonsDown[this.buttonIndex(button)];
	}

	started = (button = "tip"): bool => {
		return this.buttonsStarted[this.buttonIndex(button)];
	}

	released = (button = "tip"): bool => {
		return this.buttonsReleased[this.buttonIndex(button)];
	}

	downListener = (x: i32, y: i32, pressure: f32) => {
		this.buttonsDown[0] = true;
		this.buttonsStarted[0] = true;
		this.x = x;
		this.y = y;
		this.pressure = pressure;

		///if (!krom_android && !krom_ios)
		Input.getMouse().downListener(0, x, y);
		///end
	}

	upListener = (x: i32, y: i32, pressure: f32) => {
		///if (!krom_android && !krom_ios)
		if (this.buttonsStarted[0]) { this.buttonsStarted[0] = false; this.inUse = true; return; }
		///end

		this.buttonsDown[0] = false;
		this.buttonsReleased[0] = true;
		this.x = x;
		this.y = y;
		this.pressure = pressure;

		///if (!krom_android && !krom_ios)
		Input.getMouse().upListener(0, x, y);
		this.inUse = true; // On pen release, additional mouse down & up events are fired at once - filter those out
		///end
	}

	moveListener = (x: i32, y: i32, pressure: f32) => {
		///if krom_ios
		// Listen to pen hover if no other input is active
		if (!this.buttonsDown[0] && pressure == 0.0) {
			if (!Input.getMouse().downAny()) {
				Input.getMouse().moveListener(x, y, 0, 0);
			}
			return;
		}
		///end

		if (this.lastX == -1.0 && this.lastY == -1.0) { // First frame init
			this.lastX = x;
			this.lastY = y;
		}
		this.movementX = x - this.lastX;
		this.movementY = y - this.lastY;
		this.lastX = x;
		this.lastY = y;
		this.x = x;
		this.y = y;
		this.moved = true;
		this.pressure = pressure;
		this.connected = true;
	}

	get viewX(): f32 {
		return this.x - App.x();
	}

	get viewY(): f32 {
		return this.y - App.y();
	}
}

class Keyboard {

	static keys = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "space", "backspace", "tab", "enter", "shift", "control", "alt", "win", "escape", "delete", "up", "down", "left", "right", "back", ",", ".", ":", ";", "<", "=", ">", "?", "!", '"', "#", "$", "%", "&", "_", "(", ")", "*", "|", "{", "}", "[", "]", "~", "`", "/", "\\", "@", "+", "-", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12"];
	keysDown = new Map<string, bool>();
	keysStarted = new Map<string, bool>();
	keysReleased = new Map<string, bool>();
	keysFrame: string[] = [];
	repeatKey = false;
	repeatTime = 0.0;

	constructor() {
		this.reset();
	}

	endFrame = () => {
		if (this.keysFrame.length > 0) {
			for (let s of this.keysFrame) {
				this.keysStarted.set(s, false);
				this.keysReleased.set(s, false);
			}
			this.keysFrame.splice(0, this.keysFrame.length);
		}

		if (Time.time() - this.repeatTime > 0.05) {
			this.repeatTime = Time.time();
			this.repeatKey = true;
		}
		else this.repeatKey = false;
	}

	reset = () => {
		// Use Map for now..
		for (let s of Keyboard.keys) {
			this.keysDown.set(s, false);
			this.keysStarted.set(s, false);
			this.keysReleased.set(s, false);
		}
		this.endFrame();
	}

	down = (key: string): bool => {
		return this.keysDown.get(key);
	}

	started = (key: string): bool => {
		return this.keysStarted.get(key);
	}

	startedAny = (): bool => {
		return this.keysFrame.length > 0;
	}

	released = (key: string): bool => {
		return this.keysReleased.get(key);
	}

	repeat = (key: string): bool => {
		return this.keysStarted.get(key) || (this.repeatKey && this.keysDown.get(key));
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

	downListener = (code: KeyCode) => {
		let s = Keyboard.keyCode(code);
		this.keysFrame.push(s);
		this.keysStarted.set(s, true);
		this.keysDown.set(s, true);
		this.repeatTime = Time.time() + 0.4;

		///if krom_android_rmb // Detect right mouse button on Android..
		if (code == KeyCode.Back) {
			let m = Input.getMouse();
			if (!m.buttonsDown[1]) m.downListener(1, Math.floor(m.x), Math.floor(m.y));
		}
		///end
	}

	upListener = (code: KeyCode) => {
		let s = Keyboard.keyCode(code);
		this.keysFrame.push(s);
		this.keysReleased.set(s, true);
		this.keysDown.set(s, false);

		///if krom_android_rmb
		if (code == KeyCode.Back) {
			let m = Input.getMouse();
			m.upListener(1, Math.floor(m.x), Math.floor(m.y));
		}
		///end
	}

	pressListener(char: string) {}
}

class GamepadStick {
	x = 0.0;
	y = 0.0;
	lastX = 0.0;
	lastY = 0.0;
	moved = false;
	movementX = 0.0;
	movementY = 0.0;
	constructor() {}
}

class Gamepad {

	static buttonsPS = ["cross", "circle", "square", "triangle", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
	static buttonsXBOX = ["a", "b", "x", "y", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
	static buttons = Gamepad.buttonsPS;

	buttonsDown: f32[] = []; // Intensity 0 - 1
	buttonsStarted: bool[] = [];
	buttonsReleased: bool[] = [];

	buttonsFrame: i32[] = [];

	leftStick = new GamepadStick();
	rightStick = new GamepadStick();

	num = 0;

	constructor(i: i32) {
		for (let s of Gamepad.buttons) {
			this.buttonsDown.push(0.0);
			this.buttonsStarted.push(false);
			this.buttonsReleased.push(false);
		}
		this.num = i;
		this.reset();
	}

	endFrame = () => {
		if (this.buttonsFrame.length > 0) {
			for (let i of this.buttonsFrame) {
				this.buttonsStarted[i] = false;
				this.buttonsReleased[i] = false;
			}
			this.buttonsFrame.splice(0, this.buttonsFrame.length);
		}
		this.leftStick.moved = false;
		this.leftStick.movementX = 0;
		this.leftStick.movementY = 0;
		this.rightStick.moved = false;
		this.rightStick.movementX = 0;
		this.rightStick.movementY = 0;
	}

	reset = () => {
		for (let i = 0; i < this.buttonsDown.length; ++i) {
			this.buttonsDown[i] = 0.0;
			this.buttonsStarted[i] = false;
			this.buttonsReleased[i] = false;
		}
		this.endFrame();
	}

	static keyCode = (button: i32): string => {
		return Gamepad.buttons[button];
	}

	buttonIndex = (button: string): i32 => {
		for (let i = 0; i < Gamepad.buttons.length; ++i) if (Gamepad.buttons[i] == button) return i;
		return 0;
	}

	down = (button: string): f32 => {
		return this.buttonsDown[this.buttonIndex(button)];
	}

	started = (button: string): bool => {
		return this.buttonsStarted[this.buttonIndex(button)];
	}

	released = (button: string): bool => {
		return this.buttonsReleased[this.buttonIndex(button)];
	}

	axisListener = (axis: i32, value: f32) => {
		let stick = axis <= 1 ? this.leftStick : this.rightStick;

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

	buttonListener = (button: i32, value: f32) => {
		this.buttonsFrame.push(button);

		this.buttonsDown[button] = value;
		if (value > 0) this.buttonsStarted[button] = true; // Will trigger L2/R2 multiple times..
		else this.buttonsReleased[button] = true;
	}
}

class Sensor {
	x = 0.0;
	y = 0.0;
	z = 0.0;

	constructor() {
		// System.getSensor(Accelerometer).notify(listener);
	}

	listener = (x: f32, y: f32, z: f32) => {
		this.x = x;
		this.y = y;
		this.z = z;
	}
}

enum SensorType {
	Accelerometer,
	Gyroscope,
}

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
