#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 fragPos;
out vec3 color;
out vec3 normal;
out vec2 texCoord;
out vec4 fragPosLightSpace;

uniform mat4 u_model;
uniform mat4 u_projView;
uniform mat4 u_lightSpaceMatrix;

void main()
{
    fragPos = vec3(u_model * vec4(aPos, 1.0));
    color = aColor;
    normal = mat3(transpose(inverse(u_model))) * aNormal; // transform normal to world space
    texCoord = aTexCoord;
    fragPosLightSpace = u_lightSpaceMatrix * vec4(fragPos, 1.0);
    
    gl_Position = u_projView * u_model * vec4(aPos, 1.0);
}