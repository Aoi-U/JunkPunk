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
in vec4 fragPosLightSpace;

uniform vec3 u_cameraPos;
uniform Light u_light;

uniform sampler2D shadowMap;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

uniform bool hasDiffuseTex;
uniform bool hasSpecularTex;
uniform bool hasNormalTex;
uniform bool hasHeightTex;

#define DEBUG_MODE 0

float ShadowCalculation()
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	float currentDepth = projCoords.z;

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(u_light.position - fragPos);
	
	float cosTheta = clamp(dot(norm, lightDir), 0.0, 1.0);
	float bias = 0.05*tan(acos(cosTheta));
	bias = clamp(bias, 0, 0.001);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	if (projCoords.z > 1.0)
		shadow = 0.0;
	return shadow;
}

void main()
{	
	vec3 color = vec3(texture(texture_diffuse1, texCoord));

	vec3 lightDir = normalize(u_light.position - fragPos);
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(u_cameraPos - fragPos); // calculate the direction of the camera to the fragment

	// ambient
	vec3 ambient = u_light.ambient * color;

	// diffuse
	float diff = max(dot(lightDir, norm), 0.0);
	vec3 diffuse = diff * color;

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);

	vec3 specular = vec3(0.0);
	if (hasSpecularTex)
	{
		specular = u_light.specular * spec * texture(texture_specular1, texCoord).rgb;
	}
	else
	{
		specular = u_light.specular * spec;
	}

	float shadow = ShadowCalculation();

	vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

	FragColor = vec4(lighting, 1.0);
} 