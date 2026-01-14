#version 330 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 color;
in vec3 normal;
in vec2 texCoord;

uniform vec3 u_cameraPos;
uniform vec3 u_lightPos;
uniform vec4 u_lightColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main()
{	
	vec4 sampledColor = texture(texture_diffuse1, texCoord);
	vec3 baseColor = sampledColor.rgb;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_lightPos - fragPos);

    // caclulate the ambient light on the fragment
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * u_lightColor.rgb;

	// calculate the diffusion of light on the fragment
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * u_lightColor.rgb;
	
	// calculate the specular reflection
	float specularStrength = 0.3;
	vec3 viewDir = normalize(u_cameraPos - fragPos); // calculate the direction of the camera to the fragment
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
	vec3 specular = specularStrength * spec * u_lightColor.rgb;
	
	// calculate the final color of the fragment
	
	FragColor = vec4((ambient + diffuse + specular) * baseColor, 1.0);
	
	//FragColor = vec4((ambient + diffuse + specular) * color * sampledColor.rgb, 1.0);
	//FragColor = vec4(color, 1.0);
} 