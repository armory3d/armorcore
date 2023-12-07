package kha;

class VertexShader {
	public var shader_: Dynamic;

	public function new(buffer: js.lib.ArrayBuffer) {
		if (buffer != null) {
			shader_ = Krom.createVertexShader(buffer);
		}
	}

	public static function fromSource(source: String): VertexShader {
		var shader = new VertexShader(null);
		shader.shader_ = Krom.createVertexShaderFromSource(source);
		return shader;
	}

	public function delete() {
		Krom.deleteShader(shader_);
	}
}
