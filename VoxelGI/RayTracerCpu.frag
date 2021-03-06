#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D renderTex;

void main()
{
	fragColor = vec4(texture(renderTex, uv).rgb, 1.0);
}