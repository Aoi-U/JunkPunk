#version 400 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform int u_useTexture;

void main()
{
	if (u_useTexture == 1)
	{
		FragColor = texture(u_texture, texCoord) * u_color;
	}
	else
	{
		FragColor = u_color;
	}
}