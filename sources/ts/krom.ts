// @ts-nocheck
///if 0

declare type i8 = number;
declare type i16 = number;
declare type i32 = number;
declare type i64 = number;
declare type u8 = number;
declare type u16 = number;
declare type u32 = number;
declare type u64 = number;
declare type f32 = number;
declare type f64 = number;
declare type bool = boolean;
declare type any_ptr = any;
declare type u8_ptr = any;
declare type u32_ptr = any;
declare let __ID__: string; // file:line - ts preprocessor

declare type map_t<K, V> = any;
declare type buffer_t = u8_array_t;
declare type f32_array_t = any;
declare type u32_array_t = any;
declare type i32_array_t = any;
declare type u16_array_t = any;
declare type i16_array_t = any;
declare type u8_array_t = any;
declare type i8_array_t = any;

declare function DEREFERENCE(a: any): any;
declare function map_create<K, V>(): map_t<K, V>;
declare function buffer_create(length: i32): buffer_t;
declare function f32_array_create(length: i32): f32_array_t;
declare function f32_array_create_from_buffer(b: buffer_t): f32_array_t;
declare function f32_array_create_from_array(a: f32[]): f32_array_t;
declare function f32_array_create_x(x: f32): void;
declare function f32_array_create_xy(x: f32, y: f32): void;
declare function f32_array_create_xyz(x: f32, y: f32, z: f32): void;
declare function f32_array_create_xyzw(x: f32, y: f32, z: f32, w: f32): void;
declare function f32_array_create_xyzwv(x: f32, y: f32, z: f32, w: f32, v: f32): void;
declare function u32_array_create(length: i32): u32_array_t;
declare function u32_array_create_from_array(a: u32[]): u32_array_t;
declare function i32_array_create(length: i32): i32_array_t;
declare function i32_array_create_from_array(a: i32[]): i32_array_t;
declare function u16_array_create(length: i32): u16_array_t;
declare function i16_array_create(length: i32): i16_array_t;
declare function i16_array_create_from_array(a: i16[]): void;
declare function u8_array_create(length: i32): u8_array_t;
declare function u8_array_create_from_array(a: u8[]): u8_array_t;
declare function u8_array_create_from_string(s: string): u8_array_t;
declare function u8_array_to_string(a: u8_array_t): string;
declare function i8_array_create(length: i32): i8_array_t;

declare function math_floor(x: f32): f32;
declare function math_cos(x: f32): f32;
declare function math_sin(x: f32): f32;
declare function math_tan(x: f32): f32;
declare function math_sqrt(x: f32): f32;
declare function math_abs(x: f32): f32;
declare function math_random(): f32;
declare function math_atan2(y: f32, x: f32): f32;
declare function math_asin(x: f32): f32;
declare function math_pi(): f32;
declare function math_pow(x: f32, y: f32): f32;
declare function math_round(x: f32): f32;
declare function math_ceil(x: f32): f32;
declare function math_min(x: f32, y: f32): f32;
declare function math_max(x: f32, y: f32): f32;
declare function math_log(x: f32): f32;
declare function math_log2(x: f32): f32;
declare function math_atan(x: f32): f32;
declare function math_acos(x: f32): f32;
declare function math_exp(x: f32): f32;
declare function math_fmod(x: f32, y: f32): f32;

declare function map_get<K, V>(m: map_t<K, V>, k: any): any;
declare function map_set<K, V>(m: map_t<K, V>, k: any, v: any): void;
declare function map_delete<K, V>(m: map_t<K, V>, k: any): void;
declare function map_keys(m: map_t<any, any>): any[];
declare function array_sort(ar: any[], fn: (a: any_ptr, b: any_ptr)=>i32): void;
declare function array_push(ar: any[], e: any): void;
declare function array_pop(ar: any[]): any;
declare function array_shift(ar: any[]): any;
declare function array_splice(ar: any[], start: i32, delete_count: i32): void;
declare function array_slice(a: any[], begin: i32, end: i32): any[];
declare function array_insert(ar: any[], at: i32, e: any): void;
declare function array_concat(a: any[], b: any[]): any[];
declare function array_index_of(a: any[], search: any): i32;
declare function array_reverse(a: any[]): void;
declare function string_index_of(s: string, search: string): i32;
declare function string_index_of_pos(s: string, search: string, pos: i32): i32;
declare function string_last_index_of(s: string, search: string): i32;
declare function string_split(s: string, sep: string): string[];
declare function string_array_join(a: string[], sep: string): string;
declare function string_replace_all(s: string, search: string, replace: string): string;
declare function substring(s: string, start: i32, end: i32): string;
declare function string_from_char_code(c: i32): string;
declare function char_code_at(s: string, i: i32): i32;
declare function char_at(s: string, i: i32): string;
declare function starts_with(s: string, start: string): bool;
declare function ends_with(s: string, end: string): bool;
declare function to_lower_case(s: string): string;
declare function to_upper_case(s: string): string;
declare function buffer_slice(a: buffer_t, begin: i32, end: i32): buffer_t;
declare function buffer_get_u8(b: buffer_t, p: i32): u8;
declare function buffer_get_i8(b: buffer_t, p: i32): i8;
declare function buffer_get_u16(b: buffer_t, p: i32): u16;
declare function buffer_get_i16(b: buffer_t, p: i32): i16;
declare function buffer_get_u32(b: buffer_t, p: i32): u32;
declare function buffer_get_i32(b: buffer_t, p: i32): i32;
declare function buffer_get_f32(b: buffer_t, p: i32): f32;
declare function buffer_get_i64(b: buffer_t, p: i32): i32;
declare function buffer_set_u8(b: buffer_t, p: i32, n: u8): void;
declare function buffer_set_i8(b: buffer_t, p: i32, n: i8): void;
declare function buffer_set_u16(b: buffer_t, p: i32, n: u16): void;
declare function buffer_set_i16(b: buffer_t, p: i32, n: i16): void;
declare function buffer_set_u32(b: buffer_t, p: i32, n: u32): void;
declare function buffer_set_i32(b: buffer_t, p: i32, n: i32): void;
declare function buffer_set_f32(b: buffer_t, p: i32, n: f32): void;
declare function parse_int(s: string): i32;
declare function parse_int_hex(s: string): i32;
declare function parse_float(s: string): i32;
declare function i32_to_string(i: i32): string;
declare function i32_to_string_hex(i: i32): string;
declare function json_parse(s: string): any;

declare function json_parse_to_map(s: string): map_t<string, string>;
declare function json_encode_begin(): void;
declare function json_encode_string(k: string, v: string): void;
declare function json_encode_string_array(k: string, v: string[]): void;
declare function json_encode_i32(k: string, v: i32): void;
declare function json_encode_i32_array(k: string, v: i32[]): void;
declare function json_encode_f32(k: string, v: f32): void;
declare function json_encode_bool(k: string, v: bool): void;
declare function json_encode_end(): string;
declare function json_encode_begin_array(k: string): void;
declare function json_encode_end_array(): void;
declare function json_encode_begin_object(): void;
declare function json_encode_end_object(): void;
declare function json_encode_map(m: map<string, string>): void;
declare function uri_decode(s: string): string;

declare function js_eval(js: string): any;
declare function array_remove(ar: any[], e: any): void;
declare function trim_end(str: string): string;

declare function color_from_floats(r: f32, g: f32, b: f32, a: f32): i32;
declare function color_get_rb(c: i32): u8;
declare function color_get_gb(c: i32): u8;
declare function color_get_bb(c: i32): u8;
declare function color_get_ab(c: i32): u8;
declare function color_set_rb(c: i32, i: u8): i32;
declare function color_set_gb(c: i32, i: u8): i32;
declare function color_set_bb(c: i32, i: u8): i32;
declare function color_set_ab(c: i32, i: u8): i32;

declare function krom_init(title: string, width: i32, height: i32, vsync: bool, window_mode: i32, window_features: i32, x: i32, y: i32, frequency: i32): void;
declare function krom_set_app_name(name: string): void;
declare function krom_log(v: any): void;
declare function krom_g4_clear(flags: i32, color: i32, depth: f32): void;
declare function krom_set_update_callback(callback: ()=>void): void;
declare function krom_set_drop_files_callback(callback: (file: string)=>void): void;
declare function krom_set_cut_copy_paste_callback(on_cut: ()=>string, on_copy: ()=>string, on_paste: (text: string)=>void): void;
declare function krom_set_application_state_callback(on_foreground: ()=>void, on_resume: ()=>void, on_pause: ()=>void, on_background: ()=>void, on_shutdown: ()=>void): void;
declare function krom_set_keyboard_down_callback(callback: (code: i32)=>void): void;
declare function krom_set_keyboard_up_callback(callback: (code: i32)=>void): void;
declare function krom_set_keyboard_press_callback(callback: (char_code: i32)=>void): void;
declare function krom_set_mouse_down_callback(callback: (button: i32, x: i32, y: i32)=>void): void;
declare function krom_set_mouse_up_callback(callback: (button: i32, x: i32, y: i32)=>void): void;
declare function krom_set_mouse_move_callback(callback: (x: i32, y: i32, mx: i32, my: i32)=>void): void;
declare function krom_set_mouse_wheel_callback(callback: (button: i32)=>void): void;
declare function krom_set_touch_down_callback(callback: (index: i32, x: i32, y: i32)=>void): void;
declare function krom_set_touch_up_callback(callback: (index: i32, x: i32, y: i32)=>void): void;
declare function krom_set_touch_move_callback(callback: (index: i32, x: i32, y: i32)=>void): void;
declare function krom_set_pen_down_callback(callback: (x: i32, y: i32, pressure: f32)=>void): void;
declare function krom_set_pen_up_callback(callback: (x: i32, y: i32, pressure: f32)=>void): void;
declare function krom_set_pen_move_callback(callback: (x: i32, y: i32, pressure: f32)=>void): void;
declare function krom_set_gamepad_axis_callback(callback: (gamepad: i32, axis: i32, value: f32)=>void): void;
declare function krom_set_gamepad_button_callback(callback: (gamepad: i32, button: i32, value: f32)=>void): void;
declare function krom_lock_mouse(): void;
declare function krom_unlock_mouse(): void;
declare function krom_can_lock_mouse(): bool;
declare function krom_is_mouse_locked(): bool;
declare function krom_set_mouse_position(x: i32, y: i32): void;
declare function krom_show_mouse(show: bool): void;
declare function krom_show_keyboard(show: bool): void;

declare function krom_g4_create_index_buffer(count: i32): any;
declare function krom_g4_delete_index_buffer(buffer: any): void;
declare function krom_g4_lock_index_buffer(buffer: any): u32_array_t;
declare function krom_g4_unlock_index_buffer(buffer: any): void;
declare function krom_g4_set_index_buffer(buffer: any): void;
declare function krom_g4_create_vertex_buffer(count: i32, structure: kinc_vertex_elem_t[], usage: i32, inst_data_step_rate: i32): any;
declare function krom_g4_delete_vertex_buffer(buffer: any): void;
declare function krom_g4_lock_vertex_buffer(buffer: any): buffer_t;
declare function krom_g4_unlock_vertex_buffer(buffer: any): void;
declare function krom_g4_set_vertex_buffer(buffer: any): void;
declare function krom_g4_set_vertex_buffers(vertex_buffers: vertex_buffer_t[]): void;
declare function krom_g4_draw_indexed_vertices(start: i32, count: i32): void;
declare function krom_g4_draw_indexed_vertices_instanced(inst_count: i32, start: i32, count: i32): void;
declare function krom_g4_create_shader(data: buffer_t, type: i32): any;
declare function krom_g4_create_vertex_shader_from_source(source: string): any;
declare function krom_g4_create_fragment_shader_from_source(source: string): any;
declare function krom_g4_delete_shader(shader: any): void;
declare function krom_g4_create_pipeline(): any;
declare function krom_g4_delete_pipeline(pipeline: any): void;
declare function krom_g4_compile_pipeline(pipeline: any, structure0: any, structure1: any, structure2: any, structure3: any, length: i32, vertex_shader: any, fragment_shader: any, geometry_shader: any, state: any): void;
declare function krom_g4_set_pipeline(pipeline: any): void;
declare function krom_load_image(file: string, readable: bool): any;
declare function krom_unload_image(image: image_t): void;
declare function krom_load_sound(file: string): any;
declare function krom_unload_sound(sound: any): void;
declare function krom_play_sound(sound: any, loop: bool): void;
declare function krom_stop_sound(sound: any): void;
declare function krom_load_blob(file: string): buffer_t;
declare function krom_load_url(url: string): void;
declare function krom_copy_to_clipboard(text: string): void;

declare function krom_g4_get_constant_location(pipeline: any, name: string): any;
declare function krom_g4_get_texture_unit(pipeline: any, name: string): any;
declare function krom_g4_set_texture(stage: any, texture: any): void;
declare function krom_g4_set_render_target(stage: any, render_target: any): void;
declare function krom_g4_set_texture_depth(unit: any, texture: any): void;
declare function krom_g4_set_image_texture(stage: any, texture: any): void;
declare function krom_g4_set_texture_parameters(tex_unit: any, u_addr: i32, v_addr: i32, min_filter: i32, mag_filter: i32, mip_filter: i32): void;
declare function krom_g4_set_texture3d_parameters(tex_unit: any, u_addr: i32, v_addr: i32, w_addr: i32, min_filter: i32, mag_filter: i32, mip_filter: i32): void;
declare function krom_g4_set_bool(location: any, value: bool): void;
declare function krom_g4_set_int(location: any, value: i32): void;
declare function krom_g4_set_float(location: any, value: f32): void;
declare function krom_g4_set_float2(location: any, value1: f32, value2: f32): void;
declare function krom_g4_set_float3(location: any, value1: f32, value2: f32, value3: f32): void;
declare function krom_g4_set_float4(location: any, value1: f32, value2: f32, value3: f32, value4: f32): void;
declare function krom_g4_set_floats(location: any, values: buffer_t): void;
declare function krom_g4_set_matrix4(location: any, matrix: buffer_t): void;
declare function krom_g4_set_matrix3(location: any, matrix: buffer_t): void;

declare function krom_get_time(): f32;
declare function krom_window_width(): i32;
declare function krom_window_height(): i32;
declare function krom_set_window_title(title: string): void;
declare function krom_get_window_mode(): i32;
declare function krom_set_window_mode(mode: i32): void;
declare function krom_resize_window(width: i32, height: i32): void;
declare function krom_move_window(x: i32, y: i32): void;
declare function krom_screen_dpi(): i32;
declare function krom_system_id(): string;
declare function krom_request_shutdown(): void;
declare function krom_display_count(): i32;
declare function krom_display_width(index: i32): i32;
declare function krom_display_height(index: i32): i32;
declare function krom_display_x(index: i32): i32;
declare function krom_display_y(index: i32): i32;
declare function krom_display_frequency(index: i32): i32;
declare function krom_display_is_primary(index: i32): bool;
declare function krom_write_storage(name: string, data: buffer_t): void;
declare function krom_read_storage(name: string): buffer_t;

declare function krom_g4_create_render_target(width: i32, height: i32, format: i32, depth_buffer_bits: i32, stencil_buffer_bits: i32): any;
declare function krom_g4_create_texture(width: i32, height: i32, format: i32): any;
declare function krom_g4_create_texture3d(width: i32, height: i32, depth: i32, format: i32): any;
declare function krom_g4_create_texture_from_bytes(data: buffer_t, width: i32, height: i32, format: i32, readable: bool): any;
declare function krom_g4_create_texture_from_bytes3d(data: buffer_t, width: i32, height: i32, depth: i32, format: i32, readable: bool): any;
declare function krom_g4_create_texture_from_encoded_bytes(data: buffer_t, format: string, readable: bool): any;
declare function krom_g4_get_texture_pixels(texture: any): buffer_t;
declare function krom_g4_get_render_target_pixels(render_target: any, data: buffer_t): void;
declare function krom_g4_lock_texture(texture: any, level: i32): buffer_t;
declare function krom_g4_unlock_texture(texture: any): void;
declare function krom_g4_clear_texture(target: any, x: i32, y: i32, z: i32, width: i32, height: i32, depth: i32, color: i32): void;
declare function krom_g4_generate_texture_mipmaps(texture: any, levels: i32): void;
declare function krom_g4_generate_render_target_mipmaps(render_target: any, levels: i32): void;
declare function krom_g4_set_mipmaps(texture: any, mipmaps: image_t[]): void;
declare function krom_g4_set_depth_from(target: any, source: any): void;
declare function krom_g4_viewport(x: i32, y: i32, width: i32, height: i32): void;
declare function krom_g4_scissor(x: i32, y: i32, width: i32, height: i32): void;
declare function krom_g4_disable_scissor(): void;
declare function krom_g4_render_targets_inverted_y(): bool;
declare function krom_g4_begin(render_target: image_t, additional: image_t[]): void;
declare function krom_g4_end(): void;
declare function krom_g4_swap_buffers(): void;
declare function krom_file_save_bytes(path: string, bytes: buffer_t, length?: i32): void;
declare function krom_sys_command(cmd: string, args?: string[]): i32;
declare function krom_save_path(): string;
declare function krom_get_arg_count(): i32;
declare function krom_get_arg(index: i32): string;
declare function krom_get_files_location(): string;
declare function krom_http_request(url: string, size: i32, callback: (url: string, _: buffer_t)=>void): void;

declare function krom_g2_init(image_vert: buffer_t, image_frag: buffer_t, colored_vert: buffer_t, colored_frag: buffer_t, text_vert: buffer_t, text_frag: buffer_t): void;
declare function krom_g2_begin(): void;
declare function krom_g2_end(): void;
declare function krom_g2_draw_scaled_sub_image(image: image_t, sx: f32, sy: f32, sw: f32, sh: f32, dx: f32, dy: f32, dw: f32, dh: f32): void;
declare function krom_g2_fill_triangle(x0: f32, y0: f32, x1: f32, y1: f32, x2: f32, y2: f32): void;
declare function krom_g2_fill_rect(x: f32, y: f32, width: f32, height: f32): void;
declare function krom_g2_draw_rect(x: f32, y: f32, width: f32, height: f32, strength: f32): void;
declare function krom_g2_draw_line(x0: f32, y0: f32, x1: f32, y1: f32, strength: f32): void;
declare function krom_g2_draw_string(text: string, x: f32, y: f32): void;
declare function krom_g2_set_font(font: any, size: i32): void;
declare function krom_g2_font_init(blob: buffer_t, font_index: i32): any;
declare function krom_g2_font_13(blob: buffer_t): any;
declare function krom_g2_font_set_glyphs(glyphs: i32[]): void;
declare function krom_g2_font_count(font: any): i32;
declare function krom_g2_font_height(font: any, size: i32): i32;
declare function krom_g2_string_width(font: any, size: i32, text: string): i32;
declare function krom_g2_set_bilinear_filter(bilinear: bool): void;
declare function krom_g2_restore_render_target(): void;
declare function krom_g2_set_render_target(render_target: any): void;
declare function krom_g2_set_color(color: i32): void;
declare function krom_g2_set_pipeline(pipeline: any): void;
declare function krom_g2_set_transform(matrix: buffer_t): void;
declare function krom_g2_fill_circle(cx: f32, cy: f32, radius: f32, segments: i32): void;
declare function krom_g2_draw_circle(cx: f32, cy: f32, radius: f32, segments: i32, strength: f32): void;
declare function krom_g2_draw_cubic_bezier(x: f32[], y: f32[], segments: i32, strength: f32): void;

declare function krom_set_save_and_quit_callback(callback: (save: bool)=>void): void;
declare function krom_set_mouse_cursor(id: i32): void;
declare function krom_delay_idle_sleep(): void;
declare function krom_open_dialog(filter_list: string, default_path: string, open_multiple: bool): string[];
declare function krom_save_dialog(filter_list: string, default_path: string): string;
declare function krom_read_directory(path: string, foldersOnly: bool): string;
declare function krom_file_exists(path: string): bool;
declare function krom_delete_file(path: string): void;
declare function krom_inflate(bytes: buffer_t, raw: bool): buffer_t;
declare function krom_deflate(bytes: buffer_t, raw: bool): buffer_t;
declare function krom_write_jpg(path: string, bytes: buffer_t, w: i32, h: i32, format: i32, quality: i32): void; // RGBA, R, RGB1, RRR1, GGG1, BBB1, AAA1
declare function krom_write_png(path: string, bytes: buffer_t, w: i32, h: i32, format: i32): void;
declare function krom_encode_jpg(bytes: buffer_t, w: i32, h: i32, format: i32, quality: i32): buffer_t;
declare function krom_encode_png(bytes: buffer_t, w: i32, h: i32, format: i32): buffer_t;
declare function krom_write_mpeg(): buffer_t;
declare function krom_ml_inference(model: buffer_t, tensors: buffer_t[], input_shape?: i32[][], output_shape?: i32[], use_gpu?: bool): buffer_t;
declare function krom_ml_unload(): void;

declare function krom_raytrace_supported(): bool;
declare function krom_raytrace_init(shader: buffer_t, vb: any, ib: any, scale: f32): void;
declare function krom_raytrace_set_textures(tex0: image_t, tex1: image_t, tex2: image_t, texenv: any, tex_sobol: any, tex_scramble: any, tex_rank: any): void;
declare function krom_raytrace_dispatch_rays(target: any, cb: buffer_t): void;

declare function krom_window_x(): i32;
declare function krom_window_y(): i32;
declare function krom_language(): string;
declare function krom_io_obj_parse(file_bytes: buffer_t, split_code: i32, start_pos: i32, udim: bool): any;

declare function armpack_decode(b: buffer_t): any;

///end
