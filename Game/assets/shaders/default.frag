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

float ShadowCalculation(vec3 lightDir, vec3 norm)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	float currentDepth = projCoords.z;
	
	float diffuseFactor = dot(norm, lightDir);
	float bias = mix(0.001, 0.0, diffuseFactor);

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

	if (closestDepth + bias < currentDepth)
		return shadow;
	return 0.0;
}

#define DEBUG 0
void main()
{	
	#if DEBUG == 1 
		// Calculate projected coordinates for frustum visualization
		vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
	
		// Check if inside the light frustum [0,1] range for XY and depth
		bool insideFrustumXY = projCoords.x >= 0.0 && projCoords.x <= 1.0 && 
							   projCoords.y >= 0.0 && projCoords.y <= 1.0;
		bool insideFrustumZ = projCoords.z >= 0.0 && projCoords.z <= 1.0;
	
		if (insideFrustumXY && insideFrustumZ)
		{
			// green: Fully inside the light frustum
			FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
		}
		else if (insideFrustumXY && !insideFrustumZ)
		{
			// yellow: Inside XY bounds but outside depth range
			FragColor = vec4(1.0, 1.0, 0.0, 1.0);
			return;
		}
		else
		{
			// red: Outside the light frustum XY bounds
			FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
		}
	#endif

	vec3 sampledColor = vec3(texture(texture_diffuse1, texCoord));

	vec3 lightDir = normalize(u_light.position - fragPos);
	vec3 norm = normalize(normal);
	if (dot(norm, lightDir) < 0.0)
		norm = -norm;

	vec3 viewDir = normalize(u_cameraPos - fragPos); // calculate the direction of the camera to the fragment

	// ambient
	vec3 ambient = u_light.ambient;

	// diffuse
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * u_light.diffuse;

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
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

	float shadow = ShadowCalculation(lightDir, norm);

	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * sampledColor * color;
	FragColor = vec4(lighting, 1.0);
} 