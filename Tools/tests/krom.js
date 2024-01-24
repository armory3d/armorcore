"use strict";

// Red triangle test
(function () {

	let vs = `
	#version 330
	in vec3 pos;
	void main() {
		gl_Position = vec4(pos, 1.0);
	}
	`;

	let fs = `
	#version 330
	out vec4 fragColor;
	void main() {
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	`;

	// let vs = `
	// struct VSOut { float4 gl_Position : SV_Position; };
	// VSOut main(float3 pos : TEXCOORD0) {
	// 	VSOut output;
	// 	output.gl_Position = float4(pos, 1.0);
	// 	return output;
	// }
	// `;

	// let fs = `
	// float4 main() : SV_Target0 {
	// 	return float4(1.0, 0.0, 0.0, 1.0);
	// }
	// `;

	// let vs = `#version 300 es
	// in vec3 pos;
	// void main() {
	// 	gl_Position = vec4(pos, 1.0);
	// }
	// `;

	// let fs = `#version 300 es
	// precision highp float;
	// precision highp int;
	// out vec4 fragColor;
	// void main() {
	// 	fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	// }
	// `;

	let vertices = [
		-1.0, -1.0, 0.0,
		 1.0, -1.0, 0.0,
		 0.0,  1.0, 0.0
	];

	let indices = [0, 1, 2];

	const resizable = 1;
	const minimizable = 2;
	const maximizable = 4;
	Krom.init("ArmorCore", 640, 480, true, 0, resizable | minimizable | maximizable, -1, -1, 60);

	let pipeline = Krom.createPipeline();
	let elem = { name: "pos", data: 3 }; // Float3
	let structure0 = { elements: [elem] };
	let vert = Krom.createVertexShaderFromSource(vs);
	let frag = Krom.createFragmentShaderFromSource(fs);

	Krom.compilePipeline(pipeline, structure0, null, null, null, 1, vert, frag, null, {
		cullMode: 0,
		depthWrite: false,
		depthMode: 0,
		blendSource: 0,
		blendDestination: 0,
		alphaBlendSource: 0,
		alphaBlendDestination: 0,
		colorWriteMaskRed: [true, true, true, true, true, true, true, true],
		colorWriteMaskGreen: [true, true, true, true, true, true, true, true],
		colorWriteMaskBlue: [true, true, true, true, true, true, true, true],
		colorWriteMaskAlpha: [true, true, true, true, true, true, true, true],
		colorAttachmentCount: 0,
		colorAttachments: [0],
		depthAttachmentBits: 0
	});

	let vb = Krom.createVertexBuffer(vertices.length / 3, structure0.elements, 0, 0);
	let vb_data = new Float32Array(Krom.lockVertexBuffer(vb, 0, vertices.length / 3));
	for (let i = 0; i < vertices.length; i++) vb_data[i] = vertices[i];
	Krom.unlockVertexBuffer(vb, vertices.length / 3);

	let ib = Krom.createIndexBuffer(indices.length);
	let ib_data = Krom.lockIndexBuffer(ib);
	for (let i = 0; i < indices.length; i++) ib_data[i] = indices[i];
	Krom.unlockIndexBuffer(ib);

	function render() {
		Krom.begin(null, null);

		let flags = 0;
		flags |= 1; // Color
		flags |= 2; // Depth
		Krom.clear(flags, 0xff000000, 1.0, null);

		Krom.setPipeline(pipeline);
		Krom.setVertexBuffer(vb);
		Krom.setIndexBuffer(ib);
		Krom.drawIndexedVertices(0, -1);

		Krom.end();
	}

	Krom.setCallback(render);
})();
