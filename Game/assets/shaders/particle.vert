#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aPosSize;
layout (location = 2) in vec4 aColor;

out vec2 texCoords;
out vec4 particleColor;

uniform mat4 u_projection;
uniform mat4 u_view;

uniform vec3 u_right;
uniform vec3 u_up;

void main()
{
	float particleSize = aPosSize.w;
	vec3 particleCenterPos = aPosSize.xyz;

	vec3 particleWorldPos = particleCenterPos + (u_right * aPos.x * particleSize) + (u_up * aPos.y * particleSize);

	texCoords = aPos.xy + vec2(0.5);
	particleColor = aColor;

	gl_Position = u_projection * u_view * vec4(particleWorldPos, 1.0);
}