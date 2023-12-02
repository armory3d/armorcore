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
}
