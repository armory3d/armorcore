package kha.math;

@:structInit
class FastVector3 {
	public inline function new(x: Float = 0, y: Float = 0, z: Float = 0): Void {
		this.x = x;
		this.y = y;
		this.z = z;
	}

	public var x: Float;
	public var y: Float;
	public var z: Float;
	public var length(get, set): Float;

	@:extern public inline function setFrom(v: FastVector3): Void {
		this.x = v.x;
		this.y = v.y;
		this.z = v.z;
	}

	private inline function get_length(): Float {
		return Math.sqrt(x * x + y * y + z * z);
	}

	private function set_length(length: Float): Float {
		var currentLength = get_length();
		if (currentLength == 0) return 0;
		var mul = length / currentLength;
		x *= mul;
		y *= mul;
		z *= mul;
		return length;
	}

	@:extern public inline function add(vec: FastVector3): FastVector3 {
		return new FastVector3(x + vec.x, y + vec.y, z + vec.z);
	}

	@:extern public inline function sub(vec: FastVector3): FastVector3 {
		return new FastVector3(x - vec.x, y - vec.y, z - vec.z);
	}

	@:extern public inline function mult(value: Float): FastVector3 {
		return new FastVector3(x * value, y * value, z * value);
	}

	@:extern public inline function dot(v: FastVector3): Float {
		return x * v.x + y * v.y + z * v.z;
	}

	@:extern public inline function cross(v: FastVector3): FastVector3 {
		var _x = y * v.z - z * v.y;
		var _y = z * v.x - x * v.z;
		var _z = x * v.y - y * v.x;
		return new FastVector3(_x, _y, _z);
	}

	@:extern public inline function normalized(): FastVector3 {
		var v = new FastVector3(x, y, z);
		#if haxe4 inline #end v.set_length(1);
		return v;
	}

	public function toString() {
		return 'FastVector3($x, $y, $z)';
	}
}
