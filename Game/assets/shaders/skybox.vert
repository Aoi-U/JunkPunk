#version 400 core

layout (location = 0) in vec3 aPos;

out vec3 texCoord;

uniform mat4 u_projView;

void main()
{
	texCoord = aPos;
	vec4 pos = u_projView * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}
