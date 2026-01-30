#version 400 core

out vec4 FragColor;

in vec2 texCoords;
in vec4 particleColor;
in float particleLife;

uniform sampler2D u_particleTexture;

void main()
{
	float texIndex = floor(particleLife * 30);
	texIndex = clamp(texIndex, 0.0, 29.0);

	float col = mod(texIndex, 6.0);
	float row = floor(texIndex / 6.0);

	vec2 uvScale = 1.0 / vec2(6.0, 5.0);
	vec2 uvOffset = vec2(col, row) * uvScale;
	vec2 finalUV = texCoords * uvScale + uvOffset;

	vec4 textureColor = texture(u_particleTexture, finalUV);
	FragColor = textureColor * particleColor.a;

	//if (particleColor.a < 0.1)
	//	discard;

	//FragColor = particleColor;
}