#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "../Core/Types.h"

struct Particle
{
	glm::vec3 position;
	glm::vec3 speed;
	unsigned char r, g, b, a;
	float size;
	float angle;
	float weight;
	float life = -1.0f;
	float cameraDistance = -1.0f;

	bool operator<(const Particle& other) const
	{
		// sort in reverse order: far particles drawn first
		return this->cameraDistance > other.cameraDistance;
	}
};

struct ParticleEmitter
{
	Entity targetEntity; // entity the particle emitter is attached to (if any)
	int maxParticles = 2000;
	float spawnRate = 1000.0f;
	float life = 1.0f;
	glm::vec3 offset = glm::vec3(0.0f); // offset from vehicles rear wheel position

	std::vector<Particle> particles;
	int lastUsedParticle = 0;

	std::vector<GLfloat> particlePositionData;
	std::vector<GLubyte> particleColorData;
	int particleCount = 0;

	void Init(int max)
	{
		maxParticles = max;
		particles.resize(maxParticles);
		particlePositionData.resize(maxParticles * 4); // x, y, z, size
		particleColorData.resize(maxParticles * 4);    // r, g, b, a
	}
};
