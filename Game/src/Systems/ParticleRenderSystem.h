#pragma once

#include <memory>
#include <vector>

#include "../Core/Shader.h"
#include "../Core/Texture.h"
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
	std::unique_ptr<Texture> particleTexture;


	GLuint particleVAO{};
	GLuint billboardVBO{};
	GLuint particlePositionVBO{};
	GLuint particleColorVBO{};
	GLuint particleLifeVBO{};

};