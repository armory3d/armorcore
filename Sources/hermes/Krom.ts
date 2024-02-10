"use strict";
// declare type c_ptr = any;
// declare type c_int = number;
// declare type c_float = number;
// declare type c_char = number;
// declare type c_uchar = number;
// declare type c_size_t = number;
// declare function print(s: string): void;
// declare let $SHBuiltin: any;
type u8 = number;
type i32 = number;
type f32 = number;
type bool = boolean;
type image_t = any;
type vertex_element_t = any;

(function () {

	const c_null = $SHBuiltin.c_null();
	const _ptr_write_char = $SHBuiltin.extern_c({ declared: true },
		function _sh_ptr_write_char(ptr: c_ptr, offset: c_int, v: c_char): void {}
	);
	const _ptr_read_uchar = $SHBuiltin.extern_c({ declared: true },
		function _sh_ptr_read_uchar(ptr: c_ptr, offset: c_int): c_uchar { throw 0; }
	);
	const _malloc = $SHBuiltin.extern_c({ include: "stdlib.h" },
		function malloc(size: c_size_t): c_ptr { throw 0; }
	);
	const _free = $SHBuiltin.extern_c({ include: "stdlib.h" },
		function free(p: c_ptr): void {}
	);

	function to_c_string(s: any): c_ptr {
		// "inline";
		// "use unsafe";
		let buf = _malloc(s.length + 1);
		let i = 0;
		for (let l = s.length; i < l; ++i) {
			let code: number = s.charCodeAt(i);
			_ptr_write_char(buf, i, code);
		}
		_ptr_write_char(buf, i, 0);
		return buf;
	}

	function to_js_string(buf: c_ptr): string {
		let res = "";
		for (let i = 0; ; ++i) {
			let ch = _ptr_read_uchar(buf, i);
			if (ch == 0) break;
			res += String.fromCharCode(ch);
		}
		return res;
	}

	let krom_init = $SHBuiltin.extern_c({},
		function _krom_init(title: c_ptr, width: c_int, height: c_int,
			v_sync: c_int, window_mode: c_int, window_features: c_int,
			x: c_int, y: c_int, frequency: c_int): void {}
	);

	globalThis.krom_init = function(title: string, width: i32, height: i32,
		v_sync: bool, window_mode: i32, window_features: i32,
		x: i32, y: i32, frequency: i32) {

		krom_init(to_c_string(title), width, height,
			v_sync ? 1 : 0, window_mode, window_features,
			x, y, frequency);
	};


	let krom_set_update_callback = $SHBuiltin.extern_c({},
		function _krom_set_update_callback(callback: c_ptr): void {}
	);

	globalThis.krom_set_update_callback = function(callback: ()=>void) {
		// krom_set_update_callback(callback);
		globalThis.krom_callback = callback;
	};


	let krom_begin = $SHBuiltin.extern_c({},
		function _krom_begin(): void {}
	);

	globalThis.krom_begin = function(render_target: image_t, additional_render_targets: image_t[]) {
		krom_begin();
	};


	let krom_end = $SHBuiltin.extern_c({},
		function _krom_end(): void {}
	);

	globalThis.krom_end = function() {
		krom_end();
	};


	let krom_clear = $SHBuiltin.extern_c({},
		function _krom_clear(flags: c_int, color: c_int, depth: c_float): void {}
	);

	globalThis.krom_clear = function(flags: i32, color: i32, depth: f32) {
		krom_clear(flags, color, depth);
	};


	let krom_create_pipeline = $SHBuiltin.extern_c({},
		function _krom_create_pipeline(): c_ptr { throw 0; }
	);

	globalThis.krom_create_pipeline = function(): any {
		return krom_create_pipeline();
	};


	let krom_create_vertex_shader_from_source = $SHBuiltin.extern_c({},
		function _krom_create_vertex_shader_from_source(source: c_ptr): c_ptr { throw 0; }
	);

    globalThis.krom_create_vertex_shader_from_source = function(source: string): any {
        return krom_create_vertex_shader_from_source(to_c_string(source));
    };


	let krom_create_fragment_shader_from_source = $SHBuiltin.extern_c({},
		function _krom_create_fragment_shader_from_source(source: c_ptr): c_ptr { throw 0; }
	);

    globalThis.krom_create_fragment_shader_from_source = function(source: string): any {
        return krom_create_fragment_shader_from_source(to_c_string(source));
    };


	let krom_compile_pipeline = $SHBuiltin.extern_c({},
		function _krom_compile_pipeline(
			pipeline: c_ptr,
			name0: c_ptr, data0: c_int,
			name1: c_ptr, data1: c_int,
			name2: c_ptr, data2: c_int,
			name3: c_ptr, data3: c_int,
			name4: c_ptr, data4: c_int,
			name5: c_ptr, data5: c_int,
			name6: c_ptr, data6: c_int,
			name7: c_ptr, data7: c_int,
			cull_mode: c_int, depth_write: c_int, depth_mode: c_int,
			blend_source: c_int, blend_dest: c_int,
			alpha_blend_source: c_int, alpha_blend_dest: c_int,
			color_write_mask_red: c_int, color_write_mask_green: c_int, color_write_mask_blue: c_int, color_write_mask_alpha: c_int,
			color_attachment_count: c_int, depth_attachment_bits: c_int,
			vertex_shader: c_ptr, fragment_shader: c_ptr
		): void {}
	);

	globalThis.krom_compile_pipeline = function(pipeline: any,
		structure0: any, structure1: any, structure2: any, structure3: any, length: i32,
		vertex_shader: any, fragment_shader: any, geometry_shader: any, state: any) {

		let name0 = structure0.elements.length > 0 ? to_c_string(structure0.elements[0].name) : c_null;
		let data0 = structure0.elements.length > 0 ? structure0.elements[0].data : 0;
		let name1 = structure0.elements.length > 1 ? to_c_string(structure0.elements[1].name) : c_null;
		let data1 = structure0.elements.length > 1 ? structure0.elements[1].data : 0;
		let name2 = structure0.elements.length > 2 ? to_c_string(structure0.elements[2].name) : c_null;
		let data2 = structure0.elements.length > 2 ? structure0.elements[2].data : 0;
		let name3 = structure0.elements.length > 3 ? to_c_string(structure0.elements[3].name) : c_null;
		let data3 = structure0.elements.length > 3 ? structure0.elements[3].data : 0;
		let name4 = structure0.elements.length > 4 ? to_c_string(structure0.elements[4].name) : c_null;
		let data4 = structure0.elements.length > 4 ? structure0.elements[4].data : 0;
		let name5 = structure0.elements.length > 5 ? to_c_string(structure0.elements[5].name) : c_null;
		let data5 = structure0.elements.length > 5 ? structure0.elements[5].data : 0;
		let name6 = structure0.elements.length > 6 ? to_c_string(structure0.elements[6].name) : c_null;
		let data6 = structure0.elements.length > 6 ? structure0.elements[6].data : 0;
		let name7 = structure0.elements.length > 7 ? to_c_string(structure0.elements[7].name) : c_null;
		let data7 = structure0.elements.length > 7 ? structure0.elements[7].data : 0;
		krom_compile_pipeline(pipeline,
			name0, data0, name1, data1, name2, data2, name3, data3,
			name4, data4, name5, data5, name6, data6, name7, data7,
			state.cull_mode, state.depth_write ? 1 : 0, state.depth_mode,
			state.blend_source, state.blend_dest,
			state.alpha_blend_source, state.alpha_blend_dest,
			state.color_write_masks_red[0] ? 1 : 0, state.color_write_masks_green[0] ? 1 : 0,
			state.color_write_masks_blue[0] ? 1 : 0, state.color_write_masks_alpha[0] ? 1 : 0,
			state.color_attachment_count, state.depth_attachment_bits,
			vertex_shader, fragment_shader
		);
	};


	let krom_create_vertex_buffer = $SHBuiltin.extern_c({},
		function _krom_create_vertex_buffer(
			count: c_int,
			name0: c_ptr, data0: c_int,
			name1: c_ptr, data1: c_int,
			name2: c_ptr, data2: c_int,
			name3: c_ptr, data3: c_int,
			name4: c_ptr, data4: c_int,
			name5: c_ptr, data5: c_int,
			name6: c_ptr, data6: c_int,
			name7: c_ptr, data7: c_int,
			usage: c_int, inst_data_step_rate: c_int
		): c_ptr { throw 0; }
	);

	globalThis.krom_create_vertex_buffer = function(count: i32, elements: vertex_element_t[], usage: i32, inst_data_step_rate: i32): any {
		let name0 = elements.length > 0 ? to_c_string(elements[0].name) : c_null;
		let data0 = elements.length > 0 ? elements[0].data : 0;
		let name1 = elements.length > 1 ? to_c_string(elements[1].name) : c_null;
		let data1 = elements.length > 1 ? elements[1].data : 0;
		let name2 = elements.length > 2 ? to_c_string(elements[2].name) : c_null;
		let data2 = elements.length > 2 ? elements[2].data : 0;
		let name3 = elements.length > 3 ? to_c_string(elements[3].name) : c_null;
		let data3 = elements.length > 3 ? elements[3].data : 0;
		let name4 = elements.length > 4 ? to_c_string(elements[4].name) : c_null;
		let data4 = elements.length > 4 ? elements[4].data : 0;
		let name5 = elements.length > 5 ? to_c_string(elements[5].name) : c_null;
		let data5 = elements.length > 5 ? elements[5].data : 0;
		let name6 = elements.length > 6 ? to_c_string(elements[6].name) : c_null;
		let data6 = elements.length > 6 ? elements[6].data : 0;
		let name7 = elements.length > 7 ? to_c_string(elements[7].name) : c_null;
		let data7 = elements.length > 7 ? elements[7].data : 0;

		return krom_create_vertex_buffer(count,
			name0, data0, name1, data1, name2, data2, name3, data3,
			name4, data4, name5, data5, name6, data6, name7, data7,
			usage, inst_data_step_rate);
	};


	let krom_lock_vertex_buffer = $SHBuiltin.extern_c({},
		function _krom_lock_vertex_buffer(buffer: c_ptr): c_ptr { throw 0; }
	);

	globalThis.krom_lock_vertex_buffer = function(buffer: any): any/*ArrayBuffer*/ {
		// return krom_lock_vertex_buffer(buffer);
		krom_lock_vertex_buffer(buffer);
		return globalThis._arraybuffer;
	};


	let krom_unlock_vertex_buffer = $SHBuiltin.extern_c({},
		function _krom_unlock_vertex_buffer(buffer: c_ptr): void {}
	);

	globalThis.krom_unlock_vertex_buffer = function(buffer: any) {
		krom_unlock_vertex_buffer(buffer);
	};


	let krom_create_index_buffer = $SHBuiltin.extern_c({},
		function _krom_create_index_buffer(count: c_int): c_ptr { throw 0; }
	);

	globalThis.krom_create_index_buffer = function(count: i32): any {
		return krom_create_index_buffer(count);
	};


	let krom_lock_index_buffer = $SHBuiltin.extern_c({},
		function _krom_lock_index_buffer(buffer: c_ptr): c_ptr { throw 0; }
	);

	globalThis.krom_lock_index_buffer = function(buffer: any): any/*Uint32Array*/ {
		// return krom_lock_index_buffer(buffer);
		krom_lock_index_buffer(buffer);
		return new Uint32Array(globalThis._arraybuffer);
	};


	let krom_unlock_index_buffer = $SHBuiltin.extern_c({},
		function _krom_unlock_index_buffer(buffer: c_ptr): void {}
	);

	globalThis.krom_unlock_index_buffer = function(buffer: any) {
		krom_unlock_index_buffer(buffer);
	};


	let krom_set_pipeline = $SHBuiltin.extern_c({},
		function _krom_set_pipeline(pipeline: c_ptr): void {}
	);

	globalThis.krom_set_pipeline = function(pipeline: any) {
		krom_set_pipeline(pipeline);
	};


	let krom_set_vertex_buffer = $SHBuiltin.extern_c({},
		function _krom_set_vertex_buffer(vb: c_ptr): void {}
	);

	globalThis.krom_set_vertex_buffer = function(vb: any) {
		krom_set_vertex_buffer(vb);
	};


	let krom_set_index_buffer = $SHBuiltin.extern_c({},
		function _krom_set_index_buffer(vb: c_ptr): void {}
	);

	globalThis.krom_set_index_buffer = function(ib: any) {
		krom_set_index_buffer(ib);
	};


	let krom_draw_indexed_vertices = $SHBuiltin.extern_c({},
		function _krom_draw_indexed_vertices(start: c_int, count: c_int): void {}
	);

	globalThis.krom_draw_indexed_vertices = function(start: i32, count: i32) {
		krom_draw_indexed_vertices(start, count);
	};

})();
