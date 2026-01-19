#version 400 core

out vec4 FragColor;

in vec3 texCoord;

uniform samplerCube u_skybox;

void main()
{
	FragColor = texture(u_skybox, texCoord);
}