
class Mat3 {

	constructor(_00: f32, _10: f32, _20: f32,
				_01: f32, _11: f32, _21: f32,
				_02: f32, _12: f32, _22: f32) {

		this._00 = _00; this._10 = _10; this._20 = _20;
		this._01 = _01; this._11 = _11; this._21 = _21;
		this._02 = _02; this._12 = _12; this._22 = _22;
	}

	static identity = (): Mat3 => {
		return new Mat3(
			1, 0, 0,
			0, 1, 0,
			0, 0, 1
		);
	}

	setFrom4 = (m: Mat4) => {
		this._00 = m._00;
		this._01 = m._01;
		this._02 = m._02;
		this._10 = m._10;
		this._11 = m._11;
		this._12 = m._12;
		this._20 = m._20;
		this._21 = m._21;
		this._22 = m._22;
	}

	static translation = (x: f32, y: f32): Mat3 => {
		return new Mat3(
			1, 0, x,
			0, 1, y,
			0, 0, 1
		);
	}

	static rotation = (alpha: f32): Mat3 => {
		return new Mat3(
			Math.cos(alpha), -Math.sin(alpha), 0,
			Math.sin(alpha), Math.cos(alpha), 0,
			0, 0, 1
		);
	}

	multmat = (m: Mat3): Mat3 => {
		return new Mat3(
			this._00 * m._00 + this._10 * m._01 + this._20 * m._02, this._00 * m._10 + this._10 * m._11 + this._20 * m._12, this._00 * m._20 + this._10 * m._21 + this._20 * m._22,
			this._01 * m._00 + this._11 * m._01 + this._21 * m._02, this._01 * m._10 + this._11 * m._11 + this._21 * m._12, this._01 * m._20 + this._11 * m._21 + this._21 * m._22,
			this._02 * m._00 + this._12 * m._01 + this._22 * m._02, this._02 * m._10 + this._12 * m._11 + this._22 * m._12, this._02 * m._20 + this._12 * m._21 + this._22 * m._22
		);
	}

	buffer = new Float32Array(9);

	get _00(): f32 { return this.buffer[0]; } set _00(f: f32) { this.buffer[0] = f; }
	get _01(): f32 { return this.buffer[1]; } set _01(f: f32) { this.buffer[1] = f; }
	get _02(): f32 { return this.buffer[2]; } set _02(f: f32) { this.buffer[2] = f; }
	get _10(): f32 { return this.buffer[3]; } set _10(f: f32) { this.buffer[3] = f; }
	get _11(): f32 { return this.buffer[4]; } set _11(f: f32) { this.buffer[4] = f; }
	get _12(): f32 { return this.buffer[5]; } set _12(f: f32) { this.buffer[5] = f; }
	get _20(): f32 { return this.buffer[6]; } set _20(f: f32) { this.buffer[6] = f; }
	get _21(): f32 { return this.buffer[7]; } set _21(f: f32) { this.buffer[7] = f; }
	get _22(): f32 { return this.buffer[8]; } set _22(f: f32) { this.buffer[8] = f; }
}
