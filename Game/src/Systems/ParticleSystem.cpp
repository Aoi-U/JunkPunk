#include "ParticleSystem.h"

#include "../Components/Particles.h"
#include "../Components/Transform.h"
#include "../Components/Physics.h"
#include "../ECSController.h"

extern ECSController controller;


void ParticleSystem::Init()
{

}

void ParticleSystem::Update(float deltaTime)
{
	auto camera = controller.GetEntityByTag("Camera");
	auto& cameraTransform = controller.GetComponent<Transform>(camera);

	for (auto& entity : entities)
	{
		auto& emitter = controller.GetComponent<ParticleEmitter>(entity);

		glm::vec3 entityPos = glm::vec3(0.0f);
		float linearVelocityZ = 0.0f;

		if (controller.HasComponent<Transform>(emitter.targetEntity))
		{
			auto& targetTransform = controller.GetComponent<Transform>(emitter.targetEntity);
			entityPos = targetTransform.position;
		}
		if (controller.HasComponent<PhysicsBody>(emitter.targetEntity))
		{
			auto& targetBody = controller.GetComponent<RigidBody>(emitter.targetEntity);
			linearVelocityZ = targetBody.linearVelocity.z;
		}

		emitter.spawnAccumulator += deltaTime;
		int newParticles = static_cast<int>(emitter.spawnAccumulator * emitter.spawnRate);
		emitter.spawnAccumulator -= newParticles / emitter.spawnRate;

		if (linearVelocityZ > 1.0f)
		{
			for (int i = 0; i < newParticles; i++)
			{
				int particleIndex = FindUnusedParticle(emitter);

				Particle& p = emitter.particles[particleIndex];
				p.life = emitter.life;
				p.position = entityPos + emitter.offset;
				p.speed = glm::vec3(
					((rand() % 100) - 50.0f) / 100.0f,
					((rand() % 100) - 50.0f) / 100.0f,
					-1.0f * (linearVelocityZ * 0.5f + ((rand() % 100) / 100.0f))
				);
				p.r = 100 + (rand() % 156);
				p.g = 100 + (rand() % 156);
				p.b = 100 + (rand() % 156);
				p.a = 200 + (rand() % 56);
				p.size = 0.1f + ((rand() % 100) / 1000.0f);
			}
		}

		emitter.particleCount = 0;
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
					p.cameraDistance = std::pow(glm::length(p.position - cameraTransform.position), 2.0);

					// fill GPU data arrays
					emitter.particlePositionData[4 * emitter.particleCount + 0] = p.position.x;
					emitter.particlePositionData[4 * emitter.particleCount + 1] = p.position.y;
					emitter.particlePositionData[4 * emitter.particleCount + 2] = p.position.z;
					emitter.particlePositionData[4 * emitter.particleCount + 3] = p.size;

					emitter.particleColorData[4 * emitter.particleCount + 0] = p.r;
					emitter.particleColorData[4 * emitter.particleCount + 1] = p.g;
					emitter.particleColorData[4 * emitter.particleCount + 2] = p.b;
					emitter.particleColorData[4 * emitter.particleCount + 3] = p.a;

					emitter.particleCount++;
				}
				else
				{
					// dead particle
					p.cameraDistance = -1.0f;
				}
			}
		}
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
