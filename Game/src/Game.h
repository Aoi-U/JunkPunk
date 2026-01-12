#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "SkyBox.h"

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

	glm::mat4 cameraTarget{ 0.0f }; // change to car position later
	std::unique_ptr<Camera> camera;
	std::shared_ptr<Skybox> skybox;

	std::shared_ptr<Shader> defaultShader; 
	std::shared_ptr<Shader> lightShader;
	std::shared_ptr<Shader> skyboxShader;

	// list of all game objects and its model matrices (maybe think of a better solution)
	std::vector<std::pair<std::shared_ptr<Model>, glm::mat4>> gameObjects;

	void DrawGameObjects(const glm::mat4& projView);

	// add game related stuff 

};