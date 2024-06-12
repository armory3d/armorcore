
type vec2_t = {
	x?: f32;
	y?: f32;
	type?: string;
};

function vec2_create(x: f32 = 0.0, y: f32 = 0.0): vec2_t {
	let self: vec2_t = {};
	self.x = x;
	self.y = y;
	self.type = "vec2_t";
	return self;
}

function vec2_len(self: vec2_t): f32 {
	return math_sqrt(self.x * self.x + self.y * self.y);
}

function vec2_normalize(self: vec2_t): vec2_t {
	let n: f32 = vec2_len(self);
	if (n > 0.0) {
		let inv_n: f32 = 1.0 / n;
		self.x *= inv_n;
		self.y *= inv_n;
	}
	return self;
}
