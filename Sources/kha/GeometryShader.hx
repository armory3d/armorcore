package kha;

class GeometryShader {
	public var shader: Dynamic;

	public function new(blob: Blob) {
		shader = Krom.createGeometryShader(blob.bytes.getData());
	}

	public function delete() {
		Krom.deleteShader(shader);
		shader = null;
	}
}
