package kha;

import haxe.io.Bytes;
import haxe.io.BytesOutput;
import kha.audio2.ogg.vorbis.Reader;

/**
 * Contains compressed or uncompressed audio data.
 */
class Sound {
	public var compressedData: Bytes;
	public var uncompressedData: kha.arrays.Float32Array;
	public var length: Float = 0; // in seconds
	public var channels: Int = 0;

	public function new(bytes: Bytes) {
		var count = Std.int(bytes.length / 4);
		uncompressedData = new kha.arrays.Float32Array(count);
		for (i in 0...count) {
			uncompressedData[i] = bytes.getFloat(i * 4);
		}

		compressedData = null;
	}

	public function uncompress(done: Void->Void): Void {
		done();
	}

	public function unload() {
		compressedData = null;
		uncompressedData = null;
	}
}
