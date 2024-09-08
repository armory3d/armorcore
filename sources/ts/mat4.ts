
// _00 = [0]
// _01 = [1]
// _02 = [2]
// _03 = [3]
// _10 = [4]
// _11 = [5]
// _12 = [6]
// _13 = [7]
// _20 = [8]
// _21 = [9]
// _22 = [10]
// _23 = [11]
// _30 = [12]
// _31 = [13]
// _32 = [14]
// _33 = [15]

type mat4_t = {
	m?: f32_array_t;
};

type mat4_decomposed_t = {
	loc: vec4_t;
	rot: quat_t;
	scl: vec4_t;
}

let _mat4_vec: vec4_t = vec4_create();
let _mat4_mat: mat4_t = mat4_identity();

function mat4_create(_00: f32, _10: f32, _20: f32, _30: f32,
					 _01: f32, _11: f32, _21: f32, _31: f32,
					 _02: f32, _12: f32, _22: f32, _32: f32,
					 _03: f32, _13: f32, _23: f32, _33: f32): mat4_t {
	let self: mat4_t = {};
	self.m = f32_array_create(16);
	self.m[0] = _00;
	self.m[4] = _10;
	self.m[8] = _20;
	self.m[12] = _30;
	self.m[1] = _01;
	self.m[5] = _11;
	self.m[9] = _21;
	self.m[13] = _31;
	self.m[2] = _02;
	self.m[6] = _12;
	self.m[10] = _22;
	self.m[14] = _32;
	self.m[3] = _03;
	self.m[7] = _13;
	self.m[11] = _23;
	self.m[15] = _33;
	return self;
}

function mat4_identity(): mat4_t {
	return mat4_create(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
}

function mat4_from_f32_array(a: f32_array_t, offset: i32 = 0): mat4_t {
	return mat4_create(
		a[0 + offset], a[1 + offset], a[2 + offset], a[3 + offset],
		a[4 + offset], a[5 + offset], a[6 + offset], a[7 + offset],
		a[8 + offset], a[9 + offset], a[10 + offset], a[11 + offset],
		a[12 + offset], a[13 + offset], a[14 + offset], a[15 + offset]
	);
}

function mat4_persp(fov_y: f32, aspect: f32, zn: f32, zf: f32): mat4_t {
	let uh: f32 = 1.0 / math_tan(fov_y / 2);
	let uw: f32 = uh / aspect;
	return mat4_create(
		uw, 0, 0, 0,
		0, uh, 0, 0,
		0, 0, (zf + zn) / (zn - zf), 2 * zf * zn / (zn - zf),
		0, 0, -1, 0
	);
}

function mat4_ortho(left: f32, right: f32, bottom: f32, top: f32, znear: f32, zfar: f32): mat4_t {
	let rl: f32 = right - left;
	let tb: f32 = top - bottom;
	let fn: f32 = zfar - znear;
	let tx: f32 = -(right + left) / (rl);
	let ty: f32 = -(top + bottom) / (tb);
	let tz: f32 = -(zfar + znear) / (fn);
	return mat4_create(
		2 / rl,	0,		0,		 tx,
		0,		2 / tb,	0,		 ty,
		0,		0,		-2 / fn, tz,
		0,		0,		0,		 1
	);
}

function mat4_rot_z(alpha: f32): mat4_t {
	let ca: f32 = math_cos(alpha);
	let sa: f32 = math_sin(alpha);
	return mat4_create(
		ca, -sa, 0, 0,
		sa,  ca, 0, 0,
		0,   0, 1, 0,
		0,   0, 0, 1
	);
}

function mat4_compose(self: mat4_t, loc: vec4_t, quat: quat_t, sc: vec4_t): mat4_t {
	mat4_from_quat(self, quat);
	mat4_scale(self, sc);
	mat4_set_loc(self, loc);
	return self;
}

function mat4_decompose(m: mat4_t): mat4_decomposed_t {
	let loc: vec4_t;
	let rot: quat_t = quat_create();
	let scl: vec4_t;
	loc.x = m.m[12];
	loc.y = m.m[13];
	loc.z = m.m[14];
	scl.x = vec4_len(vec4_new(m.m[0], m.m[1], m.m[2]));
	scl.y = vec4_len(vec4_new(m.m[4], m.m[5], m.m[6]));
	scl.z = vec4_len(vec4_new(m.m[8], m.m[9], m.m[10]));
	if (mat4_determinant(m) < 0.0) {
		scl.x = -scl.x;
	}
	let invs: f32 = 1.0 / scl.x; // Scale the rotation part
	_mat4_mat.m[0] = m.m[0] * invs;
	_mat4_mat.m[1] = m.m[1] * invs;
	_mat4_mat.m[2] = m.m[2] * invs;
	invs = 1.0 / scl.y;
	_mat4_mat.m[4] = m.m[4] * invs;
	_mat4_mat.m[5] = m.m[5] * invs;
	_mat4_mat.m[6] = m.m[6] * invs;
	invs = 1.0 / scl.z;
	_mat4_mat.m[8] = m.m[8] * invs;
	_mat4_mat.m[9] = m.m[9] * invs;
	_mat4_mat.m[10] = m.m[10] * invs;
	quat_from_rot_mat(rot, _mat4_mat);

	let dec: mat4_decomposed_t = { loc: loc, rot: rot, scl: scl };
	return dec;
}

function mat4_set_loc(self: mat4_t, v: vec4_t): mat4_t {
	self.m[12] = v.x;
	self.m[13] = v.y;
	self.m[14] = v.z;
	return self;
}

function mat4_from_quat(self: mat4_t, q: quat_t): mat4_t {
	let x: f32 = q.x;
	let y: f32 = q.y;
	let z: f32 = q.z;
	let w: f32 = q.w;
	let x2: f32 = x + x;
	let y2: f32 = y + y;
	let z2: f32 = z + z;
	let xx: f32 = x * x2;
	let xy: f32 = x * y2;
	let xz: f32 = x * z2;
	let yy: f32 = y * y2;
	let yz: f32 = y * z2;
	let zz: f32 = z * z2;
	let wx: f32 = w * x2;
	let wy: f32 = w * y2;
	let wz: f32 = w * z2;

	self.m[0] = 1.0 - (yy + zz);
	self.m[4] = xy - wz;
	self.m[8] = xz + wy;

	self.m[1] = xy + wz;
	self.m[5] = 1.0 - (xx + zz);
	self.m[9] = yz - wx;

	self.m[2] = xz - wy;
	self.m[6] = yz + wx;
	self.m[10] = 1.0 - (xx + yy);

	self.m[3] = 0.0;
	self.m[7] = 0.0;
	self.m[11] = 0.0;
	self.m[12] = 0.0;
	self.m[13] = 0.0;
	self.m[14] = 0.0;
	self.m[15] = 1.0;

	return self;
}

function mat4_set_identity(self: mat4_t): mat4_t {
	self.m[0] = 1.0;
	self.m[1] = 0.0;
	self.m[2] = 0.0;
	self.m[3] = 0.0;
	self.m[4] = 0.0;
	self.m[5] = 1.0;
	self.m[6] = 0.0;
	self.m[7] = 0.0;
	self.m[8] = 0.0;
	self.m[9] = 0.0;
	self.m[10] = 1.0;
	self.m[11] = 0.0;
	self.m[12] = 0.0;
	self.m[13] = 0.0;
	self.m[14] = 0.0;
	self.m[15] = 1.0;
	return self;
}

function mat4_init_translate(self: mat4_t, x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0): mat4_t {
	self.m[0] = 1.0;
	self.m[1] = 0.0;
	self.m[2] = 0.0;
	self.m[3] = 0.0;
	self.m[4] = 0.0;
	self.m[5] = 1.0;
	self.m[6] = 0.0;
	self.m[7] = 0.0;
	self.m[8] = 0.0;
	self.m[9] = 0.0;
	self.m[10] = 1.0;
	self.m[11] = 0.0;
	self.m[12] = x;
	self.m[13] = y;
	self.m[14] = z;
	self.m[15] = 1.0;
	return self;
}

function mat4_translate(self: mat4_t, x: f32, y: f32, z: f32): mat4_t {
	self.m[0] += x * self.m[3];
	self.m[1] += y * self.m[3];
	self.m[2] += z * self.m[3];
	self.m[4] += x * self.m[7];
	self.m[5] += y * self.m[7];
	self.m[6] += z * self.m[7];
	self.m[8] += x * self.m[11];
	self.m[9] += y * self.m[11];
	self.m[10] += z * self.m[11];
	self.m[12] += x * self.m[15];
	self.m[13] += y * self.m[15];
	self.m[14] += z * self.m[15];
	return self;
}

function mat4_scale(self: mat4_t, v: vec4_t): mat4_t {
	let x: f32 = v.x;
	let y: f32 = v.y;
	let z: f32 = v.z;
	self.m[0] *= x;
	self.m[1] *= x;
	self.m[2] *= x;
	self.m[3] *= x;
	self.m[4] *= y;
	self.m[5] *= y;
	self.m[6] *= y;
	self.m[7] *= y;
	self.m[8] *= z;
	self.m[9] *= z;
	self.m[10] *= z;
	self.m[11] *= z;
	return self;
}

function mat4_mult_mats3x4(self: mat4_t, a: mat4_t, b: mat4_t): mat4_t {
	let a00: f32 = a.m[0];
	let a01: f32 = a.m[1];
	let a02: f32 = a.m[2];
	let a03: f32 = a.m[3];
	let a10: f32 = a.m[4];
	let a11: f32 = a.m[5];
	let a12: f32 = a.m[6];
	let a13: f32 = a.m[7];
	let a20: f32 = a.m[8];
	let a21: f32 = a.m[9];
	let a22: f32 = a.m[10];
	let a23: f32 = a.m[11];
	let a30: f32 = a.m[12];
	let a31: f32 = a.m[13];
	let a32: f32 = a.m[14];
	let a33: f32 = a.m[15];

	let b0: f32 = b.m[0];
	let b1: f32 = b.m[4];
	let b2: f32 = b.m[8];
	let b3: f32 = b.m[12];
	self.m[0] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[4] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[8] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[12] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[1];
	b1 = b.m[5];
	b2 = b.m[9];
	b3 = b.m[13];
	self.m[1] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[5] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[9] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[13] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[2];
	b1 = b.m[6];
	b2 = b.m[10];
	b3 = b.m[14];
	self.m[2] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[6] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[10] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[14] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	self.m[3] = 0;
	self.m[7] = 0;
	self.m[11] = 0;
	self.m[15] = 1;
	return self;
}

function mat4_mult_mats(self: mat4_t, b: mat4_t, a: mat4_t): mat4_t {
	let a00: f32 = a.m[0];
	let a01: f32 = a.m[1];
	let a02: f32 = a.m[2];
	let a03: f32 = a.m[3];
	let a10: f32 = a.m[4];
	let a11: f32 = a.m[5];
	let a12: f32 = a.m[6];
	let a13: f32 = a.m[7];
	let a20: f32 = a.m[8];
	let a21: f32 = a.m[9];
	let a22: f32 = a.m[10];
	let a23: f32 = a.m[11];
	let a30: f32 = a.m[12];
	let a31: f32 = a.m[13];
	let a32: f32 = a.m[14];
	let a33: f32 = a.m[15];

	let b0: f32 = b.m[0];
	let b1: f32 = b.m[4];
	let b2: f32 = b.m[8];
	let b3: f32 = b.m[12];
	self.m[0] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[4] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[8] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[12] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[1];
	b1 = b.m[5];
	b2 = b.m[9];
	b3 = b.m[13];
	self.m[1] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[5] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[9] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[13] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[2];
	b1 = b.m[6];
	b2 = b.m[10];
	b3 = b.m[14];
	self.m[2] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[6] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[10] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[14] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b.m[3];
	b1 = b.m[7];
	b2 = b.m[11];
	b3 = b.m[15];
	self.m[3] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[7] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[11] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[15] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	return self;
}

function mat4_mult_mat(self: mat4_t, m: mat4_t): mat4_t {
	let a00: f32 = self.m[0];
	let a01: f32 = self.m[1];
	let a02: f32 = self.m[2];
	let a03: f32 = self.m[3];
	let a10: f32 = self.m[4];
	let a11: f32 = self.m[5];
	let a12: f32 = self.m[6];
	let a13: f32 = self.m[7];
	let a20: f32 = self.m[8];
	let a21: f32 = self.m[9];
	let a22: f32 = self.m[10];
	let a23: f32 = self.m[11];
	let a30: f32 = self.m[12];
	let a31: f32 = self.m[13];
	let a32: f32 = self.m[14];
	let a33: f32 = self.m[15];

	let b0: f32 = m.m[0];
	let b1: f32 = m.m[4];
	let b2: f32 = m.m[8];
	let b3: f32 = m.m[12];
	self.m[0] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[4] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[8] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[12] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = m.m[1];
	b1 = m.m[5];
	b2 = m.m[9];
	b3 = m.m[13];
	self.m[1] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[5] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[9] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[13] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = m.m[2];
	b1 = m.m[6];
	b2 = m.m[10];
	b3 = m.m[14];
	self.m[2] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[6] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[10] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[14] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = m.m[3];
	b1 = m.m[7];
	b2 = m.m[11];
	b3 = m.m[15];
	self.m[3] = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self.m[7] = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self.m[11] = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self.m[15] = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	return self;
}

function mat4_get_inv(self: mat4_t, m: mat4_t): mat4_t {
	let a00: f32 = m.m[0];
	let a01: f32 = m.m[1];
	let a02: f32 = m.m[2];
	let a03: f32 = m.m[3];
	let a10: f32 = m.m[4];
	let a11: f32 = m.m[5];
	let a12: f32 = m.m[6];
	let a13: f32 = m.m[7];
	let a20: f32 = m.m[8];
	let a21: f32 = m.m[9];
	let a22: f32 = m.m[10];
	let a23: f32 = m.m[11];
	let a30: f32 = m.m[12];
	let a31: f32 = m.m[13];
	let a32: f32 = m.m[14];
	let a33: f32 = m.m[15];
	let b00: f32 = a00 * a11 - a01 * a10;
	let b01: f32 = a00 * a12 - a02 * a10;
	let b02: f32 = a00 * a13 - a03 * a10;
	let b03: f32 = a01 * a12 - a02 * a11;
	let b04: f32 = a01 * a13 - a03 * a11;
	let b05: f32 = a02 * a13 - a03 * a12;
	let b06: f32 = a20 * a31 - a21 * a30;
	let b07: f32 = a20 * a32 - a22 * a30;
	let b08: f32 = a20 * a33 - a23 * a30;
	let b09: f32 = a21 * a32 - a22 * a31;
	let b10: f32 = a21 * a33 - a23 * a31;
	let b11: f32 = a22 * a33 - a23 * a32;

	let det: f32 = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	if (det == 0.0) {
		return mat4_set_identity(self);
	}
	det = 1.0 / det;

	self.m[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
	self.m[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
	self.m[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
	self.m[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
	self.m[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
	self.m[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
	self.m[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
	self.m[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
	self.m[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
	self.m[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
	self.m[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
	self.m[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
	self.m[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
	self.m[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
	self.m[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
	self.m[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;

	return self;
}

function mat4_transpose(self: mat4_t): mat4_t {
	let f: f32 = self.m[1];
	self.m[1] = self.m[4];
	self.m[4] = f;

	f = self.m[2];
	self.m[2] = self.m[8];
	self.m[8] = f;

	f = self.m[3];
	self.m[3] = self.m[12];
	self.m[12] = f;

	f = self.m[6];
	self.m[6] = self.m[9];
	self.m[9] = f;

	f = self.m[7];
	self.m[7] = self.m[13];
	self.m[13] = f;

	f = self.m[11];
	self.m[11] = self.m[14];
	self.m[14] = f;

	return self;
}

function mat4_transpose3x3(self: mat4_t): mat4_t {
	let f: f32 = self.m[1];
	self.m[1] = self.m[4];
	self.m[4] = f;

	f = self.m[2];
	self.m[2] = self.m[8];
	self.m[8] = f;

	f = self.m[6];
	self.m[6] = self.m[9];
	self.m[9] = f;

	return self;
}

function mat4_clone(self: mat4_t): mat4_t {
	return mat4_create(
		self.m[0], self.m[4], self.m[8], self.m[12],
		self.m[1], self.m[5], self.m[9], self.m[13],
		self.m[2], self.m[6], self.m[10], self.m[14],
		self.m[3], self.m[7], self.m[11], self.m[15]
	);
}

function mat4_set_from_f32_array(self: mat4_t, a: f32_array_t, offset: i32 = 0): mat4_t {
	self.m[0] = a[0 + offset];
	self.m[4] = a[1 + offset];
	self.m[8] = a[2 + offset];
	self.m[12] = a[3 + offset];
	self.m[1] = a[4 + offset];
	self.m[5] = a[5 + offset];
	self.m[9] = a[6 + offset];
	self.m[13] = a[7 + offset];
	self.m[2] = a[8 + offset];
	self.m[6] = a[9 + offset];
	self.m[10] = a[10 + offset];
	self.m[14] = a[11 + offset];
	self.m[3] = a[12 + offset];
	self.m[7] = a[13 + offset];
	self.m[11] = a[14 + offset];
	self.m[15] = a[15 + offset];
	return self;
}

function mat4_set_from(self: mat4_t, m: mat4_t): mat4_t {
	self.m[0] = m.m[0];
	self.m[1] = m.m[1];
	self.m[2] = m.m[2];
	self.m[3] = m.m[3];
	self.m[4] = m.m[4];
	self.m[5] = m.m[5];
	self.m[6] = m.m[6];
	self.m[7] = m.m[7];
	self.m[8] = m.m[8];
	self.m[9] = m.m[9];
	self.m[10] = m.m[10];
	self.m[11] = m.m[11];
	self.m[12] = m.m[12];
	self.m[13] = m.m[13];
	self.m[14] = m.m[14];
	self.m[15] = m.m[15];
	return self;
}

function mat4_get_loc(self: mat4_t): vec4_t {
	return vec4_create(self.m[12], self.m[13], self.m[14], self.m[15]);
}

function mat4_get_scale(self: mat4_t): vec4_t {
	return vec4_create(
		math_sqrt(self.m[0] * self.m[0] + self.m[4] * self.m[4] + self.m[8] * self.m[8]),
		math_sqrt(self.m[1] * self.m[1] + self.m[5] * self.m[5] + self.m[9] * self.m[9]),
		math_sqrt(self.m[2] * self.m[2] + self.m[6] * self.m[6] + self.m[10] * self.m[10])
	);
}

function mat4_mult(self: mat4_t, s: f32): mat4_t {
	self.m[0] *= s;
	self.m[4] *= s;
	self.m[8] *= s;
	self.m[12] *= s;
	self.m[1] *= s;
	self.m[5] *= s;
	self.m[9] *= s;
	self.m[13] *= s;
	self.m[2] *= s;
	self.m[6] *= s;
	self.m[10] *= s;
	self.m[14] *= s;
	self.m[3] *= s;
	self.m[7] *= s;
	self.m[11] *= s;
	self.m[15] *= s;
	return self;
}

function mat4_to_rot(self: mat4_t): mat4_t {
	let scale: f32 = 1.0 / vec4_len(vec4_new(self.m[0], self.m[1], self.m[2]));
	self.m[0] = self.m[0] * scale;
	self.m[1] = self.m[1] * scale;
	self.m[2] = self.m[2] * scale;
	scale = 1.0 / vec4_len(vec4_new(self.m[4], self.m[5], self.m[6]));
	self.m[4] = self.m[4] * scale;
	self.m[5] = self.m[5] * scale;
	self.m[6] = self.m[6] * scale;
	scale = 1.0 / vec4_len(vec4_new(self.m[8], self.m[9], self.m[10]));
	self.m[8] = self.m[8] * scale;
	self.m[9] = self.m[9] * scale;
	self.m[10] = self.m[10] * scale;
	self.m[3] = 0.0;
	self.m[7] = 0.0;
	self.m[11] = 0.0;
	self.m[12] = 0.0;
	self.m[13] = 0.0;
	self.m[14] = 0.0;
	self.m[15] = 1.0;
	return self;
}

function mat4_right(self: mat4_t): vec4_t {
	return vec4_create(self.m[0], self.m[1], self.m[2]);
}

function mat4_look(self: mat4_t): vec4_t {
	return vec4_create(self.m[4], self.m[5], self.m[6]);
}

function mat4_up(self: mat4_t): vec4_t {
	return vec4_create(self.m[8], self.m[9], self.m[10]);
}

function mat4_to_f32_array(self: mat4_t): f32_array_t {
	let array: f32_array_t = f32_array_create(16);
	array[0] = self.m[0];
	array[1] = self.m[4];
	array[2] = self.m[8];
	array[3] = self.m[12];
	array[4] = self.m[1];
	array[5] = self.m[5];
	array[6] = self.m[9];
	array[7] = self.m[13];
	array[8] = self.m[2];
	array[9] = self.m[6];
	array[10] = self.m[10];
	array[11] = self.m[14];
	array[12] = self.m[3];
	array[13] = self.m[7];
	array[14] = self.m[11];
	array[15] = self.m[15];
	return array;
}

function mat4_cofactor(self: mat4_t,
					   m0: f32, m1: f32, m2: f32,
					   m3: f32, m4: f32, m5: f32,
					   m6: f32, m7: f32, m8: f32): f32 {
	return m0 * (m4 * m8 - m5 * m7) - m1 * (m3 * m8 - m5 * m6) + m2 * (m3 * m7 - m4 * m6);
}

function mat4_determinant(self: mat4_t): f32 {
	let c00: f32 = mat4_cofactor(self, self.m[5], self.m[9], self.m[13], self.m[6], self.m[10], self.m[14], self.m[7], self.m[11], self.m[15]);
	let c01: f32 = mat4_cofactor(self, self.m[4], self.m[8], self.m[12], self.m[6], self.m[10], self.m[14], self.m[7], self.m[11], self.m[15]);
	let c02: f32 = mat4_cofactor(self, self.m[4], self.m[8], self.m[12], self.m[5], self.m[9], self.m[13], self.m[7], self.m[11], self.m[15]);
	let c03: f32 = mat4_cofactor(self, self.m[4], self.m[8], self.m[12], self.m[5], self.m[9], self.m[13], self.m[6], self.m[10], self.m[14]);
	return self.m[0] * c00 - self.m[1] * c01 + self.m[2] * c02 - self.m[3] * c03;
}
