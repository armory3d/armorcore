#version 450

in vec4 pos;
in vec2 tex;

out vec2 texCoord;

uniform mat4 WVP;

void main() {
	texCoord = tex;
	gl_Position = WVP * vec4(pos.xyz, 1.0);
}
