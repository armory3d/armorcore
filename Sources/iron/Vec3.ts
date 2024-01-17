
class Vec3 {
	x: f32;
	y: f32;
	z: f32;

	constructor(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0) {
		this.x = x;
		this.y = y;
		this.z = z;
	}

	add = (v: Vec3): Vec3 => {
		this.x += v.x;
		this.y += v.y;
		this.z += v.z;
		return this;
	}

	mult = (f: f32): Vec3 => {
		this.x *= f;
		this.y *= f;
		this.z *= f;
		return this;
	}

	sub = (v: Vec3): Vec3 => {
		this.x -= v.x;
		this.y -= v.y;
		this.z -= v.z;
		return this;
	}
}
