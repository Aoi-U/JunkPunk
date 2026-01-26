#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in mat4 instanceMatrix;


uniform mat4 u_lightSpaceMatrix;
uniform mat4 u_model;

uniform bool u_isInstanced;

void main()
{
    mat4 modelMatrix = u_isInstanced ? instanceMatrix : u_model;
    gl_Position = modelMatrix * vec4(aPos, 1.0);
    //gl_Position = u_model * vec4(aPos, 1.0);
}  