#version 330 core

in vec3 vertexPos;
in vec3 vertexNormal;

out vec4 vertColor;

uniform mat4 modelView;
uniform mat4 projection;

void main()
{
    gl_Position = projection * modelView * vec4(vertexPos, 1.0);
	//gl_Position = vec4(vertexPos, 1.0);

	//vertColor = vec4((vertexNormal + vec3(1.0)) * 0.5, 1.0);
	vertColor = vec4(vertexNormal, 1.0);
	//vertColor = vec4(vec3(vertexPos.z + 0.5), 1.0);
}