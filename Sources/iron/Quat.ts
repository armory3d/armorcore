/// <reference path='./Vec4.ts'/>
/// <reference path='./Mat4.ts'/>

class Quat {

	x: f32;
	y: f32;
	z: f32;
	w: f32;

	static helpVec = new Vec4();
	static helpMat = Mat4.identity();
	static xAxis = Vec4.xAxis();
	static yAxis = Vec4.yAxis();

	static SQRT2: f32 = 1.4142135623730951;

	constructor(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
	}

	set = (x: f32, y: f32, z: f32, w: f32): Quat => {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
		return this;
	}

	fromAxisAngle = (axis: Vec4, angle: f32): Quat => {
		let s: f32 = Math.sin(angle * 0.5);
		this.x = axis.x * s;
		this.y = axis.y * s;
		this.z = axis.z * s;
		this.w = Math.cos(angle * 0.5);
		return this.normalize();
	}

	fromMat = (m: Mat4): Quat => {
		Quat.helpMat.setFrom(m);
		Quat.helpMat.toRotation();
		return this.fromRotationMat(Quat.helpMat);
	}

	fromRotationMat = (m: Mat4): Quat => {
		// Assumes the upper 3x3 is a pure rotation matrix
		let m11 = m._00; let m12 = m._10; let m13 = m._20;
		let m21 = m._01; let m22 = m._11; let m23 = m._21;
		let m31 = m._02; let m32 = m._12; let m33 = m._22;
		let tr = m11 + m22 + m33;
		let s = 0.0;

		if (tr > 0) {
			s = 0.5 / Math.sqrt(tr + 1.0);
			this.w = 0.25 / s;
			this.x = (m32 - m23) * s;
			this.y = (m13 - m31) * s;
			this.z = (m21 - m12) * s;
		}
		else if (m11 > m22 && m11 > m33) {
			s = 2.0 * Math.sqrt(1.0 + m11 - m22 - m33);
			this.w = (m32 - m23) / s;
			this.x = 0.25 * s;
			this.y = (m12 + m21) / s;
			this.z = (m13 + m31) / s;
		}
		else if (m22 > m33) {
			s = 2.0 * Math.sqrt(1.0 + m22 - m11 - m33);
			this.w = (m13 - m31) / s;
			this.x = (m12 + m21) / s;
			this.y = 0.25 * s;
			this.z = (m23 + m32) / s;
		}
		else {
			s = 2.0 * Math.sqrt(1.0 + m33 - m11 - m22);
			this.w = (m21 - m12) / s;
			this.x = (m13 + m31) / s;
			this.y = (m23 + m32) / s;
			this.z = 0.25 * s;
		}
		return this;
	}

	mult = (q: Quat): Quat => {
		return this.multquats(this, q);
	}

	multquats = (q1: Quat, q2: Quat): Quat => {
		let q1x = q1.x; let q1y = q1.y; let q1z = q1.z; let q1w = q1.w;
		let q2x = q2.x; let q2y = q2.y; let q2z = q2.z; let q2w = q2.w;
		this.x = q1x * q2w + q1w * q2x + q1y * q2z - q1z * q2y;
		this.y = q1w * q2y - q1x * q2z + q1y * q2w + q1z * q2x;
		this.z = q1w * q2z + q1x * q2y - q1y * q2x + q1z * q2w;
		this.w = q1w * q2w - q1x * q2x - q1y * q2y - q1z * q2z;
		return this;
	}

	normalize = (): Quat => {
		let l = Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w);
		if (l == 0.0) {
			this.x = 0;
			this.y = 0;
			this.z = 0;
			this.w = 0;
		}
		else {
			l = 1.0 / l;
			this.x *= l;
			this.y *= l;
			this.z *= l;
			this.w *= l;
		}
		return this;
	}

	setFrom = (q: Quat): Quat => {
		this.x = q.x;
		this.y = q.y;
		this.z = q.z;
		this.w = q.w;
		return this;
	}

	getEuler = (): Vec4 => {
		let a = -2 * (this.x * this.z - this.w * this.y);
		let b =  this.w *  this.w + this.x * this.x - this.y * this.y - this.z * this.z;
		let c =  2 * (this.x * this.y + this.w * this.z);
		let d = -2 * (this.y * this.z - this.w * this.x);
		let e =  this.w *  this.w - this.x * this.x + this.y * this.y - this.z * this.z;
		return new Vec4(Math.atan2(d, e), Math.atan2(a, b), Math.asin(c));
	}

	fromEuler = (x: f32, y: f32, z: f32): Quat => {
		let f = x / 2;
		let c1 = Math.cos(f);
		let s1 = Math.sin(f);
		f = y / 2;
		let c2 = Math.cos(f);
		let s2 = Math.sin(f);
		f = z / 2;
		let c3 = Math.cos(f);
		let s3 = Math.sin(f);
		// YZX
		this.x = s1 * c2 * c3 + c1 * s2 * s3;
		this.y = c1 * s2 * c3 + s1 * c2 * s3;
		this.z = c1 * c2 * s3 - s1 * s2 * c3;
		this.w = c1 * c2 * c3 - s1 * s2 * s3;
		return this;
	}

	lerp = (from: Quat, to: Quat, s: f32): Quat => {
		let fromx = from.x;
		let fromy = from.y;
		let fromz = from.z;
		let fromw = from.w;
		let dot: f32 = from.dot(to);
		if (dot < 0.0) {
			fromx = -fromx;
			fromy = -fromy;
			fromz = -fromz;
			fromw = -fromw;
		}
		this.x = fromx + (to.x - fromx) * s;
		this.y = fromy + (to.y - fromy) * s;
		this.z = fromz + (to.z - fromz) * s;
		this.w = fromw + (to.w - fromw) * s;
		return this.normalize();
	}

	dot = (q: Quat): f32 => {
		return (this.x * q.x) + (this.y * q.y) + (this.z * q.z) + (this.w * q.w);
	}

	fromTo = (v1: Vec4, v2: Vec4): Quat => {
		// Rotation formed by direction vectors
		// v1 and v2 should be normalized first
		let a = Quat.helpVec;
		let dot = v1.dot(v2);
		if (dot < -0.999999) {
			a.crossvecs(Quat.xAxis, v1);
			if (a.length() < 0.000001) a.crossvecs(Quat.yAxis, v1);
			a.normalize();
			this.fromAxisAngle(a, Math.PI);
		}
		else if (dot > 0.999999) {
			this.set(0, 0, 0, 1);
		}
		else {
			a.crossvecs(v1, v2);
			this.set(a.x, a.y, a.z, 1 + dot);
			this.normalize();
		}
		return this;
	}
}
