package iron;

import iron.System;
import iron.system.Time;
import iron.data.SceneFormat;
import iron.data.MaterialData;
import iron.data.ShaderData;
import iron.data.ConstData;
import iron.data.Data;
import iron.object.Object;
import iron.object.LightObject;
import iron.object.MeshObject;
import iron.object.Uniforms;

class RenderPath {

	public static var active: RenderPath;

	public var frameScissor = false;
	public var frameScissorX = 0;
	public var frameScissorY = 0;
	public var frameScissorW = 0;
	public var frameScissorH = 0;
	public var frameTime = 0.0;
	public var frame = 0;
	public var currentTarget: RenderTarget = null;
	public var light: LightObject = null;
	public var sun: LightObject = null;
	public var point: LightObject = null;
	public var currentG: Graphics4 = null;
	public var frameG: Graphics4;
	public var drawOrder = DrawOrder.Distance;
	public var paused = false;
	public var ready(get, null): Bool;
	function get_ready(): Bool { return loading == 0; }
	public var commands: Void->Void = null;
	public var setupDepthTexture: Void->Void = null;
	public var renderTargets: Map<String, RenderTarget> = new Map();
	public var depthToRenderTarget: Map<String, RenderTarget> = new Map();
	public var currentW: Int;
	public var currentH: Int;
	public var currentD: Int;
	var lastW = 0;
	var lastH = 0;
	var bindParams: Array<String>;
	var meshesSorted: Bool;
	var scissorSet = false;
	var viewportScaled = false;
	var lastFrameTime = 0.0;
	var loading = 0;
	var cachedShaderContexts: Map<String, CachedShaderContext> = new Map();
	var depthBuffers: Array<{name: String, format: String}> = [];

	#if arm_voxels
	public var voxelized = 0;
	public function voxelize() { // Returns true if scene should be voxelized
		return ++voxelized > 2 ? false : true;
	}
	#end

	public static function setActive(renderPath: RenderPath) {
		active = renderPath;
	}

	public function new() {}

	public function renderFrame(g: Graphics4) {
		if (!ready || paused || iron.App.w() == 0 || iron.App.h() == 0) return;

		if (lastW > 0 && (lastW != iron.App.w() || lastH != iron.App.h())) resize();
		lastW = iron.App.w();
		lastH = iron.App.h();

		frameTime = Time.time() - lastFrameTime;
		lastFrameTime = Time.time();

		// Render to screen or probe
		var cam = Scene.active.camera;
		frameG = g;

		currentW = iron.App.w();
		currentH = iron.App.h();
		currentD = 1;
		meshesSorted = false;

		for (l in Scene.active.lights) {
			if (l.visible) l.buildMatrix(Scene.active.camera);
			if (l.data.raw.type == "sun") sun = l;
			else point = l;
		}
		light = Scene.active.lights[0];

		commands();

		frame++;
	}

	public function setTarget(target: String, additional: Array<String> = null, viewportScale = 1.0) {
		if (target == "") { // Framebuffer
			currentD = 1;
			currentTarget = null;
			currentW = iron.App.w();
			currentH = iron.App.h();
			if (frameScissor) setFrameScissor();
			begin(frameG);
			setCurrentViewport(iron.App.w(), iron.App.h());
			setCurrentScissor(iron.App.w(), iron.App.h());
		}
		else { // Render target
			var rt = renderTargets.get(target);
			currentTarget = rt;
			var additionalImages: Array<Image> = null;
			if (additional != null) {
				additionalImages = [];
				for (s in additional) {
					var t = renderTargets.get(s);
					additionalImages.push(t.image);
				}
			}
			var targetG = rt.image.g4;
			currentW = rt.image.width;
			currentH = rt.image.height;
			if (rt.is3D) currentD = rt.image.depth;
			begin(targetG, additionalImages);
		}
		if (viewportScale != 1.0) {
			viewportScaled = true;
			var viewW = Std.int(currentW * viewportScale);
			var viewH = Std.int(currentH * viewportScale);
			currentG.viewport(0, viewH, viewW, viewH);
			currentG.scissor(0, viewH, viewW, viewH);
		}
		else if (viewportScaled) { // Reset viewport
			viewportScaled = false;
			setCurrentViewport(currentW, currentH);
			setCurrentScissor(currentW, currentH);
		}
		bindParams = null;
	}

	public function setDepthFrom(target: String, from: String) {
		var rt = renderTargets.get(target);
		rt.image.setDepthStencilFrom(renderTargets.get(from).image);
	}

	inline function begin(g: Graphics4, additionalRenderTargets: Array<Image> = null) {
		if (currentG != null) end();
		currentG = g;
		g.begin(additionalRenderTargets);
	}

	inline function end() {
		if (scissorSet) {
			currentG.disableScissor();
			scissorSet = false;
		}
		currentG.end();
		currentG = null;
		bindParams = null;
	}

	public function setCurrentViewportWithOffset(viewW:Int, viewH:Int, offsetX: Int, offsetY: Int) {
		currentG.viewport(iron.App.x() + offsetX, currentH - viewH + iron.App.y() - offsetY, viewW, viewH);
	}

	public function setCurrentViewport(viewW: Int, viewH: Int) {
		currentG.viewport(iron.App.x(), currentH - (viewH - iron.App.y()), viewW, viewH);
	}

	public function setCurrentScissor(viewW: Int, viewH: Int) {
		currentG.scissor(iron.App.x(), currentH - (viewH - iron.App.y()), viewW, viewH);
		scissorSet = true;
	}

	public function setFrameScissor() {
		frameG.scissor(frameScissorX, currentH - (frameScissorH - frameScissorY), frameScissorW, frameScissorH);
	}

	public function setViewport(viewW: Int, viewH: Int) {
		setCurrentViewport(viewW, viewH);
		setCurrentScissor(viewW, viewH);
	}

	public function clearTarget(colorFlag: Null<Int> = null, depthFlag: Null<Float> = null) {
		if (colorFlag == -1) { // -1 == 0xffffffff
			if (Scene.active.world != null) {
				colorFlag = Scene.active.world.raw.background_color;
			}
			else if (Scene.active.camera != null) {
				var cc = Scene.active.camera.data.raw.clear_color;
				if (cc != null) colorFlag = Color.fromFloats(cc[0], cc[1], cc[2]);
			}
		}
		currentG.clear(colorFlag, depthFlag, null);
	}

	public function clearImage(target: String, color: Int) {
		var rt = renderTargets.get(target);
		rt.image.clear(0, 0, 0, rt.image.width, rt.image.height, rt.image.depth, color);
	}

	public function generateMipmaps(target: String) {
		var rt = renderTargets.get(target);
		rt.image.generateMipmaps(1000);
	}

	static inline function boolToInt(b: Bool): Int {
		return b ? 1 : 0;
	}

	public static function sortMeshesDistance(meshes: Array<MeshObject>) {
		meshes.sort(function(a, b): Int {
			#if rp_depth_texture
			var depthDiff = boolToInt(a.depthRead) - boolToInt(b.depthRead);
			if (depthDiff != 0) return depthDiff;
			#end

			return a.cameraDistance >= b.cameraDistance ? 1 : -1;
		});
	}

	public static function sortMeshesShader(meshes: Array<MeshObject>) {
		meshes.sort(function(a, b): Int {
			#if rp_depth_texture
			var depthDiff = boolToInt(a.depthRead) - boolToInt(b.depthRead);
			if (depthDiff != 0) return depthDiff;
			#end

			return a.materials[0].name >= b.materials[0].name ? 1 : -1;
		});
	}

	public function drawMeshes(context: String) {
		submitDraw(context);
		end();
	}

	@:access(iron.object.MeshObject)
	function submitDraw(context: String) {
		var camera = Scene.active.camera;
		var meshes = Scene.active.meshes;
		MeshObject.lastPipeline = null;

		if (!meshesSorted && camera != null) { // Order max once per frame for now
			var camX = camera.transform.worldx();
			var camY = camera.transform.worldy();
			var camZ = camera.transform.worldz();
			for (mesh in meshes) {
				mesh.computeCameraDistance(camX, camY, camZ);
			}
			drawOrder == DrawOrder.Shader ? sortMeshesShader(meshes) : sortMeshesDistance(meshes);
			meshesSorted = true;
		}

		meshRenderLoop(currentG, context, bindParams, meshes);
	}

	static inline function meshRenderLoop(g: Graphics4, context: String, _bindParams: Array<String>, _meshes: Array<MeshObject>) {
		var isReadingDepth = false;

		for (m in _meshes) {
			m.render(g, context, _bindParams);
		}
	}

	public function drawSkydome(handle: String) {
		if (ConstData.skydomeVB == null) ConstData.createSkydomeData();
		var cc: CachedShaderContext = cachedShaderContexts.get(handle);
		if (cc.context == null) return; // World data not specified
		currentG.setPipeline(cc.context.pipeState);
		Uniforms.setContextConstants(currentG, cc.context, bindParams);
		Uniforms.setObjectConstants(currentG, cc.context, null); // External hosek
		currentG.setVertexBuffer(ConstData.skydomeVB);
		currentG.setIndexBuffer(ConstData.skydomeIB);
		currentG.drawIndexedVertices();
		end();
	}

	public function bindTarget(target: String, uniform: String) {
		if (bindParams != null) {
			bindParams.push(target);
			bindParams.push(uniform);
		}
		else bindParams = [target, uniform];
	}

	// Full-screen triangle
	public function drawShader(handle: String) {
		// file/data_name/context
		var cc: CachedShaderContext = cachedShaderContexts.get(handle);
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		currentG.setPipeline(cc.context.pipeState);
		Uniforms.setContextConstants(currentG, cc.context, bindParams);
		Uniforms.setObjectConstants(currentG, cc.context, null);
		currentG.setVertexBuffer(ConstData.screenAlignedVB);
		currentG.setIndexBuffer(ConstData.screenAlignedIB);
		currentG.drawIndexedVertices();

		end();
	}

	public function loadShader(handle: String) {
		loading++;
		var cc: CachedShaderContext = cachedShaderContexts.get(handle);
		if (cc != null) {
			loading--;
			return;
		}

		cc = new CachedShaderContext();
		cachedShaderContexts.set(handle, cc);

		// file/data_name/context
		var shaderPath = handle.split("/");

		Data.getShader(shaderPath[0], shaderPath[1], function(res: ShaderData) {
			cc.context = res.getContext(shaderPath[2]);
			loading--;
		});
	}

	public function unloadShader(handle: String) {
		cachedShaderContexts.remove(handle);

		// file/data_name/context
		var shaderPath = handle.split("/");
		// Todo: Handle context overrides (see Data.getShader())
		Data.cachedShaders.remove(shaderPath[1]);
	}

	public function unload() {
		for (rt in renderTargets) rt.unload();
	}

	public function resize() {
		if (System.width == 0 || System.height == 0) return;

		// Make sure depth buffer is attached to single target only and gets released once
		for (rt in renderTargets) {
			if (rt == null ||
				rt.raw.width > 0 ||
				rt.depthStencilFrom == "" ||
				rt == depthToRenderTarget.get(rt.depthStencilFrom)) {
				continue;
			}

			var nodepth: RenderTarget = null;
			for (rt2 in renderTargets) {
				if (rt2 == null ||
					rt2.raw.width > 0 ||
					rt2.depthStencilFrom != "" ||
					depthToRenderTarget.get(rt2.raw.depth_buffer) != null) {
					continue;
				}

				nodepth = rt2;
				break;
			}

			if (nodepth != null) {
				rt.image.setDepthStencilFrom(nodepth.image);
			}
		}

		// Resize textures
		for (rt in renderTargets) {
			if (rt != null && rt.raw.width == 0) {
				App.notifyOnInit(rt.image.unload);
				rt.image = createImage(rt.raw, rt.depthStencil);
			}
		}

		// Attach depth buffers
		for (rt in renderTargets) {
			if (rt != null && rt.depthStencilFrom != "") {
				rt.image.setDepthStencilFrom(depthToRenderTarget.get(rt.depthStencilFrom).image);
			}
		}
	}

	public function createRenderTarget(t: RenderTargetRaw): RenderTarget {
		var rt = createTarget(t);
		renderTargets.set(t.name, rt);
		return rt;
	}

	public function createDepthBuffer(name: String, format: String = null) {
		depthBuffers.push({ name: name, format: format });
	}

	function createTarget(t: RenderTargetRaw): RenderTarget {
		var rt = new RenderTarget(t);
		// With depth buffer
		if (t.depth_buffer != null) {
			rt.hasDepth = true;
			var depthTarget = depthToRenderTarget.get(t.depth_buffer);

			if (depthTarget == null) { // Create new one
				for (db in depthBuffers) {
					if (db.name == t.depth_buffer) {
						depthToRenderTarget.set(db.name, rt);
						rt.depthStencil = getDepthStencilFormat(db.format);
						rt.image = createImage(t, rt.depthStencil);
						break;
					}
				}
			}
			else { // Reuse
				rt.depthStencil = DepthStencilFormat.NoDepthAndStencil;
				rt.depthStencilFrom = t.depth_buffer;
				rt.image = createImage(t, rt.depthStencil);
				rt.image.setDepthStencilFrom(depthTarget.image);
			}
		}
		else { // No depth buffer
			rt.hasDepth = false;
			if (t.depth != null && t.depth > 1) rt.is3D = true;
			rt.depthStencil = DepthStencilFormat.NoDepthAndStencil;
			rt.image = createImage(t, rt.depthStencil);
		}
		return rt;
	}

	function createImage(t: RenderTargetRaw, depthStencil: DepthStencilFormat): Image {
		var width = t.width == 0 ? iron.App.w() : t.width;
		var height = t.height == 0 ? iron.App.h() : t.height;
		var depth = t.depth != null ? t.depth : 0;
		if (t.displayp != null) { // 1080p/..
			if (width > height) {
				width = Std.int(width * (t.displayp / height));
				height = t.displayp;
			}
			else {
				height = Std.int(height * (t.displayp / width));
				width = t.displayp;
			}
		}
		if (t.scale != null) {
			width = Std.int(width * t.scale);
			height = Std.int(height * t.scale);
			depth = Std.int(depth * t.scale);
		}
		if (width < 1) width = 1;
		if (height < 1) height = 1;
		if (t.depth != null && t.depth > 1) { // 3D texture
			// Image only
			var img = Image.create3D(width, height, depth,
				t.format != null ? getTextureFormat(t.format) : TextureFormat.RGBA32);
			if (t.mipmaps) img.generateMipmaps(1000); // Allocate mipmaps
			return img;
		}
		else { // 2D texture
			if (t.is_image != null && t.is_image) { // Image
				return Image.create(width, height,
					t.format != null ? getTextureFormat(t.format) : TextureFormat.RGBA32);
			}
			else { // Render target
				return Image.createRenderTarget(width, height,
					t.format != null ? getTextureFormat(t.format) : TextureFormat.RGBA32,
					depthStencil);
			}
		}
	}

	inline function getTextureFormat(s: String): TextureFormat {
		switch (s) {
			case "RGBA32": return TextureFormat.RGBA32;
			case "RGBA64": return TextureFormat.RGBA64;
			case "RGBA128": return TextureFormat.RGBA128;
			case "DEPTH16": return TextureFormat.DEPTH16;
			case "R32": return TextureFormat.R32;
			case "R16": return TextureFormat.R16;
			case "R8": return TextureFormat.R8;
			default: return TextureFormat.RGBA32;
		}
	}

	inline function getDepthStencilFormat(s: String): DepthStencilFormat {
		if (s == null || s == "") return DepthStencilFormat.DepthOnly;
		switch (s) {
			case "DEPTH24": return DepthStencilFormat.DepthOnly;
			case "DEPTH16": return DepthStencilFormat.Depth16;
			default: return DepthStencilFormat.DepthOnly;
		}
	}
}

class RenderTargetRaw {
	public var name: String;
	public var width: Int;
	public var height: Int;
	public var format: String = null;
	public var scale: Null<Float> = null;
	public var displayp: Null<Int> = null; // Set to 1080p/...
	public var depth_buffer: String = null; // 2D texture
	public var mipmaps: Null<Bool> = null;
	public var depth: Null<Int> = null; // 3D texture
	public var is_image: Null<Bool> = null; // Image
	public function new() {}
}

class RenderTarget {
	public var raw: RenderTargetRaw;
	public var depthStencil: DepthStencilFormat;
	public var depthStencilFrom = "";
	public var image: Image = null; // RT or image
	public var hasDepth = false;
	public var is3D = false; // sampler2D / sampler3D
	public function new(raw: RenderTargetRaw) { this.raw = raw; }
	public function unload() {
		if (image != null) image.unload();
	}
}

class CachedShaderContext {
	public var context: ShaderContext;
	public function new() {}
}

@:enum abstract DrawOrder(Int) from Int {
	var Distance = 0; // Early-z
	var Shader = 1; // Less state changes
}
