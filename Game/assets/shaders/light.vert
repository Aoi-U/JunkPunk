#version 400 core

layout (location = 0) in vec3 aPos;

uniform mat4 u_model;
uniform mat4 u_projView;

void main()
{
	gl_Position = u_projView * u_model * vec4(aPos, 1.0);
}