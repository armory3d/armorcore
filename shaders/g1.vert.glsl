#version 450

in vec3 pos;
in vec2 tex;
out vec2 tex_coord;

void main() {
	gl_Position = vec4(pos.x, pos.y, 0.5, 1.0);
	tex_coord = tex;
}
