#version 400 core

out vec4 FragColor;

in vec2 texCoords;
in vec4 particleColor;
in float particleLife;

uniform sampler2D u_particleTexture;
uniform int u_atlasColumns;
uniform int u_atlasRows;
uniform int u_atlasFrameCount;

void main()
{
	int cols = max(u_atlasColumns, 1);
	int rows = max(u_atlasRows, 1);
	int frameCountInt = max(u_atlasFrameCount, 1);

	float frameCount = float(frameCountInt);
	float life01 = clamp(particleLife, 0.0, 1.0);

	// animate from frame 0 -> last frame as particle dies
	float texIndex = floor((1.0 - life01) * (frameCount - 1.0));
	texIndex = clamp(texIndex, 0.0, frameCount - 1.0);

	float col = mod(texIndex, float(cols));
	float row = floor(texIndex / float(cols));

	vec2 uvScale = 1.0 / vec2(float(cols), float(rows));
	vec2 uvOffset = vec2(col, row) * uvScale;
	vec2 finalUV = texCoords * uvScale + uvOffset;

	vec4 textureColor = texture(u_particleTexture, finalUV);
	FragColor = textureColor * particleColor.a;
}