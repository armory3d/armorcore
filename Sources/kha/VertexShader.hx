package kha;

class VertexShader {
	public var shader: Dynamic;

	public function new(blob: Blob) {
		if (blob != null) {
			shader = Krom.createVertexShader(blob.bytes.getData());
		}
	}

	public static function fromSource(source: String): VertexShader {
		var shader = new VertexShader(null);
		shader.shader = Krom.createVertexShaderFromSource(source);
		return shader;
	}

	public function delete() {
		Krom.deleteShader(shader);
		shader = null;
	}
}
