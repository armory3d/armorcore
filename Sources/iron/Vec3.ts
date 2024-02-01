
class Vec3 {
	x: f32;
	y: f32;
	z: f32;
}

function vec3_create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0): Vec3 {
	let self = new Vec3();
	self.x = x;
	self.y = y;
	self.z = z;
	return self;
}

function vec3_add(self: Vec3, v: Vec3): Vec3 {
	self.x += v.x;
	self.y += v.y;
	self.z += v.z;
	return self;
}

function vec3_mult(self: Vec3, f: f32): Vec3 {
	self.x *= f;
	self.y *= f;
	self.z *= f;
	return self;
}

function vec3_sub(self: Vec3, v: Vec3): Vec3 {
	self.x -= v.x;
	self.y -= v.y;
	self.z -= v.z;
	return self;
}
