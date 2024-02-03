
class System {
	static renderListeners: ((g2: Graphics2Raw, g4: Graphics4Raw)=>void)[] = [];
	static foregroundListeners: (()=>void)[] = [];
	static resumeListeners: (()=>void)[] = [];
	static pauseListeners: (()=>void)[] = [];
	static backgroundListeners: (()=>void)[] = [];
	static shutdownListeners: (()=>void)[] = [];
	static dropFilesListeners: ((s: string)=>void)[] = [];
	static cutListener: ()=>string = null;
	static copyListener: ()=>string = null;
	static pasteListener: (data: string)=>void = null;

	static startTime: f32;
	static g2: Graphics2Raw;
	static g4: Graphics4Raw;
	static windowTitle: string;

	static start = (options: SystemOptions, callback: ()=>void) => {
		Krom.init(options.title, options.width, options.height, options.vsync, options.mode, options.features, options.x, options.y, options.frequency);

		System.startTime = Krom.getTime();
		System.g4 = Graphics4.create();
		System.g2 = Graphics2.create(System.g4, null);
		Krom.setCallback(System.renderCallback);
		Krom.setDropFilesCallback(System.dropFilesCallback);
		Krom.setCutCopyPasteCallback(System.cutCallback, System.copyCallback, System.pasteCallback);
		Krom.setApplicationStateCallback(System.foregroundCallback, System.resumeCallback, System.pauseCallback, System.backgroundCallback, System.shutdownCallback);
		Krom.setKeyboardDownCallback(System.keyboardDownCallback);
		Krom.setKeyboardUpCallback(System.keyboardUpCallback);
		Krom.setKeyboardPressCallback(System.keyboardPressCallback);
		Krom.setMouseDownCallback(System.mouseDownCallback);
		Krom.setMouseUpCallback(System.mouseUpCallback);
		Krom.setMouseMoveCallback(System.mouseMoveCallback);
		Krom.setMouseWheelCallback(System.mouseWheelCallback);
		Krom.setTouchDownCallback(System.touchDownCallback);
		Krom.setTouchUpCallback(System.touchUpCallback);
		Krom.setTouchMoveCallback(System.touchMoveCallback);
		Krom.setPenDownCallback(System.penDownCallback);
		Krom.setPenUpCallback(System.penUpCallback);
		Krom.setPenMoveCallback(System.penMoveCallback);
		Krom.setGamepadAxisCallback(System.gamepadAxisCallback);
		Krom.setGamepadButtonCallback(System.gamepadButtonCallback);
		Input.register();

		callback();
	}

	static notifyOnFrames = (listener: (g2: Graphics2Raw, g4: Graphics4Raw)=>void) => {
		System.renderListeners.push(listener);
	}

	static notifyOnApplicationState = (foregroundListener: ()=>void, resumeListener: ()=>void, pauseListener: ()=>void, backgroundListener: ()=>void, shutdownListener: ()=>void) => {
		if (foregroundListener != null) System.foregroundListeners.push(foregroundListener);
		if (resumeListener != null) System.resumeListeners.push(resumeListener);
		if (pauseListener != null) System.pauseListeners.push(pauseListener);
		if (backgroundListener != null) System.backgroundListeners.push(backgroundListener);
		if (shutdownListener != null) System.shutdownListeners.push(shutdownListener);
	}

	static notifyOnDropFiles = (dropFilesListener: (s: string)=>void) => {
		System.dropFilesListeners.push(dropFilesListener);
	}

	static notifyOnCutCopyPaste = (cutListener: ()=>string, copyListener: ()=>string, pasteListener: (data: string)=>void) => {
		System.cutListener = cutListener;
		System.copyListener = copyListener;
		System.pasteListener = pasteListener;
	}

	static foreground = () => {
		for (let listener of System.foregroundListeners) {
			listener();
		}
	}

	static resume = () => {
		for (let listener of System.resumeListeners) {
			listener();
		}
	}

	static pause = () => {
		for (let listener of System.pauseListeners) {
			listener();
		}
	}

	static background = () => {
		for (let listener of System.backgroundListeners) {
			listener();
		}
	}

	static shutdown = () => {
		for (let listener of System.shutdownListeners) {
			listener();
		}
	}

	static dropFiles = (filePath: string) => {
		for (let listener of System.dropFilesListeners) {
			listener(filePath);
		}
	}

	static get time(): f32 {
		return Krom.getTime() - System.startTime;
	}

	static get systemId(): string {
		return Krom.systemId();
	}

	static get language(): string {
		return Krom.language();
	}

	static stop = () => {
		Krom.requestShutdown();
	}

	static loadUrl = (url: string) => {
		Krom.loadUrl(url);
	}

	static renderCallback = () => {
		for (let listener of System.renderListeners) {
			listener(System.g2, System.g4);
		}
	}

	static dropFilesCallback = (filePath: string) => {
		System.dropFiles(filePath);
	}

	static copyCallback = (): string => {
		if (System.copyListener != null) {
			return System.copyListener();
		}
		return null;
	}

	static cutCallback = (): string => {
		if (System.cutListener != null) {
			return System.cutListener();
		}
		return null;
	}

	static pasteCallback = (data: string) => {
		if (System.pasteListener != null) {
			System.pasteListener(data);
		}
	}

	static foregroundCallback = () => {
		System.foreground();
	}

	static resumeCallback = () => {
		System.resume();
	}

	static pauseCallback = () => {
		System.pause();
	}

	static backgroundCallback = () => {
		System.background();
	}

	static shutdownCallback = () => {
		System.shutdown();
	}

	static keyboardDownCallback = (code: i32) => {
		Keyboard.downListener(code);
	}

	static keyboardUpCallback = (code: i32) => {
		Keyboard.upListener(code);
	}

	static keyboardPressCallback = (charCode: i32) => {
		Keyboard.pressListener(String.fromCharCode(charCode));
	}

	static mouseDownCallback = (button: i32, x: i32, y: i32) => {
		Mouse.downListener(button, x, y);
	}

	static mouseUpCallback = (button: i32, x: i32, y: i32) => {
		Mouse.upListener(button, x, y);
	}

	static mouseMoveCallback = (x: i32, y: i32, mx: i32, my: i32) => {
		Mouse.moveListener(x, y, mx, my);
	}

	static mouseWheelCallback = (delta: i32) => {
		Mouse.wheelListener(delta);
	}

	static touchDownCallback = (index: i32, x: i32, y: i32) => {
		///if (krom_android || krom_ios)
		Surface.onTouchDown(index, x, y);
		///end
	}

	static touchUpCallback = (index: i32, x: i32, y: i32) => {
		///if (krom_android || krom_ios)
		Surface.onTouchUp(index, x, y);
		///end
	}

	static touchMoveCallback = (index: i32, x: i32, y: i32) => {
		///if (krom_android || krom_ios)
		Surface.onTouchMove(index, x, y);
		///end
	}

	static penDownCallback = (x: i32, y: i32, pressure: f32) => {
		Pen.downListener(x, y, pressure);
	}

	static penUpCallback = (x: i32, y: i32, pressure: f32) => {
		Pen.upListener(x, y, pressure);
	}

	static penMoveCallback = (x: i32, y: i32, pressure: f32) => {
		Pen.moveListener(x, y, pressure);
	}

	static gamepadAxisCallback = (gamepad: i32, axis: i32, value: f32) => {
		Gamepad.axisListener(gamepad, axis, value);
	}

	static gamepadButtonCallback = (gamepad: i32, button: i32, value: f32) => {
		Gamepad.buttonListener(gamepad, button, value);
	}

	static lockMouse = () => {
		if (!System.isMouseLocked()){
			Krom.lockMouse();
		}
	}

	static unlockMouse = () => {
		if (System.isMouseLocked()){
			Krom.unlockMouse();
		}
	}

	static canLockMouse = (): bool => {
		return Krom.canLockMouse();
	}

	static isMouseLocked = (): bool => {
		return Krom.isMouseLocked();
	}

	static hideSystemCursor = () => {
		Krom.showMouse(false);
	}

	static showSystemCursor = () => {
		Krom.showMouse(true);
	}

	static resize = (width: i32, height: i32) => {
		Krom.resizeWindow(width, height);
	}

	static move = (x: i32, y: i32) => {
		Krom.moveWindow(x, y);
	}

	static get x(): i32 {
		return Krom.windowX();
	}

	static get y(): i32 {
		return Krom.windowY();
	}

	static get width(): i32 {
		return Krom.windowWidth();
	}

	static get height(): i32 {
		return Krom.windowHeight();
	}

	static get mode(): WindowMode {
		return Krom.getWindowMode();
	}

	static set mode(mode: WindowMode) {
		Krom.setWindowMode(mode);
	}

	static get title(): string {
		return System.windowTitle;
	}

	static set title(value: string) {
		Krom.setWindowTitle(value);
		System.windowTitle = value;
	}

	static displayPrimaryId = (): i32 => {
		for (let i = 0; i < Krom.displayCount(); ++i) {
			if (Krom.displayIsPrimary(i)) return i;
		}
		return 0;
	}

	static displayWidth = (): i32 => {
		return Krom.displayWidth(System.displayPrimaryId());
	}

	static displayHeight = (): i32 => {
		return Krom.displayHeight(System.displayPrimaryId());
	}

	static displayFrequency = (): i32 => {
		return Krom.displayFrequency(System.displayPrimaryId());
	}

	static bufferToString = (b: ArrayBuffer): string => {
		let str = "";
		let u8a = new Uint8Array(b);
		for (let i = 0; i < u8a.length; ++i) {
			str += String.fromCharCode(u8a[i]);
		}
		return str;
	}

	static stringToBuffer = (str: string): ArrayBuffer => {
		let u8a = new Uint8Array(str.length);
		for (let i = 0; i < str.length; ++i) {
			u8a[i] = str.charCodeAt(i);
		}
		return u8a.buffer;
	}

	static shaders: Map<string, ShaderRaw> = new Map();

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

	static getShaderBuffer = (name: string): ArrayBuffer => {
		///if arm_shader_embed
		let global: any = globalThis;
		return global["data/" + name + System.ext];
		///else
		return Krom.loadBlob("data/" + name + System.ext);
		///end
	}

	static getShader = (name: string): ShaderRaw => {
		let shader = System.shaders.get(name);
		if (shader == null) {
			shader = Shader.create(System.getShaderBuffer(name), name.endsWith(".frag") ? ShaderType.Fragment : name.endsWith(".vert") ? ShaderType.Vertex : ShaderType.Geometry);
			System.shaders.set(name, shader);
		}
		return shader;
	}
}

class VideoRaw {
	video_: any;
}

class Video {
	static unload = (self: VideoRaw) => {}
}

class SoundRaw {
	sound_: any;
}

class Sound {
	static create(sound_: any): SoundRaw {
		let raw = new SoundRaw();
		raw.sound_ = sound_;
		return raw;
	}

	static unload = (raw: SoundRaw) => {
		Krom.unloadSound(raw.sound_);
	}
}

class ShaderRaw {
	shader_: any;
}

class Shader {
	static create(buffer: ArrayBuffer, type: ShaderType): ShaderRaw {
		let raw = new ShaderRaw();
		if (buffer != null) {
			raw.shader_ = Krom.createShader(buffer, type);
		}
		return raw;
	}

	static fromSource = (source: string, type: ShaderType): ShaderRaw => {
		let shader = Shader.create(null, 0);
		if (type == ShaderType.Vertex) {
			shader.shader_ = Krom.createVertexShaderFromSource(source);
		}
		else if (type == ShaderType.Fragment) {
			shader.shader_ = Krom.createFragmentShaderFromSource(source);
		}
		return shader;
	}

	static delete = (raw: ShaderRaw) => {
		Krom.deleteShader(raw.shader_);
	}
}

class PipelineStateRaw {
	pipeline_: any;
	inputLayout: VertexStructureRaw[] = null;
	vertexShader: ShaderRaw = null;
	fragmentShader: ShaderRaw = null;
	geometryShader: ShaderRaw = null;
	cullMode: CullMode;
	depthWrite: bool;
	depthMode: CompareMode;
	blendSource: BlendingFactor;
	blendDestination: BlendingFactor;
	alphaBlendSource: BlendingFactor;
	alphaBlendDestination: BlendingFactor;
	colorWriteMasksRed: bool[];
	colorWriteMasksGreen: bool[];
	colorWriteMasksBlue: bool[];
	colorWriteMasksAlpha: bool[];
	colorAttachmentCount: i32;
	colorAttachments: TextureFormat[];
	depthStencilAttachment: DepthStencilFormat;
}

class PipelineState {
	static getDepthBufferBits = (depthAndStencil: DepthStencilFormat): i32 => {
		switch (depthAndStencil) {
			case DepthStencilFormat.NoDepthAndStencil: return 0;
			case DepthStencilFormat.DepthOnly: return 24;
			case DepthStencilFormat.DepthAutoStencilAuto: return 24;
			case DepthStencilFormat.Depth24Stencil8: return 24;
			case DepthStencilFormat.Depth32Stencil8: return 32;
			case DepthStencilFormat.Depth16: return 16;
		}
		return 0;
	}

	static getStencilBufferBits = (depthAndStencil: DepthStencilFormat): i32 => {
		switch (depthAndStencil) {
			case DepthStencilFormat.NoDepthAndStencil: return 0;
			case DepthStencilFormat.DepthOnly: return 0;
			case DepthStencilFormat.DepthAutoStencilAuto: return 8;
			case DepthStencilFormat.Depth24Stencil8: return 8;
			case DepthStencilFormat.Depth32Stencil8: return 8;
			case DepthStencilFormat.Depth16: return 0;
		}
		return 0;
	}

	static create(): PipelineStateRaw {
		let raw = new PipelineStateRaw();
		raw.cullMode = CullMode.None;
		raw.depthWrite = false;
		raw.depthMode = CompareMode.Always;

		raw.blendSource = BlendingFactor.BlendOne;
		raw.blendDestination = BlendingFactor.BlendZero;
		raw.alphaBlendSource = BlendingFactor.BlendOne;
		raw.alphaBlendDestination = BlendingFactor.BlendZero;

		raw.colorWriteMasksRed = [];
		raw.colorWriteMasksGreen = [];
		raw.colorWriteMasksBlue = [];
		raw.colorWriteMasksAlpha = [];
		for (let i = 0; i < 8; ++i) raw.colorWriteMasksRed.push(true);
		for (let i = 0; i < 8; ++i) raw.colorWriteMasksGreen.push(true);
		for (let i = 0; i < 8; ++i) raw.colorWriteMasksBlue.push(true);
		for (let i = 0; i < 8; ++i) raw.colorWriteMasksAlpha.push(true);

		raw.colorAttachmentCount = 1;
		raw.colorAttachments = [];
		for (let i = 0; i < 8; ++i) raw.colorAttachments.push(TextureFormat.RGBA32);
		raw.depthStencilAttachment = DepthStencilFormat.NoDepthAndStencil;

		raw.pipeline_ = Krom.createPipeline();
		return raw;
	}

	static delete = (raw: PipelineStateRaw) => {
		Krom.deletePipeline(raw.pipeline_);
	}

	static compile = (raw: PipelineStateRaw) => {
		let structure0 = raw.inputLayout.length > 0 ? raw.inputLayout[0] : null;
		let structure1 = raw.inputLayout.length > 1 ? raw.inputLayout[1] : null;
		let structure2 = raw.inputLayout.length > 2 ? raw.inputLayout[2] : null;
		let structure3 = raw.inputLayout.length > 3 ? raw.inputLayout[3] : null;
		let gs = raw.geometryShader != null ? raw.geometryShader.shader_ : null;
		let colorAttachments: i32[] = [];
		for (let i = 0; i < 8; ++i) {
			colorAttachments.push(raw.colorAttachments[i]);
		}
		Krom.compilePipeline(raw.pipeline_, structure0, structure1, structure2, structure3, raw.inputLayout.length, raw.vertexShader.shader_, raw.fragmentShader.shader_, gs, {
			cullMode: raw.cullMode,
			depthWrite: raw.depthWrite,
			depthMode: raw.depthMode,
			blendSource: raw.blendSource,
			blendDestination: raw.blendDestination,
			alphaBlendSource: raw.alphaBlendSource,
			alphaBlendDestination: raw.alphaBlendDestination,
			colorWriteMaskRed: raw.colorWriteMasksRed,
			colorWriteMaskGreen: raw.colorWriteMasksGreen,
			colorWriteMaskBlue: raw.colorWriteMasksBlue,
			colorWriteMaskAlpha: raw.colorWriteMasksAlpha,
			colorAttachmentCount: raw.colorAttachmentCount,
			colorAttachments: raw.colorAttachments,
			depthAttachmentBits: PipelineState.getDepthBufferBits(raw.depthStencilAttachment),
			stencilAttachmentBits: PipelineState.getStencilBufferBits(raw.depthStencilAttachment)
		});
	}

	static set = (raw: PipelineStateRaw) => {
		Krom.setPipeline(raw.pipeline_);
	}

	static getConstantLocation = (raw: PipelineStateRaw, name: string): ConstantLocation => {
		return Krom.getConstantLocation(raw.pipeline_, name);
	}

	static getTextureUnit = (raw: PipelineStateRaw, name: string): TextureUnit => {
		return Krom.getTextureUnit(raw.pipeline_, name);
	}
}

class VertexBufferRaw {
	buffer_: any;
	vertexCount: i32;
}

class VertexBuffer {

	static create(vertexCount: i32, structure: VertexStructureRaw, usage: Usage, instanceDataStepRate: i32 = 0): VertexBufferRaw {
		let raw = new VertexBufferRaw();
		raw.vertexCount = vertexCount;
		raw.buffer_ = Krom.createVertexBuffer(vertexCount, structure.elements, usage, instanceDataStepRate);
		return raw;
	}

	static delete = (raw: VertexBufferRaw) => {
		Krom.deleteVertexBuffer(raw.buffer_);
	}

	static lock = (raw: VertexBufferRaw): DataView => {
		return new DataView(Krom.lockVertexBuffer(raw.buffer_, 0, raw.vertexCount));
	}

	static unlock = (raw: VertexBufferRaw) => {
		Krom.unlockVertexBuffer(raw.buffer_, raw.vertexCount);
	}

	static set = (raw: VertexBufferRaw) => {
		Krom.setVertexBuffer(raw.buffer_);
	}
}

class VertexStructureRaw {
	elements: VertexElement[] = [];
	instanced: bool = false;
}

class VertexStructure {
	static create(): VertexStructureRaw {
		return new VertexStructureRaw();
	}

	static add = (raw: VertexStructureRaw, name: string, data: VertexData) => {
		raw.elements.push({ name: name, data: data });
	}

	static byteSize = (raw: VertexStructureRaw): i32 => {
		let byteSize = 0;
		for (let i = 0; i < raw.elements.length; ++i) {
			byteSize += VertexStructure.dataByteSize(raw.elements[i].data);
		}
		return byteSize;
	}

	static dataByteSize = (data: VertexData): i32 => {
		switch (data) {
			case VertexData.F32_1X:
				return 1 * 4;
			case VertexData.F32_2X:
				return 2 * 4;
			case VertexData.F32_3X:
				return 3 * 4;
			case VertexData.F32_4X:
				return 4 * 4;
			case VertexData.U8_4X_Normalized:
				return 4 * 1;
			case VertexData.I16_2X_Normalized:
				return 2 * 2;
			case VertexData.I16_4X_Normalized:
				return 4 * 2;
		}
		return 0;
	}
}

class IndexBufferRaw {
	buffer_: any;
}

class IndexBuffer {

	static create(indexCount: i32, usage: Usage): IndexBufferRaw {
		let raw = new IndexBufferRaw();
		raw.buffer_ = Krom.createIndexBuffer(indexCount);
		return raw;
	}

	static delete = (raw: IndexBufferRaw) => {
		Krom.deleteIndexBuffer(raw.buffer_);
	}

	static lock = (raw: IndexBufferRaw): Uint32Array => {
		return Krom.lockIndexBuffer(raw.buffer_);
	}

	static unlock = (raw: IndexBufferRaw) => {
		Krom.unlockIndexBuffer(raw.buffer_);
	}

	static set = (raw: IndexBufferRaw) => {
		Krom.setIndexBuffer(raw.buffer_);
	}
}

type Color = i32;

class FontRaw {
	font_: any = null;
	blob: ArrayBuffer;
	fontGlyphs: i32[] = null;
	fontIndex = 0;
}

class Font {

	static create(blob: ArrayBuffer, fontIndex = 0): FontRaw {
		let raw = new FontRaw();
		raw.blob = blob;
		raw.fontIndex = fontIndex;
		return raw;
	}

	static height = (raw: FontRaw, fontSize: i32): f32 => {
		Font.init(raw);
		return Krom.g2_font_height(raw.font_, fontSize);
	}

	static width = (raw: FontRaw, fontSize: i32, str: string): f32 => {
		Font.init(raw);
		return Krom.g2_string_width(raw.font_, fontSize, str);
	}

	static unload = (raw: FontRaw) => {
		raw.blob = null;
	}

	static setFontIndex = (raw: FontRaw, fontIndex: i32) => {
		raw.fontIndex = fontIndex;
		Graphics2.fontGlyphs = Graphics2.fontGlyphs.slice(); // Trigger atlas update
	}

	static clone = (raw: FontRaw): FontRaw => {
		return Font.create(raw.blob, raw.fontIndex);
	}

	static init = (raw: FontRaw) => {
		if (Graphics2.fontGlyphsLast != Graphics2.fontGlyphs) {
			Graphics2.fontGlyphsLast = Graphics2.fontGlyphs;
			Krom.g2_font_set_glyphs(Graphics2.fontGlyphs);
		}
		if (raw.fontGlyphs != Graphics2.fontGlyphs) {
			raw.fontGlyphs = Graphics2.fontGlyphs;
			raw.font_ = Krom.g2_font_init(raw.blob, raw.fontIndex);
		}
	}
}

class ImageRaw {
	texture_: any;
	renderTarget_: any;
	format: TextureFormat;
	readable: bool;
	graphics2: Graphics2Raw;
	graphics4: Graphics4Raw;
	pixels: ArrayBuffer = null;

	get width(): i32 { return this.texture_ == null ? this.renderTarget_.width : this.texture_.width; }

	get height(): i32 { return this.texture_ == null ? this.renderTarget_.height : this.texture_.height; }

	get depth(): i32 { return this.texture_ != null ? this.texture_.depth : 1; }

	get g2(): Graphics2Raw {
		if (this.graphics2 == null) {
			this.graphics2 = Graphics2.create(this.g4, this);
		}
		return this.graphics2;
	}

	get g4(): Graphics4Raw {
		if (this.graphics4 == null) {
			this.graphics4 = Graphics4.create(this);
		}
		return this.graphics4;
	}
}

class Image {

	static _create(texture: any): ImageRaw {
		let raw = new ImageRaw();
		raw.texture_ = texture;
		return raw;
	}

	static getDepthBufferBits = (depthAndStencil: DepthStencilFormat): i32 => {
		switch (depthAndStencil) {
			case DepthStencilFormat.NoDepthAndStencil: return -1;
			case DepthStencilFormat.DepthOnly: return 24;
			case DepthStencilFormat.DepthAutoStencilAuto: return 24;
			case DepthStencilFormat.Depth24Stencil8: return 24;
			case DepthStencilFormat.Depth32Stencil8: return 32;
			case DepthStencilFormat.Depth16: return 16;
		}
		return 0;
	}

	static getStencilBufferBits = (depthAndStencil: DepthStencilFormat): i32 => {
		switch (depthAndStencil) {
			case DepthStencilFormat.NoDepthAndStencil: return -1;
			case DepthStencilFormat.DepthOnly: return -1;
			case DepthStencilFormat.DepthAutoStencilAuto: return 8;
			case DepthStencilFormat.Depth24Stencil8: return 8;
			case DepthStencilFormat.Depth32Stencil8: return 8;
			case DepthStencilFormat.Depth16: return 0;
		}
		return 0;
	}

	static getTextureFormat = (format: TextureFormat): i32 => {
		switch (format) {
		case TextureFormat.RGBA32:
			return 0;
		case TextureFormat.RGBA128:
			return 3;
		case TextureFormat.RGBA64:
			return 4;
		case TextureFormat.R32:
			return 5;
		case TextureFormat.R16:
			return 7;
		default:
			return 1; // R8
		}
	}

	static _fromTexture = (texture: any): ImageRaw => {
		return Image._create(texture);
	}

	static fromBytes = (buffer: ArrayBuffer, width: i32, height: i32, format: TextureFormat = null, usage: Usage = null): ImageRaw => {
		if (format == null) format = TextureFormat.RGBA32;
		let readable = true;
		let image = Image._create(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes(buffer, width, height, Image.getTextureFormat(format), readable);
		return image;
	}

	static fromBytes3D = (buffer: ArrayBuffer, width: i32, height: i32, depth: i32, format: TextureFormat = null, usage: Usage = null): ImageRaw => {
		if (format == null) format = TextureFormat.RGBA32;
		let readable = true;
		let image = Image._create(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes3D(buffer, width, height, depth, Image.getTextureFormat(format), readable);
		return image;
	}

	static fromEncodedBytes = (buffer: ArrayBuffer, format: string, doneCallback: (img: ImageRaw)=>void, errorCallback: (s: string)=>void, readable: bool = false) => {
		let image = Image._create(null);
		image.texture_ = Krom.createTextureFromEncodedBytes(buffer, format, readable);
		doneCallback(image);
	}

	static create = (width: i32, height: i32, format: TextureFormat = null, usage: Usage = null): ImageRaw => {
		if (format == null) format = TextureFormat.RGBA32;
		let image = Image._create(null);
		image.format = format;
		image.texture_ = Krom.createTexture(width, height, Image.getTextureFormat(format));
		return image;
	}

	static create3D = (width: i32, height: i32, depth: i32, format: TextureFormat = null, usage: Usage = null): ImageRaw => {
		if (format == null) format = TextureFormat.RGBA32;
		let image = Image._create(null);
		image.format = format;
		image.texture_ = Krom.createTexture3D(width, height, depth, Image.getTextureFormat(format));
		return image;
	}

	static createRenderTarget = (width: i32, height: i32, format: TextureFormat = null, depthStencil: DepthStencilFormat = DepthStencilFormat.NoDepthAndStencil, antiAliasingSamples: i32 = 1): ImageRaw => {
		if (format == null) format = TextureFormat.RGBA32;
		let image = Image._create(null);
		image.format = format;
		image.renderTarget_ = Krom.createRenderTarget(width, height, format, Image.getDepthBufferBits(depthStencil), Image.getStencilBufferBits(depthStencil));
		return image;
	}

	static renderTargetsInvertedY = (): bool => {
		return Krom.renderTargetsInvertedY();
	}

	static formatByteSize = (format: TextureFormat): i32 => {
		switch(format) {
			case TextureFormat.RGBA32: return 4;
			case TextureFormat.R8: return 1;
			case TextureFormat.RGBA128: return 16;
			case TextureFormat.DEPTH16: return 2;
			case TextureFormat.RGBA64: return 8;
			case TextureFormat.R32: return 4;
			case TextureFormat.R16: return 2;
			default: return 4;
		}
	}

	static unload = (raw: ImageRaw) => {
		Krom.unloadImage(raw);
		raw.texture_ = null;
		raw.renderTarget_ = null;
	}

	static lock = (raw: ImageRaw, level: i32 = 0): ArrayBuffer => {
		return Krom.lockTexture(raw.texture_, level);
	}

	static unlock = (raw: ImageRaw) => {
		Krom.unlockTexture(raw.texture_);
	}

	static getPixels = (raw: ImageRaw): ArrayBuffer => {
		if (raw.renderTarget_ != null) {
			// Minimum size of 32x32 required after https://github.com/Kode/Kinc/commit/3797ebce5f6d7d360db3331eba28a17d1be87833
			let pixelsWidth = raw.width < 32 ? 32 : raw.width;
			let pixelsHeight = raw.height < 32 ? 32 : raw.height;
			if (raw.pixels == null) raw.pixels = new ArrayBuffer(Image.formatByteSize(raw.format) * pixelsWidth * pixelsHeight);
			Krom.getRenderTargetPixels(raw.renderTarget_, raw.pixels);
			return raw.pixels;
		}
		else {
			return Krom.getTexturePixels(raw.texture_);
		}
	}

	static generateMipmaps = (raw: ImageRaw, levels: i32) => {
		raw.texture_ == null ? Krom.generateRenderTargetMipmaps(raw.renderTarget_, levels) : Krom.generateTextureMipmaps(raw.texture_, levels);
	}

	static setMipmaps = (raw: ImageRaw, mipmaps: ImageRaw[]) => {
		Krom.setMipmaps(raw.texture_, mipmaps);
	}

	static setDepthStencilFrom = (raw: ImageRaw, image: ImageRaw) => {
		Krom.setDepthStencilFrom(raw.renderTarget_, image.renderTarget_);
	}

	static clear = (raw: ImageRaw, x: i32, y: i32, z: i32, width: i32, height: i32, depth: i32, color: Color) => {
		Krom.clearTexture(raw.texture_, x, y, z, width, height, depth, color);
	}
}

class Graphics2Raw {
	_color: Color;
	_font: FontRaw;
	_fontSize: i32 = 0;
	_pipeline: PipelineStateRaw;
	_imageScaleQuality: ImageScaleQuality;
	_transformation: Mat3 = null;

	g4: Graphics4Raw;
	renderTarget: ImageRaw;

	get color(): Color {
		return this._color;
	}

	set color(c: Color) {
		Krom.g2_set_color(c);
		this._color = c;
	}

	set_font_and_size = (font: FontRaw, fontSize: i32) => {
		Font.init(font);
		Krom.g2_set_font(font.font_, fontSize);
	}

	get font(): FontRaw {
		return this._font;
	}

	set font(f: FontRaw) {
		if (this.fontSize != 0) this.set_font_and_size(f, this.fontSize);
		this._font = f;
	}

	get fontSize(): i32 {
		return this._fontSize;
	}

	set fontSize(i: i32) {
		if (this.font.font_ != null) this.set_font_and_size(this.font, i);
		this._fontSize = i;
	}

	get pipeline(): PipelineStateRaw {
		return this._pipeline;
	}

	set pipeline(p: PipelineStateRaw) {
		Krom.g2_set_pipeline(p == null ? null : p.pipeline_);
		this._pipeline = p;
	}

	get imgeScaleQuality(): ImageScaleQuality {
		return this._imageScaleQuality;
	}

	set imageScaleQuality(q: ImageScaleQuality) {
		Krom.g2_set_bilinear_filter(q == ImageScaleQuality.High);
		this._imageScaleQuality = q;
	}

	set transformation(m: Mat3) {
		if (m == null) {
			Krom.g2_set_transform(null);
		}
		else {
			Graphics2.mat[0] = m._00; Graphics2.mat[1] = m._01; Graphics2.mat[2] = m._02;
			Graphics2.mat[3] = m._10; Graphics2.mat[4] = m._11; Graphics2.mat[5] = m._12;
			Graphics2.mat[6] = m._20; Graphics2.mat[7] = m._21; Graphics2.mat[8] = m._22;
			Krom.g2_set_transform(Graphics2.mat.buffer);
		}
	}
}

class Graphics2 {
	static makeGlyphs = (start: i32, end: i32): i32[] => {
		let ar: i32[] = [];
		for (let i = start; i < end; ++i) ar.push(i);
		return ar;
	}

	static current: Graphics2Raw;
	static fontGlyphs: i32[] = Graphics2.makeGlyphs(32, 127);
	static fontGlyphsLast: i32[] = Graphics2.fontGlyphs;
	static thrown = false;
	static mat = new Float32Array(9);
	static initialized = false;

	static create(g4: Graphics4Raw, renderTarget: ImageRaw): Graphics2Raw {
		let raw = new Graphics2Raw();
		if (!Graphics2.initialized) {
			Krom.g2_init(
				System.getShaderBuffer("painter-image.vert"),
				System.getShaderBuffer("painter-image.frag"),
				System.getShaderBuffer("painter-colored.vert"),
				System.getShaderBuffer("painter-colored.frag"),
				System.getShaderBuffer("painter-text.vert"),
				System.getShaderBuffer("painter-text.frag")
			);
			Graphics2.initialized = true;
		}
		raw.g4 = g4;
		raw.renderTarget = renderTarget;
		return raw;
	}

	static drawScaledSubImage = (img: ImageRaw, sx: f32, sy: f32, sw: f32, sh: f32, dx: f32, dy: f32, dw: f32, dh: f32) => {
		Krom.g2_draw_scaled_sub_image(img, sx, sy, sw, sh, dx, dy, dw, dh);
	}

	static drawSubImage = (img: ImageRaw, x: f32, y: f32, sx: f32, sy: f32, sw: f32, sh: f32) => {
		Graphics2.drawScaledSubImage(img, sx, sy, sw, sh, x, y, sw, sh);
	}

	static drawScaledImage = (img: ImageRaw, dx: f32, dy: f32, dw: f32, dh: f32) => {
		Graphics2.drawScaledSubImage(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
	}

	static drawImage = (img: ImageRaw, x: f32, y: f32) => {
		Graphics2.drawScaledSubImage(img, 0, 0, img.width, img.height, x, y, img.width, img.height);
	}

	static drawRect = (x: f32, y: f32, width: f32, height: f32, strength: f32 = 1.0) => {
		Krom.g2_draw_rect(x, y, width, height, strength);
	}

	static fillRect = (x: f32, y: f32, width: f32, height: f32) => {
		Krom.g2_fill_rect(x, y, width, height);
	}

	static drawString = (text: string, x: f32, y: f32) => {
		Krom.g2_draw_string(text, x, y);
	}

	static drawLine = (x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0) => {
		Krom.g2_draw_line(x0, y0, x1, y1, strength);
	}

	static fillTriangle = (x0: f32, y0: f32, x1: f32, y1: f32, x2: f32, y2: f32) => {
		Krom.g2_fill_triangle(x0, y0, x1, y1, x2, y2);
	}

	static scissor = (raw: Graphics2Raw, x: i32, y: i32, width: i32, height: i32) => {
		Krom.g2_end(); // flush
		Graphics4.scissor(x, y, width, height);
	}

	static disableScissor = (raw: Graphics2Raw) => {
		Krom.g2_end(); // flush
		Graphics4.disableScissor();
	}

	static begin = (raw: Graphics2Raw, clear = true, clearColor: Color = null) => {
		if (Graphics2.current == null) {
			Graphics2.current = raw;
		}
		else {
			if (!Graphics2.thrown) { Graphics2.thrown = true; throw "End before you begin"; }
		}

		Krom.g2_begin();

		if (raw.renderTarget != null) {
			Krom.g2_set_render_target(raw.renderTarget.renderTarget_);
		}
		else {
			Krom.g2_restore_render_target();
		}

		if (clear) Graphics2.clear(raw, clearColor);
	}

	static clear = (raw: Graphics2Raw, color = 0x00000000) => {
		Graphics4.clear(color);
	}

	static end = (raw: Graphics2Raw) => {
		Krom.g2_end();

		if (Graphics2.current == raw) {
			Graphics2.current = null;
		}
		else {
			if (!Graphics2.thrown) { Graphics2.thrown = true; throw "Begin before you end"; }
		}
	}

	static fillCircle = (cx: f32, cy: f32, radius: f32, segments: i32 = 0) => {
		Krom.g2_fill_circle(cx, cy, radius, segments);
	}

	static drawCircle = (cx: f32, cy: f32, radius: f32, segments: i32 = 0, strength: f32 = 1.0) => {
		Krom.g2_draw_circle(cx, cy, radius, segments, strength);
	}

	static drawCubicBezier = (x: f32[], y: f32[], segments: i32 = 20, strength: f32 = 1.0) => {
		Krom.g2_draw_cubic_bezier(x, y, segments, strength);
	}
}

class Graphics4Raw {
	renderTarget: ImageRaw;
}

class Graphics4 {

	static create(renderTarget: ImageRaw = null): Graphics4Raw {
		let raw = new Graphics4Raw();
		raw.renderTarget = renderTarget;
		return raw;
	}

	static begin = (raw: Graphics4Raw, additionalRenderTargets: ImageRaw[] = null) => {
		Krom.begin(raw.renderTarget, additionalRenderTargets);
	}

	static end = () => {
		Krom.end();
	}

	static clear = (color?: Color, depth?: f32, stencil?: i32) => {
		let flags: i32 = 0;
		if (color != null) flags |= 1;
		if (depth != null) flags |= 2;
		if (stencil != null) flags |= 4;
		Krom.clear(flags, color == null ? 0 : color, depth, stencil);
	}

	static viewport = (x: i32, y: i32, width: i32, height: i32) => {
		Krom.viewport(x, y, width, height);
	}

	static setVertexBuffer = (vertexBuffer: VertexBufferRaw) => {
		VertexBuffer.set(vertexBuffer);
	}

	static setVertexBuffers = (vertexBuffers: VertexBufferRaw[]) => {
		Krom.setVertexBuffers(vertexBuffers);
	}

	static setIndexBuffer = (indexBuffer: IndexBufferRaw) => {
		IndexBuffer.set(indexBuffer);
	}

	static setTexture = (unit: TextureUnit, texture: ImageRaw) => {
		if (texture == null) return;
		texture.texture_ != null ? Krom.setTexture(unit, texture.texture_) : Krom.setRenderTarget(unit, texture.renderTarget_);
	}

	static setTextureDepth = (unit: TextureUnit, texture: ImageRaw) => {
		if (texture == null) return;
		Krom.setTextureDepth(unit, texture.renderTarget_);
	}

	static setImageTexture = (unit: TextureUnit, texture: ImageRaw) => {
		if (texture == null) return;
		Krom.setImageTexture(unit, texture.texture_);
	}

	static setTextureParameters = (texunit: TextureUnit, uAddressing: TextureAddressing, vAddressing: TextureAddressing, minificationFilter: TextureFilter, magnificationFilter: TextureFilter, mipmapFilter: MipMapFilter) => {
		Krom.setTextureParameters(texunit, uAddressing, vAddressing, minificationFilter, magnificationFilter, mipmapFilter);
	}

	static setTexture3DParameters = (texunit: TextureUnit, uAddressing: TextureAddressing, vAddressing: TextureAddressing, wAddressing: TextureAddressing, minificationFilter: TextureFilter, magnificationFilter: TextureFilter, mipmapFilter: MipMapFilter) => {
		Krom.setTexture3DParameters(texunit, uAddressing, vAddressing, wAddressing, minificationFilter, magnificationFilter, mipmapFilter);
	}

	static setPipeline = (pipeline: PipelineStateRaw) => {
		PipelineState.set(pipeline);
	}

	static setBool = (location: ConstantLocation, value: bool) => {
		Krom.setBool(location, value);
	}

	static setInt = (location: ConstantLocation, value: i32) => {
		Krom.setInt(location, value);
	}

	static setFloat = (location: ConstantLocation, value: f32) => {
		Krom.setFloat(location, value);
	}

	static setFloat2 = (location: ConstantLocation, value1: f32, value2: f32) => {
		Krom.setFloat2(location, value1, value2);
	}

	static setFloat3 = (location: ConstantLocation, value1: f32, value2: f32, value3: f32) => {
		Krom.setFloat3(location, value1, value2, value3);
	}

	static setFloat4 = (location: ConstantLocation, value1: f32, value2: f32, value3: f32, value4: f32) => {
		Krom.setFloat4(location, value1, value2, value3, value4);
	}

	static setFloats = (location: ConstantLocation, values: Float32Array) => {
		Krom.setFloats(location, values.buffer);
	}

	static setVector2 = (location: ConstantLocation, value: Vec2) => {
		Krom.setFloat2(location, value.x, value.y);
	}

	static setVector3 = (location: ConstantLocation, value: Vec3) => {
		Krom.setFloat3(location, value.x, value.y, value.z);
	}

	static setVector4 = (location: ConstantLocation, value: TVec4) => {
		Krom.setFloat4(location, value.x, value.y, value.z, value.w);
	}

	static setMatrix = (location: ConstantLocation, matrix: TMat4) => {
		Krom.setMatrix(location, matrix.buffer.buffer);
	}

	static setMatrix3 = (location: ConstantLocation, matrix: Mat3) => {
		Krom.setMatrix3(location, matrix.buffer.buffer);
	}

	static drawIndexedVertices = (start: i32 = 0, count: i32 = -1) => {
		Krom.drawIndexedVertices(start, count);
	}

	static drawIndexedVerticesInstanced = (instanceCount: i32, start: i32 = 0, count: i32 = -1) => {
		Krom.drawIndexedVerticesInstanced(instanceCount, start, count);
	}

	static scissor = (x: i32, y: i32, width: i32, height: i32) => {
		Krom.scissor(x, y, width, height);
	}

	static disableScissor = () => {
		Krom.disableScissor();
	}
}

type SystemOptions = {
	title: string;
	x: i32;
	y: i32;
	width: i32;
	height: i32;
	features: WindowFeatures;
	mode: WindowMode;
	frequency: i32;
	vsync: bool;
}

type VertexElement = {
	name: string;
	data: VertexData;
}

type ConstantLocation = any;
type TextureUnit = any;

enum TextureFilter {
	PointFilter,
	LinearFilter,
	AnisotropicFilter,
}

enum MipMapFilter {
	NoMipFilter,
	PointMipFilter,
	LinearMipFilter,
}

enum TextureAddressing {
	Repeat,
	Mirror,
	Clamp,
}

enum Usage {
	StaticUsage,
	DynamicUsage,
	ReadableUsage,
}

enum TextureFormat {
	RGBA32,
	RGBA64,
	R32,
	RGBA128,
	DEPTH16,
	R8,
	R16,
}

enum DepthStencilFormat {
	NoDepthAndStencil,
	DepthOnly,
	DepthAutoStencilAuto,
	Depth24Stencil8,
	Depth32Stencil8,
	Depth16,
}

enum VertexData {
	F32_1X = 1,
	F32_2X = 2,
	F32_3X = 3,
	F32_4X = 4,
	U8_4X_Normalized = 17,
	I16_2X_Normalized = 24,
	I16_4X_Normalized = 28,
}

enum BlendingFactor {
	BlendOne,
	BlendZero,
	SourceAlpha,
	DestinationAlpha,
	InverseSourceAlpha,
	InverseDestinationAlpha,
	SourceColor,
	DestinationColor,
	InverseSourceColor,
	InverseDestinationColor,
}

enum CompareMode {
	Always,
	Never,
	Equal,
	NotEqual,
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
}

enum CullMode {
	Clockwise,
	CounterClockwise,
	None,
}

enum ShaderType {
	Fragment = 0,
	Vertex = 1,
	Geometry = 3,
}

enum WindowFeatures {
    FeatureNone = 0,
    FeatureResizable = 1,
    FeatureMinimizable = 2,
    FeatureMaximizable = 4,
}

enum WindowMode {
	Windowed,
	Fullscreen,
}

enum ImageScaleQuality {
	Low,
	High,
}
