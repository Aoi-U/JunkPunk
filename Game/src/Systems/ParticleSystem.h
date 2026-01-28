#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

#include "../Components/Particles.h"
#include "../Core/Types.h"
#include "System.h"


class ParticleSystem : public System
{
public:
	void Init();

	void Update(float deltaTime);

private:
	int FindUnusedParticle(ParticleEmitter& emitter);
};