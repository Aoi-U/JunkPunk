#include "Game.h"

#include "glm/glm.hpp"

#include "ImGuiTest.h"

Game::Game()
{
	glfwWindowHint(GLFW_SAMPLES, 32);

	inputManager = std::make_shared<InputManager>(
		[](int width, int height) {
			glViewport(0, 0, width, height);
		}
	); 

	//window = std::make_unique<Window>(1280, 720, "JunkPunk", inputManager); 
	window = std::make_shared<Window>(1280, 720, "JunkPunk", inputManager);

	glfwSwapInterval(1); // Enable vsync to limit fps

	renderer = std::make_unique<Renderer>(inputManager); 

	camera = std::make_unique<Camera>(cameraTarget, Camera::Params{}); // replace with actual values later
	skybox = std::make_shared<Skybox>();

	defaultShader = std::make_shared<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
	lightShader = std::make_shared<Shader>("assets/shaders/light.vert", "assets/shaders/light.frag");
	skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
}

//Game::~Game()
//{
//
//}

// main game function
void Game::Run()
{
	//renderer->Init();
	skybox->Init();

	
	glm::mat4 view;
	glm::mat4 projView;

	// light
	glm::vec3 lightPos = glm::vec3(-5.0f, 2.0f, -5.0f);
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);
	lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // small cube for light representation
	defaultShader->use();
	defaultShader->setVec3("u_lightPos", &lightPos.x);
	defaultShader->setVec4("u_lightColor", &(lightColor.r));

	lightShader->use();
	lightShader->setVec3("u_lightPos", &lightPos.x);
	lightShader->setVec4("u_lightColor", &lightColor.r);

	skyboxShader->use();
	skyboxShader->setInt("u_skybox", 0);



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
				vertex.normal = glm::normalize(vertex.position); // normal pointing outwards
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

	// test car model
	std::shared_ptr<Model> carModel = std::make_shared<Model>("assets/models/old_rusty_car/scene.gltf");

	// ImGui for testing
	ImGuiTest gui(window);

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

		glm::ivec2 windowSize = window->getWindowSize();
		float width = static_cast<float>(windowSize.x);
		float height = static_cast<float>(windowSize.y);

		// set up camera matrices 
		glm::mat4 projection = glm::perspective(glm::radians(90.0f), width / height, 0.1f, 100.0f);
		view = camera->GetViewMatrix();
		view = glm::rotate(view, (float)glfwGetTime() * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f)); // slowly rotate view for demo purposes
		projView = projection * view;

		// set light and camera info in renderer
		renderer->SetCamera(camera->GetPosition());

		glm::mat4 model = glm::mat4(1.0f); // identity matrix for model

		// rotating cube model
		model = glm::scale(model, glm::vec3(0.5f));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

		renderer->DrawMesh(model, projView, defaultShader, cubeMesh); // draw rotating cube

		
		model = glm::mat4(1.0f);

		// car model 
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, -10.0f));
		model = glm::scale(model, glm::vec3(0.005f)); 
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		renderer->DrawModel(model, projView, defaultShader, carModel); // draw car model


		// render light as small cube
		renderer->DrawMesh(lightModel, projView, lightShader, cubeMesh);

		// skybox rendering
		view = glm::mat4(glm::mat3(view)); // remove translation from the view matrix for skybox
		projView = projection * view;
		renderer->DrawSkybox(projView, skyboxShader, skybox); // draw skybox

		
		gui.Render(); // render imgui test window

		window->swapBuffers();
	}

	gui.Shutdown(); 
	defaultShader->Delete();
	lightShader->Delete();
	skyboxShader->Delete();
}