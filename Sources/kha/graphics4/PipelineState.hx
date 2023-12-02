package kha.graphics4;

import kha.graphics4.Graphics;
import kha.graphics4.VertexBuffer.VertexStructure;
import kha.Image.TextureFormat;
import kha.Image.DepthStencilFormat;

class PipelineState {
	public var pipeline: Dynamic;
	public var inputLayout: Array<VertexStructure>;
	public var vertexShader: VertexShader;
	public var fragmentShader: FragmentShader;
	public var geometryShader: GeometryShader;
	public var cullMode: CullMode;
	public var depthWrite: Bool;
	public var depthMode: CompareMode;

	// One, Zero deactivates blending
	public var blendSource: BlendingFactor;
	public var blendDestination: BlendingFactor;
	public var blendOperation: BlendingOperation;
	public var alphaBlendSource: BlendingFactor;
	public var alphaBlendDestination: BlendingFactor;
	public var alphaBlendOperation: BlendingOperation;

	public var colorWriteMask(never, set): Bool;
	public var colorWriteMaskRed(get, set): Bool;
	public var colorWriteMaskGreen(get, set): Bool;
	public var colorWriteMaskBlue(get, set): Bool;
	public var colorWriteMaskAlpha(get, set): Bool;
	public var colorWriteMasksRed: Array<Bool>;
	public var colorWriteMasksGreen: Array<Bool>;
	public var colorWriteMasksBlue: Array<Bool>;
	public var colorWriteMasksAlpha: Array<Bool>;

	public var colorAttachmentCount: Int;
	public var colorAttachments: Array<TextureFormat>;
	public var depthStencilAttachment: DepthStencilFormat;

	inline function set_colorWriteMask(value: Bool): Bool {
		return colorWriteMaskRed = colorWriteMaskBlue = colorWriteMaskGreen = colorWriteMaskAlpha = value;
	}

	inline function get_colorWriteMaskRed(): Bool {
		return colorWriteMasksRed[0];
	}

	inline function set_colorWriteMaskRed(value: Bool): Bool {
		return colorWriteMasksRed[0] = value;
	}

	inline function get_colorWriteMaskGreen(): Bool {
		return colorWriteMasksGreen[0];
	}

	inline function set_colorWriteMaskGreen(value: Bool): Bool {
		return colorWriteMasksGreen[0] = value;
	}

	inline function get_colorWriteMaskBlue(): Bool {
		return colorWriteMasksBlue[0];
	}

	inline function set_colorWriteMaskBlue(value: Bool): Bool {
		return colorWriteMasksBlue[0] = value;
	}

	inline function get_colorWriteMaskAlpha(): Bool {
		return colorWriteMasksAlpha[0];
	}

	inline function set_colorWriteMaskAlpha(value: Bool): Bool {
		return colorWriteMasksAlpha[0] = value;
	}

	public var conservativeRasterization: Bool;

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
		inputLayout = null;
		vertexShader = null;
		fragmentShader = null;
		geometryShader = null;

		cullMode = CullMode.None;
		depthWrite = false;
		depthMode = CompareMode.Always;

		blendSource = BlendingFactor.BlendOne;
		blendDestination = BlendingFactor.BlendZero;
		blendOperation = BlendingOperation.Add;
		alphaBlendSource = BlendingFactor.BlendOne;
		alphaBlendDestination = BlendingFactor.BlendZero;
		alphaBlendOperation = BlendingOperation.Add;

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

		conservativeRasterization = false;

		pipeline = Krom.createPipeline();
	}

	public function delete() {
		Krom.deletePipeline(pipeline);
		pipeline = null;
	}

	public function compile(): Void {
		var structure0 = inputLayout.length > 0 ? inputLayout[0] : null;
		var structure1 = inputLayout.length > 1 ? inputLayout[1] : null;
		var structure2 = inputLayout.length > 2 ? inputLayout[2] : null;
		var structure3 = inputLayout.length > 3 ? inputLayout[3] : null;
		var gs = geometryShader != null ? geometryShader.shader : null;
		var colorAttachments: Array<Int> = [];
		for (i in 0...8) {
			colorAttachments.push(getRenderTargetFormat(this.colorAttachments[i]));
		}
		Krom.compilePipeline(pipeline, structure0, structure1, structure2, structure3, inputLayout.length, vertexShader.shader, fragmentShader.shader, gs, null, null, {
			cullMode: convertCullMode(cullMode),
			depthWrite: this.depthWrite,
			depthMode: convertCompareMode(depthMode),
			blendSource: convertBlendingFactor(blendSource),
			blendDestination: convertBlendingFactor(blendDestination),
			alphaBlendSource: convertBlendingFactor(alphaBlendSource),
			alphaBlendDestination: convertBlendingFactor(alphaBlendDestination),
			colorWriteMaskRed: colorWriteMasksRed,
			colorWriteMaskGreen: colorWriteMasksGreen,
			colorWriteMaskBlue: colorWriteMasksBlue,
			colorWriteMaskAlpha: colorWriteMasksAlpha,
			colorAttachmentCount: colorAttachmentCount,
			colorAttachments: colorAttachments,
			depthAttachmentBits: getDepthBufferBits(depthStencilAttachment),
			stencilAttachmentBits: getStencilBufferBits(depthStencilAttachment),
			conservativeRasterization: conservativeRasterization
		});
	}

	public function set(): Void {
		Krom.setPipeline(pipeline);
	}

	public function getConstantLocation(name: String): ConstantLocation {
		return Krom.getConstantLocation(pipeline, name);
	}

	public function getTextureUnit(name: String): TextureUnit {
		return Krom.getTextureUnit(pipeline, name);
	}

	private static function convertCullMode(mode: CullMode): Int {
		switch (mode) {
			case Clockwise:
				return 0;
			case CounterClockwise:
				return 1;
			case None:
				return 2;
		}
	}

	private static function convertCompareMode(mode: CompareMode): Int {
		switch (mode) {
			case Always:
				return 0;
			case Never:
				return 1;
			case Equal:
				return 2;
			case NotEqual:
				return 3;
			case Less:
				return 4;
			case LessEqual:
				return 5;
			case Greater:
				return 6;
			case GreaterEqual:
				return 7;
		}
	}

	private static function convertBlendingFactor(factor: BlendingFactor): Int {
		switch (factor) {
			case Undefined, BlendOne:
				return 0;
			case BlendZero:
				return 1;
			case SourceAlpha:
				return 2;
			case DestinationAlpha:
				return 3;
			case InverseSourceAlpha:
				return 4;
			case InverseDestinationAlpha:
				return 5;
			case SourceColor:
				return 6;
			case DestinationColor:
				return 7;
			case InverseSourceColor:
				return 8;
			case InverseDestinationColor:
				return 9;
		}
	}
}

@:enum abstract BlendingFactor(Int) to Int {
	var Undefined = 0;
	var BlendOne = 1;
	var BlendZero = 2;
	var SourceAlpha = 3;
	var DestinationAlpha = 4;
	var InverseSourceAlpha = 5;
	var InverseDestinationAlpha = 6;
	var SourceColor = 7;
	var DestinationColor = 8;
	var InverseSourceColor = 9;
	var InverseDestinationColor = 10;
}

@:enum abstract BlendingOperation(Int) to Int {
	var Add = 0;
	var Subtract = 1;
	var ReverseSubtract = 2;
	var Min = 3;
	var Max = 4;
}

@:enum abstract CompareMode(Int) to Int {
	var Always = 0;
	var Never = 1;
	var Equal = 2;
	var NotEqual = 3;
	var Less = 4;
	var LessEqual = 5;
	var Greater = 6;
	var GreaterEqual = 7;
}

@:enum abstract CullMode(Int) to Int {
	var Clockwise = 0;
	var CounterClockwise = 1;
	var None = 2;
}
