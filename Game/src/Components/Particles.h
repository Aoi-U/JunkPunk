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
	Entity targetEntity = MAX_ENTITIES; // entity the particle emitter is attached to (if any)
	int maxParticles = 10000;
	float spawnRate = 5000.0f;
	float life = 1.0f;
	glm::vec3 offset = glm::vec3(0.0f); // offset from vehicles rear wheel position

	std::vector<Particle> particles;
	int lastUsedParticle = 0;

	std::vector<GLfloat> particlePositionData;
	std::vector<GLubyte> particleColorData;
	std::vector<GLfloat> particleLifeData;
	int particleCount = 0;

	int atlasColumns = 6;
	int atlasRows = 5;
	int atlasFrameCount = 30;

	bool isBlastEmitter = false; // whether this emitter is for the blast powerup or not
	bool useBlastTexture = false; // whether the blast emitter should be used or not (set to true when blast is activated and set back to false when blast is deactivated)
	int pendingBlastParticles = 0; // number of blast particles that still need to be spawned
	float burstSpeed = 25.0f;

	void Init(int max)
	{
		maxParticles = max;
		particles.resize(maxParticles);
		particlePositionData.resize(maxParticles * 4); // x, y, z, size
		particleColorData.resize(maxParticles * 4);    // r, g, b, a
		particleLifeData.resize(maxParticles);         // life
	}
};
