#version 400 core

out vec4 FragColor;

in vec2 texCoords;
in vec4 particleColor;

//uniform sampler2D particleTexture;

void main()
{
	//vec4 sampledColor = texture(particleTexture, texCoords);
	//if (sampledColor.a < 0.1)
	//	discard;

	if (particleColor.a < 0.1)
		discard;

	FragColor = particleColor;
}