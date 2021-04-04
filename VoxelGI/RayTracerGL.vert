#version 330 core

in vec3 vertexPos;

out vec2 imgPlanePos;

uniform vec2 scale;

void main()
{
	imgPlanePos = vertexPos.xy * scale;
    gl_Position = vec4(vertexPos.xy, 0.0, 1.0);
}