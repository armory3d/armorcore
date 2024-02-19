
type vec4_t = {
	x?: f32;
	y?: f32;
	z?: f32;
	w?: f32;
	type?: string;
};

function vec4_create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): vec4_t {
	let self: vec4_t = {};
	self.x = x;
	self.y = y;
	self.z = z;
	self.w = w;
	self.type = "vec4_t";
	return self;
}

function vec4_dist(v1: vec4_t, v2: vec4_t): f32 {
	return vec4_dist_f(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
}

function vec4_dist_f(v1x: f32, v1y: f32, v1z: f32, v2x: f32, v2y: f32, v2z: f32): f32 {
	let vx: f32 = v1x - v2x;
	let vy: f32 = v1y - v2y;
	let vz: f32 = v1z - v2z;
	return math_sqrt(vx * vx + vy * vy + vz * vz);
}

function vec4_x_axis(): vec4_t {
	return vec4_create(1.0, 0.0, 0.0);
}

function vec4_y_axis(): vec4_t {
	return vec4_create(0.0, 1.0, 0.0);
}

function vec4_z_axis(): vec4_t {
	return vec4_create(0.0, 0.0, 1.0);
}

function vec4_cross(self: vec4_t, v: vec4_t): vec4_t {
	let ax: f32 = self.x;
	let ay: f32 = self.y;
	let az: f32 = self.z;
	let vx: f32 = v.x;
	let vy: f32 = v.y;
	let vz: f32 = v.z;
	self.x = ay * vz - az * vy;
	self.y = az * vx - ax * vz;
	self.z = ax * vy - ay * vx;
	return self;
}

function vec4_cross_vecs(self: vec4_t, a: vec4_t, b: vec4_t): vec4_t {
	let ax: f32 = a.x;
	let ay: f32 = a.y;
	let az: f32 = a.z;
	let bx: f32 = b.x;
	let by: f32 = b.y;
	let bz: f32 = b.z;
	self.x = ay * bz - az * by;
	self.y = az * bx - ax * bz;
	self.z = ax * by - ay * bx;
	return self;
}

function vec4_set(self: vec4_t, x: f32, y: f32, z: f32, w: f32 = 1.0): vec4_t {
	self.x = x;
	self.y = y;
	self.z = z;
	self.w = w;
	return self;
}

function vec4_add(self: vec4_t, v: vec4_t): vec4_t {
	self.x += v.x;
	self.y += v.y;
	self.z += v.z;
	return self;
}

function vec4_add_f(self: vec4_t, x: f32, y: f32, z: f32): vec4_t {
	self.x += x;
	self.y += y;
	self.z += z;
	return self;
}

function vec4_add_vecs(self: vec4_t, a: vec4_t, b: vec4_t): vec4_t {
	self.x = a.x + b.x;
	self.y = a.y + b.y;
	self.z = a.z + b.z;
	return self;
}

function vec4_sub_vecs(self: vec4_t, a: vec4_t, b: vec4_t): vec4_t {
	self.x = a.x - b.x;
	self.y = a.y - b.y;
	self.z = a.z - b.z;
	return self;
}

function vec4_normalize(self: vec4_t): vec4_t {
	let n: f32 = vec4_len(self);
	if (n > 0.0) {
		let inv_n: f32 = 1.0 / n;
		self.x *= inv_n;
		self.y *= inv_n;
		self.z *= inv_n;
	}
	return self;
}

function vec4_mult(self: vec4_t, f: f32): vec4_t {
	self.x *= f;
	self.y *= f;
	self.z *= f;
	return self;
}

function vec4_dot(self: vec4_t, v: vec4_t): f32 {
	return self.x * v.x + self.y * v.y + self.z * v.z;
}

function vec4_set_from(self: vec4_t, v: vec4_t): vec4_t {
	self.x = v.x;
	self.y = v.y;
	self.z = v.z;
	self.w = v.w;
	return self;
}

function vec4_clone(self: vec4_t): vec4_t {
	return vec4_create(self.x, self.y, self.z, self.w);
}

function vec4_lerp(self: vec4_t, from: vec4_t, to: vec4_t, s: f32): vec4_t {
	self.x = from.x + (to.x - from.x) * s;
	self.y = from.y + (to.y - from.y) * s;
	self.z = from.z + (to.z - from.z) * s;
	return self;
}

function vec4_apply_proj(self: vec4_t, m: mat4_t): vec4_t {
	let x: f32 = self.x;
	let y: f32 = self.y;
	let z: f32 = self.z;
	let d: f32 = 1.0 / (m.m[3] * x + m.m[7] * y + m.m[11] * z + m.m[15]); // Perspective divide
	self.x = (m.m[0] * x + m.m[4] * y + m.m[8] * z + m.m[12]) * d;
	self.y = (m.m[1] * x + m.m[5] * y + m.m[9] * z + m.m[13]) * d;
	self.z = (m.m[2] * x + m.m[6] * y + m.m[10] * z + m.m[14]) * d;
	return self;
}

function vec4_apply_mat(self: vec4_t, m: mat4_t): vec4_t {
	let x: f32 = self.x;
	let y: f32 = self.y;
	let z: f32 = self.z;
	self.x = m.m[0] * x + m.m[4] * y + m.m[8] * z + m.m[12];
	self.y = m.m[1] * x + m.m[5] * y + m.m[9] * z + m.m[13];
	self.z = m.m[2] * x + m.m[6] * y + m.m[10] * z + m.m[14];
	return self;
}

function vec4_apply_mat4(self: vec4_t, m: mat4_t): vec4_t {
	let x: f32 = self.x;
	let y: f32 = self.y;
	let z: f32 = self.z;
	let w: f32 = self.w;
	self.x = m.m[0] * x + m.m[4] * y + m.m[8] * z + m.m[12] * w;
	self.y = m.m[1] * x + m.m[5] * y + m.m[9] * z + m.m[13] * w;
	self.z = m.m[2] * x + m.m[6] * y + m.m[10] * z + m.m[14] * w;
	self.w = m.m[3] * x + m.m[7] * y + m.m[11] * z + m.m[15] * w;
	return self;
}

function vec4_apply_quat(self: vec4_t, q: quat_t): vec4_t {
	let ix: f32 = q.w * self.x + q.y * self.z - q.z * self.y;
	let iy: f32 = q.w * self.y + q.z * self.x - q.x * self.z;
	let iz: f32 = q.w * self.z + q.x * self.y - q.y * self.x;
	let iw: f32 = -q.x * self.x - q.y * self.y - q.z * self.z;
	self.x = ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y;
	self.y = iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z;
	self.z = iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x;
	return self;
}

function vec4_len(self: vec4_t): f32 {
	return math_sqrt(self.x * self.x + self.y * self.y + self.z * self.z);
}

function vec4_sub(self: vec4_t, v: vec4_t): vec4_t {
	self.x -= v.x;
	self.y -= v.y;
	self.z -= v.z;
	return self;
}

function vec4_dist_to(self: vec4_t, p: vec4_t): f32 {
	return math_sqrt((p.x - self.x) * (p.x - self.x) + (p.y - self.y) * (p.y - self.y) + (p.z - self.z) * (p.z - self.z));
}

function vec4_reflect(self: vec4_t, n: vec4_t): vec4_t {
	let d: f32 = 2 * vec4_dot(self, n);
	self.x = self.x - d * n.x;
	self.y = self.y - d * n.y;
	self.z = self.z - d * n.z;
	return self;
}
