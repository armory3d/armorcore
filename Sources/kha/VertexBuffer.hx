package kha;

import js.lib.DataView;
import kha.Graphics4.Usage;

class VertexBuffer {
	public var buffer_: Dynamic;
	var vertexCount: Int;

	public function new(vertexCount: Int, structure: VertexStructure, usage: Usage, instanceDataStepRate: Int = 0) {
		this.vertexCount = vertexCount;
		buffer_ = Krom.createVertexBuffer(vertexCount, structure.elements, usage, instanceDataStepRate);
	}

	public function delete() {
		Krom.deleteVertexBuffer(buffer_);
	}

	public function lock(): DataView {
		return new DataView(Krom.lockVertexBuffer(buffer_, 0, vertexCount));
	}

	public function unlock(): Void {
		Krom.unlockVertexBuffer(buffer_, vertexCount);
	}

	public function set(): Void {
		Krom.setVertexBuffer(buffer_);
	}
}

class VertexStructure {
	public var elements: Array<VertexElement> = [];
	public var instanced: Bool = false;

	public function new() {}

	public function add(name: String, data: VertexData) {
		elements.push({name: name, data: data});
	}

	public function byteSize(): Int {
		var byteSize = 0;
		for (i in 0...elements.length) {
			byteSize += dataByteSize(elements[i].data);
		}
		return byteSize;
	}

	public static function dataByteSize(data: VertexData): Int {
		switch (data) {
			case F32_1X:
				return 1 * 4;
			case F32_2X:
				return 2 * 4;
			case F32_3X:
				return 3 * 4;
			case F32_4X:
				return 4 * 4;
			case U8_4X_Normalized:
				return 4 * 1;
			case I16_2X_Normalized:
				return 2 * 2;
			case I16_4X_Normalized:
				return 4 * 2;
		}
	}
}

typedef VertexElement = {
	public var name: String;
	public var data: VertexData;
}

@:enum abstract VertexData(Int) {
	var F32_1X = 1;
	var F32_2X = 2;
	var F32_3X = 3;
	var F32_4X = 4;
	var U8_4X_Normalized = 17;
	var I16_2X_Normalized = 24;
	var I16_4X_Normalized = 28;
}
