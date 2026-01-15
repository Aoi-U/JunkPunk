#version 330 core
out vec4 FragColor;
  
in vec2 texCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main()
{
	vec4 sampledColor = texture(texture_diffuse1, texCoord);

	if (sampledColor.a < 0.1)
	{
		discard;
	}

	FragColor = sampledColor;

    //FragColor = vec4(color, 1.0);
}