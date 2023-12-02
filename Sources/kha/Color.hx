package kha;

abstract Color(Int) from Int from UInt to Int to UInt {

	static inline var invMaxChannelValue: Float = 1 / 255;

	public static inline function fromValue(value: Int): Color {
		return new Color(value);
	}

	public static function fromBytes(r: Int, g: Int, b: Int, a: Int = 255): Color {
		return new Color((a << 24) | (r << 16) | (g << 8) | b);
	}

	public static function fromFloats(r: Float, g: Float, b: Float, a: Float = 1): Color {
		return new Color((Math.round(a * 255) << 24) | (Math.round(r * 255) << 16) | (Math.round(g * 255) << 8) | Math.round(b * 255));
	}

	public var Rb(get, set): Int;
	public var Gb(get, set): Int;
	public var Bb(get, set): Int;
	public var Ab(get, set): Int;
	public var R(get, set): Float;
	public var G(get, set): Float;
	public var B(get, set): Float;
	public var A(get, set): Float;

	private function new(value: Int) {
		this = value;
	}

	public var value(get, set): Int;

	private inline function get_value(): Int {
		return this;
	}

	private inline function set_value(value: Int): Int {
		this = value;
		return this;
	}

	private inline function get_Rb(): Int {
		return (this & 0x00ff0000) >>> 16;
	}

	private inline function get_Gb(): Int {
		return (this & 0x0000ff00) >>> 8;
	}

	private inline function get_Bb(): Int {
		return this & 0x000000ff;
	}

	private inline function get_Ab(): Int {
		return this >>> 24;
	}

	private inline function set_Rb(i: Int): Int {
		this = (Ab << 24) | (i << 16) | (Gb << 8) | Bb;
		return i;
	}

	private inline function set_Gb(i: Int): Int {
		this = (Ab << 24) | (Rb << 16) | (i << 8) | Bb;
		return i;
	}

	private inline function set_Bb(i: Int): Int {
		this = (Ab << 24) | (Rb << 16) | (Gb << 8) | i;
		return i;
	}

	private inline function set_Ab(i: Int): Int {
		this = (i << 24) | (Rb << 16) | (Gb << 8) | Bb;
		return i;
	}

	private inline function get_R(): Float {
		return get_Rb() * invMaxChannelValue;
	}

	private inline function get_G(): Float {
		return get_Gb() * invMaxChannelValue;
	}

	private inline function get_B(): Float {
		return get_Bb() * invMaxChannelValue;
	}

	private inline function get_A(): Float {
		return get_Ab() * invMaxChannelValue;
	}

	private inline function set_R(f: Float): Float {
		this = (Math.round(A * 255) << 24) | (Math.round(f * 255) << 16) | (Math.round(G * 255) << 8) | Math.round(B * 255);
		return f;
	}

	private inline function set_G(f: Float): Float {
		this = (Math.round(A * 255) << 24) | (Math.round(R * 255) << 16) | (Math.round(f * 255) << 8) | Math.round(B * 255);
		return f;
	}

	private inline function set_B(f: Float): Float {
		this = (Math.round(A * 255) << 24) | (Math.round(R * 255) << 16) | (Math.round(G * 255) << 8) | Math.round(f * 255);
		return f;
	}

	private inline function set_A(f: Float): Float {
		this = (Math.round(f * 255) << 24) | (Math.round(R * 255) << 16) | (Math.round(G * 255) << 8) | Math.round(B * 255);
		return f;
	}
}
