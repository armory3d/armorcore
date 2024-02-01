/// <reference path='./Vec4.ts'/>
/// <reference path='./Mat4.ts'/>

class TQuat {
	x: f32;
	y: f32;
	z: f32;
	w: f32;
}

class Quat {
	static helpVec = Vec4.create();
	static helpMat = Mat4.identity();
	static xAxis = Vec4.xAxis();
	static yAxis = Vec4.yAxis();
	static SQRT2: f32 = 1.4142135623730951;

	static create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): TQuat {
		let self = new TQuat();
		self.x = x;
		self.y = y;
		self.z = z;
		self.w = w;
		return self;
	}

	static set = (self: TQuat, x: f32, y: f32, z: f32, w: f32): TQuat => {
		self.x = x;
		self.y = y;
		self.z = z;
		self.w = w;
		return self;
	}

	static fromAxisAngle = (self: TQuat, axis: TVec4, angle: f32): TQuat => {
		let s: f32 = Math.sin(angle * 0.5);
		self.x = axis.x * s;
		self.y = axis.y * s;
		self.z = axis.z * s;
		self.w = Math.cos(angle * 0.5);
		return Quat.normalize(self);
	}

	static fromMat = (self: TQuat, m: TMat4): TQuat => {
		Mat4.setFrom(Quat.helpMat, m);
		Mat4.toRotation(Quat.helpMat);
		return Quat.fromRotationMat(self, Quat.helpMat);
	}

	static fromRotationMat = (self: TQuat, m: TMat4): TQuat => {
		// Assumes the upper 3x3 is a pure rotation matrix
		let m11 = m._00; let m12 = m._10; let m13 = m._20;
		let m21 = m._01; let m22 = m._11; let m23 = m._21;
		let m31 = m._02; let m32 = m._12; let m33 = m._22;
		let tr = m11 + m22 + m33;
		let s = 0.0;

		if (tr > 0) {
			s = 0.5 / Math.sqrt(tr + 1.0);
			self.w = 0.25 / s;
			self.x = (m32 - m23) * s;
			self.y = (m13 - m31) * s;
			self.z = (m21 - m12) * s;
		}
		else if (m11 > m22 && m11 > m33) {
			s = 2.0 * Math.sqrt(1.0 + m11 - m22 - m33);
			self.w = (m32 - m23) / s;
			self.x = 0.25 * s;
			self.y = (m12 + m21) / s;
			self.z = (m13 + m31) / s;
		}
		else if (m22 > m33) {
			s = 2.0 * Math.sqrt(1.0 + m22 - m11 - m33);
			self.w = (m13 - m31) / s;
			self.x = (m12 + m21) / s;
			self.y = 0.25 * s;
			self.z = (m23 + m32) / s;
		}
		else {
			s = 2.0 * Math.sqrt(1.0 + m33 - m11 - m22);
			self.w = (m21 - m12) / s;
			self.x = (m13 + m31) / s;
			self.y = (m23 + m32) / s;
			self.z = 0.25 * s;
		}
		return self;
	}

	static mult = (self: TQuat, q: TQuat): TQuat => {
		return Quat.multquats(self, self, q);
	}

	static multquats = (self: TQuat, q1: TQuat, q2: TQuat): TQuat => {
		let q1x = q1.x; let q1y = q1.y; let q1z = q1.z; let q1w = q1.w;
		let q2x = q2.x; let q2y = q2.y; let q2z = q2.z; let q2w = q2.w;
		self.x = q1x * q2w + q1w * q2x + q1y * q2z - q1z * q2y;
		self.y = q1w * q2y - q1x * q2z + q1y * q2w + q1z * q2x;
		self.z = q1w * q2z + q1x * q2y - q1y * q2x + q1z * q2w;
		self.w = q1w * q2w - q1x * q2x - q1y * q2y - q1z * q2z;
		return self;
	}

	static normalize = (self: TQuat): TQuat => {
		let l = Math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w);
		if (l == 0.0) {
			self.x = 0;
			self.y = 0;
			self.z = 0;
			self.w = 0;
		}
		else {
			l = 1.0 / l;
			self.x *= l;
			self.y *= l;
			self.z *= l;
			self.w *= l;
		}
		return self;
	}

	static setFrom = (self: TQuat, q: TQuat): TQuat => {
		self.x = q.x;
		self.y = q.y;
		self.z = q.z;
		self.w = q.w;
		return self;
	}

	static getEuler = (self: TQuat): TVec4 => {
		let a = -2 * (self.x * self.z - self.w * self.y);
		let b =  self.w *  self.w + self.x * self.x - self.y * self.y - self.z * self.z;
		let c =  2 * (self.x * self.y + self.w * self.z);
		let d = -2 * (self.y * self.z - self.w * self.x);
		let e =  self.w *  self.w - self.x * self.x + self.y * self.y - self.z * self.z;
		return Vec4.create(Math.atan2(d, e), Math.atan2(a, b), Math.asin(c));
	}

	static fromEuler = (self: TQuat, x: f32, y: f32, z: f32): TQuat => {
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
		self.x = s1 * c2 * c3 + c1 * s2 * s3;
		self.y = c1 * s2 * c3 + s1 * c2 * s3;
		self.z = c1 * c2 * s3 - s1 * s2 * c3;
		self.w = c1 * c2 * c3 - s1 * s2 * s3;
		return self;
	}

	static lerp = (self: TQuat, from: TQuat, to: TQuat, s: f32): TQuat => {
		let fromx = from.x;
		let fromy = from.y;
		let fromz = from.z;
		let fromw = from.w;
		let dot: f32 = Quat.dot(from, to);
		if (dot < 0.0) {
			fromx = -fromx;
			fromy = -fromy;
			fromz = -fromz;
			fromw = -fromw;
		}
		self.x = fromx + (to.x - fromx) * s;
		self.y = fromy + (to.y - fromy) * s;
		self.z = fromz + (to.z - fromz) * s;
		self.w = fromw + (to.w - fromw) * s;
		return Quat.normalize(self);
	}

	static dot = (self: TQuat, q: TQuat): f32 => {
		return (self.x * q.x) + (self.y * q.y) + (self.z * q.z) + (self.w * q.w);
	}

	static fromTo = (self: TQuat, v1: TVec4, v2: TVec4): TQuat => {
		// Rotation formed by direction vectors
		// v1 and v2 should be normalized first
		let a = Quat.helpVec;
		let dot = Vec4.dot(v1, v2);
		if (dot < -0.999999) {
			Vec4.crossvecs(a, Quat.xAxis, v1);
			if (Vec4.vec4_length(a) < 0.000001) Vec4.crossvecs(a, Quat.yAxis, v1);
			Vec4.normalize(a);
			Quat.fromAxisAngle(self, a, Math.PI);
		}
		else if (dot > 0.999999) {
			Quat.set(self, 0, 0, 0, 1);
		}
		else {
			Vec4.crossvecs(a, v1, v2);
			Quat.set(self, a.x, a.y, a.z, 1 + dot);
			Quat.normalize(self);
		}
		return self;
	}
}
