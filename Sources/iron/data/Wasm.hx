package iron.data;

class Wasm {

	public var exports: Dynamic;

	public static inline function instance(blob: kha.Blob, importObject: Dynamic = null): Wasm {
		var data = blob.toBytes().getData();
		var module = new js.lib.webassembly.Module(data);
		var exports: Dynamic = importObject == null ?
			new js.lib.webassembly.Instance(module).exports :
			new js.lib.webassembly.Instance(module, importObject).exports;
		return new Wasm(exports);
	}

	function new(exports: Dynamic) {
		this.exports = exports;
	}

	public function getString(i: Int): String { // Retrieve string from memory pointer
		var mem = getMemory(i, 32);
		var s = "";
		for (i in 0...32) {
			mem[i] == 0 ? break : s += String.fromCharCode(mem[i]);
		}
		return s;
	}

	public inline function getMemory(offset: Int, length: Int): js.lib.Uint8Array {
		return new js.lib.Uint8Array(exports.memory.buffer, offset, length);
	}

	public inline function getMemoryF32(offset: Int, length: Int): js.lib.Float32Array {
		return new js.lib.Float32Array(exports.memory.buffer).subarray( offset, length );
	}

	public inline function getMemoryU32(offset: Int, length: Int): js.lib.Uint32Array {
		return new js.lib.Uint32Array(exports.memory.buffer).subarray(offset, length);
	}
}
