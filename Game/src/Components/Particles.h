#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "../Core/Types.h"

struct Particle
{
	glm::vec3 position;
	glm::vec3 speed;
	unsigned char r, g, b, a;
	float size;
	float angle;
	float weight;
	float life;
	float cameraDistance;

	bool operator<(const Particle& other)
	{
		// sort in reverse order: far particles drawn first
		return this->cameraDistance > other.cameraDistance;
	}
};

struct ParticleEmitter
{
	Entity targetEntity; // entity the particle emitter is attached to (if any)
	int maxParticles = 1000;
	float spawnRate = 1.0f;
	float life = 2.0f;
	glm::vec3 offset = glm::vec3(0.0f); // offset from vehicles rear wheel position

	std::vector<Particle> particles;
	float spawnAccumulator = 0.0f;
	int lastUsedParticle = 0;

	std::vector<float> particlePositionData;
	std::vector<unsigned char> particleColorData;
	int particleCount = 0;

	void Init(int max)
	{
		maxParticles = max;
		particles.resize(maxParticles);
		particlePositionData.resize(maxParticles * 4); // x, y, z, size
		particleColorData.resize(maxParticles * 4);    // r, g, b, a
	}
};
