
let _armpack_pos: i32 = 0;

function armpack_decode(b: ArrayBuffer): any {
	_armpack_pos = 0;
	return armpack_read(new DataView(b));
}

function armpack_read_u8(v: DataView): i32 {
	let i: i32 = v.getUint8(_armpack_pos);
	_armpack_pos += 1;
	return i;
}

function armpack_read_i8(v: DataView): i32 {
	let i: i32 = v.getInt8(_armpack_pos);
	_armpack_pos += 1;
	return i;
}

function armpack_read_u16(v: DataView): i32 {
	let i: i32 = v.getUint16(_armpack_pos, true);
	_armpack_pos += 2;
	return i;
}

function armpack_read_i16(v: DataView): i32 {
	let i: i32 = v.getInt16(_armpack_pos, true);
	_armpack_pos += 2;
	return i;
}

function armpack_read_i32(v: DataView): i32 {
	let i: i32 = v.getInt32(_armpack_pos, true);
	_armpack_pos += 4;
	return i;
}

function armpack_read_f32(v: DataView): f32 {
	let f: f32 = v.getFloat32(_armpack_pos, true);
	_armpack_pos += 4;
	return f;
}

function armpack_read_string(v: DataView, len: i32): string {
	let s: string = "";
	for (let i: i32 = 0; i < len; ++i) {
		s += String.fromCharCode(armpack_read_u8(v));
	}
	return s;
}

function armpack_read_buffer(v: DataView, len: i32): ArrayBuffer {
	let b: ArrayBuffer = v.buffer.slice(_armpack_pos, _armpack_pos + len);
	_armpack_pos += len;
	return b;
}

function armpack_read(v: DataView): any {
	let b: i32 = armpack_read_u8(v);
	if (b == 0xc0) {
		return null;
	}
	else if (b == 0xc2) {
		return false;
	}
	else if (b == 0xc3) {
		return true;
	}
	else if (b == 0xc4) {
		return armpack_read_buffer(v, armpack_read_u8(v));
	}
	else if (b == 0xc5) {
		return armpack_read_buffer(v, armpack_read_u16(v));
	}
	else if (b == 0xc6) {
		return armpack_read_buffer(v, armpack_read_i32(v));
	}
	else if (b == 0xca) {
		return armpack_read_f32(v);
	}
	else if (b == 0xcc) {
		return armpack_read_u8(v);
	}
	else if (b == 0xcd) {
		return armpack_read_u16(v);
	}
	else if (b == 0xce) {
		return armpack_read_i32(v);
	}
	else if (b == 0xd0) {
		return armpack_read_i8(v);
	}
	else if (b == 0xd1) {
		return armpack_read_i16(v);
	}
	else if (b == 0xd2) {
		return armpack_read_i32(v);
	}
	else if (b == 0xd9) {
		return armpack_read_string(v, armpack_read_u8(v));
	}
	else if (b == 0xda) {
		return armpack_read_string(v, armpack_read_u16(v));
	}
	else if (b == 0xdb) {
		return armpack_read_string(v, armpack_read_i32(v));
	}
	else if (b == 0xdc) {
		return armpack_read_array(v, armpack_read_u16(v));
	}
	else if (b == 0xdd) {
		return armpack_read_array(v, armpack_read_i32(v));
	}
	else if (b == 0xde) {
		return armpack_read_map(v, armpack_read_u16(v));
	}
	else if (b == 0xdf) {
		return armpack_read_map(v, armpack_read_i32(v));
	}
	else {
		if (b < 0x80) {
			return b; // positive fix num
		}
		else if (b < 0x90) {
			return armpack_read_map(v, (0xf & b)); // fix map
		}
		else if (b < 0xa0) {
			return armpack_read_array(v, (0xf & b)); // fix array
		}
		else if (b < 0xc0) {
			return armpack_read_string(v, 0x1f & b); // fix string
		}
		else if (b > 0xdf) {
			return 0xffffff00 | b; // negative fix num
		}
	}
	return null;
}

function armpack_read_array(v: DataView, length: i32): any {
	let b: i32 = armpack_read_u8(v);

	if (b == 0xca) { // Typed float32
		let a: Float32Array = new Float32Array(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_f32(v);
		}
		return a;
	}
	else if (b == 0xd2) { // Typed int32
		let a: Uint32Array = new Uint32Array(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_i32(v);
		}
		return a;
	}
	else if (b == 0xd1) { // Typed int16
		let a: Int16Array = new Int16Array(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_i16(v);
		}
		return a;
	}
	else if (b == 0xc4) { // Typed uint8
		let a: Uint8Array = new Uint8Array(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_u8(v);
		}
		return a;
	}
	else { // any type-value
		_armpack_pos--;
		let a: any[] = [];
		for (let x: i32 = 0; x < length; ++x) {
			a.push(armpack_read(v));
		}
		return a;
	}
}

function armpack_read_map(v: DataView, length: i32): any {
	let out: any = {};
	for (let n: i32 = 0; n < length; ++n) {
		let k: string = String(armpack_read(v));
		let val: any = armpack_read(v);
		out[k] = val;
	}
	return out;
}

function armpack_encode(d: any): ArrayBuffer {
	_armpack_pos = 0;
	armpack_write_dummy(d);
	let b: ArrayBuffer = new ArrayBuffer(_armpack_pos);
	let v: DataView = new DataView(b);
	_armpack_pos = 0;
	armpack_write(v, d);
	return b;
}

function armpack_write_u8(v: DataView, i: i32) {
	v.setUint8(_armpack_pos, i);
	_armpack_pos += 1;
}

function armpack_write_i16(v: DataView, i: i32) {
	v.setInt16(_armpack_pos, i, true);
	_armpack_pos += 2;
}

function armpack_write_i32(v: DataView, i: i32) {
	v.setInt32(_armpack_pos, i, true);
	_armpack_pos += 4;
}

function armpack_write_f32(v: DataView, f: f32) {
	v.setFloat32(_armpack_pos, f, true);
	_armpack_pos += 4;
}

function armpack_write_string(v: DataView, str: string) {
	for (let i: i32 = 0; i < str.length; ++i) {
		armpack_write_u8(v, str.charCodeAt(i));
	}
}

function armpack_write_buffer(v: DataView, b: ArrayBuffer) {
	let u8: Uint8Array = new Uint8Array(b);
	for (let i: i32 = 0; i < b.byteLength; ++i) {
		armpack_write_u8(v, u8[i]);
	}
}

function armpack_write(v: DataView, d: any) {
	if (d == null) {
		armpack_write_u8(v, 0xc0);
	}
	else if (typeof d == "boolean") {
		armpack_write_u8(v, d ? 0xc3 : 0xc2);
	}
	else if (typeof d == "number") {
		if (Number.isInteger(d)) {
			armpack_write_u8(v, 0xd2);
			armpack_write_i32(v, d);
		}
		else {
			armpack_write_u8(v, 0xca);
			armpack_write_f32(v, d);
		}
	}
	else if (typeof d == "string") {
		armpack_write_u8(v, 0xdb);
		armpack_write_i32(v, d.length);
		armpack_write_string(v, d);
	}
	else if (d.constructor == ArrayBuffer) {
		armpack_write_u8(v, 0xc6);
		armpack_write_i32(v, d.byteLength);
		armpack_write_buffer(v, d);
	}
	else if (ArrayBuffer.isView(d)) {
		armpack_write_u8(v, 0xdd);
		armpack_write_i32(v, (d as any).length);
		if (d.constructor == Uint8Array) {
			armpack_write_u8(v, 0xc4);
			for (let i: i32 = 0; i < (d as Uint8Array).length; ++i) {
				armpack_write_u8(v, d[i]);
			}
		}
		else if (d.constructor == Int16Array) {
			armpack_write_u8(v, 0xd1);
			for (let i: i32 = 0; i < (d as Int16Array).length; ++i) {
				armpack_write_i16(v, d[i]);
			}
		}
		else if (d.constructor == Float32Array) {
			armpack_write_u8(v, 0xca);
			for (let i: i32 = 0; i < (d as Float32Array).length; ++i) {
				armpack_write_f32(v, d[i]);
			}
		}
		else if (d.constructor == Uint32Array) {
			armpack_write_u8(v, 0xd2);
			for (let i: i32 = 0; i < (d as Uint32Array).length; ++i) {
				armpack_write_i32(v, d[i]);
			}
		}
	}
	else if (Array.isArray(d)) {
		armpack_write_u8(v, 0xdd);
		armpack_write_i32(v, d.length);
		for (let i: i32 = 0; i < d.length; ++i) {
			armpack_write(v, d[i]);
		}
	}
	else {
		armpack_write_object(v, d);
	}
}

function armpack_write_object(v: DataView, d: any) {
	let f = Object.keys(d);
	armpack_write_u8(v, 0xdf);
	armpack_write_i32(v, f.length );
	for (let k of f) {
		armpack_write_u8(v, 0xdb);
		armpack_write_i32(v, k.length);
		armpack_write_string(v, k);
		armpack_write(v, d[k]);
	}
}

function armpack_write_dummy(d: any) {
	if (d == null) {
		_armpack_pos += 1;
	}
	else if (typeof d == "boolean") {
		_armpack_pos += 1;
	}
	else if (typeof d == "number") {
		_armpack_pos += 1;
		_armpack_pos += 4;
	}
	else if (typeof d == "string") {
		_armpack_pos += 1;
		_armpack_pos += 4;
		_armpack_pos += d.length;
	}
	else if (d.constructor == ArrayBuffer) {
		_armpack_pos += 1;
		_armpack_pos += 4;
		_armpack_pos += d.byteLength;
	}
	else if (ArrayBuffer.isView(d)) {
		_armpack_pos += 1;
		_armpack_pos += 4;
		if (d.constructor == Uint8Array) {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as Uint8Array).length; ++i) {
				_armpack_pos += 1;
			}
		}
		else if (d.constructor == Int16Array) {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as Int16Array).length; ++i) {
				_armpack_pos += 2;
			}
		}
		else if (d.constructor == Float32Array) {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as Float32Array).length; ++i) {
				_armpack_pos += 4;
			}
		}
		else if (d.constructor == Uint32Array) {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as Uint32Array).length; ++i) {
				_armpack_pos += 4;
			}
		}
	}
	else if (Array.isArray(d)) {
		_armpack_pos += 1;
		_armpack_pos += 4;
		for (let i: i32 = 0; i < d.length; ++i) {
			armpack_write_dummy(d[i]);
		}
	}
	else {
		armpack_write_object_dummy(d);
	}
}

function armpack_write_object_dummy(d: any) {
	let f = Object.keys(d);
	_armpack_pos += 1;
	_armpack_pos += 4;
	for (let k of f) {
		_armpack_pos += 1;
		_armpack_pos += 4;
		_armpack_pos += k.length;
		armpack_write_dummy(d[k]);
	}
}
