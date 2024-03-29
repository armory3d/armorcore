
///if arm_minits
///else

let _tween_default_overshoot: f32 = 1.70158;
let _tween_anims: tween_anim_t[] = [];
let _tween_registered: bool = false;

function tween_on_reset() {
	app_notify_on_update(tween_update);
	tween_reset();
}

function _tween_register() {
	_tween_registered = true;
	app_notify_on_update(tween_update);
	app_notify_on_reset(tween_on_reset);
}

function tween_to(anim: tween_anim_t): tween_anim_t {
	if (!_tween_registered) {
		_tween_register();
	}
	anim._time = 0;
	anim.is_playing = (anim.delay != null && anim.delay > 0.0) ? false : true;

	if (anim.ease == null) {
		anim.ease = ease_t.LINEAR;
	}

	if (anim.target != null && anim.props != null) {
		anim._comps = [];
		anim._x = [];
		anim._y = [];
		anim._z = [];
		anim._w = [];
		anim._normalize = [];
		let keys: any[] = Object.keys(anim.props);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let p: any = keys[i];
			let val: any = anim.target[p];
			if (val.type == "vec4_t" || val.type == "quat_t") {
				array_push(anim._comps, 4);
				array_push(anim._x, val.x);
				array_push(anim._y, val.y);
				array_push(anim._z, val.z);
				array_push(anim._w, val.w);
				array_push(anim._normalize, val.type == "quat_t");
			}
			else {
				array_push(anim._comps, 1);
				array_push(anim._x, val);
				array_push(anim._y, 0);
				array_push(anim._z, 0);
				array_push(anim._w, 0);
			}
		}
	}

	array_push(_tween_anims, anim);
	return anim;
}

function tween_timer(delay: f32, done: ()=>void): tween_anim_t {
	return tween_to({ target: null, props: null, duration: 0, delay: delay, done: done });
}

function tween_stop(anim: tween_anim_t) {
	anim.is_playing = false;
	array_remove(_tween_anims, anim);
}

function tween_reset() {
	_tween_anims = [];
}

function tween_update() {
	let d: f32 = time_delta();
	let i: i32 = _tween_anims.length;
	while (i-- > 0 && _tween_anims.length > 0) {
		let a: tween_anim_t = _tween_anims[i];

		if (a.delay > 0) { // Delay
			a.delay -= d;
			if (a.delay > 0) {
				continue;
			}
		}

		a._time += d;
		a.is_playing = a._time < a.duration;

		if (a.target != null) {

			if (a.target.type == "transform_t") {
				a.target.dirty = true;
			}

			let ps: string[] = Object.keys(a.props);
			for (let i: i32 = 0; i < ps.length; ++i) {
				let p: string = ps[i];
				let k: f32 = a._time / a.duration;
				if (k > 1) {
					k = 1;
				}

				if (a._comps[i] == 1) {
					let from_val: f32 = a._x[i];
					let to_val: f32 = a.props[p];
					let val: f32 = from_val + (to_val - from_val) * _tween_ease(a.ease, k);
					a.target[p] = val;
				}
				else { // _comps[i] == 4
					let obj: any = a.props[p];
					let to_x: f32 = obj["x"];
					let to_y: f32 = obj["y"];
					let to_z: f32 = obj["z"];
					let to_w: f32 = obj["w"];
					if (a._normalize[i]) {
						let qdot: f32 = (a._x[i] * to_x) + (a._y[i] * to_y) + (a._z[i] * to_z) + (a._w[i] * to_w);
						if (qdot < 0.0) {
							to_x = -to_x; to_y = -to_y; to_z = -to_z; to_w = -to_w;
						}
					}
					let x: f32 = a._x[i] + (to_x - a._x[i]) * _tween_ease(a.ease, k);
					let y: f32 = a._y[i] + (to_y - a._y[i]) * _tween_ease(a.ease, k);
					let z: f32 = a._z[i] + (to_z - a._z[i]) * _tween_ease(a.ease, k);
					let w: f32 = a._w[i] + (to_w - a._w[i]) * _tween_ease(a.ease, k);
					if (a._normalize[i]) {
						let l: i32 = math_sqrt(x * x + y * y + z * z + w * w);
						if (l > 0.0) {
							l = 1.0 / l;
							x *= l; y *= l; z *= l; w *= l;
						}
					}
					let t: any = a.target[p];
					t["x"] = x;
					t["y"] = y;
					t["z"] = z;
					t["w"] = w;
				}
			}
		}

		if (a.is_playing) {
			if (a.tick != null) {
				a.tick();
			}
		}
		else {
			array_splice(_tween_anims, i, 1);
			i--;
			a.is_playing = false;
			if (a.done != null) {
				a.done();
			}
		}
	}
}

function _tween_ease_linear(k: f32): f32 { return k; }
function _tween_ease_quad_in(k: f32): f32 { return k * k; }
function _tween_ease_quad_out(k: f32): f32 { return -k * (k - 2); }
function _tween_ease_quad_in_out(k: f32): f32 { return (k < 0.5) ? 2 * k * k : -2 * ((k -= 1) * k) + 1; }
function _tween_ease_expo_in(k: f32): f32 { return k == 0 ? 0 : math_pow(2, 10 * (k - 1)); }
function _tween_ease_expo_out(k: f32): f32 { return k == 1 ? 1 : (1 - math_pow(2, -10 * k)); }
function _tween_ease_expo_in_out(k: f32): f32 { if (k == 0) { return 0; } if (k == 1) { return 1; } if ((k /= 1 / 2.0) < 1.0) { return 0.5 * math_pow(2, 10 * (k - 1)); } return 0.5 * (2 - math_pow(2, -10 * --k)); }
function _tween_ease_bounce_in(k: f32): f32 { return 1 - _tween_ease_bounce_out(1 - k); }
function _tween_ease_bounce_out(k: f32): f32 { if (k < (1 / 2.75)) { return 7.5625 * k * k; } else if (k < (2 / 2.75)) { return 7.5625 * (k -= (1.5 / 2.75)) * k + 0.75; } else if (k < (2.5 / 2.75)) { return 7.5625 * (k -= (2.25 / 2.75)) * k + 0.9375; } else { return 7.5625 * (k -= (2.625 / 2.75)) * k + 0.984375; } }
function _tween_ease_bounce_in_out(k: f32): f32 { return (k < 0.5) ? _tween_ease_bounce_in(k * 2) * 0.5 : _tween_ease_bounce_out(k * 2 - 1) * 0.5 + 0.5; }

function _tween_ease_elastic_in(k: f32): f32 {
	let s: f32;
	let a: i32 = 0.1;
	let p: i32 = 0.4;
	if (k == 0) {
		return 0;
	}
	if (k == 1) {
		return 1;
	}
	if (a < 1) {
		a = 1;
		s = p / 4;
	}
	else {
		s = p * math_asin(1 / a) / (2 * math_pi());
	}
	return -(a * math_pow(2, 10 * (k -= 1)) * math_sin((k - s) * (2 * math_pi()) / p));
}

function _tween_ease_elastic_out(k: f32): f32 {
	let s: f32;
	let a: i32 = 0.1;
	let p: i32 = 0.4;
	if (k == 0) {
		return 0;
	}
	if (k == 1) {
		return 1;
	}
	if (a < 1) {
		a = 1;
		s = p / 4;
	}
	else {
		s = p * math_asin(1 / a) / (2 * math_pi());
	}
	return (a * math_pow(2, -10 * k) * math_sin((k - s) * (2 * math_pi()) / p) + 1);
}

function _tween_ease_elastic_in_out(k: f32): f32 {
	let s: i32;
	let a: i32 = 0.1;
	let p: i32 = 0.4;
	if (k == 0) {
		return 0;
	}
	if (k == 1) {
		return 1;
	}
	if (a != 0 || a < 1) {
		a = 1;
		s = p / 4;
	}
	else {
		s = p * math_asin(1 / a) / (2 * math_pi());
	}
	if ((k *= 2) < 1) {
		return - 0.5 * (a * math_pow(2, 10 * (k -= 1)) * math_sin((k - s) * (2 * math_pi()) / p));
	}
	return a * math_pow(2, -10 * (k -= 1)) * math_sin((k - s) * (2 * math_pi()) / p) * 0.5 + 1;
}

function _tween_ease(ease: ease_t, k: f32): f32 {
	if (ease == ease_t.LINEAR) {
		return _tween_ease_linear(k);
	}
	if (ease == ease_t.QUAD_IN) {
		return _tween_ease_quad_in(k);
	}
	if (ease == ease_t.QUAD_OUT) {
		return _tween_ease_quad_out(k);
	}
	if (ease == ease_t.QUAD_IN_OUT) {
		return _tween_ease_quad_in_out(k);
	}
	if (ease == ease_t.EXPO_IN) {
		return _tween_ease_expo_in(k);
	}
	if (ease == ease_t.EXPO_OUT) {
		return _tween_ease_expo_out(k);
	}
	if (ease == ease_t.EXPO_IN_OUT) {
		return _tween_ease_expo_in_out(k);
	}
	if (ease == ease_t.BOUNCE_IN) {
		return _tween_ease_bounce_in(k);
	}
	if (ease == ease_t.BOUNCE_OUT) {
		return _tween_ease_bounce_out(k);
	}
	if (ease == ease_t.BOUNCE_IN_OUT) {
		return _tween_ease_bounce_in_out(k);
	}
	if (ease == ease_t.ELASTIC_IN) {
		return _tween_ease_elastic_in(k);
	}
	if (ease == ease_t.ELASTIC_OUT) {
		return _tween_ease_elastic_out(k);
	}
	if (ease == ease_t.ELASTIC_IN_OUT) {
		return _tween_ease_elastic_in_out(k);
	}
	return 0.0;
}

type tween_anim_t = {
	// Base
	target?: any;
	props?: any;
	duration?: f32;
	// Opt
	is_playing?: bool;
	done?: ()=>void;
	tick?: ()=>void;
	delay?: f32;
	ease?: ease_t;
	// Internal
	_time?: f32;
	_comps?: i32[];
	_x?: f32[];
	_y?: f32[];
	_z?: f32[];
	_w?: f32[];
	_normalize?: bool[];
};

enum ease_t {
	LINEAR,
	QUAD_IN,
	QUAD_OUT,
	QUAD_IN_OUT,
	EXPO_IN,
	EXPO_OUT,
	EXPO_IN_OUT,
	BOUNCE_IN,
	BOUNCE_OUT,
	BOUNCE_IN_OUT,
	ELASTIC_IN,
	ELASTIC_OUT,
	ELASTIC_IN_OUT,
}

///end
