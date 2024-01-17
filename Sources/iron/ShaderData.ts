
class ShaderData {

	name: string;
	raw: TShaderData;
	contexts: ShaderContext[] = [];

	static get ext(): string {
		///if krom_vulkan
		return ".spirv";
		///elseif (krom_android || krom_wasm)
		return ".essl";
		///elseif krom_opengl
		return ".glsl";
		///elseif krom_metal
		return ".metal";
		///else
		return ".d3d11";
		///end
	}

	constructor(raw: TShaderData, done: (sd: ShaderData)=>void, overrideContext: TShaderOverride = null) {
		this.raw = raw;
		this.name = raw.name;

		for (let c of raw.contexts) this.contexts.push(null);
		let contextsLoaded = 0;

		for (let i = 0; i < raw.contexts.length; ++i) {
			let c = raw.contexts[i];
			new ShaderContext(c, (con: ShaderContext) => {
				this.contexts[i] = con;
				contextsLoaded++;
				if (contextsLoaded == raw.contexts.length) done(this);
			}, overrideContext);
		}
	}

	static parse = (file: string, name: string, done: (sd: ShaderData)=>void, overrideContext: TShaderOverride = null) => {
		Data.getSceneRaw(file, (format: TSceneFormat) => {
			let raw: TShaderData = Data.getShaderRawByName(format.shader_datas, name);
			if (raw == null) {
				Krom.log(`Shader data "${name}" not found!`);
				done(null);
			}
			new ShaderData(raw, done, overrideContext);
		});
	}

	delete = () => {
		for (let c of this.contexts) c.delete();
	}

	getContext = (name: string): ShaderContext => {
		for (let c of this.contexts) if (c.raw.name == name) return c;
		return null;
	}
}

class ShaderContext {
	raw: TShaderContext;
	pipeState: PipelineState;
	constants: ConstantLocation[];
	textureUnits: TextureUnit[];
	overrideContext: TShaderOverride;

	structure: VertexStructure;
	instancingType = 0;

	constructor(raw: TShaderContext, done: (sc: ShaderContext)=>void, overrideContext: TShaderOverride = null) {
		this.raw = raw;
		///if (!arm_voxels)
		if (raw.name == "voxel") {
			done(this);
			return;
		}
		///end
		this.overrideContext = overrideContext;
		this.parseVertexStructure();
		this.compile(done);
	}

	compile = (done: (sc: ShaderContext)=>void) => {
		if (this.pipeState != null) this.pipeState.delete();
		this.pipeState = new PipelineState();
		this.constants = [];
		this.textureUnits = [];

		if (this.instancingType > 0) { // Instancing
			let instStruct = new VertexStructure();
			instStruct.add("ipos", VertexData.F32_3X);
			if (this.instancingType == 2 || this.instancingType == 4) {
				instStruct.add("irot", VertexData.F32_3X);
			}
			if (this.instancingType == 3 || this.instancingType == 4) {
				instStruct.add("iscl", VertexData.F32_3X);
			}
			instStruct.instanced = true;
			this.pipeState.inputLayout = [this.structure, instStruct];
		}
		else { // Regular
			this.pipeState.inputLayout = [this.structure];
		}

		// Depth
		this.pipeState.depthWrite = this.raw.depth_write;
		this.pipeState.depthMode = this.getCompareMode(this.raw.compare_mode);

		// Cull
		this.pipeState.cullMode = this.getCullMode(this.raw.cull_mode);

		// Blending
		if (this.raw.blend_source != null) this.pipeState.blendSource = this.getBlendingFactor(this.raw.blend_source);
		if (this.raw.blend_destination != null) this.pipeState.blendDestination = this.getBlendingFactor(this.raw.blend_destination);
		if (this.raw.alpha_blend_source != null) this.pipeState.alphaBlendSource = this.getBlendingFactor(this.raw.alpha_blend_source);
		if (this.raw.alpha_blend_destination != null) this.pipeState.alphaBlendDestination = this.getBlendingFactor(this.raw.alpha_blend_destination);

		// Per-target color write mask
		if (this.raw.color_writes_red != null) for (let i = 0; i < this.raw.color_writes_red.length; ++i) this.pipeState.colorWriteMasksRed[i] = this.raw.color_writes_red[i];
		if (this.raw.color_writes_green != null) for (let i = 0; i < this.raw.color_writes_green.length; ++i) this.pipeState.colorWriteMasksGreen[i] = this.raw.color_writes_green[i];
		if (this.raw.color_writes_blue != null) for (let i = 0; i < this.raw.color_writes_blue.length; ++i) this.pipeState.colorWriteMasksBlue[i] = this.raw.color_writes_blue[i];
		if (this.raw.color_writes_alpha != null) for (let i = 0; i < this.raw.color_writes_alpha.length; ++i) this.pipeState.colorWriteMasksAlpha[i] = this.raw.color_writes_alpha[i];

		// Color attachment format
		if (this.raw.color_attachments != null) {
			this.pipeState.colorAttachmentCount = this.raw.color_attachments.length;
			for (let i = 0; i < this.raw.color_attachments.length; ++i) this.pipeState.colorAttachments[i] = this.getTextureFormat(this.raw.color_attachments[i]);
		}

		// Depth attachment format
		if (this.raw.depth_attachment != null) {
			///if (krom_windows || krom_linux || krom_darwin || krom_android || krom_ios)
			this.pipeState.depthStencilAttachment = this.getDepthStencilFormat(this.raw.depth_attachment);
			///end
		}

		// Shaders
		if (this.raw.shader_from_source) {
			this.pipeState.vertexShader = Shader.fromSource(this.raw.vertex_shader, ShaderType.Vertex);
			this.pipeState.fragmentShader = Shader.fromSource(this.raw.fragment_shader, ShaderType.Fragment);

			// Shader compile error
			if (this.pipeState.vertexShader.shader_ == null || this.pipeState.fragmentShader.shader_ == null) {
				done(null);
				return;
			}
			this.finishCompile(done);
		}
		else {

			///if arm_noembed // Load shaders manually

			let shadersLoaded = 0;
			let numShaders = 2;
			if (this.raw.geometry_shader != null) numShaders++;

			let loadShader = (file: string, type: i32) => {
				let path = file + ShaderData.ext;
				Data.getBlob(path, (b: ArrayBuffer) => {
					if (type == 0) this.pipeState.vertexShader = new Shader(b, ShaderType.Vertex);
					else if (type == 1) this.pipeState.fragmentShader = new Shader(b, ShaderType.Fragment);
					else if (type == 2) this.pipeState.geometryShader = new Shader(b, ShaderType.Geometry);
					shadersLoaded++;
					if (shadersLoaded >= numShaders) this.finishCompile(done);
				});
			}
			loadShader(this.raw.vertex_shader, 0);
			loadShader(this.raw.fragment_shader, 1);
			if (this.raw.geometry_shader != null) loadShader(this.raw.geometry_shader, 2);

			///else

			this.pipeState.fragmentShader = System.getShader(this.raw.fragment_shader);
			this.pipeState.vertexShader = System.getShader(this.raw.vertex_shader);
			if (this.raw.geometry_shader != null) {
				this.pipeState.geometryShader = System.getShader(this.raw.geometry_shader);
			}
			this.finishCompile(done);

			///end
		}
	}

	finishCompile = (done: (sc: ShaderContext)=>void) => {
		// Override specified values
		if (this.overrideContext != null) {
			if (this.overrideContext.cull_mode != null) {
				this.pipeState.cullMode = this.getCullMode(this.overrideContext.cull_mode);
			}
		}

		this.pipeState.compile();

		if (this.raw.constants != null) {
			for (let c of this.raw.constants) this.addConstant(c);
		}

		if (this.raw.texture_units != null) {
			for (let tu of this.raw.texture_units) this.addTexture(tu);
		}

		done(this);
	}

	static parseData = (data: string): VertexData => {
		if (data == "float1") return VertexData.F32_1X;
		else if (data == "float2") return VertexData.F32_2X;
		else if (data == "float3") return VertexData.F32_3X;
		else if (data == "float4") return VertexData.F32_4X;
		else if (data == "short2norm") return VertexData.I16_2X_Normalized;
		else if (data == "short4norm") return VertexData.I16_4X_Normalized;
		return VertexData.F32_1X;
	}

	parseVertexStructure = () => {
		this.structure = new VertexStructure();
		let ipos = false;
		let irot = false;
		let iscl = false;
		for (let elem of this.raw.vertex_elements) {
			if (elem.name == "ipos") { ipos = true; continue; }
			if (elem.name == "irot") { irot = true; continue; }
			if (elem.name == "iscl") { iscl = true; continue; }
			this.structure.add(elem.name, ShaderContext.parseData(elem.data));
		}
		if (ipos && !irot && !iscl) this.instancingType = 1;
		else if (ipos && irot && !iscl) this.instancingType = 2;
		else if (ipos && !irot && iscl) this.instancingType = 3;
		else if (ipos && irot && iscl) this.instancingType = 4;
	}

	delete = () => {
		if (this.pipeState.fragmentShader != null) this.pipeState.fragmentShader.delete();
		if (this.pipeState.vertexShader != null) this.pipeState.vertexShader.delete();
		if (this.pipeState.geometryShader != null) this.pipeState.geometryShader.delete();
		this.pipeState.delete();
	}

	getCompareMode = (s: string): CompareMode => {
		switch (s) {
			case "always": return CompareMode.Always;
			case "never": return CompareMode.Never;
			case "less": return CompareMode.Less;
			case "less_equal": return CompareMode.LessEqual;
			case "greater": return CompareMode.Greater;
			case "greater_equal": return CompareMode.GreaterEqual;
			case "equal": return CompareMode.Equal;
			case "not_equal": return CompareMode.NotEqual;
			default: return CompareMode.Less;
		}
	}

	getCullMode = (s: string): CullMode => {
		switch (s) {
			case "none": return CullMode.None;
			case "clockwise": return CullMode.Clockwise;
			default: return CullMode.CounterClockwise;
		}
	}

	getBlendingFactor = (s: string): BlendingFactor => {
		switch (s) {
			case "blend_one": return BlendingFactor.BlendOne;
			case "blend_zero": return BlendingFactor.BlendZero;
			case "source_alpha": return BlendingFactor.SourceAlpha;
			case "destination_alpha": return BlendingFactor.DestinationAlpha;
			case "inverse_source_alpha": return BlendingFactor.InverseSourceAlpha;
			case "inverse_destination_alpha": return BlendingFactor.InverseDestinationAlpha;
			case "source_color": return BlendingFactor.SourceColor;
			case "destination_color": return BlendingFactor.DestinationColor;
			case "inverse_source_color": return BlendingFactor.InverseSourceColor;
			case "inverse_destination_color": return BlendingFactor.InverseDestinationColor;
			default: return BlendingFactor.BlendOne;
		}
	}

	getTextureAddresing = (s: string): TextureAddressing => {
		switch (s) {
			case "repeat": return TextureAddressing.Repeat;
			case "mirror": return TextureAddressing.Mirror;
			default: return TextureAddressing.Clamp;
		}
	}

	getTextureFilter = (s: string): TextureFilter => {
		switch (s) {
			case "point": return TextureFilter.PointFilter;
			case "linear": return TextureFilter.LinearFilter;
			default: return TextureFilter.AnisotropicFilter;
		}
	}

	getMipmapFilter = (s: string): MipMapFilter => {
		switch (s) {
			case "no": return MipMapFilter.NoMipFilter;
			case "point": return MipMapFilter.PointMipFilter;
			default: return MipMapFilter.LinearMipFilter;
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
		switch (s) {
			case "DEPTH32": return DepthStencilFormat.DepthOnly;
			case "NONE": return DepthStencilFormat.NoDepthAndStencil;
			default: return DepthStencilFormat.DepthOnly;
		}
	}

	addConstant = (c: TShaderConstant) => {
		this.constants.push(this.pipeState.getConstantLocation(c.name));
	}

	addTexture = (tu: TTextureUnit) => {
		let unit = this.pipeState.getTextureUnit(tu.name);
		this.textureUnits.push(unit);
	}

	setTextureParameters = (g: Graphics4, unitIndex: i32, tex: TBindTexture) => {
		// This function is called for samplers set using material context
		let unit = this.textureUnits[unitIndex];
		g.setTextureParameters(unit,
			tex.u_addressing == null ? TextureAddressing.Repeat : this.getTextureAddresing(tex.u_addressing),
			tex.v_addressing == null ? TextureAddressing.Repeat : this.getTextureAddresing(tex.v_addressing),
			tex.min_filter == null ? TextureFilter.LinearFilter : this.getTextureFilter(tex.min_filter),
			tex.mag_filter == null ? TextureFilter.LinearFilter : this.getTextureFilter(tex.mag_filter),
			tex.mipmap_filter == null ? MipMapFilter.NoMipFilter : this.getMipmapFilter(tex.mipmap_filter));
	}
}
