
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
function u32_array_create(length: i32): u32_array_t { return new u32_array_t(length); }
function i32_array_create(length: i32): i32_array_t { return new i32_array_t(length); }
function u16_array_create(length: i32): u16_array_t { return new u16_array_t(length); }
function i16_array_create(length: i32): i16_array_t { return new i16_array_t(length); }
function u8_array_create(length: i32): u8_array_t { return new u8_array_t(length); }
function u8_array_create_from_buffer(b: buffer_t): u8_array_t { return new u8_array_t(b); }
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

function map_get<K, V>(m: map_t<K, V>, k: any): any { return m.get(k); }
function map_set<K, V>(m: map_t<K, V>, k: any, v: any) { m.set(k, v); }
function map_delete<K, V>(m: map_t<K, V>, k: any) { m.delete(k); }
function array_sort(ar: any[], fn: (a: any, b: any)=>i32) { ar.sort(fn); }
function array_push(ar: any[], e: any) { ar.push(e); }
function array_splice(ar: any[], start: i32, delete_count: i32) { ar.splice(start, delete_count); }
function array_concat(a: any[], b: any[]): any[] { return a.concat(b); }
function string_index_of(s: string, search: string): i32 { return s.indexOf(search); }
function string_last_index_of(s: string, search: string): i32 { return s.lastIndexOf(search); }
function string_split(s: string, sep: string): string[] { return s.split(sep); }
function string_replace_all(s: string, search: string, replace: string) { s.replaceAll(search, replace); }
function substring(s: string, start: i32, end: i32): string { return s.substring(start, end); };
function string_from_char_code(c: i32): string { return String.fromCharCode(c); }
function char_code_at(s: string, i: i32): i32 { return s.charCodeAt(i); }
function char_at(s: string, i: i32): string { return s.charAt(i); }
function starts_with(s: string, start: string): bool { return s.startsWith(start); }
function ends_with(s: string, end: string): bool { return s.endsWith(end); }
function to_lower_case(s: string): string { return s.toLowerCase(); }
function map_to_array(m: any): any[] { return Array.from(m.values()); }
function array_slice(a: any[], begin: i32, end: i32): any[] { return a.slice(begin, end); }
function buffer_slice(a: ArrayBuffer, begin: i32, end: i32): ArrayBuffer { return a.slice(begin, end); }
function buffer_size(b: ArrayBuffer): i32 { return b.byteLength; }
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
function is_integer(a: any): bool { return Number.isInteger(a); } // armpack
function is_view(a: any): bool { return ArrayBuffer.isView(a); } // armpack
function is_array(a: any): bool { return Array.isArray(a); } // armpack
function any_to_string(a: any): string { return String(a); } // armpack
// Object.keys() // armpack, tween
// .constructor // armpack
// globalThis // arm_shader_embed

function array_remove(ar: any[], e: any) {
    ar.splice(ar.indexOf(e), 1);
}

function trim_end(str: string): string {
    while (str.length > 0 && (str[str.length - 1] == " " || str[str.length - 1] == "\n")) {
        str = str.substring(0, str.length - 1);
    }
    return str;
}

function color_from_floats(r: f32, g: f32, b: f32, a: f32): i32 {
    return (Math.round(a * 255) << 24) | (Math.round(r * 255) << 16) | (Math.round(g * 255) << 8) | Math.round(b * 255);
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
