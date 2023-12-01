package iron.math;

class Mat3 {

	public var self: kha.math.FastMatrix3;

	public inline function new(_00: Float, _10: Float, _20: Float,
							   _01: Float, _11: Float, _21: Float,
							   _02: Float, _12: Float, _22: Float) {
		self = new kha.math.FastMatrix3(_00, _10, _20, _01, _11, _21, _02, _12, _22);
	}

	public static inline function identity(): Mat3 {
		return new Mat3(
			1, 0, 0,
			0, 1, 0,
			0, 0, 1
		);
	}

	public inline function setFrom4(m: Mat4) {
		_00 = m._00;
		_01 = m._01;
		_02 = m._02;
		_10 = m._10;
		_11 = m._11;
		_12 = m._12;
		_20 = m._20;
		_21 = m._21;
		_22 = m._22;
	}

	public var _00(get, set): Float; inline function get__00(): Float { return self._00; } inline function set__00(f: Float): Float { return self._00 = f; }
	public var _01(get, set): Float; inline function get__01(): Float { return self._01; } inline function set__01(f: Float): Float { return self._01 = f; }
	public var _02(get, set): Float; inline function get__02(): Float { return self._02; } inline function set__02(f: Float): Float { return self._02 = f; }
	public var _10(get, set): Float; inline function get__10(): Float { return self._10; } inline function set__10(f: Float): Float { return self._10 = f; }
	public var _11(get, set): Float; inline function get__11(): Float { return self._11; } inline function set__11(f: Float): Float { return self._11 = f; }
	public var _12(get, set): Float; inline function get__12(): Float { return self._12; } inline function set__12(f: Float): Float { return self._12 = f; }
	public var _20(get, set): Float; inline function get__20(): Float { return self._20; } inline function set__20(f: Float): Float { return self._20 = f; }
	public var _21(get, set): Float; inline function get__21(): Float { return self._21; } inline function set__21(f: Float): Float { return self._21 = f; }
	public var _22(get, set): Float; inline function get__22(): Float { return self._22; } inline function set__22(f: Float): Float { return self._22 = f; }
}
