package kha;

import kha.FragmentShader;
import kha.VertexShader;
import kha.GeometryShader;

class Shaders {
	static var vertexShaders: Map<String, VertexShader> = [];
	static var fragmentShaders: Map<String, FragmentShader> = [];
	static var geometryShaders: Map<String, GeometryShader> = [];
	static var ext = #if krom_vulkan ".spirv" #elseif (krom_android || krom_wasm) ".essl" #elseif krom_opengl ".glsl" #elseif krom_metal ".metal" #else ".d3d11" #end ;

	public static function getBuffer(name: String): js.lib.ArrayBuffer {
		#if arm_shader_embed
		var global = js.Syntax.code("globalThis");
		return untyped global["data/" + name + Shaders.ext];
		#else
		return Krom.loadBlob("data/" + name + Shaders.ext);
		#end
	}

	public static function getVertex(name: String): VertexShader {
		var shader = vertexShaders.get(name);
		if (shader == null) {
			shader = new VertexShader(getBuffer(name));
			vertexShaders.set(name, shader);
		}
		return shader;
	}

	public static function getFragment(name: String): FragmentShader {
		var shader = fragmentShaders.get(name);
		if (shader == null) {
			shader = new FragmentShader(getBuffer(name));
			fragmentShaders.set(name, shader);
		}
		return shader;
	}

	public static function getGeometry(name: String): GeometryShader {
		var shader = geometryShaders.get(name);
		if (shader == null) {
			shader = new GeometryShader(getBuffer(name));
			geometryShaders.set(name, shader);
		}
		return shader;
	}
}
