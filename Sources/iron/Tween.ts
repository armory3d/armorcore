
class Tween {

	static DEFAULT_OVERSHOOT: f32 = 1.70158;

	static anims: TAnim[] = [];

	static registered = false;
	static register = () => {
		Tween.registered = true;
		App.notifyOnUpdate(Tween.update);
		App.notifyOnReset(() => { App.notifyOnUpdate(Tween.update); Tween.reset(); });
	}

	static to = (anim: TAnim): TAnim => {
		if (!Tween.registered) Tween.register();
		anim._time = 0;
		anim.isPlaying = (anim.delay != null && anim.delay > 0.0) ? false : true;

		if (anim.ease == null) anim.ease = Ease.Linear;

		if (anim.target != null && anim.props != null) {

			anim._comps = []; anim._x = []; anim._y = []; anim._z = []; anim._w = []; anim._normalize = [];
			for (let p in anim.props) {
				let val: any = anim.target[p];
				if (val.constructor == TVec4 || val.constructor == TQuat) {
					anim._comps.push(4);
					anim._x.push(val.x);
					anim._y.push(val.y);
					anim._z.push(val.z);
					anim._w.push(val.w);
					anim._normalize.push(val.constructor == TQuat);
				}
				else {
					anim._comps.push(1);
					anim._x.push(val);
					anim._y.push(0);
					anim._z.push(0);
					anim._w.push(0);
				}
			}
		}

		Tween.anims.push(anim);
		return anim;
	}

	static timer = (delay: f32, done: ()=>void): TAnim => {
		return Tween.to({ target: null, props: null, duration: 0, delay: delay, done: done });
	}

	static stop = (anim: TAnim) => {
		anim.isPlaying = false;
		array_remove(Tween.anims, anim);
	}

	static reset = () => {
		Tween.anims = [];
	}

	static update = () => {
		let d = Time.delta;
		let i = Tween.anims.length;
		while (i-- > 0 && Tween.anims.length > 0) {
			let a = Tween.anims[i];

			if (a.delay > 0) { // Delay
				a.delay -= d;
				if (a.delay > 0) continue;
			}

			a._time += d;
			a.isPlaying = a._time < a.duration;

			if (a.target != null) {

				if (a.target.constructor == TransformRaw) a.target.dirty = true;

				// Way too much Reflect trickery..
				let ps = Object.keys(a.props);
				for (let i = 0; i < ps.length; ++i) {
					let p = ps[i];
					let k = a._time / a.duration;
					if (k > 1) k = 1;

					if (a._comps[i] == 1) {
						let fromVal: f32 = a._x[i];
						let toVal: f32 = a.props[p];
						let val: f32 = fromVal + (toVal - fromVal) * Tween.eases[a.ease](k);
						a.target[p] = val;
					}
					else { // _comps[i] == 4
						let obj = a.props[p];
						let toX: f32 = obj["x"];
						let toY: f32 = obj["y"];
						let toZ: f32 = obj["z"];
						let toW: f32 = obj["w"];
						if (a._normalize[i]) {
							let qdot = (a._x[i] * toX) + (a._y[i] * toY) + (a._z[i] * toZ) + (a._w[i] * toW);
							if (qdot < 0.0) {
								toX = -toX; toY = -toY; toZ = -toZ; toW = -toW;
							}
						}
						let x: f32 = a._x[i] + (toX - a._x[i]) * Tween.eases[a.ease](k);
						let y: f32 = a._y[i] + (toY - a._y[i]) * Tween.eases[a.ease](k);
						let z: f32 = a._z[i] + (toZ - a._z[i]) * Tween.eases[a.ease](k);
						let w: f32 = a._w[i] + (toW - a._w[i]) * Tween.eases[a.ease](k);
						if (a._normalize[i]) {
							let l = Math.sqrt(x * x + y * y + z * z + w * w);
							if (l > 0.0) {
								l = 1.0 / l;
								x *= l; y *= l; z *= l; w *= l;
							}
						}
						let t = a.target[p];
						t["x"] = x;
						t["y"] = y;
						t["z"] = z;
						t["w"] = w;
					}
				}
			}

			if (a.isPlaying) {
				if (a.tick != null) a.tick();
			}
			else {
				Tween.anims.splice(i, 1);
				i--;
				a.isPlaying = false;
				if (a.done != null) a.done();
			}
		}
	}

	static easeLinear = (k: f32): f32 => { return k; }
	static easeSineIn = (k: f32): f32 => { if (k == 0) { return 0; } else if (k == 1) { return 1; } else { return 1 - Math.cos(k * Math.PI / 2); } }
	static easeSineOut = (k: f32): f32 => { if (k == 0) { return 0; } else if (k == 1) { return 1; } else { return Math.sin(k * (Math.PI * 0.5)); } }
	static easeSineInOut = (k: f32): f32 => { if (k == 0) { return 0; } else if (k == 1) { return 1; } else { return -0.5 * (Math.cos(Math.PI * k) - 1); } }
	static easeQuadIn = (k: f32): f32 => { return k * k; }
	static easeQuadOut = (k: f32): f32 => { return -k * (k - 2); }
	static easeQuadInOut = (k: f32): f32 => { return (k < 0.5) ? 2 * k * k : -2 * ((k -= 1) * k) + 1; }
	static easeCubicIn = (k: f32): f32 => { return k * k * k; }
	static easeCubicOut = (k: f32): f32 => { return (k = k - 1) * k * k + 1; }
	static easeCubicInOut = (k: f32): f32 => { return ((k *= 2) < 1) ? 0.5 * k * k * k : 0.5 * ((k -= 2) * k * k + 2); }
	static easeQuartIn = (k: f32): f32 => { return (k *= k) * k; }
	static easeQuartOut = (k: f32): f32 => { return 1 - (k = (k = k - 1) * k) * k; }
	static easeQuartInOut = (k: f32): f32 => { return ((k *= 2) < 1) ? 0.5 * (k *= k) * k : -0.5 * ((k = (k -= 2) * k) * k - 2); }
	static easeQuintIn = (k: f32): f32 => { return k * (k *= k) * k; }
	static easeQuintOut = (k: f32): f32 => { return (k = k - 1) * (k *= k) * k + 1; }
	static easeQuintInOut = (k: f32): f32 => { return ((k *= 2) < 1) ? 0.5 * k * (k *= k) * k : 0.5 * (k -= 2) * (k *= k) * k + 1; }
	static easeExpoIn = (k: f32): f32 => { return k == 0 ? 0 : Math.pow(2, 10 * (k - 1)); }
	static easeExpoOut = (k: f32): f32 => { return k == 1 ? 1 : (1 - Math.pow(2, -10 * k)); }
	static easeExpoInOut = (k: f32): f32 => { if (k == 0) { return 0; } if (k == 1) { return 1; } if ((k /= 1 / 2.0) < 1.0) { return 0.5 * Math.pow(2, 10 * (k - 1)); } return 0.5 * (2 - Math.pow(2, -10 * --k)); }
	static easeCircIn = (k: f32): f32 => { return -(Math.sqrt(1 - k * k) - 1); }
	static easeCircOut = (k: f32): f32 => { return Math.sqrt(1 - (k - 1) * (k - 1)); }
	static easeCircInOut = (k: f32): f32 => { return k <= .5 ? (Math.sqrt(1 - k * k * 4) - 1) / -2 : (Math.sqrt(1 - (k * 2 - 2) * (k * 2 - 2)) + 1) / 2; }
	static easeBackIn = (k: f32): f32 => { if (k == 0) { return 0; } else if (k == 1) { return 1; } else { return k * k * ((Tween.DEFAULT_OVERSHOOT + 1) * k - Tween.DEFAULT_OVERSHOOT); } }
	static easeBackOut = (k: f32): f32 => { if (k == 0) { return 0; } else if (k == 1) { return 1; } else { return ((k = k - 1) * k * ((Tween.DEFAULT_OVERSHOOT + 1) * k + Tween.DEFAULT_OVERSHOOT) + 1); } }
	static easeBackInOut = (k: f32): f32 => { if (k == 0) { return 0; } else if (k == 1) { return 1; } else if ((k *= 2) < 1) { return (0.5 * (k * k * (((Tween.DEFAULT_OVERSHOOT * 1.525) + 1) * k - Tween.DEFAULT_OVERSHOOT * 1.525))); } else { return (0.5 * ((k -= 2) * k * (((Tween.DEFAULT_OVERSHOOT * 1.525) + 1) * k + Tween.DEFAULT_OVERSHOOT * 1.525) + 2)); } }
	static easeBounceIn = (k: f32): f32 => { return 1 - Tween.easeBounceOut(1 - k); }
	static easeBounceOut = (k: f32): f32 => { if (k < (1 / 2.75)) { return 7.5625 * k * k; } else if (k < (2 / 2.75)) { return 7.5625 * (k -= (1.5 / 2.75)) * k + 0.75; } else if (k < (2.5 / 2.75)) { return 7.5625 * (k -= (2.25 / 2.75)) * k + 0.9375; } else { return 7.5625 * (k -= (2.625 / 2.75)) * k + 0.984375; } }
	static easeBounceInOut = (k: f32): f32 => { return (k < 0.5) ? Tween.easeBounceIn(k * 2) * 0.5 : Tween.easeBounceOut(k * 2 - 1) * 0.5 + 0.5; }

	static easeElasticIn = (k: f32): f32 => {
		let s: Null<f32> = null;
		let a = 0.1, p = 0.4;
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
			s = p * Math.asin(1 / a) / (2 * Math.PI);
		}
		return -(a * Math.pow(2, 10 * (k -= 1)) * Math.sin((k - s) * (2 * Math.PI) / p));
	}

	static easeElasticOut = (k: f32): f32 => {
		let s: Null<f32> = null;
		let a = 0.1, p = 0.4;
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
			s = p * Math.asin(1 / a) / (2 * Math.PI);
		}
		return (a * Math.pow(2, -10 * k) * Math.sin((k - s) * (2 * Math.PI) / p) + 1);
	}

	static easeElasticInOut = (k: f32): f32 => {
		let s, a = 0.1, p = 0.4;
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
			s = p * Math.asin(1 / a) / (2 * Math.PI);
		}
		if ((k *= 2) < 1) return - 0.5 * (a * Math.pow(2, 10 * (k -= 1)) * Math.sin((k - s) * (2 * Math.PI) / p));
		return a * Math.pow(2, -10 * (k -= 1)) * Math.sin((k - s) * (2 * Math.PI) / p) * 0.5 + 1;
	}

	static eases: ((f: f32)=>f32)[] = [
		Tween.easeLinear,
		Tween.easeSineIn, Tween.easeSineOut, Tween.easeSineInOut,
		Tween.easeQuadIn, Tween.easeQuadOut, Tween.easeQuadInOut,
		Tween.easeCubicIn, Tween.easeCubicOut, Tween.easeCubicInOut,
		Tween.easeQuartIn, Tween.easeQuartOut, Tween.easeQuartInOut,
		Tween.easeQuintIn, Tween.easeQuintOut, Tween.easeQuintInOut,
		Tween.easeExpoIn, Tween.easeExpoOut, Tween.easeExpoInOut,
		Tween.easeCircIn, Tween.easeCircOut, Tween.easeCircInOut,
		Tween.easeBackIn, Tween.easeBackOut, Tween.easeBackInOut,
		Tween.easeBounceIn, Tween.easeBounceOut, Tween.easeBounceInOut,
		Tween.easeElasticIn, Tween.easeElasticOut, Tween.easeElasticInOut
	];
}

type TAnim = {
	target: any;
	props: any;
	duration: f32;
	isPlaying?: Null<bool>;
	done?: ()=>void;
	tick?: ()=>void;
	delay?: Null<f32>;
	ease?: Null<Ease>;
	// Internal
	_time?: Null<f32>;
	_comps?: i32[];
	_x?: f32[];
	_y?: f32[];
	_z?: f32[];
	_w?: f32[];
	_normalize?: bool[];
}

enum Ease {
	Linear,
	SineIn,
	SineOut,
	SineInOut,
	QuadIn,
	QuadOut,
	QuadInOut,
	CubicIn,
	CubicOut,
	CubicInOut,
	QuartIn,
	QuartOut,
	QuartInOut,
	QuintIn,
	QuintOut,
	QuintInOut,
	ExpoIn,
	ExpoOut,
	ExpoInOut,
	CircIn,
	CircOut,
	CircInOut,
	BackIn,
	BackOut,
	BackInOut,
	BounceIn,
	BounceOut,
	BounceInOut,
	ElasticIn,
	ElasticOut,
	ElasticInOut,
}
