#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "SkyBox.h"
#include "Time.h"
#include "Entity.h"

#include "Vehicle.h"

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

	glm::mat4 cameraTarget{ 0.0f }; // change to car position later
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Skybox> skybox;

	glm::vec3 lightPos;
	glm::vec4 lightColor;
	glm::mat4 lightModel;

	std::shared_ptr<Shader> defaultShader; 
	std::shared_ptr<Shader> lightShader;
	std::shared_ptr<Shader> skyboxShader;

	// list of all static game objects (map objects that dont move)
	std::vector<Entity> gameObjects;

	Vehicle vehicle{};

	void ShaderSetup();
	void Cleanup();
	void DrawGameObjects(const glm::mat4& projView);

	// add game related stuff 

	void CalculateCameraPanning(float current_xpos, float current_ypos);

};