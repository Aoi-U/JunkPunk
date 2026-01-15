#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in mat4 instanceMatrix;

out vec2 texCoord;

uniform mat4 u_projView;


void main()
{
    texCoord = aTexCoord;

    gl_Position = u_projView * instanceMatrix * vec4(aPos, 1.0);
}