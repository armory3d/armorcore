package kha;

class FragmentShader {
	public var shader_: Dynamic;

	public function new(buffer: js.lib.ArrayBuffer) {
		if (buffer != null) {
			shader_ = Krom.createFragmentShader(buffer);
		}
	}

	public static function fromSource(source: String): FragmentShader {
		var shader = new FragmentShader(null);
		shader.shader_ = Krom.createFragmentShaderFromSource(source);
		return shader;
	}

	public function delete() {
		Krom.deleteShader(shader_);
	}
}
