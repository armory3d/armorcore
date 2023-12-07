package kha;

import kha.Graphics4;
import kha.VertexBuffer.VertexStructure;
import kha.Image.TextureFormat;
import kha.Image.DepthStencilFormat;

class PipelineState {
	public var pipeline_: Dynamic;
	public var inputLayout: Array<VertexStructure> = null;
	public var vertexShader: VertexShader = null;
	public var fragmentShader: FragmentShader = null;
	public var geometryShader: GeometryShader = null;
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
