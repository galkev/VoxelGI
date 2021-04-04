#version 330 core

in vec3 vertexPos;

uniform mat4 modelView;
uniform mat4 projection;

void main()
{
    gl_Position = projection * modelView * vec4(vertexPos, 1.0);
}