
type vec3_t = {
	x?: f32;
	y?: f32;
	z?: f32;
	type?: string;
};

function vec3_create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0): vec3_t {
	let self: vec3_t = {};
	self.x = x;
	self.y = y;
	self.z = z;
	self.type = "vec3_t";
	return self;
}

function vec3_add(self: vec3_t, v: vec3_t): vec3_t {
	self.x += v.x;
	self.y += v.y;
	self.z += v.z;
	return self;
}

function vec3_mult(self: vec3_t, f: f32): vec3_t {
	self.x *= f;
	self.y *= f;
	self.z *= f;
	return self;
}

function vec3_sub(self: vec3_t, v: vec3_t): vec3_t {
	self.x -= v.x;
	self.y -= v.y;
	self.z -= v.z;
	return self;
}
