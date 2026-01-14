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

	lightPos = glm::vec3(0.0f, -4.0f, 0.0f);
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
	ShaderSetup(); // set up shaders
	
	
	// test car model 
	gameObjects.push_back(Entity(Model("assets/models/old_rusty_car/scene.gltf"), glm::mat4(1.0f))); // add car model to gameObjects 
	// end test car model
	
	gameObjects.push_back(Entity(Model("assets/models/classroom/scene.gltf"), glm::mat4(1.0f)));

	gameObjects.push_back(Entity(Model("assets/models/snowy_mountain_-_terrain/scene.gltf"), glm::mat4(1.0f)));
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(50.0f));
	gameObjects[2].setModelMatrix(model);
	// test classroom model

	// ImGui for testing
	//ImGuiTest gui(window);

	
	ImGuiPanel camera_debug_panel(window);
	auto camera_editor_panel_renderer = std::make_shared<CameraEditorPanelRenderer>(camera);
	camera_debug_panel.setPanelRenderer(camera_editor_panel_renderer);

	// main loop
	while (!window->shouldClose())
	{
		glfwPollEvents();
		renderer->Clear(0.0f, 0.0f, 0.0f, 0.0f); 
		time->Update();

		vehicle.Simulate(static_cast<float>(time->getDeltaTime()));

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
		
		// car model 
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, -10.0f));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.005f)); 
		gameObjects[0].setModelMatrix(model);
		
		model = glm::mat4(1.0f);

		// classroom model
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.0005f));
		gameObjects[1].setModelMatrix(model);

		DrawGameObjects(projView); // draw all game objects in the gameObjects vector

		// skybox rendering
		glm::mat4 skyView = glm::mat4(glm::mat3(view)); // remove translation from the view matrix for skybox
		projView = projection * skyView;
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

// sets shader uniforms for light and skybox (since they are static)
void Game::ShaderSetup()
{
	// light
	lightModel = glm::translate(lightModel, lightPos);
	lightModel = glm::scale(lightModel, glm::vec3(1.0f)); 
		
	// setup default shader
	defaultShader->use();
	defaultShader->setVec3("u_lightPos", &lightPos.x);
	defaultShader->setVec4("u_lightColor", &lightColor.r);

	// setup light shader
	lightShader->use();
	lightShader->setVec3("u_lightPos", &lightPos.x);
	lightShader->setVec4("u_lightColor", &lightColor.r);

	// setup skybox shader
	skyboxShader->use();
	skyboxShader->setInt("u_skybox", 0);
}

// sends render calls for all game objects
void Game::DrawGameObjects(const glm::mat4& projView)
{
	for (Entity& entity : gameObjects)
	{
		renderer->DrawEntity(projView, defaultShader, entity);
	}
}
