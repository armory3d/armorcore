
enum DrawOrder {
	Distance, // Early-z
	Shader, // Less state changes
}

class RenderPath {

	static frameTime = 0.0;
	static frame = 0;
	static currentTarget: RenderTargetRaw = null;
	static light: TLightObject = null;
	static sun: TLightObject = null;
	static point: TLightObject = null;
	static currentG: Graphics4Raw = null;
	static frameG: Graphics4Raw;
	static drawOrder = DrawOrder.Distance;
	static paused = false;

	static get ready(): bool { return RenderPath.loading == 0; }

	static commands: ()=>void = null;
	static renderTargets: Map<string, RenderTargetRaw> = new Map();
	static depthToRenderTarget: Map<string, RenderTargetRaw> = new Map();
	static currentW: i32;
	static currentH: i32;
	static currentD: i32;
	static lastW = 0;
	static lastH = 0;
	static bindParams: string[];
	static meshesSorted: bool;
	static scissorSet = false;
	static lastFrameTime = 0.0;
	static loading = 0;
	static cachedShaderContexts: Map<string, CachedShaderContext> = new Map();
	static depthBuffers: { name: string, format: string }[] = [];

	///if arm_voxels
	static voxelized = 0;
	static voxelize = () => { // Returns true if scene should be voxelized
		return ++RenderPath.voxelized > 2 ? false : true;
	}
	///end

	static renderFrame = (g: Graphics4Raw) => {
		if (!RenderPath.ready || RenderPath.paused || App.w() == 0 || App.h() == 0) return;

		if (RenderPath.lastW > 0 && (RenderPath.lastW != App.w() || RenderPath.lastH != App.h())) RenderPath.resize();
		RenderPath.lastW = App.w();
		RenderPath.lastH = App.h();

		RenderPath.frameTime = Time.time() - RenderPath.lastFrameTime;
		RenderPath.lastFrameTime = Time.time();

		// Render to screen or probe
		let cam = Scene.camera;
		RenderPath.frameG = g;

		RenderPath.currentW = App.w();
		RenderPath.currentH = App.h();
		RenderPath.currentD = 1;
		RenderPath.meshesSorted = false;

		for (let l of Scene.lights) {
			if (l.base.visible) LightObject.buildMatrix(l, Scene.camera);
			if (l.data.type == "sun") RenderPath.sun = l;
			else RenderPath.point = l;
		}
		RenderPath.light = Scene.lights[0];

		RenderPath.commands();

		RenderPath.frame++;
	}

	static setTarget = (target: string, additional: string[] = null) => {
		if (target == "") { // Framebuffer
			RenderPath.currentD = 1;
			RenderPath.currentTarget = null;
			RenderPath.currentW = App.w();
			RenderPath.currentH = App.h();
			RenderPath.begin(RenderPath.frameG);
			RenderPath.setCurrentViewport(App.w(), App.h());
			RenderPath.setCurrentScissor(App.w(), App.h());
		}
		else { // Render target
			let rt = RenderPath.renderTargets.get(target);
			RenderPath.currentTarget = rt;
			let additionalImages: ImageRaw[] = null;
			if (additional != null) {
				additionalImages = [];
				for (let s of additional) {
					let t = RenderPath.renderTargets.get(s);
					additionalImages.push(t.image);
				}
			}
			let targetG = rt.image.g4;
			RenderPath.currentW = rt.image.width;
			RenderPath.currentH = rt.image.height;
			if (rt.is3D) RenderPath.currentD = rt.image.depth;
			RenderPath.begin(targetG, additionalImages);
		}
		RenderPath.bindParams = null;
	}

	static setDepthFrom = (target: string, from: string) => {
		let rt = RenderPath.renderTargets.get(target);
		Image.setDepthStencilFrom(rt.image, RenderPath.renderTargets.get(from).image);
	}

	static begin = (g: Graphics4Raw, additionalRenderTargets: ImageRaw[] = null) => {
		if (RenderPath.currentG != null) RenderPath.end();
		RenderPath.currentG = g;
		Graphics4.begin(g, additionalRenderTargets);
	}

	static end = () => {
		if (RenderPath.scissorSet) {
			Graphics4.disableScissor();
			RenderPath.scissorSet = false;
		}
		Graphics4.end();
		RenderPath.currentG = null;
		RenderPath.bindParams = null;
	}

	static setCurrentViewport = (viewW: i32, viewH: i32) => {
		Graphics4.viewport(App.x(), RenderPath.currentH - (viewH - App.y()), viewW, viewH);
	}

	static setCurrentScissor = (viewW: i32, viewH: i32) => {
		Graphics4.scissor(App.x(), RenderPath.currentH - (viewH - App.y()), viewW, viewH);
		RenderPath.scissorSet = true;
	}

	static setViewport = (viewW: i32, viewH: i32) => {
		RenderPath.setCurrentViewport(viewW, viewH);
		RenderPath.setCurrentScissor(viewW, viewH);
	}

	static clearTarget = (colorFlag: Null<i32> = null, depthFlag: Null<f32> = null) => {
		if (colorFlag == -1) { // -1 == 0xffffffff
			if (Scene.world != null) {
				colorFlag = Scene.world.background_color;
			}
			else if (Scene.camera != null) {
				let cc = Scene.camera.data.clear_color;
				if (cc != null) colorFlag = color_from_floats(cc[0], cc[1], cc[2]);
			}
		}
		Graphics4.clear(colorFlag, depthFlag, null);
	}

	static clearImage = (target: string, color: i32) => {
		let rt = RenderPath.renderTargets.get(target);
		Image.clear(rt.image, 0, 0, 0, rt.image.width, rt.image.height, rt.image.depth, color);
	}

	static generateMipmaps = (target: string) => {
		let rt = RenderPath.renderTargets.get(target);
		Image.generateMipmaps(rt.image, 1000);
	}

	static sortMeshesDistance = (meshes: TMeshObject[]) => {
		meshes.sort((a, b): i32 => {
			return a.cameraDistance >= b.cameraDistance ? 1 : -1;
		});
	}

	static sortMeshesShader = (meshes: TMeshObject[]) => {
		meshes.sort((a, b): i32 => {
			return a.materials[0].name >= b.materials[0].name ? 1 : -1;
		});
	}

	static drawMeshes = (context: string) => {
		RenderPath.submitDraw(context);
		RenderPath.end();
	}

	static submitDraw = (context: string) => {
		let camera = Scene.camera;
		let meshes = Scene.meshes;
		MeshObject.lastPipeline = null;

		if (!RenderPath.meshesSorted && camera != null) { // Order max once per frame for now
			let camX = Transform.worldx(camera.base.transform);
			let camY = Transform.worldy(camera.base.transform);
			let camZ = Transform.worldz(camera.base.transform);
			for (let mesh of meshes) {
				MeshObject.computeCameraDistance(mesh, camX, camY, camZ);
			}
			RenderPath.drawOrder == DrawOrder.Shader ? RenderPath.sortMeshesShader(meshes) : RenderPath.sortMeshesDistance(meshes);
			RenderPath.meshesSorted = true;
		}

		for (let m of meshes) {
			MeshObject.render(m, RenderPath.currentG, context, RenderPath.bindParams);
		}
	}

	static drawSkydome = (handle: string) => {
		if (ConstData.skydomeVB == null) ConstData.createSkydomeData();
		let cc: CachedShaderContext = RenderPath.cachedShaderContexts.get(handle);
		if (cc.context == null) return; // World data not specified
		Graphics4.setPipeline(cc.context._pipeState);
		Uniforms.setContextConstants(RenderPath.currentG, cc.context, RenderPath.bindParams);
		Uniforms.setObjectConstants(RenderPath.currentG, cc.context, null); // External hosek
		Graphics4.setVertexBuffer(ConstData.skydomeVB);
		Graphics4.setIndexBuffer(ConstData.skydomeIB);
		Graphics4.drawIndexedVertices();
		RenderPath.end();
	}

	static bindTarget = (target: string, uniform: string) => {
		if (RenderPath.bindParams != null) {
			RenderPath.bindParams.push(target);
			RenderPath.bindParams.push(uniform);
		}
		else RenderPath.bindParams = [target, uniform];
	}

	// Full-screen triangle
	static drawShader = (handle: string) => {
		// file/data_name/context
		let cc: CachedShaderContext = RenderPath.cachedShaderContexts.get(handle);
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		Graphics4.setPipeline(cc.context._pipeState);
		Uniforms.setContextConstants(RenderPath.currentG, cc.context, RenderPath.bindParams);
		Uniforms.setObjectConstants(RenderPath.currentG, cc.context, null);
		Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
		Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
		Graphics4.drawIndexedVertices();

		RenderPath.end();
	}

	static loadShader = (handle: string) => {
		RenderPath.loading++;
		let cc: CachedShaderContext = RenderPath.cachedShaderContexts.get(handle);
		if (cc != null) {
			RenderPath.loading--;
			return;
		}

		cc = new CachedShaderContext();
		RenderPath.cachedShaderContexts.set(handle, cc);

		// file/data_name/context
		let shaderPath = handle.split("/");

		Data.getShader(shaderPath[0], shaderPath[1], (res: TShaderData) => {
			cc.context = ShaderData.getContext(res, shaderPath[2]);
			RenderPath.loading--;
		});
	}

	static unloadShader = (handle: string) => {
		RenderPath.cachedShaderContexts.delete(handle);

		// file/data_name/context
		let shaderPath = handle.split("/");
		// Todo: Handle context overrides (see Data.getShader())
		Data.cachedShaders.delete(shaderPath[1]);
	}

	static unload = () => {
		for (let rt of RenderPath.renderTargets.values()) RenderTarget.unload(rt);
	}

	static resize = () => {
		if (System.width == 0 || System.height == 0) return;

		// Make sure depth buffer is attached to single target only and gets released once
		for (let rt of RenderPath.renderTargets.values()) {
			if (rt == null ||
				rt.width > 0 ||
				rt.depthStencilFrom == "" ||
				rt == RenderPath.depthToRenderTarget.get(rt.depthStencilFrom)) {
				continue;
			}

			let nodepth: RenderTargetRaw = null;
			for (let rt2 of RenderPath.renderTargets.values()) {
				if (rt2 == null ||
					rt2.width > 0 ||
					rt2.depthStencilFrom != "" ||
					RenderPath.depthToRenderTarget.get(rt2.depth_buffer) != null) {
					continue;
				}

				nodepth = rt2;
				break;
			}

			if (nodepth != null) {
				Image.setDepthStencilFrom(rt.image, nodepth.image);
			}
		}

		// Resize textures
		for (let rt of RenderPath.renderTargets.values()) {
			if (rt != null && rt.width == 0) {
				let _image = rt.image;
				App.notifyOnInit(function() { Image.unload(_image); });
				rt.image = RenderPath.createImage(rt, rt.depthStencil);
			}
		}

		// Attach depth buffers
		for (let rt of RenderPath.renderTargets.values()) {
			if (rt != null && rt.depthStencilFrom != "") {
				Image.setDepthStencilFrom(rt.image, RenderPath.depthToRenderTarget.get(rt.depthStencilFrom).image);
			}
		}
	}

	static createRenderTarget = (t: RenderTargetRaw): RenderTargetRaw => {
		RenderPath.createTarget(t);
		RenderPath.renderTargets.set(t.name, t);
		return t;
	}

	static createDepthBuffer = (name: string, format: string = null) => {
		RenderPath.depthBuffers.push({ name: name, format: format });
	}

	static createTarget = (t: RenderTargetRaw): RenderTargetRaw => {
		// With depth buffer
		if (t.depth_buffer != null) {
			t.hasDepth = true;
			let depthTarget = RenderPath.depthToRenderTarget.get(t.depth_buffer);

			if (depthTarget == null) { // Create new one
				for (let db of RenderPath.depthBuffers) {
					if (db.name == t.depth_buffer) {
						RenderPath.depthToRenderTarget.set(db.name, t);
						t.depthStencil = RenderPath.getDepthStencilFormat(db.format);
						t.image = RenderPath.createImage(t, t.depthStencil);
						break;
					}
				}
			}
			else { // Reuse
				t.depthStencil = DepthStencilFormat.NoDepthAndStencil;
				t.depthStencilFrom = t.depth_buffer;
				t.image = RenderPath.createImage(t, t.depthStencil);
				Image.setDepthStencilFrom(t.image, depthTarget.image);
			}
		}
		else { // No depth buffer
			t.hasDepth = false;
			if (t.depth != null && t.depth > 1) t.is3D = true;
			t.depthStencil = DepthStencilFormat.NoDepthAndStencil;
			t.image = RenderPath.createImage(t, t.depthStencil);
		}
		return t;
	}

	static createImage = (t: RenderTargetRaw, depthStencil: DepthStencilFormat): ImageRaw => {
		let width = t.width == 0 ? App.w() : t.width;
		let height = t.height == 0 ? App.h() : t.height;
		let depth = t.depth != null ? t.depth : 0;
		if (t.scale != null) {
			width = Math.floor(width * t.scale);
			height = Math.floor(height * t.scale);
			depth = Math.floor(depth * t.scale);
		}
		if (width < 1) width = 1;
		if (height < 1) height = 1;
		if (t.depth != null && t.depth > 1) { // 3D texture
			// Image only
			let img = Image.create3D(width, height, depth,
				t.format != null ? RenderPath.getTextureFormat(t.format) : TextureFormat.RGBA32);
			if (t.mipmaps) Image.generateMipmaps(img, 1000); // Allocate mipmaps
			return img;
		}
		else { // 2D texture
			if (t.is_image != null && t.is_image) { // Image
				return Image.create(width, height,
					t.format != null ? RenderPath.getTextureFormat(t.format) : TextureFormat.RGBA32);
			}
			else { // Render target
				return Image.createRenderTarget(width, height,
					t.format != null ? RenderPath.getTextureFormat(t.format) : TextureFormat.RGBA32,
					depthStencil);
			}
		}
	}

	static getTextureFormat = (s: string): TextureFormat => {
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

	static getDepthStencilFormat = (s: string): DepthStencilFormat => {
		if (s == null || s == "") return DepthStencilFormat.DepthOnly;
		switch (s) {
			case "DEPTH24": return DepthStencilFormat.DepthOnly;
			case "DEPTH16": return DepthStencilFormat.Depth16;
			default: return DepthStencilFormat.DepthOnly;
		}
	}
}

class RenderTargetRaw {
	name: string;
	width: i32;
	height: i32;
	format: string = null;
	scale: Null<f32> = null;
	depth_buffer: string = null; // 2D texture
	mipmaps: Null<bool> = null;
	depth: Null<i32> = null; // 3D texture
	is_image: Null<bool> = null; // Image
	// Runtime:
	depthStencil: DepthStencilFormat;
	depthStencilFrom = "";
	image: ImageRaw = null; // RT or image
	hasDepth = false;
	is3D = false; // sampler2D / sampler3D
}

class RenderTarget {
	static create(): RenderTargetRaw {
		return new RenderTargetRaw();
	}

	static unload(raw: RenderTargetRaw) {
		if (raw.image != null) Image.unload(raw.image);
	}
}

class CachedShaderContext {
	context: TShaderContext;
}
