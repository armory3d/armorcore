package kha.math;

@:structInit
class FastVector4 {
	public inline function new(x: Float = 0, y: Float = 0, z: Float = 0, w: Float = 1): Void {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
	}

	public var x: Float;
	public var y: Float;
	public var z: Float;
	public var w: Float;
}
