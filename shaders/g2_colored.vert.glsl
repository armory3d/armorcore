#version 450

in vec3 pos;
in vec4 col;
uniform mat4 P;
out vec4 fragment_color;

void main() {
	gl_Position = P * vec4(pos, 1.0);
	fragment_color = col;
}
