
// _00 = [0]
// _01 = [1]
// _02 = [2]
// _10 = [3]
// _11 = [4]
// _12 = [5]
// _20 = [6]
// _21 = [7]
// _22 = [8]

type mat3_t = {
	m?: f32_array_t;
};

function mat3_create(_00: f32, _10: f32, _20: f32,
					 _01: f32, _11: f32, _21: f32,
					 _02: f32, _12: f32, _22: f32): mat3_t {
	let self: mat3_t = {};
	self.m = f32_array_create(9);
	self.m[0] = _00;
	self.m[3] = _10;
	self.m[6] = _20;
	self.m[1] = _01;
	self.m[4] = _11;
	self.m[7] = _21;
	self.m[2] = _02;
	self.m[5] = _12;
	self.m[8] = _22;
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
		math_cos(alpha), -math_sin(alpha), 0,
		math_sin(alpha), math_cos(alpha), 0,
		0, 0, 1
	);
}

function mat3_set_from4(self: mat3_t, m: mat4_t) {
	self.m[0] = m.m[0];
	self.m[1] = m.m[1];
	self.m[2] = m.m[2];
	self.m[3] = m.m[4];
	self.m[4] = m.m[5];
	self.m[5] = m.m[6];
	self.m[6] = m.m[8];
	self.m[7] = m.m[9];
	self.m[8] = m.m[10];
}

function mat3_multmat(self: mat3_t, m: mat3_t): mat3_t {
	return mat3_create(
		self.m[0] * m.m[0] + self.m[3] * m.m[1] + self.m[6] * m.m[2],
		self.m[0] * m.m[3] + self.m[3] * m.m[4] + self.m[6] * m.m[5],
		self.m[0] * m.m[6] + self.m[3] * m.m[7] + self.m[6] * m.m[8],
		self.m[1] * m.m[0] + self.m[4] * m.m[1] + self.m[7] * m.m[2],
		self.m[1] * m.m[3] + self.m[4] * m.m[4] + self.m[7] * m.m[5],
		self.m[1] * m.m[6] + self.m[4] * m.m[7] + self.m[7] * m.m[8],
		self.m[2] * m.m[0] + self.m[5] * m.m[1] + self.m[8] * m.m[2],
		self.m[2] * m.m[3] + self.m[5] * m.m[4] + self.m[8] * m.m[5],
		self.m[2] * m.m[6] + self.m[5] * m.m[7] + self.m[8] * m.m[8]
	);
}
