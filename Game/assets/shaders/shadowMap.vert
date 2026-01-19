#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

//uniform mat4 u_lightSpaceMatrix;
uniform mat4 u_model;

void main()
{
    //gl_Position = u_lightSpaceMatrix * u_model * vec4(aPos, 1.0);
    gl_Position = u_model * vec4(aPos, 1.0);
}  