// Red triangle test
// Note: Compiling HLSL shaders from source requires armorcore with d3dcompiler kincflag enabled

function dropFilesCallback(path) {}
function cutCallback() { return ""; }
function copyCallback() { return ""; }
function pasteCallback(string) {}
function foregroundCallback() {}
function resumeCallback() {}
function pauseCallback() {}
function backgroundCallback() {}
function shutdownCallback() {}
function keyboardDownCallback(key) {}
function keyboardUpCallback(key) {}
function keyboardPressCallback(char) {}
function mouseDownCallback(button, x, y) {}
function mouseUpCallback(button, x, y) {}
function mouseMoveCallback(x, y, mx, my) {}
function mouseWheelCallback(delta) {}
function gamepadAxisCallback(gamepad, axis, value) {}
function gamepadButtonCallback(gamepad, button, value) {}
function penDownCallback(x, y, pressure) {}
function penUpCallback(x, y, pressure) {}
function penMoveCallback(x, y, pressure) {}
function audioCallback(samples) {}

let vs = `
struct VSOut { float4 gl_Position : SV_Position; };
VSOut main(float3 pos : TEXCOORD0) {
	VSOut output;
	output.gl_Position = float4(pos, 1.0);
	return output;
}
`;

let fs = `
float4 main() : SV_Target0 {
	return float4(1.0, 0.0, 0.0, 1.0);
}
`;

// let vs = `
// #version 330
// in vec3 pos;
// void main() {
// 	gl_Position = vec4(pos, 1.0);
// }
// `;

// let fs = `
// #version 330
// out vec4 fragColor;
// void main() {
// 	fragColor = vec4(1.0, 0.0, 0.0, 1.0);
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
Krom.init("KromApp", 640, 480, true, 0, resizable | minimizable | maximizable, -1, -1, 60);
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
Krom.setGamepadAxisCallback(gamepadAxisCallback);
Krom.setGamepadButtonCallback(gamepadButtonCallback);
Krom.setPenDownCallback(penDownCallback);
Krom.setPenUpCallback(penUpCallback);
Krom.setPenMoveCallback(penMoveCallback);
Krom.setAudioCallback(audioCallback);

let pipeline = Krom.createPipeline();
let elem = { name: "pos", data: 2 }; // Float3
let structure0 = { elements: [elem] };
let vert = Krom.createVertexShaderFromSource(vs);
let frag = Krom.createFragmentShaderFromSource(fs);
Krom.compilePipeline(pipeline, structure0, null, null, null, 1, vert, frag, null, null, null, {
	cullMode: 0,
	depthWrite: false,
	depthMode: 0,
	stencilMode: 0,
	stencilBothPass: 0,
	stencilDepthFail: 0,
	stencilFail: 0,
	stencilReferenceValue: 0,
	stencilReadMask: 0,
	stencilWriteMask: 0,
	blendSource: 0,
	blendDestination: 0,
	alphaBlendSource: 0,
	alphaBlendDestination: 0,
	colorWriteMaskRed: [true, true, true, true, true, true, true, true],
	colorWriteMaskGreen: [true, true, true, true, true, true, true, true],
	colorWriteMaskBlue: [true, true, true, true, true, true, true, true],
	colorWriteMaskAlpha: [true, true, true, true, true, true, true, true],
	conservativeRasterization: false,
	colorAttachmentCount: 0,
	colorAttachments: [0],
	depth_attachment_bits: 0,
	stencil_attachment_bits: 0
});

let vb = Krom.createVertexBuffer(vertices.length / 3, structure0.elements, 0, 0);
let vbData = new Float32Array(Krom.lockVertexBuffer(vb, 0, vertices.length / 3));
for (i = 0; i < vertices.length; i++) vbData[i] = vertices[i];
Krom.unlockVertexBuffer(vb, vertices.length / 3);

let ib = Krom.createIndexBuffer(indices.length);
let ibData = Krom.lockIndexBuffer(ib);
for (i = 0; i < indices.length; i++) ibData[i] = indices[i];
Krom.unlockIndexBuffer(ib);

function renderCallback() {
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
