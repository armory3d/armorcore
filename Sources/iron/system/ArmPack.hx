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
package iron.system;

import js.lib.ArrayBuffer;
import js.lib.DataView;
import js.lib.Float32Array;
import js.lib.Uint32Array;
import js.lib.Int16Array;
import js.lib.Uint8Array;

class ArmPack {

	static var pos = 0;

	public static function decode<T>(b: ArrayBuffer): T {
		pos = 0;
		return read(new DataView(b));
	}

	static function readU8(v: DataView): Int {
		var i = v.getUint8(pos);
		pos++;
		return i;
	}

	static function readI8(v: DataView): Int {
		var i = v.getInt8(pos);
		pos++;
		return i;
	}

	static function readU16(v: DataView): Int {
		var i = v.getUint16(pos, true);
		pos += 2;
		return i;
	}

	static function readI16(v: DataView): Int {
		var i = v.getInt16(pos, true);
		pos += 2;
		return i;
	}

	static function readI32(v: DataView): Int {
		var i = v.getInt32(pos, true);
		pos += 4;
		return i;
	}

	static function readF32(v: DataView): Float {
		var f = v.getFloat32(pos, true);
		pos += 4;
		return f;
	}

	static function readString(v: DataView, len: Int): String {
		var s = "";
		for (i in 0...len) {
			s += String.fromCharCode(readU8(v));
		}
		return s;
	}

	static function readBuffer(v: DataView, len: Int): ArrayBuffer {
		var b = v.buffer.slice(pos, pos + len);
		pos += len;
		return b;
	}

	static function read(v: DataView): Any {
		var b = readU8(v);
		switch (b) {
			case 0xc0: return null;
			case 0xc2: return false;
			case 0xc3: return true;
			case 0xc4: return readBuffer(v, readU8(v));
			case 0xc5: return readBuffer(v, readU16(v));
			case 0xc6: return readBuffer(v, readI32(v));
			case 0xca: return readF32(v);
			case 0xcc: return readU8(v);
			case 0xcd: return readU16(v);
			case 0xce: return readI32(v);
			case 0xd0: return readI8(v);
			case 0xd1: return readI16(v);
			case 0xd2: return readI32(v);
			case 0xd9: return readString(v, readU8(v));
			case 0xda: return readString(v, readU16(v));
			case 0xdb: return readString(v, readI32(v));
			case 0xdc: return readArray(v, readU16(v));
			case 0xdd: return readArray(v, readI32(v));
			case 0xde: return readMap(v, readU16(v));
			case 0xdf: return readMap(v, readI32(v));
			default: {
				if (b < 0x80) return b; // positive fix num
				else if (b < 0x90) return readMap(v, (0xf & b)); // fix map
				else if (b < 0xa0) return readArray(v, (0xf & b)); // fix array
				else if (b < 0xc0) return readString(v, 0x1f & b); // fix string
				else if (b > 0xdf) return 0xffffff00 | b; // negative fix num
			}
		}
		return null;
	}

	static function readArray(v: DataView, length: Int): Any {
		var b = readU8(v);

		if (b == 0xca) { // Typed float32
			var a = new Float32Array(length);
			for (x in 0...length) a[x] = readF32(v);
			return a;
		}
		else if (b == 0xd2) { // Typed int32
			var a = new Uint32Array(length);
			for (x in 0...length) a[x] = readI32(v);
			return a;
		}
		else if (b == 0xd1) { // Typed int16
			var a = new Int16Array(length);
			for (x in 0...length) a[x] = readI16(v);
			return a;
		}
		else if (b == 0xc4) { // Typed uint8
			var a = new Uint8Array(length);
			for (x in 0...length) a[x] = readU8(v);
			return a;
		}
		else { // Dynamic type-value
			pos--;
			var a: Array<Dynamic> = [];
			for (x in 0...length) a.push(read(v));
			return a;
		}
	}

	static function readMap(v: DataView, length: Int): Any {
		var out = {};
		for (n in 0...length) {
			var k = Std.string(read(v));
			var val = read(v);
			Reflect.setField(out, k, val);
		}
		return out;
	}

	public static function encode(d: Dynamic): ArrayBuffer {
		pos = 0;
		writeDummy(d);
		var b = new ArrayBuffer(pos);
		var v = new DataView(b);
		pos = 0;
		write(v, d);
		return b;
	}

	static function writeU8(v: DataView, i: Int) {
		v.setUint8(pos, i);
		pos += 1;
	}

	static function writeI16(v: DataView, i: Int) {
		v.setInt16(pos, i, true);
		pos += 2;
	}

	static function writeI32(v: DataView, i: Int) {
		v.setInt32(pos, i, true);
		pos += 4;
	}

	static function writeF32(v: DataView, f: Float) {
		v.setFloat32(pos, f, true);
		pos += 4;
	}

	static function writeString(v: DataView, str: String) {
		for (i in 0...str.length) {
			writeU8(v, str.charCodeAt(i));
		}
	}

	static function writeBuffer(v: DataView, b: ArrayBuffer) {
		var u8 = new js.lib.Uint8Array(b);
		for (i in 0...b.byteLength) {
			writeU8(v, u8[i]);
		}
	}

	static function write(v: DataView, d: Dynamic) {
		switch (Type.typeof(d)) {
			case TNull: writeU8(v, 0xc0);
			case TBool: writeU8(v, d ? 0xc3 : 0xc2);
			case TInt: { writeU8(v, 0xd2); writeI32(v, d); }
			case TFloat: { writeU8(v, 0xca); writeF32(v, d); }
			case TClass(c): {
				switch (Type.getClassName(c)) {
					case "String": {
						writeU8(v, 0xdb);
						writeI32(v, d.length);
						writeString(v, d);
					}
					case "Array", null: { // js.lib.arrays give null
						var isAB = Std.isOfType(d, js.lib.ArrayBuffer);
						var isUint8 = Std.isOfType(d, js.lib.Uint8Array);
						var isInt16 = Std.isOfType(d, js.lib.Int16Array);
						var isInt = Std.isOfType(d[0], Int) && !Std.isOfType(d, js.lib.Float32Array);
						var isFloat = Std.isOfType(d[0], Float);
						writeU8(v, isAB ? 0xc6 : 0xdd);
						writeI32(v, isAB ? d.byteLength : d.length);

						if (isAB) {
							writeBuffer(v, d);
						}
						else if (isUint8) { // Uint8Array
							writeU8(v, 0xc4);
							for (i in 0...d.length) writeU8(v, d[i]);
						}
						else if (isInt16) { // Int16Array
							writeU8(v, 0xd1);
							for (i in 0...d.length) writeI16(v, d[i]);
						}
						else if (isFloat && !isInt) { // Float32Array
							writeU8(v, 0xca);
							for (i in 0...d.length) writeF32(v, d[i]);
						}
						else if (isInt) { // Uint32Array
							writeU8(v, 0xd2);
							for (i in 0...d.length) writeI32(v, d[i]);
						}
						else for (i in 0...d.length) write(v, d[i]); // Array
					}
					default: writeObject(v, d);
				}
			}
			case TObject: writeObject(v, d);
			default: {}
		}
	}

	static function writeObject(v: DataView, d: Dynamic) {
		var f = Reflect.fields(d);
		writeU8(v, 0xdf);
		writeI32(v, f.length);
		for (k in f) {
			writeU8(v, 0xdb);
			writeI32(v, k.length);
			writeString(v, k);
			write(v, Reflect.field(d, k));
		}
	}

	static function writeDummy(d: Dynamic) {
		switch (Type.typeof(d)) {
			case TNull: pos += 1;
			case TBool: pos += 1;
			case TInt: { pos += 1; pos += 4; }
			case TFloat: { pos += 1; pos += 4; }
			case TClass(c): {
				switch (Type.getClassName(c)) {
					case "String": {
						pos += 1;
						pos += 4;
						pos += d.length;
					}
					case "Array", null: { // js.lib.arrays give null
						var isAB = Std.isOfType(d, js.lib.ArrayBuffer);
						var isUint8 = Std.isOfType(d, js.lib.Uint8Array);
						var isInt16 = Std.isOfType(d, js.lib.Int16Array);
						var isInt = Std.isOfType(d[0], Int) && !Std.isOfType(d, js.lib.Float32Array);
						var isFloat = Std.isOfType(d[0], Float);
						pos += 1;
						pos += 4;

						if (isAB) {
							pos += d.byteLength;
						}
						else if (isUint8) { // Uint8Array
							pos += 1;
							for (i in 0...d.length) pos += 1;
						}
						else if (isInt16) { // Int16Array
							pos += 1;
							for (i in 0...d.length) pos += 2;
						}
						else if (isFloat && !isInt) { // Float32Array
							pos += 1;
							for (i in 0...d.length) pos += 4;
						}
						else if (isInt) { // Uint32Array
							pos += 1;
							for (i in 0...d.length) pos += 4;
						}
						else for (i in 0...d.length) writeDummy(d[i]); // Array
					}
					default: writeObjectDummy(d);
				}
			}
			case TObject: writeObjectDummy(d);
			default: {}
		}
	}

	static function writeObjectDummy(d: Dynamic) {
		var f = Reflect.fields(d);
		pos += 1;
		pos += 4;
		for (k in f) {
			pos += 1;
			pos += 4;
			pos += k.length;
			writeDummy(Reflect.field(d, k));
		}
	}
}
