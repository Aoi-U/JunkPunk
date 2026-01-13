#include "Game.h"

#include "glm/glm.hpp"

#include "ImGuiTest.h"
#include "ImGuiPanel.h"


static int camera_scroll_type = 0;
static float camera_fov = 90.0f;
static bool split_camera = false;
static glm::dvec2 previous_mouse_position;
static bool first_time_held_right_click = false;
// Setup ImGui panel for camera, putting it here to quick access 
class CameraEditorPanelRenderer : public ImGuiPanelRendererInterface {
public:
	//CameraEditorPanelRenderer(){}
	CameraEditorPanelRenderer(std::shared_ptr<Camera> mainCamera) : mainCamera_ptr(mainCamera) {}
	virtual void render() override {
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Position: (%f,%f,%f)", mainCamera_ptr->GetPosition().x, mainCamera_ptr->GetPosition().y, mainCamera_ptr->GetPosition().z);
		ImGui::Text("Distance: %f", mainCamera_ptr->GetDistance());
		ImGui::Text("Phi: %f deg", glm::degrees(mainCamera_ptr->GetPhi()));
		ImGui::Text("Theta: %f deg", glm::degrees(mainCamera_ptr->GetTheta()));
		ImGui::Text("FOV: %f deg", camera_fov);
		
		ImGui::RadioButton("scroll distance", &camera_scroll_type, 0);
		ImGui::RadioButton("scroll fov", &camera_scroll_type, 1);
		ImGui::RadioButton("scroll theta", &camera_scroll_type, 2);
		ImGui::RadioButton("scroll phi", &camera_scroll_type, 3);


		ImGui::Checkbox("split camera", &split_camera);
	}
private:
	std::shared_ptr<Camera> mainCamera_ptr = nullptr;
};

//might have to move to Camera.cpp to declutter later
void Game::CalculateCameraPanning(float current_xpos, float current_ypos) {
	//right now it snaps since the input manager only records the final mouse position rather than delta change
	float screen_width = window->getWindowSize().x;
	float screen_height = window->getWindowSize().y;
	float aspect_ratio = screen_width / screen_height;
	//get relative position based on center of screen
	float xscreen = (current_xpos / (screen_width / 2)) - 1;
	float yscreen = (current_ypos / (screen_height / 2)) - 1;

	float cam_distance = camera->GetDistance();

	//apply some aspect ratio to the movement
	if (screen_width > screen_height) {
		xscreen = ((xscreen) * aspect_ratio) / cam_distance;
		yscreen = (yscreen) / cam_distance;
	}
	else {
		xscreen = (xscreen) / cam_distance;
		yscreen = ((yscreen) * aspect_ratio) / cam_distance;
	}
	if (first_time_held_right_click == false) {
		first_time_held_right_click = true;
		previous_mouse_position = glm::dvec2(xscreen, -yscreen);
	}

	float deltaX = (xscreen - previous_mouse_position.x) * cam_distance;
	float deltaY = (yscreen + previous_mouse_position.y) * cam_distance;

	previous_mouse_position = glm::dvec2(xscreen, -yscreen);

	//apply to camera
	camera->ChangeTheta(deltaX);
	camera->ChangePhi(deltaY);
}

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

	time = std::make_unique<Time>();

	camera = std::make_shared<Camera>(cameraTarget, Camera::Params{}); // replace with actual values later
	skybox = std::make_shared<Skybox>();

	lightPos = glm::vec3(-5.0f, 2.0f, -5.0f);
	lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightModel = glm::mat4(1.0f);

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
	skybox->Init(); // load and process skybox 
	ShaderSetup(); // set up shaders with initial values
	

	// test unit cube mesh (DELETE LATER)
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
	// end test cube mesh

	// test car model 
	std::shared_ptr<Model> carModel = std::make_shared<Model>("assets/models/old_rusty_car/scene.gltf");
	// to load any other model, just add the model file to assets/models/ and create a model class with the path to the model file

	gameObjects.push_back(std::make_pair(carModel, glm::mat4(1.0f)));
	
	/*
	std::shared_ptr<Model> classroom = std::make_shared<Model>("assets/models/classroom/scene.gltf");
	gameObjects.push_back(std::make_pair(classroom, glm::mat4(1.0f)));
	*/
	

	// ImGui for testing
	//ImGuiTest gui(window);

	
	ImGuiPanel camera_debug_panel(window);
	auto camera_editor_panel_renderer = std::make_shared<CameraEditorPanelRenderer>(camera);
	camera_debug_panel.setPanelRenderer(camera_editor_panel_renderer);
	


	// main loop
	while (!window->shouldClose())
	{
		glfwPollEvents();
		renderer->Clear(0.0f, 0.3f, 0.3f, 1.0f); 
		time->Update();

		// example input handling, probably move to InputManager or some other class for readability later
		if (inputManager->IsKeyboardButtonDown(GLFW_KEY_ESCAPE))
		{
			std::cout << "Escape pressed, exiting." << std::endl;
			break;
		}
		int scroll_changed = inputManager->ScrollValueChanged();
		if (scroll_changed != 0) {
			if (camera_scroll_type == 0) {
				camera->ChangeRadius(scroll_changed * 0.1f);
			}
			else if (camera_scroll_type == 1) {
				camera_fov += scroll_changed * 0.5f;
			}
			else if (camera_scroll_type == 2) {
				camera->ChangeTheta(scroll_changed * 0.01f);
			}
			else if (camera_scroll_type == 3) {
				camera->ChangePhi(scroll_changed * 0.01f);
			}
		}
		if (inputManager->IsMouseButtonDown(1)) {
			glm::dvec2 mouse_movement = inputManager->CursorPosition();
			CalculateCameraPanning(mouse_movement.x, mouse_movement.y);
		}
		else {
			first_time_held_right_click = false;
		}
		

		glm::ivec2 windowSize = window->getWindowSize();
		float width = static_cast<float>(windowSize.x);
		float height = static_cast<float>(windowSize.y);

		if (split_camera) {
			height /= 2;//temporary testing for split camera
		}

		// set up camera matrices 
		glm::mat4 projection = glm::perspective(glm::radians(camera_fov), width / height, 0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix(); 
		glm::mat4 projView = projection * view;

		// set camera info in renderer
		renderer->SetCamera(camera->GetPosition());

		glViewport(0, 0, width, height); //temporary testing for split camera

		glm::mat4 model = glm::mat4(1.0f); // identity matrix for model

		// rotating cube model
		model = glm::scale(model, glm::vec3(0.5f));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
		renderer->DrawMesh(model, projView, defaultShader, cubeMesh); // example draw rotating cube
		// dont use the DrawMesh function, ill probably remove it later. its just here for drawing a simple manually created mesh

		
		model = glm::mat4(1.0f);

		// car model 
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, -10.0f));
		model = glm::scale(model, glm::vec3(0.005f)); 
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		gameObjects[0].second = model; // update car model matrix in gameObjects vector
		//renderer->DrawModel(model, projView, defaultShader, carModel); // example draw car model

		/*
		model = glm::mat4(1.0f);
		// classroom model
		model = glm::scale(model, glm::vec3(1.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
		gameObjects[1].second = model; // update classroom model matrix in gameObjects vector
		*/
		
		DrawGameObjects(projView); // draw all game objects in the gameObjects vector



		// render light as small cube
		renderer->DrawMesh(lightModel, projView, lightShader, cubeMesh);

		// skybox rendering
		view = glm::mat4(glm::mat3(view)); // remove translation from the view matrix for skybox
		projView = projection * view;
		renderer->DrawSkybox(projView, skyboxShader, skybox); // draw skybox

		
		//gui.Render(); // render imgui test window
		camera_debug_panel.render();// render imgui camera debugger

		window->swapBuffers();
	}

	//gui.Shutdown(); 
	defaultShader->Delete();
	lightShader->Delete();
	skyboxShader->Delete();
}

void Game::ShaderSetup()
{
	// light
	lightModel = glm::translate(lightModel, lightPos);
	lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // small cube for light representation
	defaultShader->use();
	defaultShader->setVec3("u_lightPos", &lightPos.x);
	defaultShader->setVec4("u_lightColor", &(lightColor.r));

	// setup light shader
	lightShader->use();
	lightShader->setVec3("u_lightPos", &lightPos.x);
	lightShader->setVec4("u_lightColor", &lightColor.r);

	// setup skybox shader
	skyboxShader->use();
	skyboxShader->setInt("u_skybox", 0);
}

void Game::DrawGameObjects(const glm::mat4& projView)
{
	for (const auto& obj : gameObjects)
	{
		renderer->DrawModel(obj.second, projView, defaultShader, obj.first);
	}
}
