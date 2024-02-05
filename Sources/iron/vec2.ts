
class vec2_t {
	x: f32;
	y: f32;
}

function vec2_create(x: f32 = 0.0, y: f32 = 0.0): vec2_t {
	let self = new vec2_t();
	self.x = x;
	self.y = y;
	return self;
}
