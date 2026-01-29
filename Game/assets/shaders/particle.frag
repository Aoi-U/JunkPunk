#version 400 core

out vec4 FragColor;

in vec2 texCoords;
in vec4 particleColor;

void main()
{
	if (particleColor.a < 0.1)
		discard;

	FragColor = particleColor;
}