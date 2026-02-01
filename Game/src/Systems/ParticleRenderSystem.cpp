#include <glad/glad.h>
#include "ParticleRenderSystem.h"
#include "../Components/Particles.h"
#include "../ECSController.h"

extern ECSController controller;

ParticleRenderSystem::ParticleRenderSystem()
{
	// constructor
	particleShader = std::make_unique<Shader>("assets/shaders/particle.vert", "assets/shaders/particle.frag");
	particleTexture = std::make_unique<Texture>("smoke.png");
	particleTexture->Load("assets/textures");


	glGenVertexArrays(1, &particleVAO);
	glBindVertexArray(particleVAO);

	const GLfloat vertices[] = {
	 -0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 -0.5f, 0.5f, 0.0f,
	 0.5f, 0.5f, 0.0f,
	};

	
	glGenBuffers(1, &billboardVBO);
	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particlePositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, particlePositionVBO);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particleColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, particleColorVBO);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, 1000 * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// The VBO containing the life of the particles
	glGenBuffers(1, &particleLifeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, particleLifeVBO);
	glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
}

void ParticleRenderSystem::Init()
{
}

void ParticleRenderSystem::Update(ThirdPersonCamera& tpp)
{
	particleShader->use();

	glm::vec3 right = glm::vec3(tpp.viewMatrix[0][0], tpp.viewMatrix[1][0], tpp.viewMatrix[2][0]);
	glm::vec3 up = glm::vec3(tpp.viewMatrix[0][1], tpp.viewMatrix[1][1], tpp.viewMatrix[2][1]);

	particleShader->setVec3("u_right", &right.x);
	particleShader->setVec3("u_up", &up.x);
	particleShader->setMat4("u_view", tpp.viewMatrix);
	particleShader->setMat4("u_projection", tpp.getProjectionMatrix());

	particleShader->setInt("u_particleTexture", 0);
	particleTexture->Bind(GL_TEXTURE0);

	glDepthMask(GL_FALSE);

	for (const auto& entity : entities)
	{
		glBindVertexArray(particleVAO);

		// setup particle data
		auto& emitter = controller.GetComponent<ParticleEmitter>(entity);
		//std::cout << "rendering: " << emitter.particleCount << std::endl;
		glBindBuffer(GL_ARRAY_BUFFER, particlePositionVBO);
		glBufferData(GL_ARRAY_BUFFER, emitter.maxParticles * 4 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, emitter.particleCount * 4 * sizeof(GLfloat), emitter.particlePositionData.data());

		glBindBuffer(GL_ARRAY_BUFFER, particleColorVBO);
		glBufferData(GL_ARRAY_BUFFER, emitter.maxParticles * 4 * sizeof(GLubyte), nullptr, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, emitter.particleCount * 4 * sizeof(GLubyte), emitter.particleColorData.data());

		glBindBuffer(GL_ARRAY_BUFFER, particleLifeVBO);
		glBufferData(GL_ARRAY_BUFFER, emitter.maxParticles * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, emitter.particleCount * sizeof(GLfloat), emitter.particleLifeData.data());

		// bind the billboard
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// bind particle positions
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particlePositionVBO);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// bind particle colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particleColorVBO);
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);

		// bind particle texture
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, particleLifeVBO);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_TRUE, 0, (void*)0);


		// set attribute divisors
		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
		glVertexAttribDivisor(3, 1);

		// draw particles
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, emitter.particleCount);

		// unbinds
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glBindVertexArray(0);
	}

	glDepthMask(GL_TRUE);
}
