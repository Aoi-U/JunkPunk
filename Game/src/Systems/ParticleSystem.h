#pragma once

#include "../Components/Particles.h"
#include "System.h"


class ParticleSystem : public System
{
public:
	void Init();

	void Update(float deltaTime);

private:
	int FindUnusedParticle(ParticleEmitter& emitter);
};