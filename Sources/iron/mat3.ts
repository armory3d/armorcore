
class mat3_t {
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

function mat3_create(_00: f32, _10: f32, _20: f32,
					 _01: f32, _11: f32, _21: f32,
					 _02: f32, _12: f32, _22: f32): mat3_t {
	let self = new mat3_t();
	self._00 = _00; self._10 = _10; self._20 = _20;
	self._01 = _01; self._11 = _11; self._21 = _21;
	self._02 = _02; self._12 = _12; self._22 = _22;
	return self;
}

function mat3_identity(): mat3_t {
	return mat3_create(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);
}

function mat3_translation(x: f32, y: f32): mat3_t {
	return mat3_create(
		1, 0, x,
		0, 1, y,
		0, 0, 1
	);
}

function mat3_rotation(alpha: f32): mat3_t {
	return mat3_create(
		Math.cos(alpha), -Math.sin(alpha), 0,
		Math.sin(alpha), Math.cos(alpha), 0,
		0, 0, 1
	);
}

function mat3_set_from4(self: mat3_t, m: mat4_t) {
	self._00 = m._00;
	self._01 = m._01;
	self._02 = m._02;
	self._10 = m._10;
	self._11 = m._11;
	self._12 = m._12;
	self._20 = m._20;
	self._21 = m._21;
	self._22 = m._22;
}

function mat3_multmat(self: mat3_t, m: mat3_t): mat3_t {
	return mat3_create(
		self._00 * m._00 + self._10 * m._01 + self._20 * m._02, self._00 * m._10 + self._10 * m._11 + self._20 * m._12, self._00 * m._20 + self._10 * m._21 + self._20 * m._22,
		self._01 * m._00 + self._11 * m._01 + self._21 * m._02, self._01 * m._10 + self._11 * m._11 + self._21 * m._12, self._01 * m._20 + self._11 * m._21 + self._21 * m._22,
		self._02 * m._00 + self._12 * m._01 + self._22 * m._02, self._02 * m._10 + self._12 * m._11 + self._22 * m._12, self._02 * m._20 + self._12 * m._21 + self._22 * m._22
	);
}
