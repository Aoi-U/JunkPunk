#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Mesh.h"

class Game
{
public:
	Game();
	~Game() = default;
	void Run();

private:
	std::shared_ptr<InputManager> inputManager; 
	std::unique_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;

	glm::mat4 cameraTarget{ 0.0f }; // change to car position later
	std::unique_ptr<Camera> camera;

	std::shared_ptr<Shader> defaultShader; // add shader management later


	// add game related stuff 

};