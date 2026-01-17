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
		[this](int width, int height) {
			glViewport(0, 0, width, height);
			postProcessor->Resize(width, height);
			camera->ChangeAspectRatio((float)width / (float)height);
		}
	); 

	//window = std::make_unique<Window>(1280, 720, "JunkPunk", inputManager); 
	window = std::make_shared<Window>(1280, 720, "JunkPunk", inputManager);
	postProcessor = std::make_shared<PostProcessor>(1280, 720);

	glfwSwapInterval(1); // Enable vsync to limit fps

	renderer = std::make_unique<Renderer>(inputManager);

	time = std::make_unique<Time>();

	camera = std::make_shared<Camera>(cameraTarget, Camera::Params{}, camera_fov, 1280 / 720.0f); // replace with actual values later

	skybox = std::make_shared<Skybox>();

	// initialize light properties
	lightPos = glm::vec3(0.0f, -4.0f, 0.0f);
	lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightModel = glm::mat4(1.0f);

	// initialize shaders
	postProcessShader = std::make_shared<Shader>("assets/shaders/postProcess.vert", "assets/shaders/postProcess.frag");
	defaultShader = std::make_shared<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
	defaultInstanceShader = std::make_shared<Shader>("assets/shaders/defaultInstanced.vert", "assets/shaders/defaultInstanced.frag");
	lightShader = std::make_shared<Shader>("assets/shaders/light.vert", "assets/shaders/light.frag");
	skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
	physicsShader = std::make_shared<Shader>("assets/shaders/colliders.vert", "assets/shaders/colliders.frag");
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
	player = Entity(Model("assets/models/2003_peugeot_hoggar_concept/scene.gltf"), glm::mat4(1.0f)); // add car model to gameObjects
	// end test car model
	
	//staticGameObjects.push_back(Entity(Model("assets/models/classroom/scene.gltf"), glm::mat4(1.0f)));

	staticGameObjects.push_back(Entity(Model("assets/models/snowy_mountain_-_terrain/scene.gltf"), glm::mat4(1.0f)));
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1000.0f));
	staticGameObjects[0].setModelMatrix(model);
	// test classroom model

	grass = Entity(Model("assets/models/single_grass/scene.gltf"), glm::mat4(1.0f));

	scene.InitPhysics(staticGameObjects); // initialize physics scene

	// ImGui for testing
	//ImGuiTest gui(window);

	
	ImGuiPanel camera_debug_panel(window);
	auto camera_editor_panel_renderer = std::make_shared<CameraEditorPanelRenderer>(camera);
	camera_debug_panel.setPanelRenderer(camera_editor_panel_renderer);

	// test instance translations
	unsigned int amount = 5000;
	std::vector<glm::mat4> modelMatrices(amount);
	srand(glfwGetTime()); // initialize random seed	
	float maxRadius = 15.0; // maximum radius of the circular area
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);

		// random position in a circle
		float randomRadius = sqrt((float)rand() / RAND_MAX) * maxRadius;
		float randomAngle = ((float)rand() / RAND_MAX) * glm::two_pi<float>(); // 0 to 2π

		float x = cos(randomAngle) * randomRadius;
		float z = sin(randomAngle) * randomRadius;

		// Small random height variation
		float y = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.4f;
		y = y - 5.0f; // lower to somewhat level with ground

		model = glm::translate(model, glm::vec3(x, y, z));

		// rotate to stand upright with semirandom angle variation
		float yawAngle = glm::radians((float)(rand() % 360));
		model = glm::rotate(model, yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));


		// scale between 0.005 and 0.015f
		float scale = (rand() % 10) / 1000.0f + 0.005f;
		model = glm::scale(model, glm::vec3(scale));

		modelMatrices[i] = model;
	}

	VBO instanceVBO(modelMatrices); 

	for (unsigned int i = 0; i < grass.getModel()->getMeshes().size(); i++)
	{
		Mesh& mesh = grass.getModel()->getMeshes()[i];
		mesh.SetupInstanceMesh();
	}
	// end instance translations 

	// main loop
	while (!window->shouldClose())
	{
		// ----------- rendering code setup ---------
		postProcessor->BindFBO(); 
		renderer->Clear(0.0f, 0.0f, 0.0f, 0.0f); 
		// ------------------------------------------

		time->Update();

		// set vehicle commands based on controller input
		Command command;

		// make sure window is focused and controller is connected
		if (inputManager->IsWindowFocused() && inputManager->IsControllerConnected())
		{
			command.throttle = inputManager->GetThrottleValue();
			command.brake = inputManager->GetBrakeValue();
			command.steer = inputManager->GetLStickTurnValue();

			// camera adjustment
			camera->ChangeTheta(inputManager->GetRStickTurnValueX() * time->getDeltaTime());
			camera->ChangePhi(inputManager->GetRStickTurnValueY() * time->getDeltaTime());
		}
		
		// send commands to vehicle
		scene.getVehicle().setCommand(command);
		// simulate physics scene
		scene.Simulate(time->getDeltaTime());		
	
		// update camera
		glm::mat4 playerTransform = scene.getVehicle().getTransform();
		camera->Update(time->getDeltaTime(), playerTransform);
		

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
				camera->ChangeFov(camera_fov);
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
		

		if (split_camera) {
			glm::ivec2 windowSize = window->getWindowSize();
			float width = static_cast<float>(windowSize.x);
			float height = static_cast<float>(windowSize.y);
			height /= 2;//temporary testing for split camera
		}


		// ---------------- rendering code ----------------
		renderer->SetCamera(camera->GetPosition());

		//glViewport(0, 0, width, height); //temporary testing for split camera

		glm::mat4 model = glm::mat4(1.0f); // identity matrix for model
		
		// car model 
		model = scene.getVehicle().getTransform();
		model = glm::scale(model, glm::vec3(40.0f)); 
		player.setModelMatrix(model);

		
		model = glm::mat4(1.0f);

		

		DrawGameObjects(camera->GetViewProjectionMatrix()); // draw all game objects in the gameObjects vector
		//DrawGameObjectsInstanced(projView, modelMatrices, grass); // draw instanced grass 
		DrawSkybox(); // draw skybox (make sure to draw last for optimization)

		renderer->DrawCollisionDebug(camera->GetViewProjectionMatrix(), physicsShader, scene.GetRenderBuffer(), player.getModelMatrix());

		//gui.Render(); // render imgui test window

		postProcessor->Unbind();
		renderer->Clear(1.0f, 1.0f, 1.0f, 1.0f);
		renderer->DrawQuad(postProcessShader, postProcessor);

		camera_debug_panel.render();// render imgui camera debugger
		window->swapBuffers();
		glfwPollEvents();
	}

	Cleanup();
}

// sets shader uniforms for light and skybox (since they are static)
void Game::ShaderSetup()
{
	// post processor
	postProcessShader->use();
	postProcessShader->setInt("screenTexture", 0);

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

void Game::Cleanup()
{
	for (Entity& entity : staticGameObjects)
	{
		entity.Cleanup();
	}

	grass.Cleanup();

	postProcessor->Cleanup();

	postProcessShader->Delete();
	defaultShader->Delete();
	defaultInstanceShader->Delete();
	lightShader->Delete();
	skyboxShader->Delete();
	skybox->Delete();

	scene.Cleanup();

	std::cout << "Game cleaned up and exited successfully." << std::endl;
}

// sends render calls for all game objects
void Game::DrawGameObjects(const glm::mat4& projView)
{
	for (Entity& entity : staticGameObjects)
	{
		renderer->DrawEntity(projView, defaultShader, entity);
	}

	renderer->DrawEntity(projView, defaultShader, player);
}

void Game::DrawGameObjectsInstanced(const glm::mat4& projView, const std::vector<glm::mat4> modelMatrices, Entity entity)
{
	renderer->DrawEntityInstanced(projView, defaultInstanceShader, entity, modelMatrices);
}

void Game::DrawSkybox()
{
	// remove translation from the view matrix
	glm::mat4 projView = camera->GetProjectionMatrix() * glm::mat4(glm::mat3(camera->GetViewMatrix())); 

	renderer->DrawSkybox(projView, skyboxShader, skybox);
}
