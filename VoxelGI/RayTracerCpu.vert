#version 330 core

in vec3 vertexPos;

out vec2 uv;

uniform vec2 offset;
uniform vec2 scale;

void main()
{
	uv = (vec2(vertexPos.x, -vertexPos.y) + vec2(1.0)) * 0.5;
    gl_Position = vec4(vertexPos.xy * scale + offset, 0.0, 1.0);
}