
class RenderPath {

	static active: RenderPath;
	frameScissor = false;
	frameScissorX = 0;
	frameScissorY = 0;
	frameScissorW = 0;
	frameScissorH = 0;
	frameTime = 0.0;
	frame = 0;
	currentTarget: RenderTarget = null;
	light: LightObject = null;
	sun: LightObject = null;
	point: LightObject = null;
	currentG: Graphics4 = null;
	frameG: Graphics4;
	drawOrder = DrawOrder.Distance;
	paused = false;

	get ready(): bool { return this.loading == 0; }

	commands: ()=>void = null;
	setupDepthTexture: ()=>void = null;
	renderTargets: Map<string, RenderTarget> = new Map();
	depthToRenderTarget: Map<string, RenderTarget> = new Map();
	currentW: i32;
	currentH: i32;
	currentD: i32;
	lastW = 0;
	lastH = 0;
	bindParams: string[];
	meshesSorted: bool;
	scissorSet = false;
	viewportScaled = false;
	lastFrameTime = 0.0;
	loading = 0;
	cachedShaderContexts: Map<string, CachedShaderContext> = new Map();
	depthBuffers: {name: string, format: string}[] = [];

	///if arm_voxels
	voxelized = 0;
	voxelize = () => { // Returns true if scene should be voxelized
		return ++this.voxelized > 2 ? false : true;
	}
	///end

	static setActive = (renderPath: RenderPath) => {
		RenderPath.active = renderPath;
	}

	constructor() {}

	renderFrame = (g: Graphics4) => {
		if (!this.ready || this.paused || App.w() == 0 || App.h() == 0) return;

		if (this.lastW > 0 && (this.lastW != App.w() || this.lastH != App.h())) this.resize();
		this.lastW = App.w();
		this.lastH = App.h();

		this.frameTime = Time.time() - this.lastFrameTime;
		this.lastFrameTime = Time.time();

		// Render to screen or probe
		let cam = Scene.active.camera;
		this.frameG = g;

		this.currentW = App.w();
		this.currentH = App.h();
		this.currentD = 1;
		this.meshesSorted = false;

		for (let l of Scene.active.lights) {
			if (l.visible) l.buildMatrix(Scene.active.camera);
			if (l.data.raw.type == "sun") this.sun = l;
			else this.point = l;
		}
		this.light = Scene.active.lights[0];

		this.commands();

		this.frame++;
	}

	setTarget = (target: string, additional: string[] = null, viewportScale = 1.0) => {
		if (target == "") { // Framebuffer
			this.currentD = 1;
			this.currentTarget = null;
			this.currentW = App.w();
			this.currentH = App.h();
			if (this.frameScissor) this.setFrameScissor();
			this.begin(this.frameG);
			this.setCurrentViewport(App.w(), App.h());
			this.setCurrentScissor(App.w(), App.h());
		}
		else { // Render target
			let rt = this.renderTargets.get(target);
			this.currentTarget = rt;
			let additionalImages: Image[] = null;
			if (additional != null) {
				additionalImages = [];
				for (let s of additional) {
					let t = this.renderTargets.get(s);
					additionalImages.push(t.image);
				}
			}
			let targetG = rt.image.g4;
			this.currentW = rt.image.width;
			this.currentH = rt.image.height;
			if (rt.is3D) this.currentD = rt.image.depth;
			this.begin(targetG, additionalImages);
		}
		if (viewportScale != 1.0) {
			this.viewportScaled = true;
			let viewW = Math.floor(this.currentW * viewportScale);
			let viewH = Math.floor(this.currentH * viewportScale);
			this.currentG.viewport(0, viewH, viewW, viewH);
			this.currentG.scissor(0, viewH, viewW, viewH);
		}
		else if (this.viewportScaled) { // Reset viewport
			this.viewportScaled = false;
			this.setCurrentViewport(this.currentW, this.currentH);
			this.setCurrentScissor(this.currentW, this.currentH);
		}
		this.bindParams = null;
	}

	setDepthFrom = (target: string, from: string) => {
		let rt = this.renderTargets.get(target);
		rt.image.setDepthStencilFrom(this.renderTargets.get(from).image);
	}

	begin = (g: Graphics4, additionalRenderTargets: Image[] = null) => {
		if (this.currentG != null) this.end();
		this.currentG = g;
		g.begin(additionalRenderTargets);
	}

	end = () => {
		if (this.scissorSet) {
			this.currentG.disableScissor();
			this.scissorSet = false;
		}
		this.currentG.end();
		this.currentG = null;
		this.bindParams = null;
	}

	setCurrentViewportWithOffset = (viewW: i32, viewH: i32, offsetX: i32, offsetY: i32) => {
		this.currentG.viewport(App.x() + offsetX, this.currentH - viewH + App.y() - offsetY, viewW, viewH);
	}

	setCurrentViewport = (viewW: i32, viewH: i32) => {
		this.currentG.viewport(App.x(), this.currentH - (viewH - App.y()), viewW, viewH);
	}

	setCurrentScissor = (viewW: i32, viewH: i32) => {
		this.currentG.scissor(App.x(), this.currentH - (viewH - App.y()), viewW, viewH);
		this.scissorSet = true;
	}

	setFrameScissor = () => {
		this.frameG.scissor(this.frameScissorX, this.currentH - (this.frameScissorH - this.frameScissorY), this.frameScissorW, this.frameScissorH);
	}

	setViewport = (viewW: i32, viewH: i32) => {
		this.setCurrentViewport(viewW, viewH);
		this.setCurrentScissor(viewW, viewH);
	}

	clearTarget = (colorFlag: Null<i32> = null, depthFlag: Null<f32> = null) => {
		if (colorFlag == -1) { // -1 == 0xffffffff
			if (Scene.active.world != null) {
				colorFlag = Scene.active.world.raw.background_color;
			}
			else if (Scene.active.camera != null) {
				let cc = Scene.active.camera.data.raw.clear_color;
				if (cc != null) colorFlag = color_from_floats(cc[0], cc[1], cc[2]);
			}
		}
		this.currentG.clear(colorFlag, depthFlag, null);
	}

	clearImage = (target: string, color: i32) => {
		let rt = this.renderTargets.get(target);
		rt.image.clear(0, 0, 0, rt.image.width, rt.image.height, rt.image.depth, color);
	}

	generateMipmaps = (target: string) => {
		let rt = this.renderTargets.get(target);
		rt.image.generateMipmaps(1000);
	}

	static boolToInt = (b: bool): i32 => {
		return b ? 1 : 0;
	}

	static sortMeshesDistance = (meshes: MeshObject[]) => {
		meshes.sort((a, b): i32 => {
			return a.cameraDistance >= b.cameraDistance ? 1 : -1;
		});
	}

	static sortMeshesShader = (meshes: MeshObject[]) => {
		meshes.sort((a, b): i32 => {
			return a.materials[0].name >= b.materials[0].name ? 1 : -1;
		});
	}

	drawMeshes = (context: string) => {
		this.submitDraw(context);
		this.end();
	}

	submitDraw = (context: string) => {
		let camera = Scene.active.camera;
		let meshes = Scene.active.meshes;
		MeshObject.lastPipeline = null;

		if (!this.meshesSorted && camera != null) { // Order max once per frame for now
			let camX = camera.transform.worldx();
			let camY = camera.transform.worldy();
			let camZ = camera.transform.worldz();
			for (let mesh of meshes) {
				mesh.computeCameraDistance(camX, camY, camZ);
			}
			this.drawOrder == DrawOrder.Shader ? RenderPath.sortMeshesShader(meshes) : RenderPath.sortMeshesDistance(meshes);
			this.meshesSorted = true;
		}

		RenderPath.meshRenderLoop(this.currentG, context, this.bindParams, meshes);
	}

	static meshRenderLoop = (g: Graphics4, context: string, _bindParams: string[], _meshes: MeshObject[]) => {
		let isReadingDepth = false;

		for (let m of _meshes) {
			m.render(g, context, _bindParams);
		}
	}

	drawSkydome = (handle: string) => {
		if (ConstData.skydomeVB == null) ConstData.createSkydomeData();
		let cc: CachedShaderContext = this.cachedShaderContexts.get(handle);
		if (cc.context == null) return; // World data not specified
		this.currentG.setPipeline(cc.context.pipeState);
		Uniforms.setContextConstants(this.currentG, cc.context, this.bindParams);
		Uniforms.setObjectConstants(this.currentG, cc.context, null); // External hosek
		this.currentG.setVertexBuffer(ConstData.skydomeVB);
		this.currentG.setIndexBuffer(ConstData.skydomeIB);
		this.currentG.drawIndexedVertices();
		this.end();
	}

	bindTarget = (target: string, uniform: string) => {
		if (this.bindParams != null) {
			this.bindParams.push(target);
			this.bindParams.push(uniform);
		}
		else this.bindParams = [target, uniform];
	}

	// Full-screen triangle
	drawShader = (handle: string) => {
		// file/data_name/context
		let cc: CachedShaderContext = this.cachedShaderContexts.get(handle);
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		this.currentG.setPipeline(cc.context.pipeState);
		Uniforms.setContextConstants(this.currentG, cc.context, this.bindParams);
		Uniforms.setObjectConstants(this.currentG, cc.context, null);
		this.currentG.setVertexBuffer(ConstData.screenAlignedVB);
		this.currentG.setIndexBuffer(ConstData.screenAlignedIB);
		this.currentG.drawIndexedVertices();

		this.end();
	}

	loadShader = (handle: string) => {
		this.loading++;
		let cc: CachedShaderContext = this.cachedShaderContexts.get(handle);
		if (cc != null) {
			this.loading--;
			return;
		}

		cc = new CachedShaderContext();
		this.cachedShaderContexts.set(handle, cc);

		// file/data_name/context
		let shaderPath = handle.split("/");

		Data.getShader(shaderPath[0], shaderPath[1], (res: ShaderData) => {
			cc.context = res.getContext(shaderPath[2]);
			this.loading--;
		});
	}

	unloadShader = (handle: string) => {
		this.cachedShaderContexts.delete(handle);

		// file/data_name/context
		let shaderPath = handle.split("/");
		// Todo: Handle context overrides (see Data.getShader())
		Data.cachedShaders.delete(shaderPath[1]);
	}

	unload = () => {
		for (let rt of this.renderTargets.values()) rt.unload();
	}

	resize = () => {
		if (System.width == 0 || System.height == 0) return;

		// Make sure depth buffer is attached to single target only and gets released once
		for (let rt of this.renderTargets.values()) {
			if (rt == null ||
				rt.raw.width > 0 ||
				rt.depthStencilFrom == "" ||
				rt == this.depthToRenderTarget.get(rt.depthStencilFrom)) {
				continue;
			}

			let nodepth: RenderTarget = null;
			for (let rt2 of this.renderTargets.values()) {
				if (rt2 == null ||
					rt2.raw.width > 0 ||
					rt2.depthStencilFrom != "" ||
					this.depthToRenderTarget.get(rt2.raw.depth_buffer) != null) {
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
		for (let rt of this.renderTargets.values()) {
			if (rt != null && rt.raw.width == 0) {
				App.notifyOnInit(rt.image.unload);
				rt.image = this.createImage(rt.raw, rt.depthStencil);
			}
		}

		// Attach depth buffers
		for (let rt of this.renderTargets.values()) {
			if (rt != null && rt.depthStencilFrom != "") {
				rt.image.setDepthStencilFrom(this.depthToRenderTarget.get(rt.depthStencilFrom).image);
			}
		}
	}

	createRenderTarget = (t: RenderTargetRaw): RenderTarget => {
		let rt = this.createTarget(t);
		this.renderTargets.set(t.name, rt);
		return rt;
	}

	createDepthBuffer = (name: string, format: string = null) => {
		this.depthBuffers.push({ name: name, format: format });
	}

	createTarget = (t: RenderTargetRaw): RenderTarget => {
		let rt = new RenderTarget(t);
		// With depth buffer
		if (t.depth_buffer != null) {
			rt.hasDepth = true;
			let depthTarget = this.depthToRenderTarget.get(t.depth_buffer);

			if (depthTarget == null) { // Create new one
				for (let db of this.depthBuffers) {
					if (db.name == t.depth_buffer) {
						this.depthToRenderTarget.set(db.name, rt);
						rt.depthStencil = this.getDepthStencilFormat(db.format);
						rt.image = this.createImage(t, rt.depthStencil);
						break;
					}
				}
			}
			else { // Reuse
				rt.depthStencil = DepthStencilFormat.NoDepthAndStencil;
				rt.depthStencilFrom = t.depth_buffer;
				rt.image = this.createImage(t, rt.depthStencil);
				rt.image.setDepthStencilFrom(depthTarget.image);
			}
		}
		else { // No depth buffer
			rt.hasDepth = false;
			if (t.depth != null && t.depth > 1) rt.is3D = true;
			rt.depthStencil = DepthStencilFormat.NoDepthAndStencil;
			rt.image = this.createImage(t, rt.depthStencil);
		}
		return rt;
	}

	createImage = (t: RenderTargetRaw, depthStencil: DepthStencilFormat): Image => {
		let width = t.width == 0 ? App.w() : t.width;
		let height = t.height == 0 ? App.h() : t.height;
		let depth = t.depth != null ? t.depth : 0;
		if (t.displayp != null) { // 1080p/..
			if (width > height) {
				width = Math.floor(width * (t.displayp / height));
				height = t.displayp;
			}
			else {
				height = Math.floor(height * (t.displayp / width));
				width = t.displayp;
			}
		}
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
				t.format != null ? this.getTextureFormat(t.format) : TextureFormat.RGBA32);
			if (t.mipmaps) img.generateMipmaps(1000); // Allocate mipmaps
			return img;
		}
		else { // 2D texture
			if (t.is_image != null && t.is_image) { // Image
				return Image.create(width, height,
					t.format != null ? this.getTextureFormat(t.format) : TextureFormat.RGBA32);
			}
			else { // Render target
				return Image.createRenderTarget(width, height,
					t.format != null ? this.getTextureFormat(t.format) : TextureFormat.RGBA32,
					depthStencil);
			}
		}
	}

	getTextureFormat = (s: string): TextureFormat => {
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

	getDepthStencilFormat = (s: string): DepthStencilFormat => {
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
	displayp: Null<i32> = null; // Set to 1080p/...
	depth_buffer: string = null; // 2D texture
	mipmaps: Null<bool> = null;
	depth: Null<i32> = null; // 3D texture
	is_image: Null<bool> = null; // Image
	constructor() {}
}

class RenderTarget {
	raw: RenderTargetRaw;
	depthStencil: DepthStencilFormat;
	depthStencilFrom = "";
	image: Image = null; // RT or image
	hasDepth = false;
	is3D = false; // sampler2D / sampler3D
	constructor(raw: RenderTargetRaw) { this.raw = raw; }
	unload() {
		if (this.image != null) this.image.unload();
	}
}

class CachedShaderContext {
	context: ShaderContext;
	constructor() {}
}

enum DrawOrder {
	Distance, // Early-z
	Shader, // Less state changes
}
