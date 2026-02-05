#pragma once

#include <cassert>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Systems/System.h"
#include "../Core/Mesh.h"
#include "../Core/Skybox.h"
#include "../Core/Model.h"
#include "../Core/PostProcessor.h"
#include "../Core/ShadowMapper.h"
#include "../Core/Light.h"
#include "../Core/Text.h"
#include "../Core/BoundingVolumes.h"
#include "../Components/Camera.h"
#include "ParticleRenderSystem.h"
#include "PxPhysicsAPI.h"

using namespace physx;

class Event;

class RenderSystem : public System
{
public:
	RenderSystem();

	void Clear(float r, float g, float b, float a); // clears screen and buffers

	void Init();

	void Reset();

	void Update(float fps, const PxRenderBuffer& buffer);

	//void UpdateMenu(float fps); // update function for rendering menus

private:
	void DrawShadowPass(const Frustum& frust);

	void DrawLightingPass(const Frustum& frust, const ThirdPersonCamera& tpp, glm::vec3 pos);
	
	void DrawSkybox(); // draw function for rendering skybox

	void DrawCollisionDebug(const PxRenderBuffer& renderBuffer);

	void DrawPostProcessingPass();

	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

	void BindTextures(Mesh& mesh); // bind textures for a mesh
	void ShaderSetupDefaults(); // setup all shader constnat uniforms

	void WindowSizeListener(Event& e);
	
	void ChangeGameStateListener(Event& e);

	unsigned int screenWidth = 1280;
	unsigned int screenHeight = 720;
	float zNear = 0.1f;
	float zFar = 800.0f;
	float fov = 45.0f;
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, -5.0f, 0.0f) + glm::vec3(0.0f, 1.5f, 5.0f), glm::vec3(0.0f, -5.0f, 0.0f) + glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	GLuint debugVao{}, debugVbo{};
	const size_t maxDebugLines = 10000;

	Entity camera = MAX_ENTITIES;

	Text fonts;
	VAO textVAO;
	VBO textVBO;
	
	Light light;

	std::unique_ptr<PostProcessor> postProcessor; 
	std::unique_ptr<ShadowMapper> shadowMapper;
	std::unique_ptr<Skybox> skybox;

	// shaders
	std::shared_ptr<Shader> postProcessShader;
	std::shared_ptr<Shader> shadowShader;
	std::shared_ptr<Shader> defaultShader;
	std::shared_ptr<Shader> defaultInstanceShader; // may be used for particle rendering
	std::shared_ptr<Shader> lightShader;
	std::shared_ptr<Shader> skyboxShader;
	std::shared_ptr<Shader> physicsDebugShader;
	std::shared_ptr<Shader> textShader;

	std::shared_ptr<ParticleRenderSystem> particleRenderSystem;

	glm::vec3 tintColor = glm::vec3(1.0f);
};