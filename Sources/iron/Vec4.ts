
class Vec4 {
	x: f32;
	y: f32;
	z: f32;
	w: f32;

	constructor(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
	}

	cross = (v: Vec4): Vec4 => {
		let ax = this.x;
		let ay = this.y;
		let az = this.z;
		let vx = v.x;
		let vy = v.y;
		let vz = v.z;
		this.x = ay * vz - az * vy;
		this.y = az * vx - ax * vz;
		this.z = ax * vy - ay * vx;
		return this;
	}

	crossvecs = (a: Vec4, b: Vec4): Vec4 => {
		let ax = a.x;
		let ay = a.y;
		let az = a.z;
		let bx = b.x;
		let by = b.y;
		let bz = b.z;
		this.x = ay * bz - az * by;
		this.y = az * bx - ax * bz;
		this.z = ax * by - ay * bx;
		return this;
	}

	set = (x: f32, y: f32, z: f32, w: f32 = 1.0): Vec4 => {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
		return this;
	}

	add = (v: Vec4): Vec4 => {
		this.x += v.x;
		this.y += v.y;
		this.z += v.z;
		return this;
	}

	addf = (x: f32, y: f32, z: f32): Vec4 => {
		this.x += x;
		this.y += y;
		this.z += z;
		return this;
	}

	addvecs = (a: Vec4, b: Vec4): Vec4 => {
		this.x = a.x + b.x;
		this.y = a.y + b.y;
		this.z = a.z + b.z;
		return this;
	}

	subvecs = (a: Vec4, b: Vec4): Vec4 => {
		this.x = a.x - b.x;
		this.y = a.y - b.y;
		this.z = a.z - b.z;
		return this;
	}

	normalize = (): Vec4 => {
		let n = this.length();
		if (n > 0.0) {
			let invN = 1.0 / n;
			this.x *= invN;
			this.y *= invN;
			this.z *= invN;
		}
		return this;
	}

	mult = (f: f32): Vec4 => {
		this.x *= f;
		this.y *= f;
		this.z *= f;
		return this;
	}

	dot = (v: Vec4): f32 => {
		return this.x * v.x + this.y * v.y + this.z * v.z;
	}

	setFrom = (v: Vec4): Vec4 => {
		this.x = v.x;
		this.y = v.y;
		this.z = v.z;
		this.w = v.w;
		return this;
	}

	clone = (): Vec4 => {
		return new Vec4(this.x, this.y, this.z, this.w);
	}

	lerp = (from: Vec4, to: Vec4, s: f32): Vec4 => {
		this.x = from.x + (to.x - from.x) * s;
		this.y = from.y + (to.y - from.y) * s;
		this.z = from.z + (to.z - from.z) * s;
		return this;
	}

	applyproj = (m: Mat4): Vec4 => {
		let x = this.x;
		let y = this.y;
		let z = this.z;
		let d = 1.0 / (m._03 * x + m._13 * y + m._23 * z + m._33); // Perspective divide
		this.x = (m._00 * x + m._10 * y + m._20 * z + m._30) * d;
		this.y = (m._01 * x + m._11 * y + m._21 * z + m._31) * d;
		this.z = (m._02 * x + m._12 * y + m._22 * z + m._32) * d;
		return this;
	}

	applymat = (m: Mat4): Vec4 => {
		let x = this.x;
		let y = this.y;
		let z = this.z;
		this.x = m._00 * x + m._10 * y + m._20 * z + m._30;
		this.y = m._01 * x + m._11 * y + m._21 * z + m._31;
		this.z = m._02 * x + m._12 * y + m._22 * z + m._32;
		return this;
	}

	applymat4 = (m: Mat4): Vec4 => {
		let x = this.x;
		let y = this.y;
		let z = this.z;
		let w = this.w;
		this.x = m._00 * x + m._10 * y + m._20 * z + m._30 * w;
		this.y = m._01 * x + m._11 * y + m._21 * z + m._31 * w;
		this.z = m._02 * x + m._12 * y + m._22 * z + m._32 * w;
		this.w = m._03 * x + m._13 * y + m._23 * z + m._33 * w;
		return this;
	}

	applyQuat = (q: Quat): Vec4 => {
		let ix = q.w * this.x + q.y * this.z - q.z * this.y;
		let iy = q.w * this.y + q.z * this.x - q.x * this.z;
		let iz = q.w * this.z + q.x * this.y - q.y * this.x;
		let iw = -q.x * this.x - q.y * this.y - q.z * this.z;
		this.x = ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y;
		this.y = iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z;
		this.z = iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x;
		return this;
	}

	length = (): f32 => {
		return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
	}

	sub = (v: Vec4): Vec4 => {
		this.x -= v.x;
		this.y -= v.y;
		this.z -= v.z;
		return this;
	}

	static distance = (v1: Vec4, v2: Vec4): f32 => {
		return Vec4.distancef(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
	}

	static distancef = (v1x: f32, v1y: f32, v1z: f32, v2x: f32, v2y: f32, v2z: f32): f32 => {
		let vx = v1x - v2x;
		let vy = v1y - v2y;
		let vz = v1z - v2z;
		return Math.sqrt(vx * vx + vy * vy + vz * vz);
	}

	distanceTo = (p: Vec4): f32 => {
		return Math.sqrt((p.x - this.x) * (p.x - this.x) + (p.y - this.y) * (p.y - this.y) + (p.z - this.z) * (p.z - this.z));
	}

	reflect = (n: Vec4): Vec4 => {
		let d = 2 * this.dot(n);
		this.x = this.x - d * n.x;
		this.y = this.y - d * n.y;
		this.z = this.z - d * n.z;
		return this;
	}

	static xAxis = (): Vec4 => {
		return new Vec4(1.0, 0.0, 0.0);
	}

	static yAxis = (): Vec4 => {
		return new Vec4(0.0, 1.0, 0.0);
	}

	static zAxis = (): Vec4 => {
		return new Vec4(0.0, 0.0, 1.0);
	}
}
