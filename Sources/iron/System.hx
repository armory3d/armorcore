package iron;

import js.lib.ArrayBuffer;
import js.lib.DataView;
import js.lib.Float32Array;
import js.lib.Uint32Array;
import iron.Input;
using StringTools;

class System {
	static var renderListeners: Array<Graphics2 -> Graphics4 -> Void> = [];
	static var foregroundListeners: Array<Void -> Void> = [];
	static var resumeListeners: Array<Void -> Void> = [];
	static var pauseListeners: Array<Void -> Void> = [];
	static var backgroundListeners: Array<Void -> Void> = [];
	static var shutdownListeners: Array<Void -> Void> = [];
	static var dropFilesListeners: Array<String -> Void> = [];
	static var cutListener: Void->String = null;
	static var copyListener: Void->String = null;
	static var pasteListener: String->Void = null;

	static var startTime: Float;
	static var g2: Graphics2;
	static var g4: Graphics4;
	static var keyboard: Keyboard;
	static var mouse: Mouse;
	static var surface: Surface;
	static var pen: Pen;
	static var maxGamepads: Int = 4;
	static var gamepads: Array<Gamepad>;
	static var windowTitle: String;

	public static function start(options: SystemOptions, callback: Void -> Void): Void {
		Krom.init(options.title, options.width, options.height, options.vsync, options.mode, options.features, options.x, options.y, options.frequency);

		startTime = Krom.getTime();
		g4 = new Graphics4();
		g2 = new Graphics2(g4, null);
		Krom.setCallback(renderCallback);
		Krom.setDropFilesCallback(dropFilesCallback);
		Krom.setCutCopyPasteCallback(cutCallback, copyCallback, pasteCallback);
		Krom.setApplicationStateCallback(foregroundCallback, resumeCallback, pauseCallback, backgroundCallback, shutdownCallback);
		Krom.setKeyboardDownCallback(keyboardDownCallback);
		Krom.setKeyboardUpCallback(keyboardUpCallback);
		Krom.setKeyboardPressCallback(keyboardPressCallback);
		Krom.setMouseDownCallback(mouseDownCallback);
		Krom.setMouseUpCallback(mouseUpCallback);
		Krom.setMouseMoveCallback(mouseMoveCallback);
		Krom.setMouseWheelCallback(mouseWheelCallback);
		Krom.setTouchDownCallback(touchDownCallback);
		Krom.setTouchUpCallback(touchUpCallback);
		Krom.setTouchMoveCallback(touchMoveCallback);
		Krom.setPenDownCallback(penDownCallback);
		Krom.setPenUpCallback(penUpCallback);
		Krom.setPenMoveCallback(penMoveCallback);
		Krom.setGamepadAxisCallback(gamepadAxisCallback);
		Krom.setGamepadButtonCallback(gamepadButtonCallback);

		keyboard = Input.getKeyboard();
		mouse = Input.getMouse();
		surface = Input.getSurface();
		pen = Input.getPen();
		gamepads = new Array<Gamepad>();
		for (i in 0...maxGamepads) {
			gamepads[i] = Input.getGamepad(i);
		}

		callback();
	}

	public static function notifyOnFrames(listener: Graphics2 -> Graphics4 -> Void): Void {
		renderListeners.push(listener);
	}

	public static function notifyOnApplicationState(foregroundListener: Void -> Void, resumeListener: Void -> Void,	pauseListener: Void -> Void, backgroundListener: Void-> Void, shutdownListener: Void -> Void): Void {
		if (foregroundListener != null) foregroundListeners.push(foregroundListener);
		if (resumeListener != null) resumeListeners.push(resumeListener);
		if (pauseListener != null) pauseListeners.push(pauseListener);
		if (backgroundListener != null) backgroundListeners.push(backgroundListener);
		if (shutdownListener != null) shutdownListeners.push(shutdownListener);
	}

	public static function notifyOnDropFiles(dropFilesListener: String -> Void): Void {
		dropFilesListeners.push(dropFilesListener);
	}

	public static function notifyOnCutCopyPaste(cutListener: Void->String, copyListener: Void->String, pasteListener: String->Void): Void {
		System.cutListener = cutListener;
		System.copyListener = copyListener;
		System.pasteListener = pasteListener;
	}

	static function foreground(): Void {
		for (listener in foregroundListeners) {
			listener();
		}
	}

	static function resume(): Void {
		for (listener in resumeListeners) {
			listener();
		}
	}

	static function pause(): Void {
		for (listener in pauseListeners) {
			listener();
		}
	}

	static function background(): Void {
		for (listener in backgroundListeners) {
			listener();
		}
	}

	static function shutdown(): Void {
		for (listener in shutdownListeners) {
			listener();
		}
	}

	static function dropFiles(filePath: String): Void {
		for (listener in dropFilesListeners) {
			listener(filePath);
		}
	}

	public static var time(get, null): Float;
	static function get_time(): Float {
		return Krom.getTime() - startTime;
	}

	public static var systemId(get, null): String;
	static function get_systemId(): String {
		return Krom.systemId();
	}

	public static var language(get, never): String;
	static function get_language(): String {
		return Krom.language();
	}

	public static function stop(): Void {
		Krom.requestShutdown();
	}

	public static function loadUrl(url: String): Void {
		Krom.loadUrl(url);
	}

	static function renderCallback(): Void {
		for (listener in renderListeners) {
			listener(g2, g4);
		}
	}

	static function dropFilesCallback(filePath: String): Void {
		System.dropFiles(filePath);
	}

	static function copyCallback(): String {
		if (System.copyListener != null) {
			return System.copyListener();
		}
		return null;
	}

	static function cutCallback(): String {
		if (System.cutListener != null) {
			return System.cutListener();
		}
		return null;
	}

	static function pasteCallback(data: String): Void {
		if (System.pasteListener != null) {
			System.pasteListener(data);
		}
	}

	static function foregroundCallback(): Void {
		System.foreground();
	}

	static function resumeCallback(): Void {
		System.resume();
	}

	static function pauseCallback(): Void {
		System.pause();
	}

	static function backgroundCallback(): Void {
		System.background();
	}

	static function shutdownCallback(): Void {
		System.shutdown();
	}

	static function keyboardDownCallback(code: Int): Void {
		keyboard.downListener(code);
	}

	static function keyboardUpCallback(code: Int): Void {
		keyboard.upListener(code);
	}

	static function keyboardPressCallback(charCode: Int): Void {
		keyboard.pressListener(String.fromCharCode(charCode));
	}

	static function mouseDownCallback(button: Int, x: Int, y: Int): Void {
		mouse.downListener(button, x, y);
	}

	static function mouseUpCallback(button: Int, x: Int, y: Int): Void {
		mouse.upListener(button, x, y);
	}

	static function mouseMoveCallback(x: Int, y: Int, mx: Int, my: Int): Void {
		mouse.moveListener(x, y, mx, my);
	}

	static function mouseWheelCallback(delta: Int): Void {
		mouse.wheelListener(delta);
	}

	static function touchDownCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchDown(index, x, y);
		#end
	}

	static function touchUpCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchUp(index, x, y);
		#end
	}

	static function touchMoveCallback(index: Int, x: Int, y: Int): Void {
		#if (krom_android || krom_ios)
		surface.onTouchMove(index, x, y);
		#end
	}

	static function penDownCallback(x: Int, y: Int, pressure: Float): Void {
		pen.downListener(x, y, pressure);
	}

	static function penUpCallback(x: Int, y: Int, pressure: Float): Void {
		pen.upListener(x, y, pressure);
	}

	static function penMoveCallback(x: Int, y: Int, pressure: Float): Void {
		pen.moveListener(x, y, pressure);
	}

	static function gamepadAxisCallback(gamepad: Int, axis: Int, value: Float): Void {
		gamepads[gamepad].axisListener(axis, value);
	}

	static function gamepadButtonCallback(gamepad: Int, button: Int, value: Float): Void {
		gamepads[gamepad].buttonListener(button, value);
	}

	public static function lockMouse(): Void {
		if (!isMouseLocked()){
			Krom.lockMouse();
		}
	}

	public static function unlockMouse(): Void {
		if (isMouseLocked()){
			Krom.unlockMouse();
		}
	}

	public static function canLockMouse(): Bool {
		return Krom.canLockMouse();
	}

	public static function isMouseLocked(): Bool {
		return Krom.isMouseLocked();
	}

	public static function hideSystemCursor(): Void {
		Krom.showMouse(false);
	}

	public static function showSystemCursor(): Void {
		Krom.showMouse(true);
	}

	public static function resize(width: Int, height: Int): Void {
		Krom.resizeWindow(width, height);
	}

	public static function move(x: Int, y: Int): Void {
		Krom.moveWindow(x, y);
	}

	public static var x(get, never): Int;
	static function get_x(): Int {
		return Krom.windowX();
	}

	public static var y(get, never): Int;
	static function get_y(): Int {
		return Krom.windowY();
	}

	public static var width(get, never): Int;
	static function get_width(): Int {
		return Krom.windowWidth();
	}

	public static var height(get, never): Int;
	static function get_height(): Int {
		return Krom.windowHeight();
	}

	public static var mode(get, set): WindowMode;

	static function get_mode(): WindowMode {
		return cast Krom.getWindowMode();
	}

	static function set_mode(mode: WindowMode): WindowMode {
		Krom.setWindowMode(mode);
		return mode;
	}

	public static var title(get, set): String;

	static function get_title(): String {
		return windowTitle;
	}

	static function set_title(value: String): String {
		Krom.setWindowTitle(value);
		windowTitle = value;
		return windowTitle;
	}

	static function displayPrimaryId(): Int {
		for (i in 0...Krom.displayCount()) {
			if (Krom.displayIsPrimary(i)) return i;
		}
		return 0;
	}

	public static function displayWidth(): Int {
		return Krom.displayWidth(displayPrimaryId());
	}

	public static function displayHeight(): Int {
		return Krom.displayHeight(displayPrimaryId());
	}

	public static function displayFrequency(): Int {
		return Krom.displayFrequency(displayPrimaryId());
	}

	public static function bufferToString(b: js.lib.ArrayBuffer): String {
		var str = "";
		var u8 = new js.lib.Uint8Array(b);
		for (i in 0...u8.length) {
			str += String.fromCharCode(u8[i]);
		}
		return str;
	}

	public static function stringToBuffer(str: String): js.lib.ArrayBuffer {
		var u8 = new js.lib.Uint8Array(str.length);
		for (i in 0...str.length) {
			u8[i] = str.charCodeAt(i);
		}
		return u8.buffer;
	}

	static var shaders: Map<String, Shader> = [];
	static var ext = #if krom_vulkan ".spirv" #elseif (krom_android || krom_wasm) ".essl" #elseif krom_opengl ".glsl" #elseif krom_metal ".metal" #else ".d3d11" #end ;

	public static function getShaderBuffer(name: String): js.lib.ArrayBuffer {
		#if arm_shader_embed
		var global = js.Syntax.code("globalThis");
		return untyped global["data/" + name + ext];
		#else
		return Krom.loadBlob("data/" + name + ext);
		#end
	}

	public static function getShader(name: String): Shader {
		var shader = shaders.get(name);
		if (shader == null) {
			shader = new Shader(getShaderBuffer(name), name.endsWith(".frag") ? Fragment : name.endsWith(".vert") ? Vertex : Geometry);
			shaders.set(name, shader);
		}
		return shader;
	}
}

class Video {
	public function new() : Void {}
	public function unload(): Void {}
}

class Sound {
	public var sound_: Dynamic;

	public function new(sound_: Dynamic) {
		this.sound_ = sound_;
	}

	public function unload() {
		Krom.unloadSound(sound_);
	}
}

class Shader {
	public var shader_: Dynamic;

	public function new(buffer: js.lib.ArrayBuffer, type: ShaderType) {
		if (buffer != null) {
			shader_ = Krom.createShader(buffer, type);
		}
	}

	public static function fromSource(source: String, type: ShaderType): Shader {
		var shader = new Shader(null, 0);
		if (type == Vertex) {
			shader.shader_ = Krom.createVertexShaderFromSource(source);
		}
		else if (type == Fragment) {
			shader.shader_ = Krom.createFragmentShaderFromSource(source);
		}
		return shader;
	}

	public function delete() {
		Krom.deleteShader(shader_);
	}
}

class PipelineState {
	public var pipeline_: Dynamic;
	public var inputLayout: Array<VertexStructure> = null;
	public var vertexShader: Shader = null;
	public var fragmentShader: Shader = null;
	public var geometryShader: Shader = null;
	public var cullMode: CullMode;
	public var depthWrite: Bool;
	public var depthMode: CompareMode;
	public var blendSource: BlendingFactor;
	public var blendDestination: BlendingFactor;
	public var alphaBlendSource: BlendingFactor;
	public var alphaBlendDestination: BlendingFactor;
	public var colorWriteMasksRed: Array<Bool>;
	public var colorWriteMasksGreen: Array<Bool>;
	public var colorWriteMasksBlue: Array<Bool>;
	public var colorWriteMasksAlpha: Array<Bool>;
	public var colorAttachmentCount: Int;
	public var colorAttachments: Array<TextureFormat>;
	public var depthStencilAttachment: DepthStencilFormat;

	private static function getDepthBufferBits(depthAndStencil: DepthStencilFormat): Int {
		return switch (depthAndStencil) {
			case NoDepthAndStencil: 0;
			case DepthOnly: 24;
			case DepthAutoStencilAuto: 24;
			case Depth24Stencil8: 24;
			case Depth32Stencil8: 32;
			case Depth16: 16;
		}
	}

	private static function getStencilBufferBits(depthAndStencil: DepthStencilFormat): Int {
		return switch (depthAndStencil) {
			case NoDepthAndStencil: 0;
			case DepthOnly: 0;
			case DepthAutoStencilAuto: 8;
			case Depth24Stencil8: 8;
			case Depth32Stencil8: 8;
			case Depth16: 0;
		}
	}

	public function new() {
		cullMode = CullMode.None;
		depthWrite = false;
		depthMode = CompareMode.Always;

		blendSource = BlendingFactor.BlendOne;
		blendDestination = BlendingFactor.BlendZero;
		alphaBlendSource = BlendingFactor.BlendOne;
		alphaBlendDestination = BlendingFactor.BlendZero;

		colorWriteMasksRed = [];
		colorWriteMasksGreen = [];
		colorWriteMasksBlue = [];
		colorWriteMasksAlpha = [];
		for (i in 0...8) colorWriteMasksRed.push(true);
		for (i in 0...8) colorWriteMasksGreen.push(true);
		for (i in 0...8) colorWriteMasksBlue.push(true);
		for (i in 0...8) colorWriteMasksAlpha.push(true);

		colorAttachmentCount = 1;
		colorAttachments = [];
		for (i in 0...8) colorAttachments.push(TextureFormat.RGBA32);
		depthStencilAttachment = DepthStencilFormat.NoDepthAndStencil;

		pipeline_ = Krom.createPipeline();
	}

	public function delete() {
		Krom.deletePipeline(pipeline_);
	}

	public function compile(): Void {
		var structure0 = inputLayout.length > 0 ? inputLayout[0] : null;
		var structure1 = inputLayout.length > 1 ? inputLayout[1] : null;
		var structure2 = inputLayout.length > 2 ? inputLayout[2] : null;
		var structure3 = inputLayout.length > 3 ? inputLayout[3] : null;
		var gs = geometryShader != null ? geometryShader.shader_ : null;
		var colorAttachments: Array<Int> = [];
		for (i in 0...8) {
			colorAttachments.push(this.colorAttachments[i]);
		}
		Krom.compilePipeline(pipeline_, structure0, structure1, structure2, structure3, inputLayout.length, vertexShader.shader_, fragmentShader.shader_, gs, {
			cullMode: cullMode,
			depthWrite: this.depthWrite,
			depthMode: depthMode,
			blendSource: blendSource,
			blendDestination: blendDestination,
			alphaBlendSource: alphaBlendSource,
			alphaBlendDestination: alphaBlendDestination,
			colorWriteMaskRed: colorWriteMasksRed,
			colorWriteMaskGreen: colorWriteMasksGreen,
			colorWriteMaskBlue: colorWriteMasksBlue,
			colorWriteMaskAlpha: colorWriteMasksAlpha,
			colorAttachmentCount: colorAttachmentCount,
			colorAttachments: colorAttachments,
			depthAttachmentBits: getDepthBufferBits(depthStencilAttachment),
			stencilAttachmentBits: getStencilBufferBits(depthStencilAttachment)
		});
	}

	public function set(): Void {
		Krom.setPipeline(pipeline_);
	}

	public function getConstantLocation(name: String): ConstantLocation {
		return Krom.getConstantLocation(pipeline_, name);
	}

	public function getTextureUnit(name: String): TextureUnit {
		return Krom.getTextureUnit(pipeline_, name);
	}
}

class VertexBuffer {
	public var buffer_: Dynamic;
	var vertexCount: Int;

	public function new(vertexCount: Int, structure: VertexStructure, usage: Usage, instanceDataStepRate: Int = 0) {
		this.vertexCount = vertexCount;
		buffer_ = Krom.createVertexBuffer(vertexCount, structure.elements, usage, instanceDataStepRate);
	}

	public function delete() {
		Krom.deleteVertexBuffer(buffer_);
	}

	public function lock(): DataView {
		return new DataView(Krom.lockVertexBuffer(buffer_, 0, vertexCount));
	}

	public function unlock(): Void {
		Krom.unlockVertexBuffer(buffer_, vertexCount);
	}

	public function set(): Void {
		Krom.setVertexBuffer(buffer_);
	}
}

class VertexStructure {
	public var elements: Array<VertexElement> = [];
	public var instanced: Bool = false;

	public function new() {}

	public function add(name: String, data: VertexData) {
		elements.push({name: name, data: data});
	}

	public function byteSize(): Int {
		var byteSize = 0;
		for (i in 0...elements.length) {
			byteSize += dataByteSize(elements[i].data);
		}
		return byteSize;
	}

	public static function dataByteSize(data: VertexData): Int {
		switch (data) {
			case F32_1X:
				return 1 * 4;
			case F32_2X:
				return 2 * 4;
			case F32_3X:
				return 3 * 4;
			case F32_4X:
				return 4 * 4;
			case U8_4X_Normalized:
				return 4 * 1;
			case I16_2X_Normalized:
				return 2 * 2;
			case I16_4X_Normalized:
				return 4 * 2;
		}
	}
}

class IndexBuffer {
	public var buffer_: Dynamic;

	public function new(indexCount: Int, usage: Usage) {
		buffer_ = Krom.createIndexBuffer(indexCount);
	}

	public function delete() {
		Krom.deleteIndexBuffer(buffer_);
	}

	public function lock(): Uint32Array {
		return Krom.lockIndexBuffer(buffer_);
	}

	public function unlock(): Void {
		Krom.unlockIndexBuffer(buffer_);
	}

	public function set(): Void {
		Krom.setIndexBuffer(buffer_);
	}
}

abstract Color(Int) from Int from UInt to Int to UInt {
	static inline var invMaxChannelValue: Float = 1 / 255;

	public static inline function fromValue(value: Int): Color {
		return new Color(value);
	}

	public static function fromBytes(r: Int, g: Int, b: Int, a: Int = 255): Color {
		return new Color((a << 24) | (r << 16) | (g << 8) | b);
	}

	public static function fromFloats(r: Float, g: Float, b: Float, a: Float = 1): Color {
		return new Color((Math.round(a * 255) << 24) | (Math.round(r * 255) << 16) | (Math.round(g * 255) << 8) | Math.round(b * 255));
	}

	public var Rb(get, set): Int;
	public var Gb(get, set): Int;
	public var Bb(get, set): Int;
	public var Ab(get, set): Int;
	public var R(get, set): Float;
	public var G(get, set): Float;
	public var B(get, set): Float;
	public var A(get, set): Float;

	private function new(value: Int) {
		this = value;
	}

	public var value(get, set): Int;

	private inline function get_value(): Int {
		return this;
	}

	private inline function set_value(value: Int): Int {
		this = value;
		return this;
	}

	private inline function get_Rb(): Int {
		return (this & 0x00ff0000) >>> 16;
	}

	private inline function get_Gb(): Int {
		return (this & 0x0000ff00) >>> 8;
	}

	private inline function get_Bb(): Int {
		return this & 0x000000ff;
	}

	private inline function get_Ab(): Int {
		return this >>> 24;
	}

	private inline function set_Rb(i: Int): Int {
		this = (Ab << 24) | (i << 16) | (Gb << 8) | Bb;
		return i;
	}

	private inline function set_Gb(i: Int): Int {
		this = (Ab << 24) | (Rb << 16) | (i << 8) | Bb;
		return i;
	}

	private inline function set_Bb(i: Int): Int {
		this = (Ab << 24) | (Rb << 16) | (Gb << 8) | i;
		return i;
	}

	private inline function set_Ab(i: Int): Int {
		this = (i << 24) | (Rb << 16) | (Gb << 8) | Bb;
		return i;
	}

	private inline function get_R(): Float {
		return get_Rb() * invMaxChannelValue;
	}

	private inline function get_G(): Float {
		return get_Gb() * invMaxChannelValue;
	}

	private inline function get_B(): Float {
		return get_Bb() * invMaxChannelValue;
	}

	private inline function get_A(): Float {
		return get_Ab() * invMaxChannelValue;
	}

	private inline function set_R(f: Float): Float {
		this = (Math.round(A * 255) << 24) | (Math.round(f * 255) << 16) | (Math.round(G * 255) << 8) | Math.round(B * 255);
		return f;
	}

	private inline function set_G(f: Float): Float {
		this = (Math.round(A * 255) << 24) | (Math.round(R * 255) << 16) | (Math.round(f * 255) << 8) | Math.round(B * 255);
		return f;
	}

	private inline function set_B(f: Float): Float {
		this = (Math.round(A * 255) << 24) | (Math.round(R * 255) << 16) | (Math.round(G * 255) << 8) | Math.round(f * 255);
		return f;
	}

	private inline function set_A(f: Float): Float {
		this = (Math.round(f * 255) << 24) | (Math.round(R * 255) << 16) | (Math.round(G * 255) << 8) | Math.round(B * 255);
		return f;
	}
}

class Font {
	public var font_: Dynamic = null;
	public var blob: js.lib.ArrayBuffer;
	public var fontGlyphs: Array<Int> = null;
	public var fontIndex = 0;

	public function new(blob: js.lib.ArrayBuffer, fontIndex = 0) {
		this.blob = blob;
		this.fontIndex = fontIndex;
	}

	public function height(fontSize: Int): Float {
		init();
		return Krom.g2_font_height(font_, fontSize);
	}

	public function width(fontSize: Int, str: String): Float {
		init();
		return Krom.g2_string_width(font_, fontSize, str);
	}

	public function unload() {
		blob = null;
	}

	public function setFontIndex(fontIndex: Int) {
		this.fontIndex = fontIndex;
		Graphics2.fontGlyphs = Graphics2.fontGlyphs.copy(); // Trigger atlas update
	}

	public function clone(): Font {
		return new Font(blob, fontIndex);
	}

	public function init() {
		if (Graphics2.fontGlyphsLast != Graphics2.fontGlyphs) {
			Graphics2.fontGlyphsLast = Graphics2.fontGlyphs;
			Krom.g2_font_set_glyphs(Graphics2.fontGlyphs);
		}
		if (fontGlyphs != Graphics2.fontGlyphs) {
			fontGlyphs = Graphics2.fontGlyphs;
			font_ = Krom.g2_font_init(blob, fontIndex);
		}
	}
}

class Image {
	public var texture_: Dynamic;
	public var renderTarget_: Dynamic;
	public var format: TextureFormat;
	private var readable: Bool;
	private var graphics2: Graphics2;
	private var graphics4: Graphics4;

	public function new(texture: Dynamic) {
		texture_ = texture;
	}

	private static function getDepthBufferBits(depthAndStencil: DepthStencilFormat): Int {
		return switch (depthAndStencil) {
			case NoDepthAndStencil: -1;
			case DepthOnly: 24;
			case DepthAutoStencilAuto: 24;
			case Depth24Stencil8: 24;
			case Depth32Stencil8: 32;
			case Depth16: 16;
		}
	}

	private static function getStencilBufferBits(depthAndStencil: DepthStencilFormat): Int {
		return switch (depthAndStencil) {
			case NoDepthAndStencil: -1;
			case DepthOnly: -1;
			case DepthAutoStencilAuto: 8;
			case Depth24Stencil8: 8;
			case Depth32Stencil8: 8;
			case Depth16: 0;
		}
	}

	private static function getTextureFormat(format: TextureFormat): Int {
		switch (format) {
		case RGBA32:
			return 0;
		case RGBA128:
			return 3;
		case RGBA64:
			return 4;
		case R32:
			return 5;
		case R16:
			return 7;
		default:
			return 1; // R8
		}
	}

	public static function _fromTexture(texture: Dynamic): Image {
		return new Image(texture);
	}

	public static function fromBytes(buffer: ArrayBuffer, width: Int, height: Int, format: TextureFormat = null, usage: Usage = null): Image {
		if (format == null) format = TextureFormat.RGBA32;
		var readable = true;
		var image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes(buffer, width, height, getTextureFormat(format), readable);
		return image;
	}

	public static function fromBytes3D(buffer: ArrayBuffer, width: Int, height: Int, depth: Int, format: TextureFormat = null, usage: Usage = null): Image {
		if (format == null) format = TextureFormat.RGBA32;
		var readable = true;
		var image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes3D(buffer, width, height, depth, getTextureFormat(format), readable);
		return image;
	}

	public static function fromEncodedBytes(buffer: ArrayBuffer, format: String, doneCallback: Image->Void, errorCallback: String->Void, readable: Bool = false): Void {
		var image = new Image(null);
		image.texture_ = Krom.createTextureFromEncodedBytes(buffer, format, readable);
		doneCallback(image);
	}

	public static function create(width: Int, height: Int, format: TextureFormat = null, usage: Usage = null): Image {
		if (format == null) format = TextureFormat.RGBA32;
		var image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTexture(width, height, getTextureFormat(format));
		return image;
	}

	public static function create3D(width: Int, height: Int, depth: Int, format: TextureFormat = null, usage: Usage = null): Image {
		if (format == null) format = TextureFormat.RGBA32;
		var image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTexture3D(width, height, depth, getTextureFormat(format));
		return image;
	}

	public static function createRenderTarget(width: Int, height: Int, format: TextureFormat = null, depthStencil: DepthStencilFormat = DepthStencilFormat.NoDepthAndStencil, antiAliasingSamples: Int = 1): Image {
		if (format == null) format = TextureFormat.RGBA32;
		var image = new Image(null);
		image.format = format;
		image.renderTarget_ = Krom.createRenderTarget(width, height, format, getDepthBufferBits(depthStencil), getStencilBufferBits(depthStencil));
		return image;
	}

	public static function renderTargetsInvertedY(): Bool {
		return Krom.renderTargetsInvertedY();
	}

	public function unload(): Void {
		Krom.unloadImage(this);
		texture_ = null;
		renderTarget_ = null;
	}

	public function lock(level: Int = 0): ArrayBuffer {
		return Krom.lockTexture(texture_, level);
	}

	public function unlock(): Void {
		Krom.unlockTexture(texture_);
	}

	public var pixels: ArrayBuffer = null;
	public function getPixels(): ArrayBuffer {
		if (renderTarget_ != null) {
			// Minimum size of 32x32 required after https://github.com/Kode/Kinc/commit/3797ebce5f6d7d360db3331eba28a17d1be87833
			var pixelsWidth = width < 32 ? 32 : width;
			var pixelsHeight = height < 32 ? 32 : height;
			if (pixels == null) pixels = new ArrayBuffer(formatByteSize(format) * pixelsWidth * pixelsHeight);
			Krom.getRenderTargetPixels(renderTarget_, pixels);
			return pixels;
		}
		else {
			return Krom.getTexturePixels(texture_);
		}
	}

	private static function formatByteSize(format: TextureFormat): Int {
		return switch(format) {
			case RGBA32: 4;
			case R8: 1;
			case RGBA128: 16;
			case DEPTH16: 2;
			case RGBA64: 8;
			case R32: 4;
			case R16: 2;
			default: 4;
		}
	}

	public function generateMipmaps(levels: Int): Void {
		texture_ == null ? Krom.generateRenderTargetMipmaps(renderTarget_, levels) : Krom.generateTextureMipmaps(texture_, levels);
	}

	public function setMipmaps(mipmaps: Array<Image>): Void {
		Krom.setMipmaps(texture_, mipmaps);
	}

	public function setDepthStencilFrom(image: Image): Void {
		Krom.setDepthStencilFrom(renderTarget_, image.renderTarget_);
	}

	public function clear(x: Int, y: Int, z: Int, width: Int, height: Int, depth: Int, color: Color): Void {
		Krom.clearTexture(texture_, x, y, z, width, height, depth, color);
	}

	public var width(get, null): Int;
	private function get_width(): Int { return texture_ == null ? renderTarget_.width : texture_.width; }
	public var height(get, null): Int;
	private function get_height(): Int { return texture_ == null ? renderTarget_.height : texture_.height; }
	public var depth(get, null): Int;
	private function get_depth(): Int { return texture_ != null ? texture_.depth : 1; }

	public var g2(get, null): Graphics2;
	private function get_g2(): Graphics2 {
		if (graphics2 == null) {
			graphics2 = new Graphics2(g4, this);
		}
		return graphics2;
	}

	public var g4(get, null): Graphics4;
	private function get_g4(): Graphics4 {
		if (graphics4 == null) {
			graphics4 = new Graphics4(this);
		}
		return graphics4;
	}
}

class Graphics2 {
	public static var current: Graphics2;
	public static var fontGlyphs: Array<Int> = [for (i in 32...127) i];
	public static var fontGlyphsLast: Array<Int> = fontGlyphs;
	static var thrown = false;
	static var mat = new js.lib.Float32Array(9);
	static var initialized = false;

	public var color(default, set): Color;
	public var font(default, set): Font;
	public var fontSize(default, set): Int = 0;
	public var pipeline(default, set): PipelineState;
	public var imageScaleQuality(default, set): ImageScaleQuality;
	public var transformation(default, set): Mat3 = null;
	var g4: Graphics4;
	var renderTarget: Image;

	public function new(g4: Graphics4, renderTarget: Image) {
		if (!initialized) {
			Krom.g2_init(System.getShaderBuffer("painter-image.vert"), System.getShaderBuffer("painter-image.frag"), System.getShaderBuffer("painter-colored.vert"), System.getShaderBuffer("painter-colored.frag"), System.getShaderBuffer("painter-text.vert"), System.getShaderBuffer("painter-text.frag"));
			initialized = true;
		}
		this.g4 = g4;
		this.renderTarget = renderTarget;
	}

	function set_color(c: Color): Color {
		Krom.g2_set_color(c);
		return color = c;
	}

	function set_font_and_size(font: Font, fontSize: Int) {
		font.init();
		Krom.g2_set_font(font.font_, fontSize);
	}

	function set_font(f: Font): Font {
		if (fontSize != 0) set_font_and_size(f, fontSize);
		return font = f;
	}

	function set_fontSize(i: Int): Int {
		if (font.font_ != null) set_font_and_size(font, i);
		return fontSize = i;
	}

	function set_pipeline(p: PipelineState): PipelineState {
		Krom.g2_set_pipeline(p == null ? null : p.pipeline_);
		return pipeline = p;
	}

	function set_imageScaleQuality(q: ImageScaleQuality): ImageScaleQuality {
		Krom.g2_set_bilinear_filter(q == High);
		return imageScaleQuality = q;
	}

	function set_transformation(m: Mat3): Mat3 {
		if (m == null) {
			Krom.g2_set_transform(null);
		}
		else {
			mat[0] = m._00; mat[1] = m._01; mat[2] = m._02;
			mat[3] = m._10; mat[4] = m._11; mat[5] = m._12;
			mat[6] = m._20; mat[7] = m._21; mat[8] = m._22;
			Krom.g2_set_transform(mat.buffer);
		}
		return transformation = m;
	}

	public function drawScaledSubImage(img: Image, sx: Float, sy: Float, sw: Float, sh: Float, dx: Float, dy: Float, dw: Float, dh: Float): Void {
		Krom.g2_draw_scaled_sub_image(img, sx, sy, sw, sh, dx, dy, dw, dh);
	}

	public function drawSubImage(img: Image, x: Float, y: Float, sx: Float, sy: Float, sw: Float, sh: Float): Void {
		drawScaledSubImage(img, sx, sy, sw, sh, x, y, sw, sh);
	}

	public function drawScaledImage(img: Image, dx: Float, dy: Float, dw: Float, dh: Float): Void {
		drawScaledSubImage(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
	}

	public function drawImage(img: Image, x: Float, y: Float): Void {
		drawScaledSubImage(img, 0, 0, img.width, img.height, x, y, img.width, img.height);
	}

	public function drawRect(x: Float, y: Float, width: Float, height: Float, strength: Float = 1.0): Void {
		Krom.g2_draw_rect(x, y, width, height, strength);
	}

	public function fillRect(x: Float, y: Float, width: Float, height: Float): Void {
		Krom.g2_fill_rect(x, y, width, height);
	}

	public function drawString(text: String, x: Float, y: Float): Void {
		Krom.g2_draw_string(text, x, y);
	}

	public function drawLine(x0: Float, y0: Float, x1: Float, y1: Float, strength: Float = 1.0): Void {
		Krom.g2_draw_line(x0, y0, x1, y1, strength);
	}

	public function fillTriangle(x0: Float, y0: Float, x1: Float, y1: Float, x2: Float, y2: Float) {
		Krom.g2_fill_triangle(x0, y0, x1, y1, x2, y2);
	}

	public function scissor(x: Int, y: Int, width: Int, height: Int): Void {
		Krom.g2_end(); // flush
		g4.scissor(x, y, width, height);
	}

	public function disableScissor(): Void {
		Krom.g2_end(); // flush
		g4.disableScissor();
	}

	public function begin(clear = true, clearColor: Color = null): Void {
		if (current == null) {
			current = this;
		}
		else {
			if (!thrown) { thrown = true; throw "End before you begin"; }
		}

		Krom.g2_begin();

		if (renderTarget != null) {
			Krom.g2_set_render_target(renderTarget.renderTarget_);
		}
		else {
			Krom.g2_restore_render_target();
		}

		if (clear) this.clear(clearColor);
	}

	public function clear(color = 0x00000000): Void {
		g4.clear(color);
	}

	public function end(): Void {
		Krom.g2_end();

		if (current == this) {
			current = null;
		}
		else {
			if (!thrown) { thrown = true; throw "Begin before you end"; }
		}
	}

	public function fillCircle(cx: Float, cy: Float, radius: Float, segments: Int = 0): Void {
		Krom.g2_fill_circle(cx, cy, radius, segments);
	}

	public function drawCircle(cx: Float, cy: Float, radius: Float, segments: Int = 0, strength: Float = 1.0): Void {
		Krom.g2_draw_circle(cx, cy, radius, segments, strength);
	}

	public function drawCubicBezier(x: Array<Float>, y: Array<Float>, segments: Int = 20, strength: Float = 1.0): Void {
		Krom.g2_draw_cubic_bezier(x, y, segments, strength);
	}
}

class Graphics4 {
	private var renderTarget: Image;

	public function new(renderTarget: Image = null) {
		this.renderTarget = renderTarget;
	}

	public function begin(additionalRenderTargets: Array<Image> = null): Void {
		Krom.begin(renderTarget, additionalRenderTargets);
	}

	public function end(): Void {
		Krom.end();
	}

	public function clear(?color: Color, ?depth: Float, ?stencil: Int): Void {
		var flags: Int = 0;
		if (color != null) flags |= 1;
		if (depth != null) flags |= 2;
		if (stencil != null) flags |= 4;
		Krom.clear(flags, color == null ? 0 : color.value, depth, stencil);
	}

	public function viewport(x: Int, y: Int, width: Int, height: Int): Void {
		Krom.viewport(x, y, width, height);
	}

	public function setVertexBuffer(vertexBuffer: VertexBuffer): Void {
		vertexBuffer.set();
	}

	public function setVertexBuffers(vertexBuffers: Array<VertexBuffer>): Void {
		Krom.setVertexBuffers(vertexBuffers);
	}

	public function setIndexBuffer(indexBuffer: IndexBuffer): Void {
		indexBuffer.set();
	}

	public function setTexture(unit: TextureUnit, texture: Image): Void {
		if (texture == null) return;
		texture.texture_ != null ? Krom.setTexture(unit, texture.texture_) : Krom.setRenderTarget(unit, texture.renderTarget_);
	}

	public function setTextureDepth(unit: TextureUnit, texture: Image): Void {
		if (texture == null) return;
		Krom.setTextureDepth(unit, texture.renderTarget_);
	}

	public function setImageTexture(unit: TextureUnit, texture: Image): Void {
		if (texture == null) return;
		Krom.setImageTexture(unit, texture.texture_);
	}

	public function setTextureParameters(texunit: TextureUnit, uAddressing: TextureAddressing, vAddressing: TextureAddressing, minificationFilter: TextureFilter, magnificationFilter: TextureFilter, mipmapFilter: MipMapFilter): Void {
		Krom.setTextureParameters(texunit, uAddressing, vAddressing, minificationFilter, magnificationFilter, mipmapFilter);
	}

	public function setTexture3DParameters(texunit: TextureUnit, uAddressing: TextureAddressing, vAddressing: TextureAddressing, wAddressing: TextureAddressing, minificationFilter: TextureFilter, magnificationFilter: TextureFilter, mipmapFilter: MipMapFilter): Void {
		Krom.setTexture3DParameters(texunit, uAddressing, vAddressing, wAddressing, minificationFilter, magnificationFilter, mipmapFilter);
	}

	public function setPipeline(pipeline: PipelineState): Void {
		pipeline.set();
	}

	public function setBool(location: ConstantLocation, value: Bool): Void {
		Krom.setBool(location, value);
	}

	public function setInt(location: ConstantLocation, value: Int): Void {
		Krom.setInt(location, value);
	}

	public function setFloat(location: ConstantLocation, value: Float): Void {
		Krom.setFloat(location, value);
	}

	public function setFloat2(location: ConstantLocation, value1: Float, value2: Float): Void {
		Krom.setFloat2(location, value1, value2);
	}

	public function setFloat3(location: ConstantLocation, value1: Float, value2: Float, value3: Float): Void {
		Krom.setFloat3(location, value1, value2, value3);
	}

	public function setFloat4(location: ConstantLocation, value1: Float, value2: Float, value3: Float, value4: Float): Void {
		Krom.setFloat4(location, value1, value2, value3, value4);
	}

	public function setFloats(location: ConstantLocation, values: Float32Array): Void {
		Krom.setFloats(location, values.buffer);
	}

	public function setVector2(location: ConstantLocation, value: Vec2): Void {
		Krom.setFloat2(location, value.x, value.y);
	}

	public function setVector3(location: ConstantLocation, value: Vec3): Void {
		Krom.setFloat3(location, value.x, value.y, value.z);
	}

	public function setVector4(location: ConstantLocation, value: Vec4): Void {
		Krom.setFloat4(location, value.x, value.y, value.z, value.w);
	}

	public inline function setMatrix(location: ConstantLocation, matrix: Mat4): Void {
		Krom.setMatrix(location, matrix.buffer.buffer);
	}

	public inline function setMatrix3(location: ConstantLocation, matrix: Mat3): Void {
		Krom.setMatrix3(location, matrix.buffer.buffer);
	}

	public function drawIndexedVertices(start: Int = 0, count: Int = -1): Void {
		Krom.drawIndexedVertices(start, count);
	}

	public function drawIndexedVerticesInstanced(instanceCount: Int, start: Int = 0, count: Int = -1): Void {
		Krom.drawIndexedVerticesInstanced(instanceCount, start, count);
	}

	public function scissor(x: Int, y: Int, width: Int, height: Int): Void {
		Krom.scissor(x, y, width, height);
	}

	public function disableScissor(): Void {
		Krom.disableScissor();
	}
}

typedef SystemOptions = {
	public var title: String;
	public var x: Int;
	public var y: Int;
	public var width: Int;
	public var height: Int;
	public var features: WindowFeatures;
	public var mode: WindowMode;
	public var frequency: Int;
	public var vsync: Bool;
}

typedef VertexElement = {
	public var name: String;
	public var data: VertexData;
}

typedef ConstantLocation = Dynamic;
typedef TextureUnit = Dynamic;

@:enum abstract TextureFilter(Int) from Int to Int {
	var PointFilter = 0;
	var LinearFilter = 1;
	var AnisotropicFilter = 2;
}

@:enum abstract MipMapFilter(Int) from Int to Int {
	var NoMipFilter = 0;
	var PointMipFilter = 1;
	var LinearMipFilter = 2;
}

@:enum abstract TextureAddressing(Int) from Int to Int {
	var Repeat = 0;
	var Mirror = 1;
	var Clamp = 2;
}

@:enum abstract Usage(Int) from Int to Int {
	var StaticUsage = 0;
	var DynamicUsage = 1;
	var ReadableUsage = 2;
}

@:enum abstract TextureFormat(Int) from Int to Int {
	var RGBA32 = 0;
	var RGBA64 = 1;
	var R32 = 2;
	var RGBA128 = 3;
	var DEPTH16 = 4;
	var R8 = 5;
	var R16 = 6;
}

@:enum abstract DepthStencilFormat(Int) from Int to Int {
	var NoDepthAndStencil = 0;
	var DepthOnly = 1;
	var DepthAutoStencilAuto = 2;
	var Depth24Stencil8 = 3;
	var Depth32Stencil8 = 4;
	var Depth16 = 5;
}

@:enum abstract VertexData(Int) from Int to Int {
	var F32_1X = 1;
	var F32_2X = 2;
	var F32_3X = 3;
	var F32_4X = 4;
	var U8_4X_Normalized = 17;
	var I16_2X_Normalized = 24;
	var I16_4X_Normalized = 28;
}

@:enum abstract BlendingFactor(Int) from Int to Int {
	var BlendOne = 0;
	var BlendZero = 1;
	var SourceAlpha = 2;
	var DestinationAlpha = 3;
	var InverseSourceAlpha = 4;
	var InverseDestinationAlpha = 5;
	var SourceColor = 6;
	var DestinationColor = 7;
	var InverseSourceColor = 8;
	var InverseDestinationColor = 9;
}

@:enum abstract CompareMode(Int) from Int to Int {
	var Always = 0;
	var Never = 1;
	var Equal = 2;
	var NotEqual = 3;
	var Less = 4;
	var LessEqual = 5;
	var Greater = 6;
	var GreaterEqual = 7;
}

@:enum abstract CullMode(Int) from Int to Int {
	var Clockwise = 0;
	var CounterClockwise = 1;
	var None = 2;
}

@:enum abstract ShaderType(Int) from Int to Int {
	var Fragment = 0;
	var Vertex = 1;
	var Geometry = 3;
}

@:enum abstract WindowFeatures(Int) from Int to Int {
    var FeatureNone = 0;
    var FeatureResizable = 1;
    var FeatureMinimizable = 2;
    var FeatureMaximizable = 4;
}

@:enum abstract WindowMode(Int) from Int to Int {
	var Windowed = 0;
	var Fullscreen = 1;
}

@:enum abstract ImageScaleQuality(Int) from Int to Int {
	var Low = 0;
	var High = 1;
}
