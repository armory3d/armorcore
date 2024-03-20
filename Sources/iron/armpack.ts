
///if arm_minits
///else

let _armpack_pos: i32 = 0;

function armpack_decode(b: buffer_t): any {
	_armpack_pos = 0;
	return armpack_read(buffer_view_create(b));
}

function armpack_read_u8(v: buffer_view_t): i32 {
	let i: i32 = buffer_view_get_u8(v, _armpack_pos);
	_armpack_pos += 1;
	return i;
}

function armpack_read_i8(v: buffer_view_t): i32 {
	let i: i32 = buffer_view_get_i8(v, _armpack_pos);
	_armpack_pos += 1;
	return i;
}

function armpack_read_u16(v: buffer_view_t): i32 {
	let i: i32 = buffer_view_get_u16(v, _armpack_pos);
	_armpack_pos += 2;
	return i;
}

function armpack_read_i16(v: buffer_view_t): i32 {
	let i: i32 = buffer_view_get_i16(v, _armpack_pos);
	_armpack_pos += 2;
	return i;
}

function armpack_read_i32(v: buffer_view_t): i32 {
	let i: i32 = buffer_view_get_i32(v, _armpack_pos);
	_armpack_pos += 4;
	return i;
}

function armpack_read_f32(v: buffer_view_t): f32 {
	let f: f32 = buffer_view_get_f32(v, _armpack_pos);
	_armpack_pos += 4;
	return f;
}

function armpack_read_string(v: buffer_view_t, len: i32): string {
	let s: string = "";
	for (let i: i32 = 0; i < len; ++i) {
		s += string_from_char_code(armpack_read_u8(v));
	}
	return s;
}

function armpack_read_buffer(v: buffer_view_t, len: i32): buffer_t {
	let b: buffer_t = buffer_slice(v.buffer, _armpack_pos, _armpack_pos + len);
	_armpack_pos += len;
	return b;
}

function armpack_read(v: buffer_view_t): any {
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

function armpack_read_array(v: buffer_view_t, length: i32): any {
	let b: i32 = armpack_read_u8(v);

	if (b == 0xca) { // Typed float32
		let a: f32_array_t = f32_array_create(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_f32(v);
		}
		return a;
	}
	else if (b == 0xd2) { // Typed int32
		let a: u32_array_t = u32_array_create(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_i32(v);
		}
		return a;
	}
	else if (b == 0xd1) { // Typed int16
		let a: i16_array_t = i16_array_create(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_i16(v);
		}
		return a;
	}
	else if (b == 0xc4) { // Typed uint8
		let a: u8_array_t = u8_array_create(length);
		for (let x: i32 = 0; x < length; ++x) {
			a[x] = armpack_read_u8(v);
		}
		return a;
	}
	else { // any type-value
		_armpack_pos--;
		let a: any[] = [];
		for (let x: i32 = 0; x < length; ++x) {
			array_push(a, armpack_read(v));
		}
		return a;
	}
}

function armpack_read_map(v: buffer_view_t, length: i32): any {
	let out: any = {};
	for (let n: i32 = 0; n < length; ++n) {
		let k: string = any_to_string(armpack_read(v));
		let val: any = armpack_read(v);
		out[k] = val;
	}
	return out;
}

function armpack_encode(d: any): buffer_t {
	_armpack_pos = 0;
	armpack_write_dummy(d);
	let b: buffer_t = buffer_create(_armpack_pos);
	let v: buffer_view_t = buffer_view_create(b);
	_armpack_pos = 0;
	armpack_write(v, d);
	return b;
}

function armpack_write_u8(v: buffer_view_t, i: i32) {
	buffer_view_set_u8(v, _armpack_pos, i);
	_armpack_pos += 1;
}

function armpack_write_i16(v: buffer_view_t, i: i32) {
	buffer_view_set_i16(v, _armpack_pos, i);
	_armpack_pos += 2;
}

function armpack_write_i32(v: buffer_view_t, i: i32) {
	buffer_view_set_i32(v, _armpack_pos, i);
	_armpack_pos += 4;
}

function armpack_write_f32(v: buffer_view_t, f: f32) {
	buffer_view_set_f32(v, _armpack_pos, f);
	_armpack_pos += 4;
}

function armpack_write_string(v: buffer_view_t, str: string) {
	for (let i: i32 = 0; i < str.length; ++i) {
		armpack_write_u8(v, char_code_at(str, i));
	}
}

function armpack_write_buffer(v: buffer_view_t, b: buffer_t) {
	let u8: u8_array_t = u8_array_create_from_buffer(b);
	for (let i: i32 = 0; i < buffer_size(b); ++i) {
		armpack_write_u8(v, u8[i]);
	}
}

function armpack_write(v: buffer_view_t, d: any) {
	if (d == null) {
		armpack_write_u8(v, 0xc0);
	}
	else if (typeof d == "boolean") {
		armpack_write_u8(v, d ? 0xc3 : 0xc2);
	}
	else if (typeof d == "number") {
		if (is_integer(d)) {
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
	else if (d.constructor.name == "buffer_t") {
		armpack_write_u8(v, 0xc6);
		armpack_write_i32(v, buffer_size(d));
		armpack_write_buffer(v, d);
	}
	else if (is_view(d)) {
		armpack_write_u8(v, 0xdd);
		armpack_write_i32(v, (d as any).length);
		if (d.constructor.name == "u8_array_t") {
			armpack_write_u8(v, 0xc4);
			for (let i: i32 = 0; i < (d as u8_array_t).length; ++i) {
				armpack_write_u8(v, d[i]);
			}
		}
		else if (d.constructor.name == "i16_array_t") {
			armpack_write_u8(v, 0xd1);
			for (let i: i32 = 0; i < (d as i16_array_t).length; ++i) {
				armpack_write_i16(v, d[i]);
			}
		}
		else if (d.constructor.name == "f32_array_t") {
			armpack_write_u8(v, 0xca);
			for (let i: i32 = 0; i < (d as f32_array_t).length; ++i) {
				armpack_write_f32(v, d[i]);
			}
		}
		else if (d.constructor.name == "i32_array_t") {
			armpack_write_u8(v, 0xd2);
			for (let i: i32 = 0; i < (d as i32_array_t).length; ++i) {
				armpack_write_i32(v, d[i]);
			}
		}
		else if (d.constructor.name == "u32_array_t") {
			armpack_write_u8(v, 0xd2);
			for (let i: i32 = 0; i < (d as u32_array_t).length; ++i) {
				armpack_write_i32(v, d[i]);
			}
		}
	}
	else if (is_array(d)) {
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

function armpack_write_object(v: buffer_view_t, d: any) {
	let f: string[] = Object.keys(d);
	armpack_write_u8(v, 0xdf);
	armpack_write_i32(v, f.length);
	for (let i: i32 = 0; i < f.length; ++i) {
		let k: string = f[i];
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
	else if (d.constructor.name == "buffer_t") {
		_armpack_pos += 1;
		_armpack_pos += 4;
		_armpack_pos += buffer_size(d);
	}
	else if (is_view(d)) {
		_armpack_pos += 1;
		_armpack_pos += 4;
		if (d.constructor.name == "u8_array_t") {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as u8_array_t).length; ++i) {
				_armpack_pos += 1;
			}
		}
		else if (d.constructor.name == "i16_array_t") {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as i16_array_t).length; ++i) {
				_armpack_pos += 2;
			}
		}
		else if (d.constructor.name == "f32_array_t") {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as f32_array_t).length; ++i) {
				_armpack_pos += 4;
			}
		}
		else if (d.constructor.name == "i32_array_t") {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as i32_array_t).length; ++i) {
				_armpack_pos += 4;
			}
		}
		else if (d.constructor.name == "u32_array_t") {
			_armpack_pos += 1;
			for (let i: i32 = 0; i < (d as u32_array_t).length; ++i) {
				_armpack_pos += 4;
			}
		}
	}
	else if (is_array(d)) {
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
	let f: string[] = Object.keys(d);
	_armpack_pos += 1;
	_armpack_pos += 4;
	for (let i: i32 = 0; i < f.length; ++i) {
		let k: string = f[i];
		_armpack_pos += 1;
		_armpack_pos += 4;
		_armpack_pos += k.length;
		armpack_write_dummy(d[k]);
	}
}

///end
