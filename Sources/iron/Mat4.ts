/// <reference path='./Vec4.ts'/>

class Mat4 {

	static identity = (): Mat4 => {
		return new Mat4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		);
	}

	static helpVec = new Vec4();
	static helpMat = Mat4.identity();

	constructor(_00: f32, _10: f32, _20: f32, _30: f32,
				_01: f32, _11: f32, _21: f32, _31: f32,
				_02: f32, _12: f32, _22: f32, _32: f32,
				_03: f32, _13: f32, _23: f32, _33: f32) {

		this._00 = _00; this._10 = _10; this._20 = _20; this._30 = _30;
		this._01 = _01; this._11 = _11; this._21 = _21; this._31 = _31;
		this._02 = _02; this._12 = _12; this._22 = _22; this._32 = _32;
		this._03 = _03; this._13 = _13; this._23 = _23; this._33 = _33;
	}

	compose = (loc: Vec4, quat: Quat, sc: Vec4): Mat4 => {
		this.fromQuat(quat);
		this.scale(sc);
		this.setLoc(loc);
		return this;
	}

	decompose = (loc: Vec4, quat: Quat, scale: Vec4): Mat4 => {
		loc.x = this._30; loc.y = this._31; loc.z = this._32;
		scale.x = Mat4.helpVec.set(this._00, this._01, this._02).length();
		scale.y = Mat4.helpVec.set(this._10, this._11, this._12).length();
		scale.z = Mat4.helpVec.set(this._20, this._21, this._22).length();
		if (this.determinant() < 0.0) scale.x = -scale.x;
		let invs = 1.0 / scale.x; // Scale the rotation part
		Mat4.helpMat._00 = this._00 * invs;
		Mat4.helpMat._01 = this._01 * invs;
		Mat4.helpMat._02 = this._02 * invs;
		invs = 1.0 / scale.y;
		Mat4.helpMat._10 = this._10 * invs;
		Mat4.helpMat._11 = this._11 * invs;
		Mat4.helpMat._12 = this._12 * invs;
		invs = 1.0 / scale.z;
		Mat4.helpMat._20 = this._20 * invs;
		Mat4.helpMat._21 = this._21 * invs;
		Mat4.helpMat._22 = this._22 * invs;
		quat.fromRotationMat(Mat4.helpMat);
		return this;
	}

	setLoc = (v: Vec4): Mat4 => {
		this._30 = v.x;
		this._31 = v.y;
		this._32 = v.z;
		return this;
	}

	fromQuat = (q: Quat): Mat4 => {
		let x = q.x; let y = q.y; let z = q.z; let w = q.w;
		let x2 = x + x; let y2 = y + y; let z2 = z + z;
		let xx = x * x2; let xy = x * y2; let xz = x * z2;
		let yy = y * y2; let yz = y * z2; let zz = z * z2;
		let wx = w * x2; let wy = w * y2; let wz = w * z2;

		this._00 = 1.0 - (yy + zz);
		this._10 = xy - wz;
		this._20 = xz + wy;

		this._01 = xy + wz;
		this._11 = 1.0 - (xx + zz);
		this._21 = yz - wx;

		this._02 = xz - wy;
		this._12 = yz + wx;
		this._22 = 1.0 - (xx + yy);

		this._03 = 0.0;
		this._13 = 0.0;
		this._23 = 0.0;
		this._30 = 0.0;
		this._31 = 0.0;
		this._32 = 0.0;
		this._33 = 1.0;

		return this;
	}

	static fromFloat32Array = (a: Float32Array, offset = 0): Mat4 => {
		return new Mat4(
			a[0 + offset], a[1 + offset], a[2 + offset], a[3 + offset],
			a[4 + offset], a[5 + offset], a[6 + offset], a[7 + offset],
			a[8 + offset], a[9 + offset], a[10 + offset], a[11 + offset],
			a[12 + offset], a[13 + offset], a[14 + offset], a[15 + offset]
		);
	}

	setIdentity = (): Mat4 => {
		this._00 = 1.0; this._01 = 0.0; this._02 = 0.0; this._03 = 0.0;
		this._10 = 0.0; this._11 = 1.0; this._12 = 0.0; this._13 = 0.0;
		this._20 = 0.0; this._21 = 0.0; this._22 = 1.0; this._23 = 0.0;
		this._30 = 0.0; this._31 = 0.0; this._32 = 0.0; this._33 = 1.0;
		return this;
	}

	initTranslate = (x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0): Mat4 => {
		this._00 = 1.0; this._01 = 0.0; this._02 = 0.0; this._03 = 0.0;
		this._10 = 0.0; this._11 = 1.0; this._12 = 0.0; this._13 = 0.0;
		this._20 = 0.0; this._21 = 0.0; this._22 = 1.0; this._23 = 0.0;
		this._30 = x;   this._31 = y;   this._32 = z;   this._33 = 1.0;
		return this;
	}

	translate = (x: f32, y: f32, z: f32): Mat4 => {
		this._00 += x * this._03; this._01 += y * this._03; this._02 += z * this._03;
		this._10 += x * this._13; this._11 += y * this._13; this._12 += z * this._13;
		this._20 += x * this._23; this._21 += y * this._23; this._22 += z * this._23;
		this._30 += x * this._33; this._31 += y * this._33; this._32 += z * this._33;
		return this;
	}

	scale = (v: Vec4): Mat4 => {
		let x = v.x; let y = v.y; let z = v.z;
		this._00 *= x;
		this._01 *= x;
		this._02 *= x;
		this._03 *= x;
		this._10 *= y;
		this._11 *= y;
		this._12 *= y;
		this._13 *= y;
		this._20 *= z;
		this._21 *= z;
		this._22 *= z;
		this._23 *= z;
		return this;
	}

	multmats3x4 = (a: Mat4, b: Mat4): Mat4 => {
		let a00 = a._00; let a01 = a._01; let a02 = a._02; let a03 = a._03;
		let a10 = a._10; let a11 = a._11; let a12 = a._12; let a13 = a._13;
		let a20 = a._20; let a21 = a._21; let a22 = a._22; let a23 = a._23;
		let a30 = a._30; let a31 = a._31; let a32 = a._32; let a33 = a._33;

		let b0 = b._00; let b1 = b._10; let b2 = b._20; let b3 = b._30;
		this._00 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._10 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._20 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._30 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = b._01; b1 = b._11; b2 = b._21; b3 = b._31;
		this._01 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._11 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._21 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._31 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = b._02; b1 = b._12; b2 = b._22; b3 = b._32;
		this._02 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._12 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._22 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._32 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		this._03 = 0;
		this._13 = 0;
		this._23 = 0;
		this._33 = 1;
		return this;
	}

	multmats = (b: Mat4, a: Mat4): Mat4 => {
		let a00 = a._00; let a01 = a._01; let a02 = a._02; let a03 = a._03;
		let a10 = a._10; let a11 = a._11; let a12 = a._12; let a13 = a._13;
		let a20 = a._20; let a21 = a._21; let a22 = a._22; let a23 = a._23;
		let a30 = a._30; let a31 = a._31; let a32 = a._32; let a33 = a._33;

		let b0 = b._00; let b1 = b._10; let b2 = b._20; let b3 = b._30;
		this._00 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._10 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._20 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._30 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = b._01; b1 = b._11; b2 = b._21; b3 = b._31;
		this._01 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._11 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._21 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._31 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = b._02; b1 = b._12; b2 = b._22; b3 = b._32;
		this._02 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._12 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._22 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._32 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = b._03; b1 = b._13; b2 = b._23; b3 = b._33;
		this._03 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._13 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._23 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._33 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		return this;
	}

	multmat = (m: Mat4): Mat4 => {
		let a00 = this._00; let a01 = this._01; let a02 = this._02; let a03 = this._03;
		let a10 = this._10; let a11 = this._11; let a12 = this._12; let a13 = this._13;
		let a20 = this._20; let a21 = this._21; let a22 = this._22; let a23 = this._23;
		let a30 = this._30; let a31 = this._31; let a32 = this._32; let a33 = this._33;

		let b0 = m._00; let b1 = m._10; let b2 = m._20; let b3 = m._30;
		this._00 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._10 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._20 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._30 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = m._01; b1 = m._11; b2 = m._21; b3 = m._31;
		this._01 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._11 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._21 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._31 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = m._02; b1 = m._12; b2 = m._22; b3 = m._32;
		this._02 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._12 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._22 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._32 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		b0 = m._03; b1 = m._13; b2 = m._23; b3 = m._33;
		this._03 = a00 * b0 + a01 * b1 + a02 * b2 + a03 * b3;
		this._13 = a10 * b0 + a11 * b1 + a12 * b2 + a13 * b3;
		this._23 = a20 * b0 + a21 * b1 + a22 * b2 + a23 * b3;
		this._33 = a30 * b0 + a31 * b1 + a32 * b2 + a33 * b3;

		return this;
	}

	getInverse = (m: Mat4): Mat4 => {
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
		if (det == 0.0) return this.setIdentity();
		det = 1.0 / det;

		this._00 = (a11 * b11 - a12 * b10 + a13 * b09) * det;
		this._01 = (a02 * b10 - a01 * b11 - a03 * b09) * det;
		this._02 = (a31 * b05 - a32 * b04 + a33 * b03) * det;
		this._03 = (a22 * b04 - a21 * b05 - a23 * b03) * det;
		this._10 = (a12 * b08 - a10 * b11 - a13 * b07) * det;
		this._11 = (a00 * b11 - a02 * b08 + a03 * b07) * det;
		this._12 = (a32 * b02 - a30 * b05 - a33 * b01) * det;
		this._13 = (a20 * b05 - a22 * b02 + a23 * b01) * det;
		this._20 = (a10 * b10 - a11 * b08 + a13 * b06) * det;
		this._21 = (a01 * b08 - a00 * b10 - a03 * b06) * det;
		this._22 = (a30 * b04 - a31 * b02 + a33 * b00) * det;
		this._23 = (a21 * b02 - a20 * b04 - a23 * b00) * det;
		this._30 = (a11 * b07 - a10 * b09 - a12 * b06) * det;
		this._31 = (a00 * b09 - a01 * b07 + a02 * b06) * det;
		this._32 = (a31 * b01 - a30 * b03 - a32 * b00) * det;
		this._33 = (a20 * b03 - a21 * b01 + a22 * b00) * det;

		return this;
	}

	transpose = (): Mat4 => {
		let f = this._01; this._01 = this._10; this._10 = f;
		f = this._02; this._02 = this._20; this._20 = f;
		f = this._03; this._03 = this._30; this._30 = f;
		f = this._12; this._12 = this._21; this._21 = f;
		f = this._13; this._13 = this._31; this._31 = f;
		f = this._23; this._23 = this._32; this._32 = f;
		return this;
	}

	transpose3x3 = (): Mat4 => {
		let f = this._01; this._01 = this._10; this._10 = f;
		f = this._02; this._02 = this._20; this._20 = f;
		f = this._12; this._12 = this._21; this._21 = f;
		return this;
	}

	clone = (): Mat4 => {
		return new Mat4(
			this._00, this._10, this._20, this._30,
			this._01, this._11, this._21, this._31,
			this._02, this._12, this._22, this._32,
			this._03, this._13, this._23, this._33
		);
	}

	setF32 = (a: Float32Array, offset = 0): Mat4 => {
		this._00 = a[0 + offset]; this._10 = a[1 + offset]; this._20 = a[2 + offset]; this._30 = a[3 + offset];
		this._01 = a[4 + offset]; this._11 = a[5 + offset]; this._21 = a[6 + offset]; this._31 = a[7 + offset];
		this._02 = a[8 + offset]; this._12 = a[9 + offset]; this._22 = a[10 + offset]; this._32 = a[11 + offset];
		this._03 = a[12 + offset]; this._13 = a[13 + offset]; this._23 = a[14 + offset]; this._33 = a[15 + offset];
		return this;
	}

	setFrom = (m: Mat4): Mat4 => {
		this._00 = m._00; this._01 = m._01; this._02 = m._02; this._03 = m._03;
		this._10 = m._10; this._11 = m._11; this._12 = m._12; this._13 = m._13;
		this._20 = m._20; this._21 = m._21; this._22 = m._22; this._23 = m._23;
		this._30 = m._30; this._31 = m._31; this._32 = m._32; this._33 = m._33;
		return this;
	}

	getLoc = (): Vec4 => {
		return new Vec4(this._30, this._31, this._32, this._33);
	}

	getScale = (): Vec4 => {
		return new Vec4(
			Math.sqrt(this._00 * this._00 + this._10 * this._10 + this._20 * this._20),
			Math.sqrt(this._01 * this._01 + this._11 * this._11 + this._21 * this._21),
			Math.sqrt(this._02 * this._02 + this._12 * this._12 + this._22 * this._22)
		);
	}

	mult = (s: f32): Mat4 => {
		this._00 *= s; this._10 *= s; this._20 *= s; this._30 *= s;
		this._01 *= s; this._11 *= s; this._21 *= s; this._31 *= s;
		this._02 *= s; this._12 *= s; this._22 *= s; this._32 *= s;
		this._03 *= s; this._13 *= s; this._23 *= s; this._33 *= s;
		return this;
	}

	toRotation = (): Mat4 => {
		let scale = 1.0 / Mat4.helpVec.set(this._00, this._01, this._02).length();
		this._00 = this._00 * scale;
		this._01 = this._01 * scale;
		this._02 = this._02 * scale;
		scale = 1.0 / Mat4.helpVec.set(this._10, this._11, this._12).length();
		this._10 = this._10 * scale;
		this._11 = this._11 * scale;
		this._12 = this._12 * scale;
		scale = 1.0 / Mat4.helpVec.set(this._20, this._21, this._22).length();
		this._20 = this._20 * scale;
		this._21 = this._21 * scale;
		this._22 = this._22 * scale;
		this._03 = 0.0;
		this._13 = 0.0;
		this._23 = 0.0;
		this._30 = 0.0;
		this._31 = 0.0;
		this._32 = 0.0;
		this._33 = 1.0;
		return this;
	}

	static persp = (fovY: f32, aspect: f32, zn: f32, zf: f32): Mat4 => {
		let uh = 1.0 / Math.tan(fovY / 2);
		let uw = uh / aspect;
		return new Mat4(
			uw, 0, 0, 0,
			0, uh, 0, 0,
			0, 0, (zf + zn) / (zn - zf), 2 * zf * zn / (zn - zf),
			0, 0, -1, 0
		);
	}

	static ortho = (left: f32, right: f32, bottom: f32, top: f32, near: f32, far: f32): Mat4 => {
		let rl = right - left;
		let tb = top - bottom;
		let fn = far - near;
		let tx = -(right + left) / (rl);
		let ty = -(top + bottom) / (tb);
		let tz = -(far + near) / (fn);
		return new Mat4(
			2 / rl,	0,		0,		 tx,
			0,		2 / tb,	0,		 ty,
			0,		0,		-2 / fn, tz,
			0,		0,		0,		 1
		);
	}

	right = (): Vec4 => {
		return new Vec4(this._00, this._01, this._02);
	}

	look = (): Vec4 => {
		return new Vec4(this._10, this._11, this._12);
	}

	up = (): Vec4 => {
		return new Vec4(this._20, this._21, this._22);
	}

	toFloat32Array = (): Float32Array => {
		let array = new Float32Array(16);
		array[0] = this._00;
		array[1] = this._10;
		array[2] = this._20;
		array[3] = this._30;
		array[4] = this._01;
		array[5] = this._11;
		array[6] = this._21;
		array[7] = this._31;
		array[8] = this._02;
		array[9] = this._12;
		array[10] = this._22;
		array[11] = this._32;
		array[12] = this._03;
		array[13] = this._13;
		array[14] = this._23;
		array[15] = this._33;
		return array;
	}

	static rotationZ = (alpha: f32): Mat4 => {
		let ca = Math.cos(alpha);
		let sa = Math.sin(alpha);
		return new Mat4(
			ca, -sa, 0, 0,
			sa,  ca, 0, 0,
			0,   0, 1, 0,
			0,   0, 0, 1
		);
	}

	cofactor = (m0: f32, m1: f32, m2: f32,
			    m3: f32, m4: f32, m5: f32,
			    m6: f32, m7: f32, m8: f32): f32 => {
		return m0 * ( m4 * m8 - m5 * m7 ) - m1 * ( m3 * m8 - m5 * m6 ) + m2 * ( m3 * m7 - m4 * m6 );
	}

	determinant = (): f32 => {
		let c00 = this.cofactor(this._11, this._21, this._31, this._12, this._22, this._32, this._13, this._23, this._33);
		let c01 = this.cofactor(this._10, this._20, this._30, this._12, this._22, this._32, this._13, this._23, this._33);
		let c02 = this.cofactor(this._10, this._20, this._30, this._11, this._21, this._31, this._13, this._23, this._33);
		let c03 = this.cofactor(this._10, this._20, this._30, this._11, this._21, this._31, this._12, this._22, this._32);
		return this._00 * c00 - this._01 * c01 + this._02 * c02 - this._03 * c03;
	}

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
