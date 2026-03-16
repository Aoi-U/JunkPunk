#include "ParticleSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <algorithm>

#include "../Components/Transform.h"
#include "../Components/Physics.h"
#include "../ECSController.h"

extern ECSController controller;


void ParticleSystem::Init()
{

}

void ParticleSystem::Update(float deltaTime)
{
	auto camera = controller.GetEntityByTag("Camera1");
	auto& cameraTransform = controller.GetComponent<Transform>(camera);

	for (auto& entity : entities)
	{
		auto& emitter = controller.GetComponent<ParticleEmitter>(entity);

		glm::vec3 entityPos = glm::vec3(0.0f);
		glm::quat targetRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float linearVelocity = 0.0f;

		if (controller.HasComponent<Transform>(emitter.targetEntity))
		{
			auto& targetTransform = controller.GetComponent<Transform>(emitter.targetEntity);
			entityPos = targetTransform.position;
			targetRot = targetTransform.quatRotation;

			entityPos += targetRot * emitter.offset; // apply offset in the direction the vehicle is facing
		}
		if (controller.HasComponent<VehicleBody>(emitter.targetEntity))
		{
			auto& targetBody = controller.GetComponent<VehicleBody>(emitter.targetEntity);
			linearVelocity = glm::length(targetBody.linearVelocity);
		}

		int newParticles = 0;
		float minSpeed = 0.2f;
		float maxSpeed = 60.0f;
		if (linearVelocity > minSpeed) // scale the amount of particles spawning with vehicle speed (not working)
		{
			float speedFactor = (linearVelocity - minSpeed) / (maxSpeed - minSpeed);
			if (speedFactor > 1.0f)
				speedFactor = 1.0f;
			newParticles = (int)(deltaTime * emitter.spawnRate * speedFactor);
		}
		if (newParticles > (int)(0.016f * 20000.0))
		{
			newParticles = (int)(0.016f * 20000.0);
		}

		for (int i = 0; i < newParticles; i++)
		{
			int particleIndex = FindUnusedParticle(emitter);
			emitter.particles[particleIndex].life = emitter.life; // This particle will live for 2 seconds.
			emitter.particles[particleIndex].position = entityPos;
			glm::vec3 mainDir = glm::vec3(0.0f, 1.0f, -1.0f);
			glm::vec3 randomDir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f
			);

			emitter.particles[particleIndex].speed = mainDir + randomDir * 0.5f;

			emitter.particles[particleIndex].r = rand() % 256;
			emitter.particles[particleIndex].g = rand() % 256;
			emitter.particles[particleIndex].b = rand() % 256;
			emitter.particles[particleIndex].a = 255;

			emitter.particles[particleIndex].size = (rand() % 1000) / 4000.0f + 0.1f;
		}

		int particleCount = 0;
		for (int i = 0; i < emitter.maxParticles; i++)
		{
			Particle& p = emitter.particles[i];

			if (p.life > 0.0f)
			{
				p.life -= deltaTime;

				if (p.life > 0.0f)
				{
					p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime * 0.5f;
					p.position += p.speed * deltaTime;
					p.cameraDistance = glm::length2(p.position - cameraTransform.position);

					emitter.particlePositionData[4 * particleCount + 0] = p.position.x;
					emitter.particlePositionData[4 * particleCount + 1] = p.position.y;
					emitter.particlePositionData[4 * particleCount + 2] = p.position.z;
					emitter.particlePositionData[4 * particleCount + 3] = p.size;

					emitter.particleColorData[4 * particleCount + 0] = p.r;
					emitter.particleColorData[4 * particleCount + 1] = p.g;
					emitter.particleColorData[4 * particleCount + 2] = p.b;

					emitter.particleLifeData[particleCount] = p.life;

					float lifeRatio = p.life / emitter.life;
					p.a = (unsigned char)(lifeRatio * 255.0f);
					emitter.particleColorData[4 * particleCount + 3] = p.a;
				}
				else
				{
					p.cameraDistance = -1.0f;
				}
				particleCount++;
			}
		}
		emitter.particleCount = particleCount;
		std::sort(emitter.particles.begin(), emitter.particles.end());

	}
}

int ParticleSystem::FindUnusedParticle(ParticleEmitter& emitter)
{
	for (int i = emitter.lastUsedParticle; i < emitter.maxParticles; i++)
	{
		if (emitter.particles[i].life < 0.0f)
		{
			emitter.lastUsedParticle = i;
			return i;
		}
	}
	
	for (int i = 0; i < emitter.lastUsedParticle; i++)
	{
		if (emitter.particles[i].life < 0.0f)
		{
			emitter.lastUsedParticle = i;
			return i;
		}
	}

	return 0;
}
