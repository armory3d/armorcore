
class TVec4 {
	x: f32;
	y: f32;
	z: f32;
	w: f32;
}

class Vec4 {

	static create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): TVec4 {
		let self = new TVec4();
		self.x = x;
		self.y = y;
		self.z = z;
		self.w = w;
		return self;
	}

	static distance = (v1: TVec4, v2: TVec4): f32 => {
		return Vec4.distancef(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
	}

	static distancef = (v1x: f32, v1y: f32, v1z: f32, v2x: f32, v2y: f32, v2z: f32): f32 => {
		let vx = v1x - v2x;
		let vy = v1y - v2y;
		let vz = v1z - v2z;
		return Math.sqrt(vx * vx + vy * vy + vz * vz);
	}

	static xAxis = (): TVec4 => {
		return Vec4.create(1.0, 0.0, 0.0);
	}

	static yAxis = (): TVec4 => {
		return Vec4.create(0.0, 1.0, 0.0);
	}

	static zAxis = (): TVec4 => {
		return Vec4.create(0.0, 0.0, 1.0);
	}

	static cross = (self: TVec4, v: TVec4): TVec4 => {
		let ax = self.x;
		let ay = self.y;
		let az = self.z;
		let vx = v.x;
		let vy = v.y;
		let vz = v.z;
		self.x = ay * vz - az * vy;
		self.y = az * vx - ax * vz;
		self.z = ax * vy - ay * vx;
		return self;
	}

	static crossvecs = (self: TVec4, a: TVec4, b: TVec4): TVec4 => {
		let ax = a.x;
		let ay = a.y;
		let az = a.z;
		let bx = b.x;
		let by = b.y;
		let bz = b.z;
		self.x = ay * bz - az * by;
		self.y = az * bx - ax * bz;
		self.z = ax * by - ay * bx;
		return self;
	}

	static set = (self: TVec4, x: f32, y: f32, z: f32, w: f32 = 1.0): TVec4 => {
		self.x = x;
		self.y = y;
		self.z = z;
		self.w = w;
		return self;
	}

	static add = (self: TVec4, v: TVec4): TVec4 => {
		self.x += v.x;
		self.y += v.y;
		self.z += v.z;
		return self;
	}

	static addf = (self: TVec4, x: f32, y: f32, z: f32): TVec4 => {
		self.x += x;
		self.y += y;
		self.z += z;
		return self;
	}

	static addvecs = (self: TVec4, a: TVec4, b: TVec4): TVec4 => {
		self.x = a.x + b.x;
		self.y = a.y + b.y;
		self.z = a.z + b.z;
		return self;
	}

	static subvecs = (self: TVec4, a: TVec4, b: TVec4): TVec4 => {
		self.x = a.x - b.x;
		self.y = a.y - b.y;
		self.z = a.z - b.z;
		return self;
	}

	static normalize = (self: TVec4): TVec4 => {
		let n = Vec4.vec4_length(self);
		if (n > 0.0) {
			let invN = 1.0 / n;
			self.x *= invN;
			self.y *= invN;
			self.z *= invN;
		}
		return self;
	}

	static mult = (self: TVec4, f: f32): TVec4 => {
		self.x *= f;
		self.y *= f;
		self.z *= f;
		return self;
	}

	static dot = (self: TVec4, v: TVec4): f32 => {
		return self.x * v.x + self.y * v.y + self.z * v.z;
	}

	static setFrom = (self: TVec4, v: TVec4): TVec4 => {
		self.x = v.x;
		self.y = v.y;
		self.z = v.z;
		self.w = v.w;
		return self;
	}

	static clone = (self: TVec4): TVec4 => {
		return Vec4.create(self.x, self.y, self.z, self.w);
	}

	static lerp = (self: TVec4, from: TVec4, to: TVec4, s: f32): TVec4 => {
		self.x = from.x + (to.x - from.x) * s;
		self.y = from.y + (to.y - from.y) * s;
		self.z = from.z + (to.z - from.z) * s;
		return self;
	}

	static applyproj = (self: TVec4, m: TMat4): TVec4 => {
		let x = self.x;
		let y = self.y;
		let z = self.z;
		let d = 1.0 / (m._03 * x + m._13 * y + m._23 * z + m._33); // Perspective divide
		self.x = (m._00 * x + m._10 * y + m._20 * z + m._30) * d;
		self.y = (m._01 * x + m._11 * y + m._21 * z + m._31) * d;
		self.z = (m._02 * x + m._12 * y + m._22 * z + m._32) * d;
		return self;
	}

	static applymat = (self: TVec4, m: TMat4): TVec4 => {
		let x = self.x;
		let y = self.y;
		let z = self.z;
		self.x = m._00 * x + m._10 * y + m._20 * z + m._30;
		self.y = m._01 * x + m._11 * y + m._21 * z + m._31;
		self.z = m._02 * x + m._12 * y + m._22 * z + m._32;
		return self;
	}

	static applymat4 = (self: TVec4, m: TMat4): TVec4 => {
		let x = self.x;
		let y = self.y;
		let z = self.z;
		let w = self.w;
		self.x = m._00 * x + m._10 * y + m._20 * z + m._30 * w;
		self.y = m._01 * x + m._11 * y + m._21 * z + m._31 * w;
		self.z = m._02 * x + m._12 * y + m._22 * z + m._32 * w;
		self.w = m._03 * x + m._13 * y + m._23 * z + m._33 * w;
		return self;
	}

	static applyQuat = (self: TVec4, q: TQuat): TVec4 => {
		let ix = q.w * self.x + q.y * self.z - q.z * self.y;
		let iy = q.w * self.y + q.z * self.x - q.x * self.z;
		let iz = q.w * self.z + q.x * self.y - q.y * self.x;
		let iw = -q.x * self.x - q.y * self.y - q.z * self.z;
		self.x = ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y;
		self.y = iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z;
		self.z = iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x;
		return self;
	}

	static vec4_length = (self: TVec4): f32 => {
		return Math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z);
	}

	static sub = (self: TVec4, v: TVec4): TVec4 => {
		self.x -= v.x;
		self.y -= v.y;
		self.z -= v.z;
		return self;
	}

	static distanceTo = (self: TVec4, p: TVec4): f32 => {
		return Math.sqrt((p.x - self.x) * (p.x - self.x) + (p.y - self.y) * (p.y - self.y) + (p.z - self.z) * (p.z - self.z));
	}

	static reflect = (self: TVec4, n: TVec4): TVec4 => {
		let d = 2 * Vec4.dot(self, n);
		self.x = self.x - d * n.x;
		self.y = self.y - d * n.y;
		self.z = self.z - d * n.z;
		return self;
	}
}
