package kha;

class FragmentShader {
	public var shader: Dynamic;

	public function new(blob: Blob) {
		if (blob != null) {
			shader = Krom.createFragmentShader(blob.bytes.getData());
		}
	}

	public static function fromSource(source: String): FragmentShader {
		var shader = new FragmentShader(null);
		shader.shader = Krom.createFragmentShaderFromSource(source);
		return shader;
	}

	public function delete() {
		Krom.deleteShader(shader);
		shader = null;
	}
}
