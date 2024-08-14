#version 450

uniform sampler2D stex;
in vec2 tex_coord;
in vec4 fragment_color;
out vec4 frag_color;

void main() {
	frag_color = vec4(fragment_color.rgb, texture(stex, tex_coord).r * fragment_color.a);
}
