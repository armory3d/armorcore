package kha.graphics2;

import kha.arrays.ByteArray;
import kha.graphics4.PipelineState;
import kha.graphics4.DepthStencilFormat;
import kha.graphics4.BlendingOperation;
import kha.graphics4.BlendingFactor;
import kha.graphics4.ConstantLocation;
import kha.graphics4.CullMode;
import kha.graphics4.IndexBuffer;
import kha.graphics4.MipMapFilter;
import kha.graphics4.TextureAddressing;
import kha.graphics4.TextureFilter;
import kha.graphics4.TextureFormat;
import kha.graphics4.TextureUnit;
import kha.graphics4.Usage;
import kha.graphics4.VertexBuffer;
import kha.graphics4.VertexData;
import kha.graphics4.VertexStructure;
import kha.math.FastMatrix3;
import kha.math.FastMatrix4;
import kha.math.FastVector2;
import kha.Canvas;
import kha.Color;
import kha.FastFloat;
import kha.Font;
import kha.Image;
import kha.Shaders;

class InternalPipeline {
	public var pipeline: PipelineState;
	public var projectionLocation: ConstantLocation;
	public var textureLocation: TextureUnit;

	public function new(pipeline: PipelineState, projectionLocation: ConstantLocation, textureLocation: TextureUnit) {
		this.pipeline = pipeline;
		this.projectionLocation = projectionLocation;
		this.textureLocation = textureLocation;
	}
}

interface PipelineCache {
	function get(colorFormat: Array<TextureFormat>, depthStencilFormat: DepthStencilFormat): InternalPipeline;
}

class SimplePipelineCache implements PipelineCache {
	var pipeline: InternalPipeline;

	public function new(pipeline: PipelineState, texture: Bool) {
		var projectionLocation: ConstantLocation = null;
		try {
			projectionLocation = pipeline.getConstantLocation("projectionMatrix");
		}
		catch (x: Dynamic) {
			trace(x);
		}

		var textureLocation: TextureUnit = null;
		if (texture) {
			try {
				textureLocation = pipeline.getTextureUnit("tex");
			}
			catch (x: Dynamic) {
				trace(x);
			}
		}

		this.pipeline = new InternalPipeline(pipeline, projectionLocation, textureLocation);
	}

	public function get(colorFormats: Array<TextureFormat>, depthStencilFormat: DepthStencilFormat): InternalPipeline {
		return pipeline;
	}
}

class PerFramebufferPipelineCache implements PipelineCache {
	var pipelines: Array<InternalPipeline> = [];

	public function new(pipeline: PipelineState, texture: Bool) {
		pipeline.compile();

		var projectionLocation: ConstantLocation = null;
		try {
			projectionLocation = pipeline.getConstantLocation("projectionMatrix");
		}
		catch (x: Dynamic) {
			trace(x);
		}

		var textureLocation: TextureUnit = null;
		if (texture) {
			try {
				textureLocation = pipeline.getTextureUnit("tex");
			}
			catch (x: Dynamic) {
				trace(x);
			}
		}

		pipelines.push(new InternalPipeline(pipeline, projectionLocation, textureLocation));
	}

	public function get(colorFormats: Array<TextureFormat>, depthStencilFormat: DepthStencilFormat): InternalPipeline {
		return pipelines[hash(colorFormats, depthStencilFormat)];
	}

	function hash(colorFormats: Array<TextureFormat>, depthStencilFormat: DepthStencilFormat) {
		return 0;
	}
}

class ImageShaderPainter {
	var projectionMatrix: FastMatrix4;

	static var standardImagePipeline: PipelineCache = null;
	static var structure: VertexStructure = null;
	static inline var bufferSize: Int = 1500;
	static inline var vertexSize: Int = 6;
	static var bufferStart: Int;
	static var bufferIndex: Int;
	static var rectVertexBuffer: VertexBuffer;
	static var rectVertices: ByteArray;
	static var indexBuffer: IndexBuffer;
	static var lastTexture: Image;

	var bilinear: Bool = false;
	var bilinearMipmaps: Bool = false;
	var g: kha.graphics4.Graphics;
	var myPipeline: PipelineCache = null;

	public var pipeline(get, set): PipelineCache;

	public function new(g4: kha.graphics4.Graphics) {
		this.g = g4;
		bufferStart = 0;
		bufferIndex = 0;
		initShaders();
		myPipeline = standardImagePipeline;
		initBuffers();
	}

	function get_pipeline(): PipelineCache {
		return myPipeline;
	}

	function set_pipeline(pipe: PipelineCache): PipelineCache {
		myPipeline = pipe != null ? pipe : standardImagePipeline;
		return myPipeline;
	}

	public function setProjection(projectionMatrix: FastMatrix4): Void {
		this.projectionMatrix = projectionMatrix;
	}

	static function initShaders(): Void {
		if (structure == null) {
			structure = Graphics.createImageVertexStructure();
		}
		if (standardImagePipeline == null) {
			var pipeline = Graphics.createImagePipeline(structure);
			standardImagePipeline = new PerFramebufferPipelineCache(pipeline, true);
		}
	}

	function initBuffers(): Void {
		if (rectVertexBuffer == null) {
			rectVertexBuffer = new VertexBuffer(bufferSize * 4, structure, Usage.DynamicUsage);
			rectVertices = rectVertexBuffer.lock();

			indexBuffer = new IndexBuffer(bufferSize * 3 * 2, Usage.StaticUsage);
			var indices = indexBuffer.lock();
			for (i in 0...bufferSize) {
				indices[i * 3 * 2 + 0] = i * 4 + 0;
				indices[i * 3 * 2 + 1] = i * 4 + 1;
				indices[i * 3 * 2 + 2] = i * 4 + 2;
				indices[i * 3 * 2 + 3] = i * 4 + 0;
				indices[i * 3 * 2 + 4] = i * 4 + 2;
				indices[i * 3 * 2 + 5] = i * 4 + 3;
			}
			indexBuffer.unlock();
		}
	}

	inline function setRectVertices(bottomleftx: FastFloat, bottomlefty: FastFloat, topleftx: FastFloat, toplefty: FastFloat, toprightx: FastFloat,
			toprighty: FastFloat, bottomrightx: FastFloat, bottomrighty: FastFloat): Void {
		var baseIndex: Int = (bufferIndex - bufferStart) * vertexSize * 4;
		rectVertices.setFloat32((baseIndex + 0) * 4, bottomleftx);
		rectVertices.setFloat32((baseIndex + 1) * 4, bottomlefty);
		rectVertices.setFloat32((baseIndex + 2) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 6) * 4, topleftx);
		rectVertices.setFloat32((baseIndex + 7) * 4, toplefty);
		rectVertices.setFloat32((baseIndex + 8) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 12) * 4, toprightx);
		rectVertices.setFloat32((baseIndex + 13) * 4, toprighty);
		rectVertices.setFloat32((baseIndex + 14) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 18) * 4, bottomrightx);
		rectVertices.setFloat32((baseIndex + 19) * 4, bottomrighty);
		rectVertices.setFloat32((baseIndex + 20) * 4, -5.0);
	}

	inline function setRectTexCoords(left: FastFloat, top: FastFloat, right: FastFloat, bottom: FastFloat): Void {
		var baseIndex: Int = (bufferIndex - bufferStart) * vertexSize * 4;
		rectVertices.setFloat32((baseIndex + 3) * 4, left);
		rectVertices.setFloat32((baseIndex + 4) * 4, bottom);

		rectVertices.setFloat32((baseIndex + 9) * 4, left);
		rectVertices.setFloat32((baseIndex + 10) * 4, top);

		rectVertices.setFloat32((baseIndex + 15) * 4, right);
		rectVertices.setFloat32((baseIndex + 16) * 4, top);

		rectVertices.setFloat32((baseIndex + 21) * 4, right);
		rectVertices.setFloat32((baseIndex + 22) * 4, bottom);
	}

	inline function setRectColor(r: FastFloat, g: FastFloat, b: FastFloat, a: FastFloat): Void {
		var baseIndex: Int = (bufferIndex - bufferStart) * vertexSize * 4 * 4;
		rectVertices.setUint8(baseIndex + 5 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 5 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 5 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 5 * 4 + 3, Std.int(a * 255));

		rectVertices.setUint8(baseIndex + 11 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 11 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 11 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 11 * 4 + 3, Std.int(a * 255));

		rectVertices.setUint8(baseIndex + 17 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 17 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 17 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 17 * 4 + 3, Std.int(a * 255));

		rectVertices.setUint8(baseIndex + 23 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 23 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 23 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 23 * 4 + 3, Std.int(a * 255));
	}

	function drawBuffer(end: Bool): Void {
		if (bufferIndex - bufferStart == 0) {
			return;
		}

		rectVertexBuffer.unlock((bufferIndex - bufferStart) * 4);
		var pipeline = myPipeline.get(null, Depth24Stencil8);
		g.setPipeline(pipeline.pipeline);
		g.setVertexBuffer(rectVertexBuffer);
		g.setIndexBuffer(indexBuffer);
		g.setTexture(pipeline.textureLocation, lastTexture);
		g.setTextureParameters(pipeline.textureLocation, TextureAddressing.Clamp, TextureAddressing.Clamp,
			bilinear ? TextureFilter.LinearFilter : TextureFilter.PointFilter, bilinear ? TextureFilter.LinearFilter : TextureFilter.PointFilter,
			bilinearMipmaps ? MipMapFilter.LinearMipFilter : MipMapFilter.NoMipFilter);
		g.setMatrix(pipeline.projectionLocation, projectionMatrix);

		g.drawIndexedVertices(bufferStart * 2 * 3, (bufferIndex - bufferStart) * 2 * 3);

		g.setTexture(pipeline.textureLocation, null);

		if (end || (bufferStart + bufferIndex + 1) * 4 >= bufferSize) {
			bufferStart = 0;
			bufferIndex = 0;
			rectVertices = rectVertexBuffer.lock(0);
		}
		else {
			bufferStart = bufferIndex;
			rectVertices = rectVertexBuffer.lock(bufferStart * 4);
		}
	}

	public function setBilinearFilter(bilinear: Bool): Void {
		drawBuffer(false);
		lastTexture = null;
		this.bilinear = bilinear;
	}

	public function setBilinearMipmapFilter(bilinear: Bool): Void {
		drawBuffer(false);
		lastTexture = null;
		this.bilinearMipmaps = bilinear;
	}

	public inline function drawImage(img: kha.Image, bottomleftx: FastFloat, bottomlefty: FastFloat, topleftx: FastFloat, toplefty: FastFloat,
			toprightx: FastFloat, toprighty: FastFloat, bottomrightx: FastFloat, bottomrighty: FastFloat, opacity: FastFloat, color: Color): Void {
		var tex = img;
		if (bufferStart + bufferIndex + 1 >= bufferSize || (lastTexture != null && tex != lastTexture))
			drawBuffer(false);

		setRectColor(color.R, color.G, color.B, color.A * opacity);
		setRectTexCoords(0, 0, 1, 1);
		setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

		++bufferIndex;
		lastTexture = tex;
	}

	public inline function drawImage2(img: kha.Image, sx: FastFloat, sy: FastFloat, sw: FastFloat, sh: FastFloat, bottomleftx: FastFloat,
			bottomlefty: FastFloat, topleftx: FastFloat, toplefty: FastFloat, toprightx: FastFloat, toprighty: FastFloat, bottomrightx: FastFloat,
			bottomrighty: FastFloat, opacity: FastFloat, color: Color): Void {
		var tex = img;
		if (bufferStart + bufferIndex + 1 >= bufferSize || (lastTexture != null && tex != lastTexture))
			drawBuffer(false);

		setRectTexCoords(sx / tex.width, sy / tex.height, (sx + sw) / tex.width, (sy + sh) / tex.height);
		setRectColor(color.R, color.G, color.B, color.A * opacity);
		setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

		++bufferIndex;
		lastTexture = tex;
	}

	public inline function drawImageScale(img: kha.Image, sx: FastFloat, sy: FastFloat, sw: FastFloat, sh: FastFloat, left: FastFloat, top: FastFloat,
			right: FastFloat, bottom: FastFloat, opacity: FastFloat, color: Color): Void {
		var tex = img;
		if (bufferStart + bufferIndex + 1 >= bufferSize || (lastTexture != null && tex != lastTexture))
			drawBuffer(false);

		setRectTexCoords(sx / tex.width, sy / tex.height, (sx + sw) / tex.width, (sy + sh) / tex.height);
		setRectColor(color.R, color.G, color.B, opacity);
		setRectVertices(left, bottom, left, top, right, top, right, bottom);

		++bufferIndex;
		lastTexture = tex;
	}

	public function end(): Void {
		if (bufferIndex > 0) {
			drawBuffer(true);
		}
		lastTexture = null;
	}
}

class ColoredShaderPainter {
	var projectionMatrix: FastMatrix4;

	static var standardColorPipeline: PipelineCache = null;
	static var structure: VertexStructure = null;

	static inline var bufferSize: Int = 1000;
	static var bufferIndex: Int;
	static var rectVertexBuffer: VertexBuffer;
	static var rectVertices: ByteArray;
	static var indexBuffer: IndexBuffer;

	static inline var triangleBufferSize: Int = 1000;
	static var triangleBufferIndex: Int;
	static var triangleVertexBuffer: VertexBuffer;
	static var triangleVertices: ByteArray;
	static var triangleIndexBuffer: IndexBuffer;

	var g: kha.graphics4.Graphics;
	var myPipeline: PipelineCache = null;

	public var pipeline(get, set): PipelineCache;

	public function new(g4: kha.graphics4.Graphics) {
		this.g = g4;
		bufferIndex = 0;
		triangleBufferIndex = 0;
		initShaders();
		myPipeline = standardColorPipeline;
		initBuffers();
	}

	function get_pipeline(): PipelineCache {
		return myPipeline;
	}

	function set_pipeline(pipe: PipelineCache): PipelineCache {
		myPipeline = pipe != null ? pipe : standardColorPipeline;
		return myPipeline;
	}

	public function setProjection(projectionMatrix: FastMatrix4): Void {
		this.projectionMatrix = projectionMatrix;
	}

	static function initShaders(): Void {
		if (structure == null) {
			structure = Graphics.createColoredVertexStructure();
		}
		if (standardColorPipeline == null) {
			var pipeline = Graphics.createColoredPipeline(structure);
			standardColorPipeline = new PerFramebufferPipelineCache(pipeline, false);
		}
	}

	function initBuffers(): Void {
		if (rectVertexBuffer == null) {
			rectVertexBuffer = new VertexBuffer(bufferSize * 4, structure, Usage.DynamicUsage);
			rectVertices = rectVertexBuffer.lock();

			indexBuffer = new IndexBuffer(bufferSize * 3 * 2, Usage.StaticUsage);
			var indices = indexBuffer.lock();
			for (i in 0...bufferSize) {
				indices[i * 3 * 2 + 0] = i * 4 + 0;
				indices[i * 3 * 2 + 1] = i * 4 + 1;
				indices[i * 3 * 2 + 2] = i * 4 + 2;
				indices[i * 3 * 2 + 3] = i * 4 + 0;
				indices[i * 3 * 2 + 4] = i * 4 + 2;
				indices[i * 3 * 2 + 5] = i * 4 + 3;
			}
			indexBuffer.unlock();

			triangleVertexBuffer = new VertexBuffer(triangleBufferSize * 3, structure, Usage.DynamicUsage);
			triangleVertices = triangleVertexBuffer.lock();

			triangleIndexBuffer = new IndexBuffer(triangleBufferSize * 3, Usage.StaticUsage);
			var triIndices = triangleIndexBuffer.lock();
			for (i in 0...bufferSize) {
				triIndices[i * 3 + 0] = i * 3 + 0;
				triIndices[i * 3 + 1] = i * 3 + 1;
				triIndices[i * 3 + 2] = i * 3 + 2;
			}
			triangleIndexBuffer.unlock();
		}
	}

	public function setRectVertices(bottomleftx: Float, bottomlefty: Float, topleftx: Float, toplefty: Float, toprightx: Float, toprighty: Float,
			bottomrightx: Float, bottomrighty: Float): Void {
		var baseIndex: Int = bufferIndex * 4 * 4;
		rectVertices.setFloat32((baseIndex + 0) * 4, bottomleftx);
		rectVertices.setFloat32((baseIndex + 1) * 4, bottomlefty);
		rectVertices.setFloat32((baseIndex + 2) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 4) * 4, topleftx);
		rectVertices.setFloat32((baseIndex + 5) * 4, toplefty);
		rectVertices.setFloat32((baseIndex + 6) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 8) * 4, toprightx);
		rectVertices.setFloat32((baseIndex + 9) * 4, toprighty);
		rectVertices.setFloat32((baseIndex + 10) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 12) * 4, bottomrightx);
		rectVertices.setFloat32((baseIndex + 13) * 4, bottomrighty);
		rectVertices.setFloat32((baseIndex + 14) * 4, -5.0);
	}

	public function setRectColors(opacity: FastFloat, color: Color): Void {
		var baseIndex: Int = bufferIndex * 4 * 4 * 4;

		var a: FastFloat = opacity * color.A;
		var r: FastFloat = a * color.R;
		var g: FastFloat = a * color.G;
		var b: FastFloat = a * color.B;

		rectVertices.setUint8(baseIndex + 3 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 3 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 3 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 3 * 4 + 3, Std.int(a * 255));

		rectVertices.setUint8(baseIndex + 7 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 7 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 7 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 7 * 4 + 3, Std.int(a * 255));

		rectVertices.setUint8(baseIndex + 11 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 11 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 11 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 11 * 4 + 3, Std.int(a * 255));

		rectVertices.setUint8(baseIndex + 15 * 4 + 0, Std.int(r * 255));
		rectVertices.setUint8(baseIndex + 15 * 4 + 1, Std.int(g * 255));
		rectVertices.setUint8(baseIndex + 15 * 4 + 2, Std.int(b * 255));
		rectVertices.setUint8(baseIndex + 15 * 4 + 3, Std.int(a * 255));
	}

	function setTriVertices(x1: Float, y1: Float, x2: Float, y2: Float, x3: Float, y3: Float): Void {
		var baseIndex: Int = triangleBufferIndex * 4 * 3;

		triangleVertices.setFloat32((baseIndex + 0) * 4, x1);
		triangleVertices.setFloat32((baseIndex + 1) * 4, y1);
		triangleVertices.setFloat32((baseIndex + 2) * 4, -5.0);

		triangleVertices.setFloat32((baseIndex + 4) * 4, x2);
		triangleVertices.setFloat32((baseIndex + 5) * 4, y2);
		triangleVertices.setFloat32((baseIndex + 6) * 4, -5.0);

		triangleVertices.setFloat32((baseIndex + 8) * 4, x3);
		triangleVertices.setFloat32((baseIndex + 9) * 4, y3);
		triangleVertices.setFloat32((baseIndex + 10) * 4, -5.0);
	}

	function setTriColors(opacity: FastFloat, color: Color): Void {
		var baseIndex: Int = triangleBufferIndex * 4 * 4 * 3;

		var a: FastFloat = opacity * color.A;
		var r: FastFloat = a * color.R;
		var g: FastFloat = a * color.G;
		var b: FastFloat = a * color.B;

		triangleVertices.setUint8(baseIndex + 3 * 4 + 0, Std.int(r * 255));
		triangleVertices.setUint8(baseIndex + 3 * 4 + 1, Std.int(g * 255));
		triangleVertices.setUint8(baseIndex + 3 * 4 + 2, Std.int(b * 255));
		triangleVertices.setUint8(baseIndex + 3 * 4 + 3, Std.int(a * 255));

		triangleVertices.setUint8(baseIndex + 7 * 4 + 0, Std.int(r * 255));
		triangleVertices.setUint8(baseIndex + 7 * 4 + 1, Std.int(g * 255));
		triangleVertices.setUint8(baseIndex + 7 * 4 + 2, Std.int(b * 255));
		triangleVertices.setUint8(baseIndex + 7 * 4 + 3, Std.int(a * 255));

		triangleVertices.setUint8(baseIndex + 11 * 4 + 0, Std.int(r * 255));
		triangleVertices.setUint8(baseIndex + 11 * 4 + 1, Std.int(g * 255));
		triangleVertices.setUint8(baseIndex + 11 * 4 + 2, Std.int(b * 255));
		triangleVertices.setUint8(baseIndex + 11 * 4 + 3, Std.int(a * 255));
	}

	function drawBuffer(trisDone: Bool): Void {
		if (bufferIndex == 0) {
			return;
		}

		if (!trisDone)
			endTris(true);

		rectVertexBuffer.unlock(bufferIndex * 4);
		var pipeline = myPipeline.get(null, Depth24Stencil8);
		g.setPipeline(pipeline.pipeline);
		g.setVertexBuffer(rectVertexBuffer);
		g.setIndexBuffer(indexBuffer);
		g.setMatrix(pipeline.projectionLocation, projectionMatrix);

		g.drawIndexedVertices(0, bufferIndex * 2 * 3);

		bufferIndex = 0;
		rectVertices = rectVertexBuffer.lock();
	}

	function drawTriBuffer(rectsDone: Bool): Void {
		if (!rectsDone)
			endRects(true);

		triangleVertexBuffer.unlock(triangleBufferIndex * 3);
		var pipeline = myPipeline.get(null, Depth24Stencil8);
		g.setPipeline(pipeline.pipeline);
		g.setVertexBuffer(triangleVertexBuffer);
		g.setIndexBuffer(triangleIndexBuffer);
		g.setMatrix(pipeline.projectionLocation, projectionMatrix);

		g.drawIndexedVertices(0, triangleBufferIndex * 3);

		triangleBufferIndex = 0;
		triangleVertices = triangleVertexBuffer.lock();
	}

	public function fillRect(opacity: FastFloat, color: Color, bottomleftx: Float, bottomlefty: Float, topleftx: Float, toplefty: Float, toprightx: Float,
			toprighty: Float, bottomrightx: Float, bottomrighty: Float): Void {
		if (triangleBufferIndex > 0)
			drawTriBuffer(true); // Flush other buffer for right render order

		if (bufferIndex + 1 >= bufferSize)
			drawBuffer(false);

		setRectColors(opacity, color);
		setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);
		++bufferIndex;
	}

	public function fillTriangle(opacity: FastFloat, color: Color, x1: Float, y1: Float, x2: Float, y2: Float, x3: Float, y3: Float) {
		if (bufferIndex > 0)
			drawBuffer(true); // Flush other buffer for right render order

		if (triangleBufferIndex + 1 >= triangleBufferSize)
			drawTriBuffer(false);

		setTriColors(opacity, color);
		setTriVertices(x1, y1, x2, y2, x3, y3);
		++triangleBufferIndex;
	}

	public inline function endTris(rectsDone: Bool): Void {
		if (triangleBufferIndex > 0)
			drawTriBuffer(rectsDone);
	}

	public inline function endRects(trisDone: Bool): Void {
		if (bufferIndex > 0)
			drawBuffer(trisDone);
	}

	public inline function end(): Void {
		endTris(false);
		endRects(false);
	}
}

class TextShaderPainter {
	var projectionMatrix: FastMatrix4;

	static var standardTextPipeline: PipelineCache = null;
	static var structure: VertexStructure = null;
	static inline var bufferSize: Int = 1000;
	static var bufferIndex: Int;
	static var rectVertexBuffer: VertexBuffer;
	static var rectVertices: ByteArray;
	static var indexBuffer: IndexBuffer;

	var font: Font;

	static var lastTexture: Image;

	var g: kha.graphics4.Graphics;
	var myPipeline: PipelineCache = null;

	public var pipeline(get, set): PipelineCache;
	public var fontSize: Int;

	var bilinear: Bool = false;

	public function new(g4: kha.graphics4.Graphics) {
		this.g = g4;
		bufferIndex = 0;
		initShaders();
		myPipeline = standardTextPipeline;
		initBuffers();
	}

	function get_pipeline(): PipelineCache {
		return myPipeline;
	}

	function set_pipeline(pipe: PipelineCache): PipelineCache {
		myPipeline = pipe != null ? pipe : standardTextPipeline;
		return myPipeline;
	}

	public function setProjection(projectionMatrix: FastMatrix4): Void {
		this.projectionMatrix = projectionMatrix;
	}

	static function initShaders(): Void {
		if (structure == null) {
			structure = Graphics.createTextVertexStructure();
		}
		if (standardTextPipeline == null) {
			var pipeline = Graphics.createTextPipeline(structure);
			standardTextPipeline = new PerFramebufferPipelineCache(pipeline, true);
		}
	}

	function initBuffers(): Void {
		if (rectVertexBuffer == null) {
			rectVertexBuffer = new VertexBuffer(bufferSize * 4, structure, Usage.DynamicUsage);
			rectVertices = rectVertexBuffer.lock();

			indexBuffer = new IndexBuffer(bufferSize * 3 * 2, Usage.StaticUsage);
			var indices = indexBuffer.lock();
			for (i in 0...bufferSize) {
				indices[i * 3 * 2 + 0] = i * 4 + 0;
				indices[i * 3 * 2 + 1] = i * 4 + 1;
				indices[i * 3 * 2 + 2] = i * 4 + 2;
				indices[i * 3 * 2 + 3] = i * 4 + 0;
				indices[i * 3 * 2 + 4] = i * 4 + 2;
				indices[i * 3 * 2 + 5] = i * 4 + 3;
			}
			indexBuffer.unlock();
		}
	}

	function setRectVertices(bottomleftx: Float, bottomlefty: Float, topleftx: Float, toplefty: Float, toprightx: Float, toprighty: Float,
			bottomrightx: Float, bottomrighty: Float): Void {
		var baseIndex: Int = bufferIndex * 9 * 4;
		rectVertices.setFloat32((baseIndex + 0) * 4, bottomleftx);
		rectVertices.setFloat32((baseIndex + 1) * 4, bottomlefty);
		rectVertices.setFloat32((baseIndex + 2) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 9) * 4, topleftx);
		rectVertices.setFloat32((baseIndex + 10) * 4, toplefty);
		rectVertices.setFloat32((baseIndex + 11) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 18) * 4, toprightx);
		rectVertices.setFloat32((baseIndex + 19) * 4, toprighty);
		rectVertices.setFloat32((baseIndex + 20) * 4, -5.0);

		rectVertices.setFloat32((baseIndex + 27) * 4, bottomrightx);
		rectVertices.setFloat32((baseIndex + 28) * 4, bottomrighty);
		rectVertices.setFloat32((baseIndex + 29) * 4, -5.0);
	}

	function setRectTexCoords(left: Float, top: Float, right: Float, bottom: Float): Void {
		var baseIndex: Int = bufferIndex * 9 * 4;
		rectVertices.setFloat32((baseIndex + 3) * 4, left);
		rectVertices.setFloat32((baseIndex + 4) * 4, bottom);

		rectVertices.setFloat32((baseIndex + 12) * 4, left);
		rectVertices.setFloat32((baseIndex + 13) * 4, top);

		rectVertices.setFloat32((baseIndex + 21) * 4, right);
		rectVertices.setFloat32((baseIndex + 22) * 4, top);

		rectVertices.setFloat32((baseIndex + 30) * 4, right);
		rectVertices.setFloat32((baseIndex + 31) * 4, bottom);
	}

	function setRectColors(opacity: FastFloat, color: Color): Void {
		var baseIndex: Int = bufferIndex * 9 * 4;
		var a: FastFloat = opacity * color.A;
		rectVertices.setFloat32((baseIndex + 5) * 4, color.R);
		rectVertices.setFloat32((baseIndex + 6) * 4, color.G);
		rectVertices.setFloat32((baseIndex + 7) * 4, color.B);
		rectVertices.setFloat32((baseIndex + 8) * 4, a);

		rectVertices.setFloat32((baseIndex + 14) * 4, color.R);
		rectVertices.setFloat32((baseIndex + 15) * 4, color.G);
		rectVertices.setFloat32((baseIndex + 16) * 4, color.B);
		rectVertices.setFloat32((baseIndex + 17) * 4, a);

		rectVertices.setFloat32((baseIndex + 23) * 4, color.R);
		rectVertices.setFloat32((baseIndex + 24) * 4, color.G);
		rectVertices.setFloat32((baseIndex + 25) * 4, color.B);
		rectVertices.setFloat32((baseIndex + 26) * 4, a);

		rectVertices.setFloat32((baseIndex + 32) * 4, color.R);
		rectVertices.setFloat32((baseIndex + 33) * 4, color.G);
		rectVertices.setFloat32((baseIndex + 34) * 4, color.B);
		rectVertices.setFloat32((baseIndex + 35) * 4, a);
	}

	function drawBuffer(): Void {
		if (bufferIndex == 0) {
			return;
		}

		rectVertexBuffer.unlock(bufferIndex * 4);
		var pipeline = myPipeline.get(null, Depth24Stencil8);
		g.setPipeline(pipeline.pipeline);
		g.setVertexBuffer(rectVertexBuffer);
		g.setIndexBuffer(indexBuffer);
		g.setMatrix(pipeline.projectionLocation, projectionMatrix);
		g.setTexture(pipeline.textureLocation, lastTexture);
		g.setTextureParameters(pipeline.textureLocation, TextureAddressing.Clamp, TextureAddressing.Clamp,
			bilinear ? TextureFilter.LinearFilter : TextureFilter.PointFilter, bilinear ? TextureFilter.LinearFilter : TextureFilter.PointFilter,
			MipMapFilter.NoMipFilter);

		g.drawIndexedVertices(0, bufferIndex * 2 * 3);

		g.setTexture(pipeline.textureLocation, null);
		bufferIndex = 0;
		rectVertices = rectVertexBuffer.lock();
	}

	public function setBilinearFilter(bilinear: Bool): Void {
		end();
		this.bilinear = bilinear;
	}

	public function setFont(font: Font): Void {
		this.font = font;
	}

	static function findIndex(charCode: Int): Int {
		// var glyphs = kha.graphics2.Graphics.fontGlyphs;
		var blocks = Font.KravurImage.charBlocks;
		var offset = 0;
		for (i in 0...Std.int(blocks.length / 2)) {
			var start = blocks[i * 2];
			var end = blocks[i * 2 + 1];
			if (charCode >= start && charCode <= end)
				return offset + charCode - start;
			offset += end - start + 1;
		}
		return 0;
	}

	var bakedQuadCache = new kha.Font.AlignedQuad();

	public function drawString(text: String, opacity: FastFloat, color: Color, x: Float, y: Float, transformation: FastMatrix3): Void {
		var font = this.font._get(fontSize);
		var tex = font.getTexture();
		if (lastTexture != null && tex != lastTexture)
			drawBuffer();
		lastTexture = tex;

		var xpos = x;
		var ypos = y;
		for (i in 0...text.length) {
			var charCode = StringTools.fastCodeAt(text, i);
			var q = font.getBakedQuad(bakedQuadCache, findIndex(charCode), xpos, ypos);
			if (q != null) {
				if (bufferIndex + 1 >= bufferSize)
					drawBuffer();
				setRectColors(opacity, color);
				setRectTexCoords(q.s0, q.t0, q.s1, q.t1);
				var p0 = transformation.multvec(new FastVector2(q.x0, q.y1)); // bottom-left
				var p1 = transformation.multvec(new FastVector2(q.x0, q.y0)); // top-left
				var p2 = transformation.multvec(new FastVector2(q.x1, q.y0)); // top-right
				var p3 = transformation.multvec(new FastVector2(q.x1, q.y1)); // bottom-right
				setRectVertices(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
				xpos += q.xadvance;
				++bufferIndex;
			}
		}
	}

	public function drawCharacters(text: Array<Int>, start: Int, length: Int, opacity: FastFloat, color: Color, x: Float, y: Float,
			transformation: FastMatrix3): Void {
		var font = this.font._get(fontSize);
		var tex = font.getTexture();
		if (lastTexture != null && tex != lastTexture)
			drawBuffer();
		lastTexture = tex;

		var xpos = x;
		var ypos = y;
		for (i in start...start + length) {
			var q = font.getBakedQuad(bakedQuadCache, findIndex(text[i]), xpos, ypos);
			if (q != null) {
				if (bufferIndex + 1 >= bufferSize)
					drawBuffer();
				setRectColors(opacity, color);
				setRectTexCoords(q.s0, q.t0, q.s1, q.t1);
				var p0 = transformation.multvec(new FastVector2(q.x0, q.y1)); // bottom-left
				var p1 = transformation.multvec(new FastVector2(q.x0, q.y0)); // top-left
				var p2 = transformation.multvec(new FastVector2(q.x1, q.y0)); // top-right
				var p3 = transformation.multvec(new FastVector2(q.x1, q.y1)); // bottom-right
				setRectVertices(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
				xpos += q.xadvance;
				++bufferIndex;
			}
		}
	}

	public function end(): Void {
		if (bufferIndex > 0)
			drawBuffer();
		lastTexture = null;
	}
}

class Graphics {
	var myColor: Color;
	var myFont: Font;
	var projectionMatrix: FastMatrix4;

	public var imageScaleQuality(get, set): ImageScaleQuality;
	public var mipmapScaleQuality(get, set): ImageScaleQuality;
	public var color(get, set): Color;
	public var font(get, set): Font;
	public var fontSize(get, set): Int;

	public static var fontGlyphs: Array<Int> = [for (i in 32...256) i];

	// works on the top of the transformation stack
	public var transformation(get, set): FastMatrix3;

	inline function get_transformation(): FastMatrix3 {
		return transformations[transformationIndex];
	}

	inline function set_transformation(transformation: FastMatrix3): FastMatrix3 {
		setTransformation(transformation);
		transformations[transformationIndex].setFrom(transformation);
		return transformation;
	}

	public inline function pushTransformation(trans: FastMatrix3): Void {
		transformationIndex++;
		if (transformationIndex == transformations.length) {
			transformations.push(FastMatrix3.identity());
		}
		transformations[transformationIndex].setFrom(trans);
		setTransformation(get_transformation());
	}

	public function popTransformation(): FastMatrix3 {
		transformationIndex--;
		setTransformation(get_transformation());
		return transformations[transformationIndex + 1];
	}

	function setTransformation(transformation: FastMatrix3): Void {}

	function setOpacity(opacity: Float): Void {}

	public function scale(x: FastFloat, y: FastFloat): Void {
		transformation.setFrom(kha.math.FastMatrix3.scale(x, y).multmat(transformation));
	}

	inline function translation(tx: FastFloat, ty: FastFloat): FastMatrix3 {
		return FastMatrix3.translation(tx, ty).multmat(transformation);
	}

	public function translate(tx: FastFloat, ty: FastFloat): Void {
		transformation.setFrom(translation(tx, ty));
	}

	public function pushTranslation(tx: FastFloat, ty: FastFloat): Void {
		pushTransformation(translation(tx, ty));
	}

	inline function rotation(angle: FastFloat, centerx: FastFloat, centery: FastFloat): FastMatrix3 {
		return FastMatrix3.translation(centerx, centery)
			.multmat(FastMatrix3.rotation(angle))
			.multmat(FastMatrix3.translation(-centerx, -centery))
			.multmat(transformation);
	}

	public function rotate(angle: FastFloat, centerx: FastFloat, centery: FastFloat): Void {
		transformation.setFrom(rotation(angle, centerx, centery));
	}

	public function pushRotation(angle: FastFloat, centerx: FastFloat, centery: FastFloat): Void {
		pushTransformation(rotation(angle, centerx, centery));
	}

	public var opacity(get, set): Float; // works on the top of the opacity stack

	public function pushOpacity(opacity: Float): Void {
		setOpacity(opacity);
		opacities.push(opacity);
	}

	public function popOpacity(): Float {
		var ret = opacities.pop();
		setOpacity(get_opacity());
		return ret;
	}

	function get_opacity(): Float {
		return opacities[opacities.length - 1];
	}

	function set_opacity(opacity: Float): Float {
		setOpacity(opacity);
		return opacities[opacities.length - 1] = opacity;
	}

	private var pipe: PipelineState;

	public var pipeline(get, set): PipelineState;

	private function get_pipeline(): PipelineState {
		return pipe;
	}

	private function set_pipeline(pipeline: PipelineState): PipelineState {
		setPipeline(pipeline);
		return pipe = pipeline;
	}

	var transformations: Array<FastMatrix3>;
	var transformationIndex: Int;
	var opacities: Array<Float>;
	var myFontSize: Int;

	var imagePainter: ImageShaderPainter;
	var coloredPainter: ColoredShaderPainter;
	var textPainter: TextShaderPainter;

	public static var videoPipeline: PipelineState;

	var canvas: Canvas;
	var g: kha.graphics4.Graphics;

	static var current: Graphics = null;

	public function new(canvas: Canvas) {
		transformations = [FastMatrix3.identity()];
		transformationIndex = 0;
		opacities = [1];
		myFontSize = 12;
		pipe = null;

		color = Color.White;
		this.canvas = canvas;
		g = canvas.g4;
		imagePainter = new ImageShaderPainter(g);
		coloredPainter = new ColoredShaderPainter(g);
		textPainter = new TextShaderPainter(g);
		textPainter.fontSize = fontSize;
		projectionMatrix = FastMatrix4.identity();
		setProjection();

		if (videoPipeline == null) {
			videoPipeline = createImagePipeline(createImageVertexStructure());
			videoPipeline.fragmentShader = Shaders.getFragment("painter-video.frag");
			videoPipeline.vertexShader = Shaders.getVertex("painter-video.vert");
			videoPipeline.compile();
		}
	}

	static function upperPowerOfTwo(v: Int): Int {
		v--;
		v |= v >>> 1;
		v |= v >>> 2;
		v |= v >>> 4;
		v |= v >>> 8;
		v |= v >>> 16;
		v++;
		return v;
	}

	function setProjection(): Void {
		var width = canvas.width;
		var height = canvas.height;
		if (Std.isOfType(canvas, Framebuffer)) {
			projectionMatrix.setFrom(FastMatrix4.orthogonalProjection(0, width, height, 0, 0.1, 1000));
		}
		else {
			if (!Image.nonPow2Supported) {
				width = upperPowerOfTwo(width);
				height = upperPowerOfTwo(height);
			}
			if (Image.renderTargetsInvertedY()) {
				projectionMatrix.setFrom(FastMatrix4.orthogonalProjection(0, width, 0, height, 0.1, 1000));
			}
			else {
				projectionMatrix.setFrom(FastMatrix4.orthogonalProjection(0, width, height, 0, 0.1, 1000));
			}
		}
		imagePainter.setProjection(projectionMatrix);
		coloredPainter.setProjection(projectionMatrix);
		textPainter.setProjection(projectionMatrix);
	}

	public function drawImage(img: kha.Image, x: FastFloat, y: FastFloat): Void {
		coloredPainter.end();
		textPainter.end();
		var xw: FastFloat = x + img.width;
		var yh: FastFloat = y + img.height;
		var p1 = transformation.multvec(new FastVector2(x, yh));
		var p2 = transformation.multvec(new FastVector2(x, y));
		var p3 = transformation.multvec(new FastVector2(xw, y));
		var p4 = transformation.multvec(new FastVector2(xw, yh));
		imagePainter.drawImage(img, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, opacity, this.color);
	}

	public function drawScaledSubImage(img: kha.Image, sx: FastFloat, sy: FastFloat, sw: FastFloat, sh: FastFloat, dx: FastFloat, dy: FastFloat,
			dw: FastFloat, dh: FastFloat): Void {
		coloredPainter.end();
		textPainter.end();
		var p1 = transformation.multvec(new FastVector2(dx, dy + dh));
		var p2 = transformation.multvec(new FastVector2(dx, dy));
		var p3 = transformation.multvec(new FastVector2(dx + dw, dy));
		var p4 = transformation.multvec(new FastVector2(dx + dw, dy + dh));
		imagePainter.drawImage2(img, sx, sy, sw, sh, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, opacity, this.color);
	}

	function get_color(): Color {
		return myColor;
	}

	function set_color(color: Color): Color {
		return myColor = color;
	}

	public function drawRect(x: Float, y: Float, width: Float, height: Float, strength: Float = 1.0): Void {
		imagePainter.end();
		textPainter.end();

		var p1 = transformation.multvec(new FastVector2(x - strength / 2, y + strength / 2)); // bottom-left
		var p2 = transformation.multvec(new FastVector2(x - strength / 2, y - strength / 2)); // top-left
		var p3 = transformation.multvec(new FastVector2(x + width + strength / 2, y - strength / 2)); // top-right
		var p4 = transformation.multvec(new FastVector2(x + width + strength / 2, y + strength / 2)); // bottom-right
		coloredPainter.fillRect(opacity, color, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y); // top

		p1.setFrom(transformation.multvec(new FastVector2(x - strength / 2, y + height - strength / 2)));
		p2.setFrom(transformation.multvec(new FastVector2(x - strength / 2, y + strength / 2)));
		p3.setFrom(transformation.multvec(new FastVector2(x + strength / 2, y + strength / 2)));
		p4.setFrom(transformation.multvec(new FastVector2(x + strength / 2, y + height - strength / 2)));
		coloredPainter.fillRect(opacity, color, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y); // left

		p1.setFrom(transformation.multvec(new FastVector2(x - strength / 2, y + height + strength / 2)));
		p2.setFrom(transformation.multvec(new FastVector2(x - strength / 2, y + height - strength / 2)));
		p3.setFrom(transformation.multvec(new FastVector2(x + width + strength / 2, y + height - strength / 2)));
		p4.setFrom(transformation.multvec(new FastVector2(x + width + strength / 2, y + height + strength / 2)));
		coloredPainter.fillRect(opacity, color, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y); // bottom

		p1.setFrom(transformation.multvec(new FastVector2(x + width - strength / 2, y + height - strength / 2)));
		p2.setFrom(transformation.multvec(new FastVector2(x + width - strength / 2, y + strength / 2)));
		p3.setFrom(transformation.multvec(new FastVector2(x + width + strength / 2, y + strength / 2)));
		p4.setFrom(transformation.multvec(new FastVector2(x + width + strength / 2, y + height - strength / 2)));
		coloredPainter.fillRect(opacity, color, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y); // right
	}

	public function fillRect(x: Float, y: Float, width: Float, height: Float): Void {
		imagePainter.end();
		textPainter.end();

		var p1 = transformation.multvec(new FastVector2(x, y + height));
		var p2 = transformation.multvec(new FastVector2(x, y));
		var p3 = transformation.multvec(new FastVector2(x + width, y));
		var p4 = transformation.multvec(new FastVector2(x + width, y + height));
		coloredPainter.fillRect(opacity, color, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
	}

	public function drawString(text: String, x: Float, y: Float): Void {
		imagePainter.end();
		coloredPainter.end();

		textPainter.drawString(text, opacity, color, x, y, transformation);
	}

	public function drawCharacters(text: Array<Int>, start: Int, length: Int, x: Float, y: Float): Void {
		imagePainter.end();
		coloredPainter.end();

		textPainter.drawCharacters(text, start, length, opacity, color, x, y, transformation);
	}

	function get_font(): Font {
		return myFont;
	}

	function set_font(font: Font): Font {
		textPainter.setFont(font);
		return myFont = font;
	}

	function get_fontSize(): Int {
		return myFontSize;
	}

	function set_fontSize(value: Int): Int {
		return myFontSize = textPainter.fontSize = value;
	}

	public function drawLine(x1: Float, y1: Float, x2: Float, y2: Float, strength: Float = 1.0): Void {
		imagePainter.end();
		textPainter.end();

		var vec = new FastVector2();
		if (y2 == y1)
			vec.setFrom(new FastVector2(0, -1));
		else
			vec.setFrom(new FastVector2(1, -(x2 - x1) / (y2 - y1)));
		vec.length = strength;
		var p1 = new FastVector2(x1 + 0.5 * vec.x, y1 + 0.5 * vec.y);
		var p2 = new FastVector2(x2 + 0.5 * vec.x, y2 + 0.5 * vec.y);
		var p3 = p1.sub(vec);
		var p4 = p2.sub(vec);

		p1.setFrom(transformation.multvec(p1));
		p2.setFrom(transformation.multvec(p2));
		p3.setFrom(transformation.multvec(p3));
		p4.setFrom(transformation.multvec(p4));

		coloredPainter.fillTriangle(opacity, color, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
		coloredPainter.fillTriangle(opacity, color, p3.x, p3.y, p2.x, p2.y, p4.x, p4.y);
	}

	public function fillTriangle(x1: Float, y1: Float, x2: Float, y2: Float, x3: Float, y3: Float) {
		imagePainter.end();
		textPainter.end();

		var p1 = transformation.multvec(new FastVector2(x1, y1));
		var p2 = transformation.multvec(new FastVector2(x2, y2));
		var p3 = transformation.multvec(new FastVector2(x3, y3));
		coloredPainter.fillTriangle(opacity, color, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
	}

	var myImageScaleQuality: ImageScaleQuality = ImageScaleQuality.Low;

	function get_imageScaleQuality(): ImageScaleQuality {
		return myImageScaleQuality;
	}

	function set_imageScaleQuality(value: ImageScaleQuality): ImageScaleQuality {
		if (value == myImageScaleQuality) {
			return value;
		}
		imagePainter.setBilinearFilter(value == ImageScaleQuality.High);
		textPainter.setBilinearFilter(value == ImageScaleQuality.High);
		return myImageScaleQuality = value;
	}

	var myMipmapScaleQuality: ImageScaleQuality = ImageScaleQuality.Low;

	function get_mipmapScaleQuality(): ImageScaleQuality {
		return myMipmapScaleQuality;
	}

	function set_mipmapScaleQuality(value: ImageScaleQuality): ImageScaleQuality {
		imagePainter.setBilinearMipmapFilter(value == ImageScaleQuality.High);
		// textPainter.setBilinearMipmapFilter(value == ImageScaleQuality.High); // TODO (DK) implement for fonts as well?
		return myMipmapScaleQuality = value;
	}

	var pipelineCache = new Map<PipelineState, PipelineCache>();
	var lastPipeline: PipelineState = null;

	function setPipeline(pipeline: PipelineState): Void {
		if (pipeline == lastPipeline) {
			return;
		}
		lastPipeline = pipeline;
		flush();
		if (pipeline == null) {
			imagePainter.pipeline = null;
			coloredPainter.pipeline = null;
			textPainter.pipeline = null;
		}
		else {
			var cache = pipelineCache[pipeline];
			if (cache == null) {
				cache = new SimplePipelineCache(pipeline, true);
				pipelineCache[pipeline] = cache;
			}
			imagePainter.pipeline = cache;
			coloredPainter.pipeline = cache;
			textPainter.pipeline = cache;
		}
	}

	var scissorEnabled = false;
	var scissorX: Int = -1;
	var scissorY: Int = -1;
	var scissorW: Int = -1;
	var scissorH: Int = -1;

	public function scissor(x: Int, y: Int, width: Int, height: Int): Void {
		// if (!scissorEnabled || x != scissorX || y != scissorY || width != scissorW || height != scissorH) {
		scissorEnabled = true;
		scissorX = x;
		scissorY = y;
		scissorW = width;
		scissorH = height;
		flush();
		g.scissor(x, y, width, height);
		// }
	}

	public function disableScissor(): Void {
		// if (scissorEnabled) {
		scissorEnabled = false;
		flush();
		g.disableScissor();
		// }
	}

	static var thrown = false;

	public function begin(clear: Bool = true, clearColor: Color = null): Void {
		if (current == null) {
			current = this;
		}
		else {
			if (!thrown) { thrown = true; throw "End before you begin"; }
		}

		g.begin();
		if (clear)
			this.clear(clearColor);
		setProjection();
	}

	public function clear(color: Color = null): Void {
		flush();
		g.clear(color == null ? Color.Black : color);
	}

	public function flush(): Void {
		imagePainter.end();
		textPainter.end();
		coloredPainter.end();
	}

	public function end(): Void {
		flush();
		g.end();

		if (current == this) {
			current = null;
		}
		else {
			if (!thrown) { thrown = true; throw "Begin before you end"; }
		}
	}

	function drawVideoInternal(video: kha.Video, x: Float, y: Float, width: Float, height: Float): Void {}

	public function drawVideo(video: kha.Video, x: Float, y: Float, width: Float, height: Float): Void {
		setPipeline(videoPipeline);
		drawVideoInternal(video, x, y, width, height);
		setPipeline(null);
	}

	public static function createImageVertexStructure(): VertexStructure {
		var structure = new VertexStructure();
		structure.add("vertexPosition", VertexData.Float32_3X);
		structure.add("vertexUV", VertexData.Float32_2X);
		structure.add("vertexColor", VertexData.UInt8_4X_Normalized);
		return structure;
	}

	public static function createImagePipeline(structure: VertexStructure): PipelineState {
		var shaderPipeline = new PipelineState();
		shaderPipeline.fragmentShader = Shaders.getFragment("painter-image.frag");
		shaderPipeline.vertexShader = Shaders.getVertex("painter-image.vert");
		shaderPipeline.inputLayout = [structure];
		shaderPipeline.blendSource = BlendingFactor.BlendOne;
		shaderPipeline.blendDestination = BlendingFactor.InverseSourceAlpha;
		shaderPipeline.alphaBlendSource = BlendingFactor.BlendOne;
		shaderPipeline.alphaBlendDestination = BlendingFactor.InverseSourceAlpha;
		return shaderPipeline;
	}

	public static function createColoredVertexStructure(): VertexStructure {
		var structure = new VertexStructure();
		structure.add("vertexPosition", VertexData.Float32_3X);
		structure.add("vertexColor", VertexData.UInt8_4X_Normalized);
		return structure;
	}

	public static function createColoredPipeline(structure: VertexStructure): PipelineState {
		var shaderPipeline = new PipelineState();
		shaderPipeline.fragmentShader = Shaders.getFragment("painter-colored.frag");
		shaderPipeline.vertexShader = Shaders.getVertex("painter-colored.vert");
		shaderPipeline.inputLayout = [structure];
		shaderPipeline.blendSource = BlendingFactor.BlendOne;
		shaderPipeline.blendDestination = BlendingFactor.InverseSourceAlpha;
		shaderPipeline.alphaBlendSource = BlendingFactor.BlendOne;
		shaderPipeline.alphaBlendDestination = BlendingFactor.InverseSourceAlpha;
		return shaderPipeline;
	}

	public static function createTextVertexStructure(): VertexStructure {
		var structure = new VertexStructure();
		structure.add("vertexPosition", VertexData.Float32_3X);
		structure.add("vertexUV", VertexData.Float32_2X);
		structure.add("vertexColor", VertexData.Float32_4X);
		return structure;
	}

	public static function createTextPipeline(structure: VertexStructure): PipelineState {
		var shaderPipeline = new PipelineState();
		shaderPipeline.fragmentShader = Shaders.getFragment("painter-text.frag");
		shaderPipeline.vertexShader = Shaders.getVertex("painter-text.vert");
		shaderPipeline.inputLayout = [structure];
		shaderPipeline.blendSource = BlendingFactor.SourceAlpha;
		shaderPipeline.blendDestination = BlendingFactor.InverseSourceAlpha;
		shaderPipeline.alphaBlendSource = BlendingFactor.SourceAlpha;
		shaderPipeline.alphaBlendDestination = BlendingFactor.InverseSourceAlpha;
		return shaderPipeline;
	}

	public function drawSubImage(img: Image, x: FastFloat, y: FastFloat, sx: FastFloat, sy: FastFloat, sw: FastFloat, sh: FastFloat): Void {
		drawScaledSubImage(img, sx, sy, sw, sh, x, y, sw, sh);
	}

	public function drawScaledImage(img: Image, dx: FastFloat, dy: FastFloat, dw: FastFloat, dh: FastFloat): Void {
		drawScaledSubImage(img, 0, 0, img.width, img.height, dx, dy, dw, dh);
	}
}
