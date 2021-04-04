#version 330 core

in vec2 vertexPos;

out vec2 planePoint;

void main()
{
	planePoint = vertexPos;
    gl_Position = vec4(vertexPos, 0.0, 1.0);
}