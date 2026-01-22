#pragma once

#include <cassert>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "InputManager.h"
#include "Shader.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Model.h"
#include "PostProcessor.h"
#include "ShadowMapper.h"
#include "Light.h"
#include "CameraEntity.h"
#include "PhysicsEntity.h"
#include "BaseEntity.h"


#include "PxPhysicsAPI.h"
using namespace physx;


class Renderer
{
public:
	Renderer(std::shared_ptr<InputManager> inputMgr);
	//~Renderer() = default;

	void Init(CameraParams params);
	void Clear(float r, float g, float b, float a); // clears screen and buffers

	void BeginRender(CameraParams params);

	void SetupShadowPass();
	void DrawShadowPass(BaseEntity* entities);
	void EndShadowPass();

	void SetupLightingPass();
	void DrawLightingPass(BaseEntity* entities);
	void EndLightingPass();

	void SetupPostProcessingPass();
	void DrawPostProcessingPass();
	void EndPostProcessingPass();

	void EndRender();

	//void DrawEntityInstanced(const glm::mat4& projView, std::shared_ptr<Shader> shader, Model* model, const std::vector<glm::mat4>& matrices); // draw function for rendering instanced entities

	void DrawSkybox(); // draw function for rendering skybox

	void DrawCollisionDebug(const PxRenderBuffer& renderBuffer);


	std::unique_ptr<PostProcessor> postProcessor; 
private:
	std::shared_ptr<InputManager> inputManager; // not used but maybe useful later idk

	GLuint debugVao{}, debugVbo{};
	const size_t maxDebugLines = 10000;


	CameraParams cameraState{};
	Frustum cameraFrustum{};

	Light light;
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


	void BindTextures(Mesh& mesh); // bind textures for a mesh
	void ShaderSetup(); // setup all shader constnat uniforms
};