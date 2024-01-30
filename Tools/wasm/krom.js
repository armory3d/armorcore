// G2 test

const resizable = 1;
const minimizable = 2;
const maximizable = 4;
Krom.init("ArmorCore", 640, 480, true, 0, resizable | minimizable | maximizable, -1, -1, 60);
Krom.setCallback(renderCallback);

let logo = { texture_: Krom.loadImage("logo.png", false) };
let painter_image_vert = Krom.loadBlob("painter-image-webgl2.vert");
let painter_image_frag = Krom.loadBlob("painter-image-webgl2.frag");
let painter_colored_vert = Krom.loadBlob("painter-colored-webgl2.vert");
let painter_colored_frag = Krom.loadBlob("painter-colored-webgl2.frag");
let painter_text_vert = Krom.loadBlob("painter-text-webgl2.vert");
let painter_text_frag = Krom.loadBlob("painter-text-webgl2.frag");
Krom.g2_init(painter_image_vert, painter_image_frag, painter_colored_vert, painter_colored_frag, painter_text_vert, painter_text_frag);

function renderCallback() {
	Krom.begin(null, null);

	let flags = 0;
	flags |= 1; // Color
	Krom.clear(flags, 0xff000000, 1.0, null);

	Krom.g2_begin();
	Krom.g2_draw_scaled_sub_image(logo, 0, 0, 400, 400, 120, 40, 400, 400);
	Krom.g2_end();

	Krom.end();
}
