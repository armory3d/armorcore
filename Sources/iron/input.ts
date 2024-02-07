
let _input_occupied = false;
let _input_registered = false;

function input_reset() {
	_input_occupied = false;
	mouse_reset();
	pen_reset();
	keyboard_reset();
	gamepad_reset();
}

function input_end_frame() {
	mouse_end_frame();
	pen_end_frame();
	keyboard_end_frame();
	gamepad_end_frame();
}

function input_register() {
	if (_input_registered) {
		return;
	}
	_input_registered = true;
	app_notify_on_end_frame(input_end_frame);
	app_notify_on_reset(input_reset);
	// Reset mouse delta on foreground
	sys_notify_on_app_state(function() { mouse_reset(); }, null, null, null, null);
	keyboard_reset();
	gamepad_reset();
}

let _mouse_buttons = ["left", "right", "middle", "side1", "side2"];
let _mouse_buttons_down = [false, false, false, false, false];
let _mouse_buttons_started = [false, false, false, false, false];
let _mouse_buttons_released = [false, false, false, false, false];

let mouse_x = 0.0;
let mouse_y = 0.0;
let mouse_moved = false;
let mouse_movement_x = 0.0;
let mouse_movement_y = 0.0;
let mouse_wheel_delta = 0.0;
let mouse_locked = false;
let mouse_hidden = false;
let mouse_last_x = -1.0;
let mouse_last_y = -1.0;

function mouse_end_frame() {
	_mouse_buttons_started[0] = _mouse_buttons_started[1] = _mouse_buttons_started[2] = _mouse_buttons_started[3] = _mouse_buttons_started[4] = false;
	_mouse_buttons_released[0] = _mouse_buttons_released[1] = _mouse_buttons_released[2] = _mouse_buttons_released[3] = _mouse_buttons_released[4] = false;
	mouse_moved = false;
	mouse_movement_x = 0;
	mouse_movement_y = 0;
	mouse_wheel_delta = 0;
}

function mouse_reset() {
	_mouse_buttons_down[0] = _mouse_buttons_down[1] = _mouse_buttons_down[2] = _mouse_buttons_down[3] = _mouse_buttons_down[4] = false;
	mouse_end_frame();
}

function mouse_button_index(button: string): i32 {
	for (let i = 0; i < _mouse_buttons.length; ++i) {
		if (_mouse_buttons[i] == button) {
			return i;
		}
	}
	return 0;
}

function mouse_down(button = "left"): bool {
	return _mouse_buttons_down[mouse_button_index(button)];
}

function mouse_down_any(): bool {
	return _mouse_buttons_down[0] || _mouse_buttons_down[1] || _mouse_buttons_down[2] || _mouse_buttons_down[3] || _mouse_buttons_down[4];
}

function mouse_started(button = "left"): bool {
	return _mouse_buttons_started[mouse_button_index(button)];
}

function mouse_started_any(): bool {
	return _mouse_buttons_started[0] || _mouse_buttons_started[1] || _mouse_buttons_started[2] || _mouse_buttons_started[3] || _mouse_buttons_started[4];
}

function mouse_released(button = "left"): bool {
	return _mouse_buttons_released[mouse_button_index(button)];
}

function mouse_lock() {
	if (sys_can_lock_mouse()) {
		sys_lock_mouse();
		mouse_locked = true;
		mouse_hidden = true;
	}
}

function mouse_unlock() {
	if (sys_can_lock_mouse()) {
		sys_unlock_mouse();
		mouse_locked = false;
		mouse_hidden = false;
	}
}

function mouse_hide() {
	sys_hide_system_cursor();
	mouse_hidden = true;
}

function mouse_show() {
	sys_show_system_cursor();
	mouse_hidden = false;
}

function mouse_down_listener(index: i32, x: i32, y: i32) {
	if (pen_in_use) {
		return;
	}

	_mouse_buttons_down[index] = true;
	_mouse_buttons_started[index] = true;
	mouse_x = x;
	mouse_y = y;
	///if (krom_android || krom_ios) // For movement delta using touch
	if (index == 0) {
		mouse_last_x = x;
		mouse_last_y = y;
	}
	///end
}

function mouse_up_listener(index: i32, x: i32, y: i32) {
	if (pen_in_use) {
		return;
	}

	_mouse_buttons_down[index] = false;
	_mouse_buttons_released[index] = true;
	mouse_x = x;
	mouse_y = y;
}

function mouse_move_listener(x: i32, y: i32, movement_x: i32, movement_y: i32) {
	if (mouse_last_x == -1.0 && mouse_last_y == -1.0) { // First frame init
		mouse_last_x = x;
		mouse_last_y = y;
	}
	if (mouse_locked) {
		// Can be called multiple times per frame
		mouse_movement_x += movement_x;
		mouse_movement_y += movement_y;
	}
	else {
		mouse_movement_x += x - mouse_last_x;
		mouse_movement_y += y - mouse_last_y;
	}
	mouse_last_x = x;
	mouse_last_y = y;
	mouse_x = x;
	mouse_y = y;
	mouse_moved = true;
}

function mouse_wheel_listener(delta: i32) {
	mouse_wheel_delta = delta;
}

///if (krom_android || krom_ios)
function mouse_on_touch_down(index: i32, x: i32, y: i32) {
	if (index == 1) { // Two fingers down - right mouse button
		_mouse_buttons_down[0] = false;
		mouse_down_listener(1, Math.floor(mouse_x), Math.floor(mouse_y));
		mouse_pinch_started = true;
		mouse_pinch_total = 0.0;
		mouse_pinch_dist = 0.0;
	}
	else if (index == 2) { // Three fingers down - middle mouse button
		_mouse_buttons_down[1] = false;
		mouse_down_listener(2, Math.floor(mouse_x), Math.floor(mouse_y));
	}
}

function mouse_on_touch_up(index: i32, x: i32, y: i32) {
	if (index == 1) {
		mouse_up_listener(1, Math.floor(mouse_x), Math.floor(mouse_y));
	}
	else if (index == 2) {
		mouse_up_listener(2, Math.floor(mouse_x), Math.floor(mouse_y));
	}
}

let mouse_pinch_dist = 0.0;
let mouse_pinch_total = 0.0;
let mouse_pinch_started = false;

function mouse_on_touch_move(index: i32, x: i32, y: i32) {
	// Pinch to zoom - mouse wheel
	if (index == 1) {
		let last_dist = mouse_pinch_dist;
		let dx = mouse_x - x;
		let dy = mouse_y - y;
		mouse_pinch_dist = Math.sqrt(dx * dx + dy * dy);
		mouse_pinch_total += last_dist != 0 ? last_dist - mouse_pinch_dist : 0;
		if (!mouse_pinch_started) {
			mouse_wheel_delta = mouse_pinch_total / 10;
			if (mouse_wheel_delta != 0) {
				mouse_pinch_total = 0.0;
			}
		}
		mouse_pinch_started = false;
	}
}
///end

function mouse_view_x(): f32 {
	return mouse_x - app_x();
}

function mouse_view_y(): f32 {
	return mouse_y - app_y();
}

let pen_buttons = ["tip"];
let pen_buttons_down = [false];
let pen_buttons_started = [false];
let pen_buttons_released = [false];

let pen_x = 0.0;
let pen_y = 0.0;
let pen_moved = false;
let pen_movement_x = 0.0;
let pen_movement_y = 0.0;
let pen_pressure = 0.0;
let pen_connected = false;
let pen_in_use = false;
let pen_last_x = -1.0;
let pen_last_y = -1.0;

function pen_end_frame() {
	pen_buttons_started[0] = false;
	pen_buttons_released[0] = false;
	pen_moved = false;
	pen_movement_x = 0;
	pen_movement_y = 0;
	pen_in_use = false;
}

function pen_reset() {
	pen_buttons_down[0] = false;
	pen_end_frame();
}

function pen_button_index(button: string): i32 {
	return 0;
}

function pen_down(button = "tip"): bool {
	return pen_buttons_down[pen_button_index(button)];
}

function pen_started(button = "tip"): bool {
	return pen_buttons_started[pen_button_index(button)];
}

function pen_released(button = "tip"): bool {
	return pen_buttons_released[pen_button_index(button)];
}

function pen_down_listener(x: i32, y: i32, pressure: f32) {
	pen_buttons_down[0] = true;
	pen_buttons_started[0] = true;
	pen_x = x;
	pen_y = y;
	pen_pressure = pressure;

	///if (!krom_android && !krom_ios)
	mouse_down_listener(0, x, y);
	///end
}

function pen_up_listener(x: i32, y: i32, pressure: f32) {
	///if (!krom_android && !krom_ios)
	if (pen_buttons_started[0]) {
		pen_buttons_started[0] = false;
		pen_in_use = true;
		return;
	}
	///end

	pen_buttons_down[0] = false;
	pen_buttons_released[0] = true;
	pen_x = x;
	pen_y = y;
	pen_pressure = pressure;

	///if (!krom_android && !krom_ios)
	mouse_up_listener(0, x, y);
	pen_in_use = true; // On pen release, additional mouse down & up events are fired at once - filter those out
	///end
}

function pen_move_listener(x: i32, y: i32, pressure: f32) {
	///if krom_ios
	// Listen to pen hover if no other input is active
	if (!pen_buttons_down[0] && pressure == 0.0) {
		if (!mouse_down_any()) {
			mouse_move_listener(x, y, 0, 0);
		}
		return;
	}
	///end

	if (pen_last_x == -1.0 && pen_last_y == -1.0) { // First frame init
		pen_last_x = x;
		pen_last_y = y;
	}
	pen_movement_x = x - pen_last_x;
	pen_movement_y = y - pen_last_y;
	pen_last_x = x;
	pen_last_y = y;
	pen_x = x;
	pen_y = y;
	pen_moved = true;
	pen_pressure = pressure;
	pen_connected = true;
}

function pen_view_x(): f32 {
	return pen_x - app_x();
}

function pen_view_y(): f32 {
	return pen_y - app_y();
}

let keyboard_keys = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "space", "backspace", "tab", "enter", "shift", "control", "alt", "win", "escape", "delete", "up", "down", "left", "right", "back", ",", ".", ":", ";", "<", "=", ">", "?", "!", '"', "#", "$", "%", "&", "_", "(", ")", "*", "|", "{", "}", "[", "]", "~", "`", "/", "\\", "@", "+", "-", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12"];
let keyboard_keys_down = new Map<string, bool>();
let keyboard_keys_started = new Map<string, bool>();
let keyboard_keys_released = new Map<string, bool>();
let keyboard_keys_frame: string[] = [];
let keyboard_repeat_key = false;
let keyboard_repeat_time = 0.0;

function keyboard_end_frame() {
	if (keyboard_keys_frame.length > 0) {
		for (let s of keyboard_keys_frame) {
			keyboard_keys_started.set(s, false);
			keyboard_keys_released.set(s, false);
		}
		keyboard_keys_frame.splice(0, keyboard_keys_frame.length);
	}

	if (time_time() - keyboard_repeat_time > 0.05) {
		keyboard_repeat_time = time_time();
		keyboard_repeat_key = true;
	}
	else {
		keyboard_repeat_key = false;
	}
}

function keyboard_reset() {
	// Use Map for now..
	for (let s of keyboard_keys) {
		keyboard_keys_down.set(s, false);
		keyboard_keys_started.set(s, false);
		keyboard_keys_released.set(s, false);
	}
	keyboard_end_frame();
}

function keyboard_down(key: string): bool {
	return keyboard_keys_down.get(key);
}

function keyboard_started(key: string): bool {
	return keyboard_keys_started.get(key);
}

function keyboard_started_any(): bool {
	return keyboard_keys_frame.length > 0;
}

function keyboard_released(key: string): bool {
	return keyboard_keys_released.get(key);
}

function keyboard_repeat(key: string): bool {
	return keyboard_keys_started.get(key) || (keyboard_repeat_key && keyboard_keys_down.get(key));
}

function keyboard_key_code(key: KeyCode): string {
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

function keyboard_down_listener(code: KeyCode) {
	let s = keyboard_key_code(code);
	keyboard_keys_frame.push(s);
	keyboard_keys_started.set(s, true);
	keyboard_keys_down.set(s, true);
	keyboard_repeat_time = time_time() + 0.4;

	///if krom_android_rmb // Detect right mouse button on Android..
	if (code == KeyCode.Back) {
		if (!_mouse_buttons_down[1]) {
			mouse_down_listener(1, Math.floor(mouse_x), Math.floor(mouse_y));
		}
	}
	///end
}

function keyboard_up_listener(code: KeyCode) {
	let s = keyboard_key_code(code);
	keyboard_keys_frame.push(s);
	keyboard_keys_released.set(s, true);
	keyboard_keys_down.set(s, false);

	///if krom_android_rmb
	if (code == KeyCode.Back) {
		mouse_up_listener(1, Math.floor(mouse_x), Math.floor(mouse_y));
	}
	///end
}

function keyboard_press_listener(char: string) {}

class gamepad_stick_t {
	x = 0.0;
	y = 0.0;
	last_x = 0.0;
	last_y = 0.0;
	moved = false;
	movement_x = 0.0;
	movement_y = 0.0;
}

class gamepad_t {
	buttons_down: f32[] = []; // Intensity 0 - 1
	buttons_started: bool[] = [];
	buttons_released: bool[] = [];
	buttons_frame: i32[] = [];
	left_stick = new gamepad_stick_t();
	right_stick = new gamepad_stick_t();
}

let gamepad_buttons_ps = ["cross", "circle", "square", "triangle", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
let gamepad_buttons_xbox = ["a", "b", "x", "y", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
let gamepad_buttons = gamepad_buttons_ps;
let gamepad_raws: gamepad_t[];

function gamepad_end_frame() {
	for (let g of gamepad_raws) {
		if (g.buttons_frame.length > 0) {
			for (let i of g.buttons_frame) {
				g.buttons_started[i] = false;
				g.buttons_released[i] = false;
			}
			g.buttons_frame.splice(0, g.buttons_frame.length);
		}
		g.left_stick.moved = false;
		g.left_stick.movement_x = 0;
		g.left_stick.movement_y = 0;
		g.right_stick.moved = false;
		g.right_stick.movement_x = 0;
		g.right_stick.movement_y = 0;
	}
}

function gamepad_reset() {
	gamepad_raws = [];
	for (let i = 0; i < 4; ++i) {
		let g = new gamepad_t();
		gamepad_raws.push(g);
		for (let s of gamepad_buttons) {
			g.buttons_down.push(0.0);
			g.buttons_started.push(false);
			g.buttons_released.push(false);
		}
	}

	gamepad_end_frame();
}

function gamepad_key_code(button: i32): string {
	return gamepad_buttons[button];
}

function gamepad_button_index(button: string): i32 {
	for (let i = 0; i < gamepad_buttons.length; ++i) {
		if (gamepad_buttons[i] == button) {
			return i;
		}
	}
	return 0;
}

function gamepad_down(i: i32, button: string): f32 {
	return gamepad_raws[i].buttons_down[gamepad_button_index(button)];
}

function gamepad_started(i: i32, button: string): bool {
	return gamepad_raws[i].buttons_started[gamepad_button_index(button)];
}

function gamepad_released(i: i32, button: string): bool {
	return gamepad_raws[i].buttons_released[gamepad_button_index(button)];
}

function gamepad_axis_listener(i: i32, axis: i32, value: f32) {
	let stick = axis <= 1 ? gamepad_raws[i].left_stick : gamepad_raws[i].right_stick;

	if (axis == 0 || axis == 2) { // X
		stick.last_x = stick.x;
		stick.x = value;
		stick.movement_x = stick.x - stick.last_x;
	}
	else if (axis == 1 || axis == 3) { // Y
		stick.last_y = stick.y;
		stick.y = value;
		stick.movement_y = stick.y - stick.last_y;
	}
	stick.moved = true;
}

function gamepad_button_listener(i: i32, button: i32, value: f32) {
	gamepad_raws[i].buttons_frame.push(button);

	gamepad_raws[i].buttons_down[button] = value;
	if (value > 0) {
		gamepad_raws[i].buttons_started[button] = true; // Will trigger L2/R2 multiple times..
	}
	else {
		gamepad_raws[i].buttons_released[button] = true;
	}
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
