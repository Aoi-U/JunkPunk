#version 400 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * vec4(aPos, 0.0, 1.0);
	texCoord = aTexCoord;
}