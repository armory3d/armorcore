#version 450

uniform sampler2D texy;
in vec2 tex_coord;
out vec4 frag_color;

void main() {
	frag_color = texture(texy, tex_coord);
}
