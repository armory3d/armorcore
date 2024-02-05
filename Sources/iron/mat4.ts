/// <reference path='./vec4.ts'/>

class mat4_t {
	buffer = new Float32Array(16);

	get _00(): f32 { return this.buffer[0]; } set _00(f: f32) { this.buffer[0] = f; }
	get _01(): f32 { return this.buffer[1]; } set _01(f: f32) { this.buffer[1] = f; }
	get _02(): f32 { return this.buffer[2]; } set _02(f: f32) { this.buffer[2] = f; }
	get _03(): f32 { return this.buffer[3]; } set _03(f: f32) { this.buffer[3] = f; }
	get _10(): f32 { return this.buffer[4]; } set _10(f: f32) { this.buffer[4] = f; }
	get _11(): f32 { return this.buffer[5]; } set _11(f: f32) { this.buffer[5] = f; }
	get _12(): f32 { return this.buffer[6]; } set _12(f: f32) { this.buffer[6] = f; }
	get _13(): f32 { return this.buffer[7]; } set _13(f: f32) { this.buffer[7] = f; }
	get _20(): f32 { return this.buffer[8]; } set _20(f: f32) { this.buffer[8] = f; }
	get _21(): f32 { return this.buffer[9]; } set _21(f: f32) { this.buffer[9] = f; }
	get _22(): f32 { return this.buffer[10]; } set _22(f: f32) { this.buffer[10] = f; }
	get _23(): f32 { return this.buffer[11]; } set _23(f: f32) { this.buffer[11] = f; }
	get _30(): f32 { return this.buffer[12]; } set _30(f: f32) { this.buffer[12] = f; }
	get _31(): f32 { return this.buffer[13]; } set _31(f: f32) { this.buffer[13] = f; }
	get _32(): f32 { return this.buffer[14]; } set _32(f: f32) { this.buffer[14] = f; }
	get _33(): f32 { return this.buffer[15]; } set _33(f: f32) { this.buffer[15] = f; }
}

let _mat4_vec = vec4_create();
let _mat4_mat = mat4_identity();

function mat4_create(_00: f32, _10: f32, _20: f32, _30: f32,
					 _01: f32, _11: f32, _21: f32, _31: f32,
					 _02: f32, _12: f32, _22: f32, _32: f32,
					 _03: f32, _13: f32, _23: f32, _33: f32): mat4_t {
	let self = new mat4_t();
	self._00 = _00; self._10 = _10; self._20 = _20; self._30 = _30;
	self._01 = _01; self._11 = _11; self._21 = _21; self._31 = _31;
	self._02 = _02; self._12 = _12; self._22 = _22; self._32 = _32;
	self._03 = _03; self._13 = _13; self._23 = _23; self._33 = _33;
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

function mat4_from_f32_array(a: Float32Array, offset = 0): mat4_t {
	return mat4_create(
		a[0 + offset], a[1 + offset], a[2 + offset], a[3 + offset],
		a[4 + offset], a[5 + offset], a[6 + offset], a[7 + offset],
		a[8 + offset], a[9 + offset], a[10 + offset], a[11 + offset],
		a[12 + offset], a[13 + offset], a[14 + offset], a[15 + offset]
	);
}

function mat4_persp(fov_y: f32, aspect: f32, zn: f32, zf: f32): mat4_t {
	let uh = 1.0 / Math.tan(fov_y / 2);
	let uw = uh / aspect;
	return mat4_create(
		uw, 0, 0, 0,
		0, uh, 0, 0,
		0, 0, (zf + zn) / (zn - zf), 2 * zf * zn / (zn - zf),
		0, 0, -1, 0
	);
}

function mat4_ortho(left: f32, right: f32, bottom: f32, top: f32, near: f32, far: f32): mat4_t {
	let rl = right - left;
	let tb = top - bottom;
	let fn = far - near;
	let tx = -(right + left) / (rl);
	let ty = -(top + bottom) / (tb);
	let tz = -(far + near) / (fn);
	return mat4_create(
		2 / rl,	0,		0,		 tx,
		0,		2 / tb,	0,		 ty,
		0,		0,		-2 / fn, tz,
		0,		0,		0,		 1
	);
}

function mat4_rot_z(alpha: f32): mat4_t {
	let ca = Math.cos(alpha);
	let sa = Math.sin(alpha);
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

function mat4_decompose(self: mat4_t, loc: vec4_t, quat: quat_t, scale: vec4_t): mat4_t {
	loc.x = self._30; loc.y = self._31; loc.z = self._32;
	scale.x = vec4_len(vec4_set(_mat4_vec, self._00, self._01, self._02));
	scale.y = vec4_len(vec4_set(_mat4_vec, self._10, self._11, self._12));
	scale.z = vec4_len(vec4_set(_mat4_vec, self._20, self._21, self._22));
	if (mat4_determinant(self) < 0.0) scale.x = -scale.x;
	let invs = 1.0 / scale.x; // Scale the rotation part
	_mat4_mat._00 = self._00 * invs;
	_mat4_mat._01 = self._01 * invs;
	_mat4_mat._02 = self._02 * invs;
	invs = 1.0 / scale.y;
	_mat4_mat._10 = self._10 * invs;
	_mat4_mat._11 = self._11 * invs;
	_mat4_mat._12 = self._12 * invs;
	invs = 1.0 / scale.z;
	_mat4_mat._20 = self._20 * invs;
	_mat4_mat._21 = self._21 * invs;
	_mat4_mat._22 = self._22 * invs;
	quat_from_rot_mat(quat, _mat4_mat);
	return self;
}

function mat4_set_loc(self: mat4_t, v: vec4_t): mat4_t {
	self._30 = v.x;
	self._31 = v.y;
	self._32 = v.z;
	return self;
}

function mat4_from_quat(self: mat4_t, q: quat_t): mat4_t {
	let x = q.x; let y = q.y; let z = q.z; let w = q.w;
	let x2 = x + x; let y2 = y + y; let z2 = z + z;
	let xx = x * x2; let xy = x * y2; let xz = x * z2;
	let yy = y * y2; let yz = y * z2; let zz = z * z2;
	let wx = w * x2; let wy = w * y2; let wz = w * z2;

	self._00 = 1.0 - (yy + zz);
	self._10 = xy - wz;
	self._20 = xz + wy;

	self._01 = xy + wz;
	self._11 = 1.0 - (xx + zz);
	self._21 = yz - wx;

	self._02 = xz - wy;
	self._12 = yz + wx;
	self._22 = 1.0 - (xx + yy);

	self._03 = 0.0;
	self._13 = 0.0;
	self._23 = 0.0;
	self._30 = 0.0;
	self._31 = 0.0;
	self._32 = 0.0;
	self._33 = 1.0;

	return self;
}

function mat4_set_identity(self: mat4_t): mat4_t {
	self._00 = 1.0; self._01 = 0.0; self._02 = 0.0; self._03 = 0.0;
	self._10 = 0.0; self._11 = 1.0; self._12 = 0.0; self._13 = 0.0;
	self._20 = 0.0; self._21 = 0.0; self._22 = 1.0; self._23 = 0.0;
	self._30 = 0.0; self._31 = 0.0; self._32 = 0.0; self._33 = 1.0;
	return self;
}

function mat4_init_translate(self: mat4_t, x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0): mat4_t {
	self._00 = 1.0; self._01 = 0.0; self._02 = 0.0; self._03 = 0.0;
	self._10 = 0.0; self._11 = 1.0; self._12 = 0.0; self._13 = 0.0;
	self._20 = 0.0; self._21 = 0.0; self._22 = 1.0; self._23 = 0.0;
	self._30 = x;   self._31 = y;   self._32 = z;   self._33 = 1.0;
	return self;
}

function mat4_translate(self: mat4_t, x: f32, y: f32, z: f32): mat4_t {
	self._00 += x * self._03; self._01 += y * self._03; self._02 += z * self._03;
	self._10 += x * self._13; self._11 += y * self._13; self._12 += z * self._13;
	self._20 += x * self._23; self._21 += y * self._23; self._22 += z * self._23;
	self._30 += x * self._33; self._31 += y * self._33; self._32 += z * self._33;
	return self;
}

function mat4_scale(self: mat4_t, v: vec4_t): mat4_t {
	let x = v.x; let y = v.y; let z = v.z;
	self._00 *= x;
	self._01 *= x;
	self._02 *= x;
	self._03 *= x;
	self._10 *= y;
	self._11 *= y;
	self._12 *= y;
	self._13 *= y;
	self._20 *= z;
	self._21 *= z;
	self._22 *= z;
	self._23 *= z;
	return self;
}

function mat4_mult_mats3x4(self: mat4_t, a: mat4_t, b: mat4_t): mat4_t {
	let a00 = a._00; let a01 = a._01; let a02 = a._02; let a03 = a._03;
	let a10 = a._10; let a11 = a._11; let a12 = a._12; let a13 = a._13;
	let a20 = a._20; let a21 = a._21; let a22 = a._22; let a23 = a._23;
	let a30 = a._30; let a31 = a._31; let a32 = a._32; let a33 = a._33;

	let b0 = b._00; let b1 = b._10; let b2 = b._20; let b3 = b._30;
	self._00 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._10 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._20 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._30 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b._01; b1 = b._11; b2 = b._21; b3 = b._31;
	self._01 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._11 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._21 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._31 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b._02; b1 = b._12; b2 = b._22; b3 = b._32;
	self._02 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._12 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._22 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._32 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	self._03 = 0;
	self._13 = 0;
	self._23 = 0;
	self._33 = 1;
	return self;
}

function mat4_mult_mats(self: mat4_t, b: mat4_t, a: mat4_t): mat4_t {
	let a00 = a._00; let a01 = a._01; let a02 = a._02; let a03 = a._03;
	let a10 = a._10; let a11 = a._11; let a12 = a._12; let a13 = a._13;
	let a20 = a._20; let a21 = a._21; let a22 = a._22; let a23 = a._23;
	let a30 = a._30; let a31 = a._31; let a32 = a._32; let a33 = a._33;

	let b0 = b._00; let b1 = b._10; let b2 = b._20; let b3 = b._30;
	self._00 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._10 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._20 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._30 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b._01; b1 = b._11; b2 = b._21; b3 = b._31;
	self._01 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._11 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._21 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._31 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b._02; b1 = b._12; b2 = b._22; b3 = b._32;
	self._02 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._12 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._22 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._32 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = b._03; b1 = b._13; b2 = b._23; b3 = b._33;
	self._03 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._13 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._23 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._33 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	return self;
}

function mat4_mult_mat(self: mat4_t, m: mat4_t): mat4_t {
	let a00 = self._00; let a01 = self._01; let a02 = self._02; let a03 = self._03;
	let a10 = self._10; let a11 = self._11; let a12 = self._12; let a13 = self._13;
	let a20 = self._20; let a21 = self._21; let a22 = self._22; let a23 = self._23;
	let a30 = self._30; let a31 = self._31; let a32 = self._32; let a33 = self._33;

	let b0 = m._00; let b1 = m._10; let b2 = m._20; let b3 = m._30;
	self._00 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._10 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._20 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._30 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = m._01; b1 = m._11; b2 = m._21; b3 = m._31;
	self._01 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._11 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._21 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._31 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = m._02; b1 = m._12; b2 = m._22; b3 = m._32;
	self._02 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._12 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._22 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._32 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	b0 = m._03; b1 = m._13; b2 = m._23; b3 = m._33;
	self._03 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
	self._13 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
	self._23 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
	self._33 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

	return self;
}

function mat4_get_inv(self: mat4_t, m: mat4_t): mat4_t {
	let a00 = m._00; let a01 = m._01; let a02 = m._02; let a03 = m._03;
	let a10 = m._10; let a11 = m._11; let a12 = m._12; let a13 = m._13;
	let a20 = m._20; let a21 = m._21; let a22 = m._22; let a23 = m._23;
	let a30 = m._30; let a31 = m._31; let a32 = m._32; let a33 = m._33;
	let b00 = a00 * a11 - a01 * a10;
	let b01 = a00 * a12 - a02 * a10;
	let b02 = a00 * a13 - a03 * a10;
	let b03 = a01 * a12 - a02 * a11;
	let b04 = a01 * a13 - a03 * a11;
	let b05 = a02 * a13 - a03 * a12;
	let b06 = a20 * a31 - a21 * a30;
	let b07 = a20 * a32 - a22 * a30;
	let b08 = a20 * a33 - a23 * a30;
	let b09 = a21 * a32 - a22 * a31;
	let b10 = a21 * a33 - a23 * a31;
	let b11 = a22 * a33 - a23 * a32;

	let det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	if (det == 0.0) return mat4_set_identity(self);
	det = 1.0 / det;

	self._00 = (a11 * b11 - a12 * b10 + a13 * b09) * det;
	self._01 = (a02 * b10 - a01 * b11 - a03 * b09) * det;
	self._02 = (a31 * b05 - a32 * b04 + a33 * b03) * det;
	self._03 = (a22 * b04 - a21 * b05 - a23 * b03) * det;
	self._10 = (a12 * b08 - a10 * b11 - a13 * b07) * det;
	self._11 = (a00 * b11 - a02 * b08 + a03 * b07) * det;
	self._12 = (a32 * b02 - a30 * b05 - a33 * b01) * det;
	self._13 = (a20 * b05 - a22 * b02 + a23 * b01) * det;
	self._20 = (a10 * b10 - a11 * b08 + a13 * b06) * det;
	self._21 = (a01 * b08 - a00 * b10 - a03 * b06) * det;
	self._22 = (a30 * b04 - a31 * b02 + a33 * b00) * det;
	self._23 = (a21 * b02 - a20 * b04 - a23 * b00) * det;
	self._30 = (a11 * b07 - a10 * b09 - a12 * b06) * det;
	self._31 = (a00 * b09 - a01 * b07 + a02 * b06) * det;
	self._32 = (a31 * b01 - a30 * b03 - a32 * b00) * det;
	self._33 = (a20 * b03 - a21 * b01 + a22 * b00) * det;

	return self;
}

function mat4_transpose(self: mat4_t): mat4_t {
	let f = self._01; self._01 = self._10; self._10 = f;
	f = self._02; self._02 = self._20; self._20 = f;
	f = self._03; self._03 = self._30; self._30 = f;
	f = self._12; self._12 = self._21; self._21 = f;
	f = self._13; self._13 = self._31; self._31 = f;
	f = self._23; self._23 = self._32; self._32 = f;
	return self;
}

function mat4_transpose3x3(self: mat4_t): mat4_t {
	let f = self._01; self._01 = self._10; self._10 = f;
	f = self._02; self._02 = self._20; self._20 = f;
	f = self._12; self._12 = self._21; self._21 = f;
	return self;
}

function mat4_clone(self: mat4_t): mat4_t {
	return mat4_create(
		self._00, self._10, self._20, self._30,
		self._01, self._11, self._21, self._31,
		self._02, self._12, self._22, self._32,
		self._03, self._13, self._23, self._33
	);
}

function mat4_set_from_f32_array(self: mat4_t, a: Float32Array, offset = 0): mat4_t {
	self._00 = a[0 + offset]; self._10 = a[1 + offset]; self._20 = a[2 + offset]; self._30 = a[3 + offset];
	self._01 = a[4 + offset]; self._11 = a[5 + offset]; self._21 = a[6 + offset]; self._31 = a[7 + offset];
	self._02 = a[8 + offset]; self._12 = a[9 + offset]; self._22 = a[10 + offset]; self._32 = a[11 + offset];
	self._03 = a[12 + offset]; self._13 = a[13 + offset]; self._23 = a[14 + offset]; self._33 = a[15 + offset];
	return self;
}

function mat4_set_from(self: mat4_t, m: mat4_t): mat4_t {
	self._00 = m._00; self._01 = m._01; self._02 = m._02; self._03 = m._03;
	self._10 = m._10; self._11 = m._11; self._12 = m._12; self._13 = m._13;
	self._20 = m._20; self._21 = m._21; self._22 = m._22; self._23 = m._23;
	self._30 = m._30; self._31 = m._31; self._32 = m._32; self._33 = m._33;
	return self;
}

function mat4_get_loc(self: mat4_t): vec4_t {
	return vec4_create(self._30, self._31, self._32, self._33);
}

function mat4_get_scale(self: mat4_t): vec4_t {
	return vec4_create(
		Math.sqrt(self._00 * self._00 + self._10 * self._10 + self._20 * self._20),
		Math.sqrt(self._01 * self._01 + self._11 * self._11 + self._21 * self._21),
		Math.sqrt(self._02 * self._02 + self._12 * self._12 + self._22 * self._22)
	);
}

function mat4_mult(self: mat4_t, s: f32): mat4_t {
	self._00 *= s; self._10 *= s; self._20 *= s; self._30 *= s;
	self._01 *= s; self._11 *= s; self._21 *= s; self._31 *= s;
	self._02 *= s; self._12 *= s; self._22 *= s; self._32 *= s;
	self._03 *= s; self._13 *= s; self._23 *= s; self._33 *= s;
	return self;
}

function mat4_to_rot(self: mat4_t): mat4_t {
	let scale = 1.0 / vec4_len(vec4_set(_mat4_vec, self._00, self._01, self._02));
	self._00 = self._00 * scale;
	self._01 = self._01 * scale;
	self._02 = self._02 * scale;
	scale = 1.0 / vec4_len(vec4_set(_mat4_vec, self._10, self._11, self._12));
	self._10 = self._10 * scale;
	self._11 = self._11 * scale;
	self._12 = self._12 * scale;
	scale = 1.0 / vec4_len(vec4_set(_mat4_vec, self._20, self._21, self._22));
	self._20 = self._20 * scale;
	self._21 = self._21 * scale;
	self._22 = self._22 * scale;
	self._03 = 0.0;
	self._13 = 0.0;
	self._23 = 0.0;
	self._30 = 0.0;
	self._31 = 0.0;
	self._32 = 0.0;
	self._33 = 1.0;
	return self;
}

function mat4_right(self: mat4_t): vec4_t {
	return vec4_create(self._00, self._01, self._02);
}

function mat4_look(self: mat4_t): vec4_t {
	return vec4_create(self._10, self._11, self._12);
}

function mat4_up(self: mat4_t): vec4_t {
	return vec4_create(self._20, self._21, self._22);
}

function mat4_to_f32_array(self: mat4_t): Float32Array {
	let array = new Float32Array(16);
	array[0] = self._00;
	array[1] = self._10;
	array[2] = self._20;
	array[3] = self._30;
	array[4] = self._01;
	array[5] = self._11;
	array[6] = self._21;
	array[7] = self._31;
	array[8] = self._02;
	array[9] = self._12;
	array[10] = self._22;
	array[11] = self._32;
	array[12] = self._03;
	array[13] = self._13;
	array[14] = self._23;
	array[15] = self._33;
	return array;
}

function mat4_cofactor(self: mat4_t,
			m0: f32, m1: f32, m2: f32,
			m3: f32, m4: f32, m5: f32,
			m6: f32, m7: f32, m8: f32): f32 {
	return m0 * ( m4 * m8 - m5 * m7 ) - m1 * ( m3 * m8 - m5 * m6 ) + m2 * ( m3 * m7 - m4 * m6 );
}

function mat4_determinant(self: mat4_t): f32 {
	let c00 = mat4_cofactor(self, self._11, self._21, self._31, self._12, self._22, self._32, self._13, self._23, self._33);
	let c01 = mat4_cofactor(self, self._10, self._20, self._30, self._12, self._22, self._32, self._13, self._23, self._33);
	let c02 = mat4_cofactor(self, self._10, self._20, self._30, self._11, self._21, self._31, self._13, self._23, self._33);
	let c03 = mat4_cofactor(self, self._10, self._20, self._30, self._11, self._21, self._31, self._12, self._22, self._32);
	return self._00 * c00 - self._01 * c01 + self._02 * c02 - self._03 * c03;
}
