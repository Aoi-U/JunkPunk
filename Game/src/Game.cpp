#include "Game.h"

#include "glm/glm.hpp"

Game::Game()
{
	glfwWindowHint(GLFW_SAMPLES, 32);

	inputManager = std::make_shared<InputManager>(
		[](int width, int height) {
			glViewport(0, 0, width, height);
		}
	); 

	window = std::make_unique<Window>(1280, 720, "JunkPunk", inputManager); 

	renderer = std::make_unique<Renderer>(inputManager); 

	camera = std::make_unique<Camera>(cameraTarget, Camera::Params{}); // replace with actual values later

	defaultShader = std::make_shared<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
}

//Game::~Game()
//{
//
//}

// main game function
void Game::Run()
{
	//renderer->Init();

	glm::ivec2 windowSize = window->getWindowSize();
	float width = static_cast<float>(windowSize.x);
	float height = static_cast<float>(windowSize.y);

	// set up matrices for 3D rendering
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
	glm::mat4 view;
	glm::mat4 projView;

	// test unit cube mesh
	std::vector<Vertex> cubeVertices;
	
	for (float x = -0.5f; x <= 0.5f; x += 1.0f)
	{
		for (float y = -0.5f; y <= 0.5f; y += 1.0f)
		{
			for (float z = -0.5f; z <= 0.5f; z += 1.0f)
			{
				Vertex vertex;
				vertex.position = glm::vec3(x, y, z);
				vertex.color = glm::vec3((x + 0.5f), (y + 0.5f), (z + 0.5f)); // color based on position
				cubeVertices.push_back(vertex);
			}
		}
	}
	
	std::vector<GLuint> cubeIndices = {
		// Left face (x = -0.5)
		0, 1, 3, 3, 2, 0,
		// Right face (x = 0.5)
		4, 6, 7, 7, 5, 4,
		// Bottom face (y = -0.5)
		0, 4, 5, 5, 1, 0,
		// Top face (y = 0.5)
		2, 3, 7, 7, 6, 2,
		// Back face (z = -0.5)
		0, 2, 6, 6, 4, 0,
		// Front face (z = 0.5)
		1, 5, 7, 7, 3, 1
	};
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>(cubeVertices, cubeIndices);

	// main loop
	while (!window->shouldClose())
	{
		glfwPollEvents();
		renderer->Clear(0.0f, 0.3f, 0.3f, 1.0f); 

		// example input handling, probably move to InputManager or some other class for readability later
		if (inputManager->IsKeyboardButtonDown(GLFW_KEY_ESCAPE))
		{
			std::cout << "Escape pressed, exiting." << std::endl;
			break;
		}

		view = camera->GetViewMatrix();
		projView = projection * view;

		glm::mat4 model = glm::mat4(1.0f); // identity matrix for model

		model = glm::scale(model, glm::vec3(0.5f));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

		renderer->DrawMesh(model, projView, defaultShader, cubeMesh);

		window->swapBuffers();
	}
}