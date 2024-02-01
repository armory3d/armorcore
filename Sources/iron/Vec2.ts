
class Vec2 {
	x: f32;
	y: f32;
}

function vec2_create(x: f32 = 0.0, y: f32 = 0.0): Vec2 {
	let self = new Vec2();
	self.x = x;
	self.y = y;
	return self;
}
