package kha;

import js.lib.Uint32Array;
import kha.Graphics4.Usage;

class IndexBuffer {
	public var buffer_: Dynamic;

	public function new(indexCount: Int, usage: Usage) {
		buffer_ = Krom.createIndexBuffer(indexCount);
	}

	public function delete() {
		Krom.deleteIndexBuffer(buffer_);
	}

	public function lock(): Uint32Array {
		return Krom.lockIndexBuffer(buffer_);
	}

	public function unlock(): Void {
		Krom.unlockIndexBuffer(buffer_);
	}

	public function set(): Void {
		Krom.setIndexBuffer(buffer_);
	}
}
