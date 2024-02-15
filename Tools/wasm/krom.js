// G2 test

const resizable = 1;
const minimizable = 2;
const maximizable = 4;
krom_init("ArmorCore", 640, 480, true, 0, resizable | minimizable | maximizable, -1, -1, 60);
krom_set_update_callback(render);

let logo = { texture_: krom_load_image("logo.png", false) };
let painter_image_vert = krom_load_blob("painter-image-webgl2.vert");
let painter_image_frag = krom_load_blob("painter-image-webgl2.frag");
let painter_colored_vert = krom_load_blob("painter-colored-webgl2.vert");
let painter_colored_frag = krom_load_blob("painter-colored-webgl2.frag");
let painter_text_vert = krom_load_blob("painter-text-webgl2.vert");
let painter_text_frag = krom_load_blob("painter-text-webgl2.frag");
krom_g2_init(painter_image_vert, painter_image_frag, painter_colored_vert, painter_colored_frag, painter_text_vert, painter_text_frag);

function render() {
	krom_begin(null, null);

	let flags = 0;
	flags |= 1; // Color
	krom_clear(flags, 0xff000000, 1.0, null);

	krom_g2_begin();
	krom_g2_draw_scaled_sub_image(logo, 0, 0, 400, 400, 120, 40, 400, 400);
	krom_g2_end();

	krom_end();
}
