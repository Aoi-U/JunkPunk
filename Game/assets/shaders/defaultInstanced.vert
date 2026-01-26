#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in mat4 instanceMatrix;

out vec3 fragPos;
out vec3 color;
out vec3 normal;
out vec2 texCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform bool u_isInstanced;

void main()
{
    mat4 modelMatrix = u_isInstanced ? instanceMatrix : u_model;

    fragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    color = aColor;
    normal = mat3(transpose(inverse(modelMatrix))) * aNormal; // transform normal to world space
    texCoord = aTexCoord;

    gl_Position = u_projection * u_view * modelMatrix * vec4(aPos, 1.0);
}