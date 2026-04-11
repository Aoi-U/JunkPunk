#version 400 core
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
uniform vec3 u_lightDir;
uniform float u_farPlane;
uniform mat4 u_view;
uniform Light u_light;

uniform sampler2DArray depthMaps;
layout (std140) uniform LightSpaceMatrices
{
	mat4 lightSpaceMatrices[16];
};

uniform float cascadePlaneDistances[16];
uniform int u_cascadeCount;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

uniform bool hasDiffuseTex;
uniform bool hasSpecularTex;
uniform bool hasNormalTex;
uniform bool hasHeightTex;
uniform bool u_useFlatColor;
uniform vec3 u_flatColor;

float ShadowCalculation(vec3 fragPosWorldSpace, vec3 norm)
{
	vec4 fragPosViewSpace = u_view * vec4(fragPosWorldSpace, 1.0);
	float depthValue = abs(fragPosViewSpace.z);

	// choose the shadow layer based on depth
	int layer = -1;
	for (int i = 0; i < u_cascadeCount; ++i)
	{
		if (depthValue < cascadePlaneDistances[i])
		{
			layer = i;
			break;
		}
	}
	if (layer == -1)
	{
		layer = u_cascadeCount;
	}
	vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float currentDepth = projCoords.z;
	if (currentDepth > 1.0)
	{
		return 0.0;
	}

	float ndotl = max(dot(normalize(norm), normalize(u_lightDir)), 0.0); // calculate angle between normal and light direction
	float bias = max(0.0015 * (1.0 - ndotl), 0.0002);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(depthMaps, 0));
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(depthMaps, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
			shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	return shadow;
}

void main()
{
	vec3 sampledColor = hasDiffuseTex ? texture(texture_diffuse1, texCoord).rgb : (u_useFlatColor ? u_flatColor : vec3(1.0));
	vec3 norm = normalize(normal);

	// ambient
	vec3 ambient = u_light.ambient * sampledColor;

	// diffuse
	float diff = max(dot(norm, normalize(u_lightDir)), 0.0);
	vec3 diffuse = diff * u_light.diffuse;

	// specular
	vec3 viewDir = normalize(u_cameraPos - fragPos);
	vec3 halfwayDir = normalize(normalize(u_lightDir) + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.0);
	if (hasSpecularTex)
	{
		specular = u_light.specular * spec * texture(texture_specular1, texCoord).rgb;
	}
	else
	{
		specular = u_light.specular * spec;
	}

	float shadow = ShadowCalculation(fragPos, norm);

	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * sampledColor * color;
	FragColor = vec4(lighting, 1.0);
}
