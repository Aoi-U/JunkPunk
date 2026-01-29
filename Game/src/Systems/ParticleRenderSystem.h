#pragma once

#include <memory>

#include "../Core/Shader.h"
#include "System.h"
//#include "../Core/Types.h"

class Event;

class ParticleRenderSystem : public System
{
public:
	void Init();

private:
	void RenderParticleListener(Event& e);

	std::unique_ptr<Shader> particleShader;

	GLuint particleVAO{};
	GLuint billboardVBO{};
	GLuint particlePositionVBO{};
	GLuint particleColorVBO{};

};