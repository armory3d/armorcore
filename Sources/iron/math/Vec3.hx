package iron.math;

class Vec3 {
	public var x: Float;
	public var y: Float;
	public var z: Float;

	public inline function new(x: Float = 0.0, y: Float = 0.0, z: Float = 0.0) {
		this.x = x;
		this.y = y;
		this.z = z;
	}

	public inline function add(v: Vec3): Vec3 {
		x += v.x;
		y += v.y;
		z += v.z;
		return this;
	}

	public inline function mult(f: Float): Vec3 {
		x *= f;
		y *= f;
		z *= f;
		return this;
	}

	public inline function sub(v: Vec3): Vec3 {
		x -= v.x; y -= v.y; z -= v.z;
		return this;
	}
}
