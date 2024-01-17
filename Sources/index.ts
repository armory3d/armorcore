
let array_remove = (ar: any[], e: any) => {
    ar.splice(ar.indexOf(e), 1);
}

let trim_end = (str: string): string => {
    while (str.length > 0 && str[str.length - 1] == " ") str = str.substring(0, str.length - 1);
    return str;
}

let color_from_floats = (r: f32, g: f32, b: f32, a: f32 = 1): i32 => {
    return (Math.round(a * 255) << 24) | (Math.round(r * 255) << 16) | (Math.round(g * 255) << 8) | Math.round(b * 255);
}

let color_get_rb = (c: i32): u8 => {
    return (c & 0x00ff0000) >>> 16;
}

let color_get_gb = (c: i32): u8 => {
    return (c & 0x0000ff00) >>> 8;
}

let color_get_bb = (c: i32): u8 => {
    return c & 0x000000ff;
}

let color_get_ab = (c: i32): u8 => {
    return c & 0x000000ff;
}

let color_set_rb = (c: i32, i: u8): i32 => {
    return (color_get_ab(c) << 24) | (i << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

let color_set_gb = (c: i32, i: u8): i32 => {
    return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (i << 8) | color_get_bb(c);
}

let color_set_bb = (c: i32, i: u8): i32 => {
    return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | i;
}

let color_set_ab = (c: i32, i: u8): i32 => {
    return (i << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}
