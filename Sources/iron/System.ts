
class System {
	static renderListeners: ((g2: Graphics2, g4: Graphics4)=>void)[] = [];
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
	static g2: Graphics2;
	static g4: Graphics4;
	static windowTitle: string;

	static start = (options: SystemOptions, callback: ()=>void) => {
		Krom.init(options.title, options.width, options.height, options.vsync, options.mode, options.features, options.x, options.y, options.frequency);

		System.startTime = Krom.getTime();
		System.g4 = new Graphics4();
		System.g2 = new Graphics2(System.g4, null);
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

	static notifyOnFrames = (listener: (g2: Graphics2, g4: Graphics4)=>void) => {
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

	static shaders: Map<string, Shader> = new Map();

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

	static getShader = (name: string): Shader => {
		let shader = System.shaders.get(name);
		if (shader == null) {
			shader = new Shader(System.getShaderBuffer(name), name.endsWith(".frag") ? ShaderType.Fragment : name.endsWith(".vert") ? ShaderType.Vertex : ShaderType.Geometry);
			System.shaders.set(name, shader);
		}
		return shader;
	}
}

class Video {
	constructor() {}
	unload = () => {}
}

class Sound {
	sound_: any;

	constructor(sound_: any) {
		this.sound_ = sound_;
	}

	unload = () => {
		Krom.unloadSound(this.sound_);
	}
}

class Shader {
	shader_: any;

	constructor(buffer: ArrayBuffer, type: ShaderType) {
		if (buffer != null) {
			this.shader_ = Krom.createShader(buffer, type);
		}
	}

	static fromSource = (source: string, type: ShaderType): Shader => {
		let shader = new Shader(null, 0);
		if (type == ShaderType.Vertex) {
			shader.shader_ = Krom.createVertexShaderFromSource(source);
		}
		else if (type == ShaderType.Fragment) {
			shader.shader_ = Krom.createFragmentShaderFromSource(source);
		}
		return shader;
	}

	delete = () => {
		Krom.deleteShader(this.shader_);
	}
}

class PipelineState {
	pipeline_: any;
	inputLayout: VertexStructure[] = null;
	vertexShader: Shader = null;
	fragmentShader: Shader = null;
	geometryShader: Shader = null;
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

	constructor() {
		this.cullMode = CullMode.None;
		this.depthWrite = false;
		this.depthMode = CompareMode.Always;

		this.blendSource = BlendingFactor.BlendOne;
		this.blendDestination = BlendingFactor.BlendZero;
		this.alphaBlendSource = BlendingFactor.BlendOne;
		this.alphaBlendDestination = BlendingFactor.BlendZero;

		this.colorWriteMasksRed = [];
		this.colorWriteMasksGreen = [];
		this.colorWriteMasksBlue = [];
		this.colorWriteMasksAlpha = [];
		for (let i = 0; i < 8; ++i) this.colorWriteMasksRed.push(true);
		for (let i = 0; i < 8; ++i) this.colorWriteMasksGreen.push(true);
		for (let i = 0; i < 8; ++i) this.colorWriteMasksBlue.push(true);
		for (let i = 0; i < 8; ++i) this.colorWriteMasksAlpha.push(true);

		this.colorAttachmentCount = 1;
		this.colorAttachments = [];
		for (let i = 0; i < 8; ++i) this.colorAttachments.push(TextureFormat.RGBA32);
		this.depthStencilAttachment = DepthStencilFormat.NoDepthAndStencil;

		this.pipeline_ = Krom.createPipeline();
	}

	delete = () => {
		Krom.deletePipeline(this.pipeline_);
	}

	compile = () => {
		let structure0 = this.inputLayout.length > 0 ? this.inputLayout[0] : null;
		let structure1 = this.inputLayout.length > 1 ? this.inputLayout[1] : null;
		let structure2 = this.inputLayout.length > 2 ? this.inputLayout[2] : null;
		let structure3 = this.inputLayout.length > 3 ? this.inputLayout[3] : null;
		let gs = this.geometryShader != null ? this.geometryShader.shader_ : null;
		let colorAttachments: i32[] = [];
		for (let i = 0; i < 8; ++i) {
			colorAttachments.push(this.colorAttachments[i]);
		}
		Krom.compilePipeline(this.pipeline_, structure0, structure1, structure2, structure3, this.inputLayout.length, this.vertexShader.shader_, this.fragmentShader.shader_, gs, {
			cullMode: this.cullMode,
			depthWrite: this.depthWrite,
			depthMode: this.depthMode,
			blendSource: this.blendSource,
			blendDestination: this.blendDestination,
			alphaBlendSource: this.alphaBlendSource,
			alphaBlendDestination: this.alphaBlendDestination,
			colorWriteMaskRed: this.colorWriteMasksRed,
			colorWriteMaskGreen: this.colorWriteMasksGreen,
			colorWriteMaskBlue: this.colorWriteMasksBlue,
			colorWriteMaskAlpha: this.colorWriteMasksAlpha,
			colorAttachmentCount: this.colorAttachmentCount,
			colorAttachments: this.colorAttachments,
			depthAttachmentBits: PipelineState.getDepthBufferBits(this.depthStencilAttachment),
			stencilAttachmentBits: PipelineState.getStencilBufferBits(this.depthStencilAttachment)
		});
	}

	set = () => {
		Krom.setPipeline(this.pipeline_);
	}

	getConstantLocation = (name: string): ConstantLocation => {
		return Krom.getConstantLocation(this.pipeline_, name);
	}

	getTextureUnit = (name: string): TextureUnit => {
		return Krom.getTextureUnit(this.pipeline_, name);
	}
}

class VertexBuffer {
	buffer_: any;
	vertexCount: i32;

	constructor(vertexCount: i32, structure: VertexStructure, usage: Usage, instanceDataStepRate: i32 = 0) {
		this.vertexCount = vertexCount;
		this.buffer_ = Krom.createVertexBuffer(vertexCount, structure.elements, usage, instanceDataStepRate);
	}

	delete = () => {
		Krom.deleteVertexBuffer(this.buffer_);
	}

	lock = (): DataView => {
		return new DataView(Krom.lockVertexBuffer(this.buffer_, 0, this.vertexCount));
	}

	unlock = () => {
		Krom.unlockVertexBuffer(this.buffer_, this.vertexCount);
	}

	set = () => {
		Krom.setVertexBuffer(this.buffer_);
	}
}

class VertexStructure {
	elements: VertexElement[] = [];
	instanced: bool = false;

	constructor() {}

	add = (name: string, data: VertexData) => {
		this.elements.push({name: name, data: data});
	}

	byteSize = (): i32 => {
		let byteSize = 0;
		for (let i = 0; i < this.elements.length; ++i) {
			byteSize += VertexStructure.dataByteSize(this.elements[i].data);
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

class IndexBuffer {
	buffer_: any;

	constructor(indexCount: i32, usage: Usage) {
		this.buffer_ = Krom.createIndexBuffer(indexCount);
	}

	delete = () => {
		Krom.deleteIndexBuffer(this.buffer_);
	}

	lock = (): Uint32Array => {
		return Krom.lockIndexBuffer(this.buffer_);
	}

	unlock = () => {
		Krom.unlockIndexBuffer(this.buffer_);
	}

	set = () => {
		Krom.setIndexBuffer(this.buffer_);
	}
}

type Color = i32;

class Font {
	font_: any = null;
	blob: ArrayBuffer;
	fontGlyphs: i32[] = null;
	fontIndex = 0;

	constructor(blob: ArrayBuffer, fontIndex = 0) {
		this.blob = blob;
		this.fontIndex = fontIndex;
	}

	height = (fontSize: i32): f32 => {
		this.init();
		return Krom.g2_font_height(this.font_, fontSize);
	}

	width = (fontSize: i32, str: string): f32 => {
		this.init();
		return Krom.g2_string_width(this.font_, fontSize, str);
	}

	unload = () => {
		this.blob = null;
	}

	setFontIndex = (fontIndex: i32) => {
		this.fontIndex = fontIndex;
		Graphics2.fontGlyphs = Graphics2.fontGlyphs.slice(); // Trigger atlas update
	}

	clone = (): Font => {
		return new Font(this.blob, this.fontIndex);
	}

	init = () => {
		if (Graphics2.fontGlyphsLast != Graphics2.fontGlyphs) {
			Graphics2.fontGlyphsLast = Graphics2.fontGlyphs;
			Krom.g2_font_set_glyphs(Graphics2.fontGlyphs);
		}
		if (this.fontGlyphs != Graphics2.fontGlyphs) {
			this.fontGlyphs = Graphics2.fontGlyphs;
			this.font_ = Krom.g2_font_init(this.blob, this.fontIndex);
		}
	}
}

class Image {
	texture_: any;
	renderTarget_: any;
	format: TextureFormat;
	readable: bool;
	graphics2: Graphics2;
	graphics4: Graphics4;
	pixels: ArrayBuffer = null;

	constructor(texture: any) {
		this.texture_ = texture;
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

	static _fromTexture = (texture: any): Image => {
		return new Image(texture);
	}

	static fromBytes = (buffer: ArrayBuffer, width: i32, height: i32, format: TextureFormat = null, usage: Usage = null): Image => {
		if (format == null) format = TextureFormat.RGBA32;
		let readable = true;
		let image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes(buffer, width, height, Image.getTextureFormat(format), readable);
		return image;
	}

	static fromBytes3D = (buffer: ArrayBuffer, width: i32, height: i32, depth: i32, format: TextureFormat = null, usage: Usage = null): Image => {
		if (format == null) format = TextureFormat.RGBA32;
		let readable = true;
		let image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes3D(buffer, width, height, depth, Image.getTextureFormat(format), readable);
		return image;
	}

	static fromEncodedBytes = (buffer: ArrayBuffer, format: string, doneCallback: (img: Image)=>void, errorCallback: (s: string)=>void, readable: bool = false) => {
		let image = new Image(null);
		image.texture_ = Krom.createTextureFromEncodedBytes(buffer, format, readable);
		doneCallback(image);
	}

	static create = (width: i32, height: i32, format: TextureFormat = null, usage: Usage = null): Image => {
		if (format == null) format = TextureFormat.RGBA32;
		let image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTexture(width, height, Image.getTextureFormat(format));
		return image;
	}

	static create3D = (width: i32, height: i32, depth: i32, format: TextureFormat = null, usage: Usage = null): Image => {
		if (format == null) format = TextureFormat.RGBA32;
		let image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTexture3D(width, height, depth, Image.getTextureFormat(format));
		return image;
	}

	static createRenderTarget = (width: i32, height: i32, format: TextureFormat = null, depthStencil: DepthStencilFormat = DepthStencilFormat.NoDepthAndStencil, antiAliasingSamples: i32 = 1): Image => {
		if (format == null) format = TextureFormat.RGBA32;
		let image = new Image(null);
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

	unload = () => {
		Krom.unloadImage(this);
		this.texture_ = null;
		this.renderTarget_ = null;
	}

	lock = (level: i32 = 0): ArrayBuffer => {
		return Krom.lockTexture(this.texture_, level);
	}

	unlock = () => {
		Krom.unlockTexture(this.texture_);
	}

	getPixels = (): ArrayBuffer => {
		if (this.renderTarget_ != null) {
			// Minimum size of 32x32 required after https://github.com/Kode/Kinc/commit/3797ebce5f6d7d360db3331eba28a17d1be87833
			let pixelsWidth = this.width < 32 ? 32 : this.width;
			let pixelsHeight = this.height < 32 ? 32 : this.height;
			if (this.pixels == null) this.pixels = new ArrayBuffer(Image.formatByteSize(this.format) * pixelsWidth * pixelsHeight);
			Krom.getRenderTargetPixels(this.renderTarget_, this.pixels);
			return this.pixels;
		}
		else {
			return Krom.getTexturePixels(this.texture_);
		}
	}

	generateMipmaps = (levels: i32) => {
		this.texture_ == null ? Krom.generateRenderTargetMipmaps(this.renderTarget_, levels) : Krom.generateTextureMipmaps(this.texture_, levels);
	}

	setMipmaps = (mipmaps: Image[]) => {
		Krom.setMipmaps(this.texture_, mipmaps);
	}

	setDepthStencilFrom = (image: Image) => {
		Krom.setDepthStencilFrom(this.renderTarget_, image.renderTarget_);
	}

	clear = (x: i32, y: i32, z: i32, width: i32, height: i32, depth: i32, color: Color) => {
		Krom.clearTexture(this.texture_, x, y, z, width, height, depth, color);
	}

	get width(): i32 { return this.texture_ == null ? this.renderTarget_.width : this.texture_.width; }

	get height(): i32 { return this.texture_ == null ? this.renderTarget_.height : this.texture_.height; }

	get depth(): i32 { return this.texture_ != null ? this.texture_.depth : 1; }

	get g2(): Graphics2 {
		if (this.graphics2 == null) {
			this.graphics2 = new Graphics2(this.g4, this);
		}
		return this.graphics2;
	}

	get g4(): Graphics4 {
		if (this.graphics4 == null) {
			this.graphics4 = new Graphics4(this);
		}
		return this.graphics4;
	}
}

class Graphics2 {
	static makeGlyphs = (start: i32, end: i32): i32[] => {
		let ar: i32[] = [];
		for (let i = start; i < end; ++i) ar.push(i);
		return ar;
	}

	static current: Graphics2;
	static fontGlyphs: i32[] = Graphics2.makeGlyphs(32, 127);
	static fontGlyphsLast: i32[] = Graphics2.fontGlyphs;
	static thrown = false;
	static mat = new Float32Array(9);
	static initialized = false;

	_color: Color;
	_font: Font;
	_fontSize: i32 = 0;
	_pipeline: PipelineState;
	_imageScaleQuality: ImageScaleQuality;
	_transformation: Mat3 = null;

	g4: Graphics4;
	renderTarget: Image;

	constructor(g4: Graphics4, renderTarget: Image) {
		if (!Graphics2.initialized) {
			Krom.g2_init(System.getShaderBuffer("painter-image.vert"), System.getShaderBuffer("painter-image.frag"), System.getShaderBuffer("painter-colored.vert"), System.getShaderBuffer("painter-colored.frag"), System.getShaderBuffer("painter-text.vert"), System.getShaderBuffer("painter-text.frag"));
			Graphics2.initialized = true;
		}
		this.g4 = g4;
		this.renderTarget = renderTarget;
	}

	get color(): Color {
		return this._color;
	}

	set color(c: Color) {
		Krom.g2_set_color(c);
		this._color = c;
	}

	set_font_and_size = (font: Font, fontSize: i32) => {
		font.init();
		Krom.g2_set_font(font.font_, fontSize);
	}

	get font(): Font {
		return this._font;
	}

	set font(f: Font) {
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

	get pipeline(): PipelineState {
		return this._pipeline;
	}

	set pipeline(p: PipelineState) {
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

	drawScaledSubImage = (img: Image, sx: f32, sy: f32, sw: f32, sh: f32, dx: f32, dy: f32, dw: f32, dh: f32) => {
		Krom.g2_draw_scaled_sub_image(img, sx, sy, sw, sh, dx, dy, dw, dh);
	}

	drawSubImage = (img: Image, x: f32, y: f32, sx: f32, sy: f32, sw: f32, sh: f32) => {
		this.drawScaledSubImage(img, sx, sy, sw, sh, x, y, sw, sh);
	}

	drawScaledImage = (img: Image, dx: f32, dy: f32, dw: f32, dh: f32) => {
		this.drawScaledSubImage(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
	}

	drawImage = (img: Image, x: f32, y: f32) => {
		this.drawScaledSubImage(img, 0, 0, img.width, img.height, x, y, img.width, img.height);
	}

	drawRect = (x: f32, y: f32, width: f32, height: f32, strength: f32 = 1.0) => {
		Krom.g2_draw_rect(x, y, width, height, strength);
	}

	fillRect = (x: f32, y: f32, width: f32, height: f32) => {
		Krom.g2_fill_rect(x, y, width, height);
	}

	drawString = (text: string, x: f32, y: f32) => {
		Krom.g2_draw_string(text, x, y);
	}

	drawLine = (x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0) => {
		Krom.g2_draw_line(x0, y0, x1, y1, strength);
	}

	fillTriangle = (x0: f32, y0: f32, x1: f32, y1: f32, x2: f32, y2: f32) => {
		Krom.g2_fill_triangle(x0, y0, x1, y1, x2, y2);
	}

	scissor = (x: i32, y: i32, width: i32, height: i32) => {
		Krom.g2_end(); // flush
		this.g4.scissor(x, y, width, height);
	}

	disableScissor = () => {
		Krom.g2_end(); // flush
		this.g4.disableScissor();
	}

	begin = (clear = true, clearColor: Color = null) => {
		if (Graphics2.current == null) {
			Graphics2.current = this;
		}
		else {
			if (!Graphics2.thrown) { Graphics2.thrown = true; throw "End before you begin"; }
		}

		Krom.g2_begin();

		if (this.renderTarget != null) {
			Krom.g2_set_render_target(this.renderTarget.renderTarget_);
		}
		else {
			Krom.g2_restore_render_target();
		}

		if (clear) this.clear(clearColor);
	}

	clear = (color = 0x00000000) => {
		this.g4.clear(color);
	}

	end = () => {
		Krom.g2_end();

		if (Graphics2.current == this) {
			Graphics2.current = null;
		}
		else {
			if (!Graphics2.thrown) { Graphics2.thrown = true; throw "Begin before you end"; }
		}
	}

	fillCircle = (cx: f32, cy: f32, radius: f32, segments: i32 = 0) => {
		Krom.g2_fill_circle(cx, cy, radius, segments);
	}

	drawCircle = (cx: f32, cy: f32, radius: f32, segments: i32 = 0, strength: f32 = 1.0) => {
		Krom.g2_draw_circle(cx, cy, radius, segments, strength);
	}

	drawCubicBezier = (x: f32[], y: f32[], segments: i32 = 20, strength: f32 = 1.0) => {
		Krom.g2_draw_cubic_bezier(x, y, segments, strength);
	}
}

class Graphics4 {
	renderTarget: Image;

	constructor(renderTarget: Image = null) {
		this.renderTarget = renderTarget;
	}

	begin = (additionalRenderTargets: Image[] = null) => {
		Krom.begin(this.renderTarget, additionalRenderTargets);
	}

	end = () => {
		Krom.end();
	}

	clear = (color?: Color, depth?: f32, stencil?: i32) => {
		let flags: i32 = 0;
		if (color != null) flags |= 1;
		if (depth != null) flags |= 2;
		if (stencil != null) flags |= 4;
		Krom.clear(flags, color == null ? 0 : color, depth, stencil);
	}

	viewport = (x: i32, y: i32, width: i32, height: i32) => {
		Krom.viewport(x, y, width, height);
	}

	setVertexBuffer = (vertexBuffer: VertexBuffer) => {
		vertexBuffer.set();
	}

	setVertexBuffers = (vertexBuffers: VertexBuffer[]) => {
		Krom.setVertexBuffers(vertexBuffers);
	}

	setIndexBuffer = (indexBuffer: IndexBuffer) => {
		indexBuffer.set();
	}

	setTexture = (unit: TextureUnit, texture: Image) => {
		if (texture == null) return;
		texture.texture_ != null ? Krom.setTexture(unit, texture.texture_) : Krom.setRenderTarget(unit, texture.renderTarget_);
	}

	setTextureDepth = (unit: TextureUnit, texture: Image) => {
		if (texture == null) return;
		Krom.setTextureDepth(unit, texture.renderTarget_);
	}

	setImageTexture = (unit: TextureUnit, texture: Image) => {
		if (texture == null) return;
		Krom.setImageTexture(unit, texture.texture_);
	}

	setTextureParameters = (texunit: TextureUnit, uAddressing: TextureAddressing, vAddressing: TextureAddressing, minificationFilter: TextureFilter, magnificationFilter: TextureFilter, mipmapFilter: MipMapFilter) => {
		Krom.setTextureParameters(texunit, uAddressing, vAddressing, minificationFilter, magnificationFilter, mipmapFilter);
	}

	setTexture3DParameters = (texunit: TextureUnit, uAddressing: TextureAddressing, vAddressing: TextureAddressing, wAddressing: TextureAddressing, minificationFilter: TextureFilter, magnificationFilter: TextureFilter, mipmapFilter: MipMapFilter) => {
		Krom.setTexture3DParameters(texunit, uAddressing, vAddressing, wAddressing, minificationFilter, magnificationFilter, mipmapFilter);
	}

	setPipeline = (pipeline: PipelineState) => {
		pipeline.set();
	}

	setBool = (location: ConstantLocation, value: bool) => {
		Krom.setBool(location, value);
	}

	setInt = (location: ConstantLocation, value: i32) => {
		Krom.setInt(location, value);
	}

	setFloat = (location: ConstantLocation, value: f32) => {
		Krom.setFloat(location, value);
	}

	setFloat2 = (location: ConstantLocation, value1: f32, value2: f32) => {
		Krom.setFloat2(location, value1, value2);
	}

	setFloat3 = (location: ConstantLocation, value1: f32, value2: f32, value3: f32) => {
		Krom.setFloat3(location, value1, value2, value3);
	}

	setFloat4 = (location: ConstantLocation, value1: f32, value2: f32, value3: f32, value4: f32) => {
		Krom.setFloat4(location, value1, value2, value3, value4);
	}

	setFloats = (location: ConstantLocation, values: Float32Array) => {
		Krom.setFloats(location, values.buffer);
	}

	setVector2 = (location: ConstantLocation, value: Vec2) => {
		Krom.setFloat2(location, value.x, value.y);
	}

	setVector3 = (location: ConstantLocation, value: Vec3) => {
		Krom.setFloat3(location, value.x, value.y, value.z);
	}

	setVector4 = (location: ConstantLocation, value: TVec4) => {
		Krom.setFloat4(location, value.x, value.y, value.z, value.w);
	}

 	setMatrix = (location: ConstantLocation, matrix: TMat4) => {
		Krom.setMatrix(location, matrix.buffer.buffer);
	}

 	setMatrix3 = (location: ConstantLocation, matrix: Mat3) => {
		Krom.setMatrix3(location, matrix.buffer.buffer);
	}

	drawIndexedVertices = (start: i32 = 0, count: i32 = -1) => {
		Krom.drawIndexedVertices(start, count);
	}

	drawIndexedVerticesInstanced = (instanceCount: i32, start: i32 = 0, count: i32 = -1) => {
		Krom.drawIndexedVerticesInstanced(instanceCount, start, count);
	}

	scissor = (x: i32, y: i32, width: i32, height: i32) => {
		Krom.scissor(x, y, width, height);
	}

	disableScissor = () => {
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
