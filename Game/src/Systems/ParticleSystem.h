#pragma once

#include "../Components/Particles.h"
#include "../Core/Types.h"
#include "System.h"

class Event;

class ParticleSystem : public System
{
public:
	void Init();

	void Update(float deltaTime);

private:
	int FindUnusedParticle(ParticleEmitter& emitter);

	void BlastEventListener(Event& e);
};