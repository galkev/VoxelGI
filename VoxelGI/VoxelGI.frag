#version 330 core

out vec4 fragColor;

in vec4 vertColor;

uniform mat4 modelview;

void main()
{
	//fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	fragColor = vertColor;
}