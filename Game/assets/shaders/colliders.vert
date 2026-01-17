#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec4 color;

uniform mat4 u_projView;

void main()
{
    gl_Position = u_projView * vec4(aPos, 1.0);
    color = vec4(aColor, 1.0); // Green color for debug lines
}
