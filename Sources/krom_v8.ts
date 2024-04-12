
declare type i8 = number;
declare type i16 = number;
declare type i32 = number;
declare type u8 = number;
declare type u16 = number;
declare type u32 = number;
declare type f32 = number;
declare type f64 = number;
declare type bool = boolean;
declare let __ID__: string; // file:line - ts preprocessor

class map_t<K, V> extends Map<K, V> {}
class buffer_t extends ArrayBuffer {}
class buffer_view_t extends DataView {}
class f32_array_t extends Float32Array {}
class u32_array_t extends Uint32Array {}
class i32_array_t extends Int32Array {}
class u16_array_t extends Uint16Array {}
class i16_array_t extends Int16Array {}
class u8_array_t extends Uint8Array {}
class i8_array_t extends Int8Array {}

function map_create<K, V>(): map_t<K, V> { return new map_t(); }
function buffer_create(length: i32): buffer_t { return new buffer_t(length); }
function buffer_view_create(b: buffer_t): buffer_view_t { return new buffer_view_t(b); }
function f32_array_create(length: i32): f32_array_t { return new f32_array_t(length); }
function f32_array_create_from_buffer(b: buffer_t): f32_array_t { return new f32_array_t(b); }
function f32_array_create_from_array(a: f32[]): f32_array_t { return new f32_array_t(a); }
function f32_array_create_x(x: f32) { return new f32_array_t([x]); }
function f32_array_create_xy(x: f32, y: f32) { return new f32_array_t([x, y]); }
function f32_array_create_xyz(x: f32, y: f32, z: f32) { return new f32_array_t([x, y, z]); }
function f32_array_create_xyzw(x: f32, y: f32, z: f32, w: f32) { return new f32_array_t([x, y, z, w]); }
function f32_array_create_xyzwv(x: f32, y: f32, z: f32, w: f32, v: f32) { return new f32_array_t([x, y, z, w, v]); }
function u32_array_create(length: i32): u32_array_t { return new u32_array_t(length); }
function u32_array_create_from_array(a: u32[]): u32_array_t { return new u32_array_t(a); }
function i32_array_create(length: i32): i32_array_t { return new i32_array_t(length); }
function i32_array_create_from_array(a: i32[]): i32_array_t { return new i32_array_t(a); }
function u16_array_create(length: i32): u16_array_t { return new u16_array_t(length); }
function i16_array_create(length: i32): i16_array_t { return new i16_array_t(length); }
function i16_array_create_from_array(a: i16[]) { return new i16_array_t(a); }
function u8_array_create(length: i32): u8_array_t { return new u8_array_t(length); }
function u8_array_create_from_buffer(b: buffer_t): u8_array_t { return new u8_array_t(b); }
function u8_array_create_from_array(a: u8[]): u8_array_t { return new u8_array_t(a); }
function i8_array_create(length: i32): i8_array_t { return new i8_array_t(length); }

function math_floor(x: f32): f32 { return Math.floor(x); }
function math_cos(x: f32): f32 { return Math.cos(x); }
function math_sin(x: f32): f32 { return Math.sin(x); }
function math_tan(x: f32): f32 { return Math.tan(x); }
function math_sqrt(x: f32): f32 { return Math.sqrt(x); }
function math_abs(x: f32): f32 { return Math.abs(x); }
function math_random(): f32 { return Math.random(); }
function math_atan2(y: f32, x: f32): f32 { return Math.atan2(y, x); }
function math_asin(x: f32): f32 { return Math.asin(x); }
function math_pi(): f32 { return Math.PI; }
function math_pow(x: f32, y: f32): f32 { return Math.pow(x, y); }
function math_round(x: f32): f32 { return Math.round(x); }
function math_ceil(x: f32): f32 { return Math.ceil(x); }
function math_min(x: f32, y: f32): f32 { return Math.min(x, y); }
function math_max(x: f32, y: f32): f32 { return Math.max(x, y); }
function math_log(x: f32): f32 { return Math.log(x); }
function math_log2(x: f32): f32 { return Math.log2(x); }
function math_atan(x: f32): f32 { return Math.atan(x); }
function math_acos(x: f32): f32 { return Math.acos(x); }
function math_exp(x: f32): f32 { return Math.exp(x); }

function map_get<K, V>(m: map_t<K, V>, k: any): any { return m.get(k); }
function map_set<K, V>(m: map_t<K, V>, k: any, v: any) { m.set(k, v); }
function map_delete<K, V>(m: map_t<K, V>, k: any) { m.delete(k); }
function map_keys(m: map_t<any, any>): any[] { return Array.from(m.keys()); }
function array_sort(ar: any[], fn: (a: any, b: any)=>i32) { ar.sort(fn); }
function array_push(ar: any[], e: any) { ar.push(e); }
function array_pop(ar: any[]): any { return ar.pop(); }
function array_splice(ar: any[], start: i32, delete_count: i32) { ar.splice(start, delete_count); }
function array_slice(a: any[], begin: i32, end: i32): any[] { return a.slice(begin, end); }
function array_insert(ar: any[], at: i32, e: any) { ar.splice(at, 0, e); }
function array_concat(a: any[], b: any[]): any[] { return a.concat(b); }
function array_index_of(a: any[], search: any): i32 { return a.indexOf(search); }
function string_index_of(s: string, search: string): i32 { return s.indexOf(search); }
function string_index_of_pos(s: string, search: string, pos: i32) { return s.indexOf(search, pos); }
function string_last_index_of(s: string, search: string): i32 { return s.lastIndexOf(search); }
function string_split(s: string, sep: string): string[] { return s.split(sep); }
function string_replace_all(s: string, search: string, replace: string): string { return (s as any).replaceAll(search, replace); }
function substring(s: string, start: i32, end: i32): string { return s.substring(start, end); };
function string_from_char_code(c: i32): string { return String.fromCharCode(c); }
function char_code_at(s: string, i: i32): i32 { return s.charCodeAt(i); }
function char_at(s: string, i: i32): string { return s.charAt(i); }
function starts_with(s: string, start: string): bool { return s.startsWith(start); }
function ends_with(s: string, end: string): bool { return s.endsWith(end); }
function to_lower_case(s: string): string { return s.toLowerCase(); }
function buffer_slice(a: buffer_t, begin: i32, end: i32): buffer_t { return a.slice(begin, end); }
function buffer_size(b: buffer_t): i32 { return b.byteLength; }
function buffer_view_size(v: buffer_view_t): i32 { return v.byteLength; }
function buffer_view_get_u8(v: buffer_view_t, p: i32): u8 { return v.getUint8(p); }
function buffer_view_get_i8(v: buffer_view_t, p: i32): i8 { return v.getInt8(p); }
function buffer_view_get_u16(v: buffer_view_t, p: i32): u16 { return v.getUint16(p, true); }
function buffer_view_get_i16(v: buffer_view_t, p: i32): i16 { return v.getInt16(p, true); }
function buffer_view_get_u32(v: buffer_view_t, p: i32): u32 { return v.getUint32(p, true); }
function buffer_view_get_i32(v: buffer_view_t, p: i32): i32 { return v.getInt32(p, true); }
function buffer_view_get_f32(v: buffer_view_t, p: i32): f32 { return v.getFloat32(p, true); }
function buffer_view_set_u8(v: buffer_view_t, p: i32, n: u8) { v.setUint8(p, n); }
function buffer_view_set_i8(v: buffer_view_t, p: i32, n: i8) { v.setInt8(p, n); }
function buffer_view_set_u16(v: buffer_view_t, p: i32, n: u16) { v.setUint16(p, n, true); }
function buffer_view_set_i16(v: buffer_view_t, p: i32, n: i16) { v.setInt16(p, n, true); }
function buffer_view_set_u32(v: buffer_view_t, p: i32, n: u32) { v.setUint32(p, n, true); }
function buffer_view_set_i32(v: buffer_view_t, p: i32, n: i32) { v.setInt32(p, n, true); }
function buffer_view_set_f32(v: buffer_view_t, p: i32, n: f32) { v.setFloat32(p, n, true); }
function parse_int(s: string): i32 { return parseInt(s); }
function parse_int_hex(s: string): i32 { return parseInt(s, 16); }
function parse_float(s: string): i32 { return parseFloat(s); }
function is_integer(a: any): bool { return Number.isInteger(a); } // armpack
function is_view(a: any): bool { return buffer_t.isView(a); } // armpack
function is_array(a: any): bool { return Array.isArray(a); } // armpack
function any_to_string(a: any): string { return String(a); } // armpack
function i32_to_string(i: i32): string { return i.toString(); }
function i32_to_string_hex(i: i32): string { return i.toString(16); }
// Object.keys() // armpack, tween
// .constructor // armpack
// globalThis // arm_shader_embed
function json_parse(s: string): any { return JSON.parse(s); }
// @ts-ignore
function json_parse_to_map(s: string): map_t<string, string> { return new Map(Object.entries(json_parse(s))); }
function json_stringify(a: any): string { return JSON.stringify(a); }
function uri_decode(s: string): string { return decodeURIComponent(s); }

function js_eval(js: string, context: string = ""): any {
    let result: any;
    try {
        // (1, eval)(js); // Global scope
        // globalThis.eval
        result = eval(js); // Local scope
    }
    catch(e: any) {
        krom_log(context);
        krom_log(e);
    }
    return result;
}

function array_remove(ar: any[], e: any) {
    array_splice(ar, array_index_of(ar, e), 1);
}

function trim_end(str: string): string {
    while (str.length > 0 && (str[str.length - 1] == " " || str[str.length - 1] == "\n")) {
        str = substring(str, 0, str.length - 1);
    }
    return str;
}

function color_from_floats(r: f32, g: f32, b: f32, a: f32): i32 {
    return (math_round(a * 255) << 24) | (math_round(r * 255) << 16) | (math_round(g * 255) << 8) | math_round(b * 255);
}

function color_get_rb(c: i32): u8 {
    return (c & 0x00ff0000) >>> 16;
}

function color_get_gb(c: i32): u8 {
    return (c & 0x0000ff00) >>> 8;
}

function color_get_bb(c: i32): u8 {
    return c & 0x000000ff;
}

function color_get_ab(c: i32): u8 {
    return c & 0x000000ff;
}

function color_set_rb(c: i32, i: u8): i32 {
    return (color_get_ab(c) << 24) | (i << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

function color_set_gb(c: i32, i: u8): i32 {
    return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (i << 8) | color_get_bb(c);
}

function color_set_bb(c: i32, i: u8): i32 {
    return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | i;
}

function color_set_ab(c: i32, i: u8): i32 {
    return (i << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

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

declare function krom_zui_init(ops: any): any;
declare function krom_zui_get_scale(ui: any): f32;
declare function krom_zui_set_scale(ui: any, factor: f32): void;
declare function krom_zui_set_font(ui: any, font: any): void;
declare function krom_zui_begin(ui: any): void;
declare function krom_zui_end(last: bool): void;
declare function krom_zui_begin_region(ui: any, x: i32, y: i32, w: i32): void;
declare function krom_zui_end_region(last: bool): void;
declare function krom_zui_begin_sticky(): void;
declare function krom_zui_end_sticky(): void;
declare function krom_zui_end_input(): void;
declare function krom_zui_end_window(bind_global_g: bool): void;
declare function krom_zui_end_element(element_size: f32): void;
declare function krom_zui_start_text_edit(handle: any, align: i32): void;
declare function krom_zui_input_in_rect(x: f32, y: f32, w: f32, h: f32): bool;
declare function krom_zui_window(handle: any, x: i32, y: i32, w: i32, h: i32, drag: bool): bool;
declare function krom_zui_button(text: string, align: i32, label: string): bool;
declare function krom_zui_check(handle: any, text: string, label: string): bool;
declare function krom_zui_radio(handle: any, position: i32, text: string, label: string): bool;
declare function krom_zui_combo(handle: any, texts: string[], label: string, show_label: bool, align: i32, search_bar: bool): i32;
declare function krom_zui_slider(handle: any, text: string, from: f32, to: f32, filled: bool, precision: f32, display_value: bool, align: i32, text_edit: bool): f32;
declare function krom_zui_image(image: image_t, tint: i32, h: i32, sx: i32, sy: i32, sw: i32, sh: i32): i32;
declare function krom_zui_text(text: string, align: i32, bg: i32): i32;
declare function krom_zui_text_input(handle: any, label: string, align: i32, editable: bool, live_update: bool): string;
declare function krom_zui_tab(handle: any, text: string, vertical: bool, color: i32): bool;
declare function krom_zui_panel(handle: any, text: string, isTree: bool, filled: bool, pack: bool): bool;
declare function krom_zui_handle(ops: any): any;
declare function krom_zui_separator(h: i32, fill: bool): void;
declare function krom_zui_tooltip(text: string): void;
declare function krom_zui_tooltip_image(image: image_t, max_width: i32): void;
declare function krom_zui_row(ratios: f32[]): void;
declare function krom_zui_fill(x: f32, y: f32, w: f32, h: f32, color: i32): void;
declare function krom_zui_rect(x: f32, y: f32, w: f32, h: f32, color: i32, strength: f32): void;
declare function krom_zui_draw_rect(fill: bool, x: f32, y: f32, w: f32, h: f32, strength: f32): void;
declare function krom_zui_draw_string(text: string, x_offset: f32, y_offset: f32, align: i32, truncation: bool): void;
declare function krom_zui_get_hovered_tab_name(): string;
declare function krom_zui_set_hovered_tab_name(name: string): void;
declare function krom_zui_begin_menu(): void;
declare function krom_zui_end_menu(): void;
declare function krom_zui_menu_button(text: string): bool;
declare function krom_zui_float_input(handle: any, label: string, align: i32, precision: f32): f32;
declare function krom_zui_inline_radio(handle: any, texts: string[], align: i32): i32;
declare function krom_zui_color_wheel(handle: any, alpha: bool, w: f32, h: f32, color_preview: bool, picker: ()=>void): i32;
declare function krom_zui_text_area(handle: any, align: i32, editable: bool, label: string, word_wrap: bool): string;
declare function krom_zui_text_area_coloring(packed: buffer_t): void;
declare function krom_zui_nodes_init(): any;
declare function krom_zui_node_canvas(nodes: any, packed: buffer_t): buffer_t;
declare function krom_zui_nodes_rgba_popup(handle: any, val: buffer_t, x: i32, y: i32): void;
declare function krom_zui_nodes_scale(): f32;
declare function krom_zui_nodes_pan_x(): f32;
declare function krom_zui_nodes_pan_y(): f32;
declare function krom_zui_get(ui: any, name: string): any;
declare function krom_zui_set(ui: any, name: string, val: any): void;
declare function krom_zui_handle_get(handle: any, name: string): any;
declare function krom_zui_handle_set(handle: any, name: string, val: any): void;
declare function krom_zui_handle_ptr(handle: any): i32;
declare function krom_zui_theme_init(): any;
declare function krom_zui_theme_get(theme: any, name: string): any;
declare function krom_zui_theme_set(theme: any, name: string, val: any): void;
declare function krom_zui_nodes_get(nodes: any, name: string): any;
declare function krom_zui_nodes_set(nodes: any, name: string, val: any): void;
declare function krom_zui_set_on_border_hover(f: (handle: any, side: i32)=>void): void;
declare function krom_zui_set_on_text_hover(f: ()=>void): void;
declare function krom_zui_set_on_deselect_text(f: ()=>void): void;
declare function krom_zui_set_on_tab_drop(f: (to_handle: any, to_position: i32, from_handle: any, from_position: i32)=>void): void;
declare function krom_zui_nodes_set_enum_texts(f: (node_type: string)=>string[]): void;
declare function krom_zui_nodes_set_on_custom_button(f: (node_id: i32, button_name: string)=>void): void;
declare function krom_zui_nodes_set_on_canvas_control(f: ()=>any): void;
declare function krom_zui_nodes_set_on_canvas_released(f: ()=>void): void;
declare function krom_zui_nodes_set_on_socket_released(f: (socket_id: i32)=>void): void;
declare function krom_zui_nodes_set_on_link_drag(f: (link_drag_id: i32, is_new_link: bool)=>void): void;
