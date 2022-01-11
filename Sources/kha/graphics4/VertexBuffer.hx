package kha.graphics4;

import kha.arrays.ByteArray;
import kha.graphics4.Usage;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;

class VertexBuffer {
	public var buffer: Dynamic;
	public var _data: ByteArray;
	private var vertexCount: Int;
	private var structure: VertexStructure;

	public function new(vertexCount: Int, structure: VertexStructure, usage: Usage, instanceDataStepRate: Int = 0, canRead: Bool = false) {
		this.vertexCount = vertexCount;
		this.structure = structure;
		buffer = Krom.createVertexBuffer(vertexCount, structure.elements, usage, instanceDataStepRate);
	}

	public function delete() {
		Krom.deleteVertexBuffer(buffer);
		buffer = null;
	}

	var lastLockCount: Int = 0;

	public function lock(?start: Int, ?count: Int): ByteArray {
		if (start == null) start = 0;
		if (count == null) count = this.count();
		lastLockCount = count;
		_data = new ByteArray(Krom.lockVertexBuffer(buffer, start, count));
		return _data;
	}

	public function unlock(?count: Int): Void {
		Krom.unlockVertexBuffer(buffer, count == null ? lastLockCount : count);
	}

	public function stride(): Int {
		return structure.byteSize();
	}

	public function count(): Int {
		return vertexCount;
	}

	public function set(offset: Int): Int {
		Krom.setVertexBuffer(buffer);
		return 0;
	}
}
