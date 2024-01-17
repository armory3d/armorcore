// Msgpack parser with typed arrays
// Based on https://github.com/aaulia/msgpack-haxe
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

class ArmPack {

	static pos = 0;

	static decode = (b: ArrayBuffer): any => {
		ArmPack.pos = 0;
		return ArmPack.read(new DataView(b));
	}

	static readU8 = (v: DataView): i32 => {
		let i = v.getUint8(ArmPack.pos);
		ArmPack.pos += 1;
		return i;
	}

	static readI8 = (v: DataView): i32 => {
		let i = v.getInt8(ArmPack.pos);
		ArmPack.pos += 1;
		return i;
	}

	static readU16 = (v: DataView): i32 => {
		let i = v.getUint16(ArmPack.pos, true);
		ArmPack.pos += 2;
		return i;
	}

	static readI16 = (v: DataView): i32 => {
		let i = v.getInt16(ArmPack.pos, true);
		ArmPack.pos += 2;
		return i;
	}

	static readI32 = (v: DataView): i32 => {
		let i = v.getInt32(ArmPack.pos, true);
		ArmPack.pos += 4;
		return i;
	}

	static readF32 = (v: DataView): f32 => {
		let f = v.getFloat32(ArmPack.pos, true);
		ArmPack.pos += 4;
		return f;
	}

	static readString = (v: DataView, len: i32): string => {
		let s = "";
		for (let i = 0; i < len; ++i) {
			s += String.fromCharCode(ArmPack.readU8(v));
		}
		return s;
	}

	static readBuffer = (v: DataView, len: i32): ArrayBuffer => {
		let b = v.buffer.slice(ArmPack.pos, ArmPack.pos + len);
		ArmPack.pos += len;
		return b;
	}

	static read = (v: DataView): any => {
		let b = ArmPack.readU8(v);
		switch (b) {
			case 0xc0: return null;
			case 0xc2: return false;
			case 0xc3: return true;
			case 0xc4: return ArmPack.readBuffer(v, ArmPack.readU8(v));
			case 0xc5: return ArmPack.readBuffer(v, ArmPack.readU16(v));
			case 0xc6: return ArmPack.readBuffer(v, ArmPack.readI32(v));
			case 0xca: return ArmPack.readF32(v);
			case 0xcc: return ArmPack.readU8(v);
			case 0xcd: return ArmPack.readU16(v);
			case 0xce: return ArmPack.readI32(v);
			case 0xd0: return ArmPack.readI8(v);
			case 0xd1: return ArmPack.readI16(v);
			case 0xd2: return ArmPack.readI32(v);
			case 0xd9: return ArmPack.readString(v, ArmPack.readU8(v));
			case 0xda: return ArmPack.readString(v, ArmPack.readU16(v));
			case 0xdb: return ArmPack.readString(v, ArmPack.readI32(v));
			case 0xdc: return ArmPack.readArray(v, ArmPack.readU16(v));
			case 0xdd: return ArmPack.readArray(v, ArmPack.readI32(v));
			case 0xde: return ArmPack.readMap(v, ArmPack.readU16(v));
			case 0xdf: return ArmPack.readMap(v, ArmPack.readI32(v));
			default: {
				if (b < 0x80) return b; // positive fix num
				else if (b < 0x90) return ArmPack.readMap(v, (0xf & b)); // fix map
				else if (b < 0xa0) return ArmPack.readArray(v, (0xf & b)); // fix array
				else if (b < 0xc0) return ArmPack.readString(v, 0x1f & b); // fix string
				else if (b > 0xdf) return 0xffffff00 | b; // negative fix num
			}
		}
		return null;
	}

	static readArray = (v: DataView, length: i32): any => {
		let b = ArmPack.readU8(v);

		if (b == 0xca) { // Typed float32
			let a = new Float32Array(length);
			for (let x = 0; x < length; ++x) a[x] = ArmPack.readF32(v);
			return a;
		}
		else if (b == 0xd2) { // Typed int32
			let a = new Uint32Array(length);
			for (let x = 0; x < length; ++x) a[x] = ArmPack.readI32(v);
			return a;
		}
		else if (b == 0xd1) { // Typed int16
			let a = new Int16Array(length);
			for (let x = 0; x < length; ++x) a[x] = ArmPack.readI16(v);
			return a;
		}
		else if (b == 0xc4) { // Typed uint8
			let a = new Uint8Array(length);
			for (let x = 0; x < length; ++x) a[x] = ArmPack.readU8(v);
			return a;
		}
		else { // any type-value
			ArmPack.pos--;
			let a: any[] = [];
			for (let x = 0; x < length; ++x) a.push(ArmPack.read(v));
			return a;
		}
	}

	static readMap = (v: DataView, length: i32): any => {
		let out: any = {};
		for (let n = 0; n < length; ++n) {
			let k = String(ArmPack.read(v));
			let val = ArmPack.read(v);
			out[k] = val;
		}
		return out;
	}

	static encode = (d: any): ArrayBuffer => {
		ArmPack.pos = 0;
		ArmPack.writeDummy(d);
		let b = new ArrayBuffer(ArmPack.pos);
		let v = new DataView(b);
		ArmPack.pos = 0;
		ArmPack.write(v, d);
		return b;
	}

	static writeU8 = (v: DataView, i: i32) => {
		v.setUint8(ArmPack.pos, i);
		ArmPack.pos += 1;
	}

	static writeI16 = (v: DataView, i: i32) => {
		v.setInt16(ArmPack.pos, i, true);
		ArmPack.pos += 2;
	}

	static writeI32 = (v: DataView, i: i32) => {
		v.setInt32(ArmPack.pos, i, true);
		ArmPack.pos += 4;
	}

	static writeF32 = (v: DataView, f: f32) => {
		v.setFloat32(ArmPack.pos, f, true);
		ArmPack.pos += 4;
	}

	static writeString = (v: DataView, str: string) => {
		for (let i = 0; i < str.length; ++i) {
			ArmPack.writeU8(v, str.charCodeAt(i));
		}
	}

	static writeBuffer = (v: DataView, b: ArrayBuffer) => {
		let u8 = new Uint8Array(b);
		for (let i = 0; i < b.byteLength; ++i) {
			ArmPack.writeU8(v, u8[i]);
		}
	}

	static write = (v: DataView, d: any) => {
		if (typeof d == null) {
			ArmPack.writeU8(v, 0xc0);
		}
		else if (typeof d == "boolean") {
			ArmPack.writeU8(v, d ? 0xc3 : 0xc2);
		}
		else if (typeof d == "number") {
			if (Number.isInteger(d)) {
				ArmPack.writeU8(v, 0xd2);
				ArmPack.writeI32(v, d);
			}
			else {
				ArmPack.writeU8(v, 0xca);
				ArmPack.writeF32(v, d);
			}
		}
		else if (typeof d == "string") {
			ArmPack.writeU8(v, 0xdb);
			ArmPack.writeI32(v, d.length);
			ArmPack.writeString(v, d);
		}
		else if (d.constructor == ArrayBuffer) {
			ArmPack.writeU8(v, 0xc6);
			ArmPack.writeI32(v, d.byteLength);
			ArmPack.writeBuffer(v, d);
		}
		else if (ArrayBuffer.isView(d)) {
			ArmPack.writeU8(v, 0xdd);
			ArmPack.writeI32(v, (d as any).length);
			if (d.constructor == Uint8Array) {
				ArmPack.writeU8(v, 0xc4);
				for (let i = 0; i < (d as Uint8Array).length; ++i) ArmPack.writeU8(v, d[i]);
			}
			else if (d.constructor == Int16Array) {
				ArmPack.writeU8(v, 0xd1);
				for (let i = 0; i < (d as Int16Array).length; ++i) ArmPack.writeI16(v, d[i]);
			}
			else if (d.constructor == Float32Array) {
				ArmPack.writeU8(v, 0xca);
				for (let i = 0; i < (d as Float32Array).length; ++i) ArmPack.writeF32(v, d[i]);
			}
			else if (d.constructor == Uint32Array) {
				ArmPack.writeU8(v, 0xd2);
				for (let i = 0; i < (d as Uint32Array).length; ++i) ArmPack.writeI32(v, d[i]);
			}
		}
		else if (Array.isArray(d)) {
			ArmPack.writeU8(v, 0xdd);
			ArmPack.writeI32(v, d.length);
			for (let i = 0; i < d.length; ++i) ArmPack.write(v, d[i]);
		}
		else {
			ArmPack.writeObject(v, d);
		}
	}

	static writeObject = (v: DataView, d: any) => {
		let f = Object.keys(d);
		ArmPack.writeU8(v, 0xdf);
		ArmPack.writeI32(v, f.length );
		for (let k of f) {
			ArmPack.writeU8(v, 0xdb);
			ArmPack.writeI32(v, k.length);
			ArmPack.writeString(v, k);
			ArmPack.write(v, d[k]);
		}
	}

	static writeDummy = (d: any) => {
		if (typeof d == null) {
			ArmPack.pos += 1;
		}
		else if (typeof d == "boolean") {
			ArmPack.pos += 1;
		}
		else if (typeof d == "number") {
			ArmPack.pos += 1;
			ArmPack.pos += 4;
		}
		else if (typeof d == "string") {
			ArmPack.pos += 1;
			ArmPack.pos += 4;
			ArmPack.pos += d.length;
		}
		else if (d.constructor == ArrayBuffer) {
			ArmPack.pos += 1;
			ArmPack.pos += 4;
			ArmPack.pos += d.byteLength;
		}
		else if (ArrayBuffer.isView(d)) {
			ArmPack.pos += 1;
			ArmPack.pos += 4;
			if (d.constructor == Uint8Array) {
				ArmPack.pos += 1;
				for (let i = 0; i < (d as Uint8Array).length; ++i) ArmPack.pos += 1;
			}
			else if (d.constructor == Int16Array) {
				ArmPack.pos += 1;
				for (let i = 0; i < (d as Int16Array).length; ++i) ArmPack.pos += 2;
			}
			else if (d.constructor == Float32Array) {
				ArmPack.pos += 1;
				for (let i = 0; i < (d as Float32Array).length; ++i) ArmPack.pos += 4;
			}
			else if (d.constructor == Uint32Array) {
				ArmPack.pos += 1;
				for (let i = 0; i < (d as Uint32Array).length; ++i) ArmPack.pos += 4;
			}
		}
		else if (Array.isArray(d)) {
			ArmPack.pos += 1;
			ArmPack.pos += 4;
			for (let i = 0; i < d.length; ++i) ArmPack.writeDummy(d[i]);
		}
		else {
			ArmPack.writeObjectDummy(d);
		}
	}

	static writeObjectDummy = (d: any) => {
		let f = Object.keys(d);
		ArmPack.pos += 1;
		ArmPack.pos += 4;
		for (let k of f) {
			ArmPack.pos += 1;
			ArmPack.pos += 4;
			ArmPack.pos += k.length;
			ArmPack.writeDummy(d[k]);
		}
	}
}
