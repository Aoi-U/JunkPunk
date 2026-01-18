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
	//vec4 sampledColor = texture(texture_diffuse1, texCoord);
	//
    //vec3 norm = normalize(normal);
    //vec3 lightDir = normalize(u_lightPos - fragPos);
	//
    //// caclulate the ambient light on the fragment
	//float ambientStrength = 0.5;
	//vec3 ambient = ambientStrength * u_lightColor.rgb;
	//
	//// calculate the diffusion of light on the fragment
	//float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = diff * u_lightColor.rgb;
	//
	//// calculate the specular reflection
	//float specularStrength = 0.5;
	//vec3 viewDir = normalize(u_cameraPos - fragPos); // calculate the direction of the camera to the fragment
	//vec3 reflectDir = reflect(-lightDir, norm);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	//vec3 specular = specularStrength * spec * u_lightColor.rgb;
	
	// calculate the final color of the fragment

	// ambient
	vec3 ambient = u_light.ambient * vec3(texture(texture_diffuse1, texCoord));

	// diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(u_light.position - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = u_light.diffuse * diff * vec3(texture(texture_diffuse1, texCoord));

	// specular
	vec3 viewDir;
	vec3 reflectDir;
	float spec;
	vec3 specular;
	if (hasSpecularTex == false)
	{
		float specularStrength = 0.5;
		viewDir = normalize(u_cameraPos - fragPos); // calculate the direction of the camera tothefragment
		reflectDir = reflect(-lightDir, norm);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		specular = specularStrength * spec * vec3(1.0f);
	}
	else
	{
		viewDir = normalize(u_cameraPos - fragPos);
		reflectDir = reflect(-lightDir, norm);
		spec = pow(max(dot(u_cameraPos, reflectDir), 0.0), 64);
		specular = u_light.specular * spec * texture(texture_specular1, texCoord).rgb;
	}

	vec3 result = ambient + diffuse + specular;
	
	FragColor = vec4(result, 1.0);
	
	//FragColor = vec4((ambient + diffuse + specular) * color * sampledColor.rgb, 1.0);
	//FragColor = vec4(sampledColor.rgb, 1.0);
	
} 