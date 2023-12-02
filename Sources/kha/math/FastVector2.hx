package kha.math;

@:structInit
class FastVector2 {
	public inline function new(x: Float = 0, y: Float = 0): Void {
		this.x = x;
		this.y = y;
	}

	public var x: Float;
	public var y: Float;
}
