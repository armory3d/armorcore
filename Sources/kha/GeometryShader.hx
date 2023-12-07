package kha;

class GeometryShader {
	public var shader_: Dynamic;

	public function new(buffer: js.lib.ArrayBuffer) {
		shader_ = Krom.createGeometryShader(buffer);
	}

	public function delete() {
		Krom.deleteShader(shader_);
	}
}
