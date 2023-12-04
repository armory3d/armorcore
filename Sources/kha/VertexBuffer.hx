package kha;

import js.lib.DataView;
import kha.Graphics4.Usage;

class VertexBuffer {
	public var buffer: Dynamic;
	public var _data: DataView;
	private var vertexCount: Int;
	private var structure: VertexStructure;

	public function new(vertexCount: Int, structure: VertexStructure, usage: Usage, instanceDataStepRate: Int = 0) {
		this.vertexCount = vertexCount;
		this.structure = structure;
		buffer = Krom.createVertexBuffer(vertexCount, structure.elements, usage, instanceDataStepRate);
	}

	public function delete() {
		Krom.deleteVertexBuffer(buffer);
		buffer = null;
	}

	var lastLockCount: Int = 0;

	public function lock(?start: Int, ?count: Int): DataView {
		if (start == null) start = 0;
		if (count == null) count = vertexCount - start;
		lastLockCount = count;
		_data = new DataView(Krom.lockVertexBuffer(buffer, start, count));
		return _data;
	}

	public function unlock(?count: Int): Void {
		Krom.unlockVertexBuffer(buffer, count == null ? lastLockCount : count);
	}

	public function set(offset: Int): Int {
		Krom.setVertexBuffer(buffer);
		return 0;
	}
}

class VertexStructure {
	public var elements: Array<VertexElement>;
	public var instanced: Bool;

	public function new() {
		elements = new Array<VertexElement>();
		instanced = false;
	}

	public function add(name: String, data: VertexData) {
		elements.push(new VertexElement(name, data));
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

class VertexElement {
	public var name: String;
	public var data: VertexData;

	public function new(name: String, data: VertexData) {
		this.name = name;
		this.data = data;
	}
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
