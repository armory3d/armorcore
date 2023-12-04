package iron.math;

class Mat3 {

	public inline function new(_00: Float, _10: Float, _20: Float,
							   _01: Float, _11: Float, _21: Float,
							   _02: Float, _12: Float, _22: Float) {

		this._00 = _00; this._10 = _10; this._20 = _20;
		this._01 = _01; this._11 = _11; this._21 = _21;
		this._02 = _02; this._12 = _12; this._22 = _22;
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

	public static inline function translation(x: Float, y: Float): Mat3 {
		return new Mat3(
			1, 0, x,
			0, 1, y,
			0, 0, 1
		);
	}

	public static inline function rotation(alpha: Float): Mat3 {
		return new Mat3(
			Math.cos(alpha), -Math.sin(alpha), 0,
			Math.sin(alpha), Math.cos(alpha), 0,
			0, 0, 1
		);
	}

	public inline function multmat(m: Mat3): Mat3 {
		return new Mat3(
			_00 * m._00 + _10 * m._01 + _20 * m._02, _00 * m._10 + _10 * m._11 + _20 * m._12, _00 * m._20 + _10 * m._21 + _20 * m._22,
			_01 * m._00 + _11 * m._01 + _21 * m._02, _01 * m._10 + _11 * m._11 + _21 * m._12, _01 * m._20 + _11 * m._21 + _21 * m._22,
			_02 * m._00 + _12 * m._01 + _22 * m._02, _02 * m._10 + _12 * m._11 + _22 * m._12, _02 * m._20 + _12 * m._21 + _22 * m._22
		);
	}

	public var _00: Float;
	public var _01: Float;
	public var _02: Float;
	public var _10: Float;
	public var _11: Float;
	public var _12: Float;
	public var _20: Float;
	public var _21: Float;
	public var _22: Float;
}
