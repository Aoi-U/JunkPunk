#pragma once

#include "Window.h"
#include "InputManager.h"
#include "Renderer.h"
#include "Camera.h"
#include "SkyBox.h"
#include "Time.h"
#include "Entity.h"
#include "PostProcessor.h"
#include "Physics/PhysicsScene.h"
#include "Light.h"
#include "ShadowMapper.h"

class Game
{
public:
	Game();
	~Game() = default;


	void Run();

private:
	std::shared_ptr<InputManager> inputManager; 
	std::shared_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Time> time;

	glm::mat4 cameraTarget{ 0.0 }; // change to car position later
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Skybox> skybox;

	std::shared_ptr<Light> light;

	std::shared_ptr<PostProcessor> postProcessor; // default size, will be updated in Run()	
	std::shared_ptr<ShadowMapper> shadowMapper;

	std::shared_ptr<Shader> postProcessShader;
	std::shared_ptr<Shader> shadowShader;
	std::shared_ptr<Shader> defaultShader; 
	std::shared_ptr<Shader> defaultInstanceShader; // may be used for particle rendering
	std::shared_ptr<Shader> lightShader;
	std::shared_ptr<Shader> skyboxShader;
	std::shared_ptr<Shader> physicsShader;

	// list of all static game objects (map objects that dont move)
	std::vector<std::shared_ptr<Entity>> staticGameObjects;
	std::shared_ptr<Entity> player;
	//Entity grass;

	PhysicsScene pScene;

	void ShaderSetup();
	void Cleanup();

	void RenderShadowPhysicsScene();
	void RenderPhysicsScene();
	void DrawGameObjectsInstanced(const std::vector<glm::mat4> modelMatrices, std::shared_ptr<Entity> entity);

	// add game related stuff 

	void CalculateCameraPanning(float current_xpos, float current_ypos);
};