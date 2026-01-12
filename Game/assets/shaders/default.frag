#version 330 core
out vec4 FragColor;

in vec3 color;
in vec3 normal;
in vec3 texCord;

uniform vec4 u_lightColor;

void main()
{
    FragColor = vec4(color, 1.0) * u_lightColor;
} 