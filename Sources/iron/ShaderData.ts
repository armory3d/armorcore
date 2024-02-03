
class ShaderData {

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

	static create(raw: TShaderData, done: (sd: TShaderData)=>void, overrideContext: TShaderOverride = null) {
		raw._contexts = [];
		for (let c of raw.contexts) raw._contexts.push(null);
		let contextsLoaded = 0;

		for (let i = 0; i < raw.contexts.length; ++i) {
			let c = raw.contexts[i];
			ShaderContext.create(c, (con: TShaderContext) => {
				raw._contexts[i] = con;
				contextsLoaded++;
				if (contextsLoaded == raw.contexts.length) done(raw);
			}, overrideContext);
		}
	}

	static parse = (file: string, name: string, done: (sd: TShaderData)=>void, overrideContext: TShaderOverride = null) => {
		Data.getSceneRaw(file, (format: TSceneFormat) => {
			let raw: TShaderData = Data.getShaderRawByName(format.shader_datas, name);
			if (raw == null) {
				Krom.log(`Shader data "${name}" not found!`);
				done(null);
			}
			ShaderData.create(raw, done, overrideContext);
		});
	}

	static delete = (raw: TShaderData) => {
		for (let c of raw._contexts) ShaderContext.delete(c);
	}

	static getContext = (raw: TShaderData, name: string): TShaderContext => {
		for (let c of raw._contexts) if (c.name == name) return c;
		return null;
	}
}

class ShaderContext {

	static create(raw: TShaderContext, done: (sc: TShaderContext)=>void, overrideContext: TShaderOverride = null) {
		///if (!arm_voxels)
		if (raw.name == "voxel") {
			done(raw);
			return;
		}
		///end
		raw._overrideContext = overrideContext;
		ShaderContext.parseVertexStructure(raw);
		ShaderContext.compile(raw, done);
	}

	static compile = (raw: TShaderContext, done: (sc: TShaderContext)=>void) => {
		if (raw._pipeState != null) PipelineState.delete(raw._pipeState);
		raw._pipeState = PipelineState.create();
		raw._constants = [];
		raw._textureUnits = [];

		if (raw._instancingType > 0) { // Instancing
			let instStruct = VertexStructure.create();
			VertexStructure.add(instStruct, "ipos", VertexData.F32_3X);
			if (raw._instancingType == 2 || raw._instancingType == 4) {
				VertexStructure.add(instStruct, "irot", VertexData.F32_3X);
			}
			if (raw._instancingType == 3 || raw._instancingType == 4) {
				VertexStructure.add(instStruct, "iscl", VertexData.F32_3X);
			}
			instStruct.instanced = true;
			raw._pipeState.inputLayout = [raw._structure, instStruct];
		}
		else { // Regular
			raw._pipeState.inputLayout = [raw._structure];
		}

		// Depth
		raw._pipeState.depthWrite = raw.depth_write;
		raw._pipeState.depthMode = ShaderContext.getCompareMode(raw.compare_mode);

		// Cull
		raw._pipeState.cullMode = ShaderContext.getCullMode(raw.cull_mode);

		// Blending
		if (raw.blend_source != null) raw._pipeState.blendSource = ShaderContext.getBlendingFactor(raw.blend_source);
		if (raw.blend_destination != null) raw._pipeState.blendDestination = ShaderContext.getBlendingFactor(raw.blend_destination);
		if (raw.alpha_blend_source != null) raw._pipeState.alphaBlendSource = ShaderContext.getBlendingFactor(raw.alpha_blend_source);
		if (raw.alpha_blend_destination != null) raw._pipeState.alphaBlendDestination = ShaderContext.getBlendingFactor(raw.alpha_blend_destination);

		// Per-target color write mask
		if (raw.color_writes_red != null) for (let i = 0; i < raw.color_writes_red.length; ++i) raw._pipeState.colorWriteMasksRed[i] = raw.color_writes_red[i];
		if (raw.color_writes_green != null) for (let i = 0; i < raw.color_writes_green.length; ++i) raw._pipeState.colorWriteMasksGreen[i] = raw.color_writes_green[i];
		if (raw.color_writes_blue != null) for (let i = 0; i < raw.color_writes_blue.length; ++i) raw._pipeState.colorWriteMasksBlue[i] = raw.color_writes_blue[i];
		if (raw.color_writes_alpha != null) for (let i = 0; i < raw.color_writes_alpha.length; ++i) raw._pipeState.colorWriteMasksAlpha[i] = raw.color_writes_alpha[i];

		// Color attachment format
		if (raw.color_attachments != null) {
			raw._pipeState.colorAttachmentCount = raw.color_attachments.length;
			for (let i = 0; i < raw.color_attachments.length; ++i) raw._pipeState.colorAttachments[i] = ShaderContext.getTextureFormat(raw.color_attachments[i]);
		}

		// Depth attachment format
		if (raw.depth_attachment != null) {
			raw._pipeState.depthStencilAttachment = ShaderContext.getDepthStencilFormat(raw.depth_attachment);
		}

		// Shaders
		if (raw.shader_from_source) {
			raw._pipeState.vertexShader = Shader.fromSource(raw.vertex_shader, ShaderType.Vertex);
			raw._pipeState.fragmentShader = Shader.fromSource(raw.fragment_shader, ShaderType.Fragment);

			// Shader compile error
			if (raw._pipeState.vertexShader.shader_ == null || raw._pipeState.fragmentShader.shader_ == null) {
				done(null);
				return;
			}
			ShaderContext.finishCompile(raw, done);
		}
		else {

			///if arm_noembed // Load shaders manually

			let shadersLoaded = 0;
			let numShaders = 2;
			if (raw.geometry_shader != null) numShaders++;

			let loadShader = (file: string, type: i32) => {
				let path = file + ShaderData.ext;
				Data.getBlob(path, (b: ArrayBuffer) => {
					if (type == 0) raw._pipeState.vertexShader = Shader.create(b, ShaderType.Vertex);
					else if (type == 1) raw._pipeState.fragmentShader = Shader.create(b, ShaderType.Fragment);
					else if (type == 2) raw._pipeState.geometryShader = Shader.create(b, ShaderType.Geometry);
					shadersLoaded++;
					if (shadersLoaded >= numShaders) ShaderContext.finishCompile(raw, done);
				});
			}
			loadShader(raw.vertex_shader, 0);
			loadShader(raw.fragment_shader, 1);
			if (raw.geometry_shader != null) loadShader(raw.geometry_shader, 2);

			///else

			raw._pipeState.fragmentShader = System.getShader(raw.fragment_shader);
			raw._pipeState.vertexShader = System.getShader(raw.vertex_shader);
			if (raw.geometry_shader != null) {
				raw._pipeState.geometryShader = System.getShader(raw.geometry_shader);
			}
			ShaderContext.finishCompile(raw, done);

			///end
		}
	}

	static finishCompile = (raw: TShaderContext, done: (sc: TShaderContext)=>void) => {
		// Override specified values
		if (raw._overrideContext != null) {
			if (raw._overrideContext.cull_mode != null) {
				raw._pipeState.cullMode = ShaderContext.getCullMode(raw._overrideContext.cull_mode);
			}
		}

		PipelineState.compile(raw._pipeState);

		if (raw.constants != null) {
			for (let c of raw.constants) ShaderContext.addConstant(raw, c);
		}

		if (raw.texture_units != null) {
			for (let tu of raw.texture_units) ShaderContext.addTexture(raw, tu);
		}

		done(raw);
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

	static parseVertexStructure = (raw: TShaderContext) => {
		raw._structure = VertexStructure.create();
		let ipos = false;
		let irot = false;
		let iscl = false;
		for (let elem of raw.vertex_elements) {
			if (elem.name == "ipos") { ipos = true; continue; }
			if (elem.name == "irot") { irot = true; continue; }
			if (elem.name == "iscl") { iscl = true; continue; }
			VertexStructure.add(raw._structure, elem.name, ShaderContext.parseData(elem.data));
		}
		if (ipos && !irot && !iscl) raw._instancingType = 1;
		else if (ipos && irot && !iscl) raw._instancingType = 2;
		else if (ipos && !irot && iscl) raw._instancingType = 3;
		else if (ipos && irot && iscl) raw._instancingType = 4;
	}

	static delete = (raw: TShaderContext) => {
		if (raw._pipeState.fragmentShader != null) Shader.delete(raw._pipeState.fragmentShader);
		if (raw._pipeState.vertexShader != null) Shader.delete(raw._pipeState.vertexShader);
		if (raw._pipeState.geometryShader != null) Shader.delete(raw._pipeState.geometryShader);
		PipelineState.delete(raw._pipeState);
	}

	static getCompareMode = (s: string): CompareMode => {
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

	static getCullMode = (s: string): CullMode => {
		switch (s) {
			case "none": return CullMode.None;
			case "clockwise": return CullMode.Clockwise;
			default: return CullMode.CounterClockwise;
		}
	}

	static getBlendingFactor = (s: string): BlendingFactor => {
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

	static getTextureAddresing = (s: string): TextureAddressing => {
		switch (s) {
			case "repeat": return TextureAddressing.Repeat;
			case "mirror": return TextureAddressing.Mirror;
			default: return TextureAddressing.Clamp;
		}
	}

	static getTextureFilter = (s: string): TextureFilter => {
		switch (s) {
			case "point": return TextureFilter.PointFilter;
			case "linear": return TextureFilter.LinearFilter;
			default: return TextureFilter.AnisotropicFilter;
		}
	}

	static getMipmapFilter = (s: string): MipMapFilter => {
		switch (s) {
			case "no": return MipMapFilter.NoMipFilter;
			case "point": return MipMapFilter.PointMipFilter;
			default: return MipMapFilter.LinearMipFilter;
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
		switch (s) {
			case "DEPTH32": return DepthStencilFormat.DepthOnly;
			case "NONE": return DepthStencilFormat.NoDepthAndStencil;
			default: return DepthStencilFormat.DepthOnly;
		}
	}

	static addConstant = (raw: TShaderContext, c: TShaderConstant) => {
		raw._constants.push(PipelineState.getConstantLocation(raw._pipeState, c.name));
	}

	static addTexture = (raw: TShaderContext, tu: TTextureUnit) => {
		let unit = PipelineState.getTextureUnit(raw._pipeState, tu.name);
		raw._textureUnits.push(unit);
	}

	static setTextureParameters = (raw: TShaderContext, g: Graphics4Raw, unitIndex: i32, tex: TBindTexture) => {
		// This function is called for samplers set using material context
		let unit = raw._textureUnits[unitIndex];
		Graphics4.setTextureParameters(unit,
			tex.u_addressing == null ? TextureAddressing.Repeat : ShaderContext.getTextureAddresing(tex.u_addressing),
			tex.v_addressing == null ? TextureAddressing.Repeat : ShaderContext.getTextureAddresing(tex.v_addressing),
			tex.min_filter == null ? TextureFilter.LinearFilter : ShaderContext.getTextureFilter(tex.min_filter),
			tex.mag_filter == null ? TextureFilter.LinearFilter : ShaderContext.getTextureFilter(tex.mag_filter),
			tex.mipmap_filter == null ? MipMapFilter.NoMipFilter : ShaderContext.getMipmapFilter(tex.mipmap_filter));
	}
}
