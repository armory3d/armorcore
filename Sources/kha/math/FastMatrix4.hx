package kha.math;

class FastMatrix4 {

	public var _00: Float; public var _10: Float; public var _20: Float; public var _30: Float;
	public var _01: Float; public var _11: Float; public var _21: Float; public var _31: Float;
	public var _02: Float; public var _12: Float; public var _22: Float; public var _32: Float;
	public var _03: Float; public var _13: Float; public var _23: Float; public var _33: Float;

	public inline function new(_00: Float, _10: Float, _20: Float, _30: Float,
								_01: Float, _11: Float, _21: Float, _31: Float,
								_02: Float, _12: Float, _22: Float, _32: Float,
								_03: Float, _13: Float, _23: Float, _33: Float) {
		this._00 = _00; this._10 = _10; this._20 = _20; this._30 = _30;
		this._01 = _01; this._11 = _11; this._21 = _21; this._31 = _31;
		this._02 = _02; this._12 = _12; this._22 = _22; this._32 = _32;
		this._03 = _03; this._13 = _13; this._23 = _23; this._33 = _33;
	}

	@:extern public static inline function identity(): FastMatrix4 {
		return new FastMatrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		);
	}

	@:extern public static inline function rotationZ(alpha: Float): FastMatrix4 {
		var ca = Math.cos(alpha);
		var sa = Math.sin(alpha);
		return new FastMatrix4(
			ca, -sa, 0, 0,
			sa,  ca, 0, 0,
			0,   0, 1, 0,
			0,   0, 0, 1
		);
	}

	@:extern public inline function cofactor(m0: Float, m1: Float, m2: Float,
											m3: Float, m4: Float, m5: Float,
											m6: Float, m7: Float, m8: Float): Float {
		return m0 * ( m4 * m8 - m5 * m7 ) - m1 * ( m3 * m8 - m5 * m6 ) + m2 * ( m3 * m7 - m4 * m6 );
	}

	@:extern public inline function determinant(): Float {
		var c00 = cofactor(_11, _21, _31, _12, _22, _32, _13, _23, _33);
		var c01 = cofactor(_10, _20, _30, _12, _22, _32, _13, _23, _33);
		var c02 = cofactor(_10, _20, _30, _11, _21, _31, _13, _23, _33);
		var c03 = cofactor(_10, _20, _30, _11, _21, _31, _12, _22, _32);
		return _00 * c00 - _01 * c01 + _02 * c02 - _03 * c03;
	}

}
