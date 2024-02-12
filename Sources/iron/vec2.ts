
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
