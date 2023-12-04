package kha;

import haxe.io.Bytes;
import kha.Graphics4.Usage;

class Image implements Canvas {
	public var texture_: Dynamic;
	public var renderTarget_: Dynamic;

	public var format: TextureFormat;
	private var readable: Bool;

	private var graphics2: kha.Graphics2;
	private var graphics4: kha.Graphics4;

	private function new(texture: Dynamic) {
		texture_ = texture;
	}

	private static function getRenderTargetFormat(format: TextureFormat): Int {
		switch (format) {
		case RGBA32:	// Target32Bit
			return 0;
		case RGBA64:	// Target64BitFloat
			return 1;
		case A32:		// Target32BitRedFloat
			return 2;
		case RGBA128:	// Target128BitFloat
			return 3;
		case DEPTH16:	// Target16BitDepth
			return 4;
		case L8:
			return 5;	// Target8BitRed
		case A16:
			return 6;	// Target16BitRedFloat
		default:
			return 0;
		}
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
		case A32:
			return 5;
		case A16:
			return 7;
		default:
			return 1; // Grey8
		}
	}

	public static function _fromTexture(texture: Dynamic): Image {
		return new Image(texture);
	}

	public static function fromBytes(bytes: Bytes, width: Int, height: Int, format: TextureFormat = null, usage: Usage = null): Image {
		if (format == null) format = TextureFormat.RGBA32;
		var readable = true;
		var image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes(bytes.getData(), width, height, getTextureFormat(format), readable);
		return image;
	}

	public static function fromBytes3D(bytes: Bytes, width: Int, height: Int, depth: Int, format: TextureFormat = null, usage: Usage = null): Image {
		if (format == null) format = TextureFormat.RGBA32;
		var readable = true;
		var image = new Image(null);
		image.format = format;
		image.texture_ = Krom.createTextureFromBytes3D(bytes.getData(), width, height, depth, getTextureFormat(format), readable);
		return image;
	}

	public static function fromEncodedBytes(bytes: Bytes, format: String, doneCallback: Image->Void, errorCallback: String->Void, readable: Bool = false): Void {
		var image = new Image(null);
		image.texture_ = Krom.createTextureFromEncodedBytes(bytes.getData(), format, readable);
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
		image.renderTarget_ = Krom.createRenderTarget(width, height, getRenderTargetFormat(format), getDepthBufferBits(depthStencil), getStencilBufferBits(depthStencil));
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

	public function lock(level: Int = 0): Bytes {
		return Bytes.ofData(Krom.lockTexture(texture_, level));
	}

	public function unlock(): Void {
		Krom.unlockTexture(texture_);
	}

	private var pixels: Bytes = null;
	public function getPixels(): Bytes {
		if (renderTarget_ != null) {
			// Minimum size of 32x32 required after https://github.com/Kode/Kinc/commit/3797ebce5f6d7d360db3331eba28a17d1be87833
			var pixelsWidth = width < 32 ? 32 : width;
			var pixelsHeight = height < 32 ? 32 : height;
			if (pixels == null) pixels = Bytes.alloc(formatByteSize(format) * pixelsWidth * pixelsHeight);
			Krom.getRenderTargetPixels(renderTarget_, pixels.getData());
			return pixels;
		}
		else {
			return Bytes.ofData(Krom.getTexturePixels(texture_));
		}
	}

	private static function formatByteSize(format: TextureFormat): Int {
		return switch(format) {
			case RGBA32: 4;
			case L8: 1;
			case RGBA128: 16;
			case DEPTH16: 2;
			case RGBA64: 8;
			case A32: 4;
			case A16: 2;
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

	public var g2(get, null): kha.Graphics2;
	private function get_g2(): kha.Graphics2 {
		if (graphics2 == null) {
			graphics2 = new kha.Graphics2(this);
		}
		return graphics2;
	}

	public var g4(get, null): kha.Graphics4;
	private function get_g4(): kha.Graphics4 {
		if (graphics4 == null) {
			graphics4 = new kha.Graphics4(this);
		}
		return graphics4;
	}
}

@:enum abstract TextureFormat(Int) to Int {
	var RGBA32 = 0;
	var L8 = 1;
	var RGBA128 = 2; // Floats
	var DEPTH16 = 3;
	var RGBA64 = 4; // Half floats
	var A32 = 5; // Float
	var A16 = 6; // Half float
}

@:enum abstract DepthStencilFormat(Int) to Int {
	var NoDepthAndStencil = 0;
	var DepthOnly = 1;
	var DepthAutoStencilAuto = 2;
	var Depth24Stencil8 = 3;
	var Depth32Stencil8 = 4;
	var Depth16 = 5;
}
