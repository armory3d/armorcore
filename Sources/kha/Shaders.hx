package kha;

import kha.graphics4.FragmentShader;
import kha.graphics4.VertexShader;
import kha.graphics4.GeometryShader;

class Shaders {

	static var vertexShaders: Map<String, VertexShader> = [];
	static var fragmentShaders: Map<String, FragmentShader> = [];
	static var geometryShaders: Map<String, GeometryShader> = [];
	static var ext = #if kha_vulkan ".spirv" #elseif kha_opengl ".glsl" #elseif kha_metal ".metal" #else ".d3d11" #end ;

	public static function init() {
		// When running a game, precompile shaders here
	}

	static function getBlob(name: String): kha.Blob {
		#if arm_shader_embed
		var global = js.Syntax.code("globalThis");
		return untyped kha.Blob.fromBytes(haxe.io.Bytes.ofData(global["data/" + name + ext]));
		#else
		return kha.Blob.fromBytes(haxe.io.Bytes.ofData(Krom.loadBlob("data/" + name + ext)));
		#end
	}

	public static function getVertex(name: String): VertexShader {
		var shader = vertexShaders.get(name);
		if (shader == null) {
			shader = new VertexShader([getBlob(name)], [name]);
			vertexShaders.set(name, shader);
		}
		return shader;
	}

	public static function getFragment(name: String): FragmentShader {
		var shader = fragmentShaders.get(name);
		if (shader == null) {
			shader = new FragmentShader([getBlob(name)], [name]);
			fragmentShaders.set(name, shader);
		}
		return shader;
	}

	public static function getGeometry(name: String): GeometryShader {
		var shader = geometryShaders.get(name);
		if (shader == null) {
			shader = new GeometryShader([getBlob(name)], [name]);
			geometryShaders.set(name, shader);
		}
		return shader;
	}
}
