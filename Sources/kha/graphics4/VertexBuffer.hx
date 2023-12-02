package kha.graphics4;

import js.lib.DataView;
import kha.graphics4.Graphics.Usage;

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
			case Float32_1X:
				return 1 * 4;
			case Float32_2X:
				return 2 * 4;
			case Float32_3X:
				return 3 * 4;
			case Float32_4X:
				return 4 * 4;
			case Float32_4X4:
				return 4 * 4 * 4;
			case Int8_1X, UInt8_1X, Int8_1X_Normalized, UInt8_1X_Normalized:
				return 1 * 1;
			case Int8_2X, UInt8_2X, Int8_2X_Normalized, UInt8_2X_Normalized:
				return 2 * 1;
			case Int8_4X, UInt8_4X, Int8_4X_Normalized, UInt8_4X_Normalized:
				return 4 * 1;
			case Int16_1X, UInt16_1X, Int16_1X_Normalized, UInt16_1X_Normalized:
				return 1 * 2;
			case Int16_2X, UInt16_2X, Int16_2X_Normalized, UInt16_2X_Normalized:
				return 2 * 2;
			case Int16_4X, UInt16_4X, Int16_4X_Normalized, UInt16_4X_Normalized:
				return 4 * 2;
			case Int32_1X, UInt32_1X:
				return 1 * 4;
			case Int32_2X, UInt32_2X:
				return 2 * 4;
			case Int32_3X, UInt32_3X:
				return 3 * 4;
			case Int32_4X,UInt32_4X:
				return 4 * 4;
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
	var Float32_1X = 0;
	var Float32_2X = 1;
	var Float32_3X = 2;
	var Float32_4X = 3;
	var Float32_4X4 = 4;
	var Int8_1X = 5;
	var UInt8_1X = 6;
	var Int8_1X_Normalized = 7;
	var UInt8_1X_Normalized = 8;
	var Int8_2X = 9;
	var UInt8_2X = 10;
	var Int8_2X_Normalized = 11;
	var UInt8_2X_Normalized = 12;
	var Int8_4X = 13;
	var UInt8_4X = 14;
	var Int8_4X_Normalized = 15;
	var UInt8_4X_Normalized = 16;
	var Int16_1X = 17;
	var UInt16_1X = 18;
	var Int16_1X_Normalized = 19;
	var UInt16_1X_Normalized = 20;
	var Int16_2X = 21;
	var UInt16_2X = 22;
	var Int16_2X_Normalized = 23;
	var UInt16_2X_Normalized = 24;
	var Int16_4X = 25;
	var UInt16_4X = 26;
	var Int16_4X_Normalized = 27;
	var UInt16_4X_Normalized = 28;
	var Int32_1X = 29;
	var UInt32_1X = 30;
	var Int32_2X = 31;
	var UInt32_2X = 32;
	var Int32_3X = 33;
	var UInt32_3X = 34;
	var Int32_4X = 35;
	var UInt32_4X = 36;

	// deprecated
	var Float1 = Float32_1X;
	var Float2 = Float32_2X;
	var Float3 = Float32_3X;
	var Float4 = Float32_4X;
	var Float4x4 = Float32_4X4;
	var Short2Norm = Int16_2X_Normalized;
	var Short4Norm = Int16_4X_Normalized;
}
