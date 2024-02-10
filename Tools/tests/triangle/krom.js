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
	krom_init("ArmorCore", 640, 480, true, 0, resizable | minimizable | maximizable, -1, -1, 60);

	let pipeline = krom_g4_create_pipeline();
	let elem = { name: "pos", data: 3 }; // Float3
	let structure0 = { elements: [elem] };
	let vert = krom_g4_create_vertex_shader_from_source(vs);
	let frag = krom_g4_create_fragment_shader_from_source(fs);

	krom_g4_compile_pipeline(pipeline, structure0, null, null, null, 1, vert, frag, null, {
		cull_mode: 0,
		depth_write: false,
		depth_mode: 0,
		blend_source: 0,
		blend_dest: 0,
		alpha_blend_source: 0,
		alpha_blend_dest: 0,
		color_write_masks_red: [true, true, true, true, true, true, true, true],
		color_write_masks_green: [true, true, true, true, true, true, true, true],
		color_write_masks_blue: [true, true, true, true, true, true, true, true],
		color_write_masks_alpha: [true, true, true, true, true, true, true, true],
		color_attachment_count: 0,
		color_attachments: [0],
		depth_attachment_bits: 0
	});

	let vb = krom_g4_create_vertex_buffer(vertices.length / 3, structure0.elements, 0, 0);
	let vb_data = new Float32Array(krom_g4_lock_vertex_buffer(vb, 0, vertices.length / 3));
	for (let i = 0; i < vertices.length; i++) {
		vb_data[i] = vertices[i];
	}
	krom_g4_unlock_vertex_buffer(vb, vertices.length / 3);

	let ib = krom_g4_create_index_buffer(indices.length);
	let ib_data = krom_g4_lock_index_buffer(ib);
	for (let i = 0; i < indices.length; i++) {
		ib_data[i] = indices[i];
	}
	krom_g4_unlock_index_buffer(ib);

	function render() {
		krom_g4_begin(null, null);

		let flags = 0;
		flags |= 1; // Color
		flags |= 2; // Depth
		krom_g4_clear(flags, 0xff000000, 1.0);

		krom_g4_set_pipeline(pipeline);
		krom_g4_set_vertex_buffer(vb);
		krom_g4_set_index_buffer(ib);
		krom_g4_draw_indexed_vertices(0, -1);

		krom_g4_end();
	}

	krom_set_update_callback(render);
})();
