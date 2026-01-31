#pragma once

#include <memory>
#include <vector>

#include "../Core/Shader.h"
#include "../Core/Texture.h"
#include "../Components/Camera.h"
#include "System.h"

class Event;

class ParticleRenderSystem : public System
{
public:
	void Init();

	void Update(ThirdPersonCamera& camera);
private:

	std::unique_ptr<Shader> particleShader;
	std::unique_ptr<Texture> particleTexture;


	GLuint particleVAO{};
	GLuint billboardVBO{};
	GLuint particlePositionVBO{};
	GLuint particleColorVBO{};
	GLuint particleLifeVBO{};

};