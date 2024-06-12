#version 450

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D myTexture;

void main() {
	fragColor = texture(myTexture, texCoord);
}
