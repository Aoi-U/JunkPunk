#version 330 core
out vec4 FragColor;

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 fragPos;
in vec3 color;
in vec3 normal;
in vec2 texCoord;

uniform vec3 u_cameraPos;
uniform Light u_light;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

uniform bool hasDiffuseTex;
uniform bool hasSpecularTex;
uniform bool hasNormalTex;
uniform bool hasHeightTex;

void main()
{	
	vec3 color = u_light.ambient * vec3(texture(texture_diffuse1, texCoord));

	// ambient
	vec3 ambient = color;

	// diffuse
	vec3 lightDir = normalize(u_light.position - fragPos);
	vec3 norm = normalize(normal);
	float diff = max(dot(lightDir, norm), 0.0);
	vec3 diffuse = u_light.diffuse * diff * color;

	// specular
	vec3 viewDir = normalize(u_cameraPos - fragPos); // calculate the direction of the camera tothefragment
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);

	vec3 specular = vec3(0.0f);
	if (hasSpecularTex)
	{
		specular = u_light.specular * spec * texture(texture_specular1, texCoord).rgb;
	}
	else
	{
		specular = u_light.specular * spec;
	}

	FragColor = vec4(ambient + diffuse + specular, 1.0);
} 