#include "Game.h"

#include "glm/glm.hpp"

#include "ImGuiTest.h"
#include "ImGuiPanel.h"

//#include "Entity.h"
#include "Renderer.h"


static glm::vec3 vehicle_position = glm::vec3(0.0f, 0.0f, 00.0f);
static glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 0.0f);
static int camera_scroll_type = 0;
static float camera_fov = 90.0f;
static bool split_camera = false;
static glm::dvec2 previous_mouse_position;
static bool first_time_held_right_click = false;
static PxVec3 vehicleVelocity = PxVec3(0.0f, 0.0f, 0.0f);
// Setup ImGui panel for camera, putting it here to quick access 
class CameraEditorPanelRenderer : public ImGuiPanelRendererInterface {
public:
	//CameraEditorPanelRenderer(){}
	CameraEditorPanelRenderer(CameraEntity* camera) : mainCamera_ptr(camera) {}
	virtual void render() override {
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Vehicle position: (%f,%f,%f)", vehicle_position.x, vehicle_position.y, vehicle_position.z);
		ImGui::Text("Camera position: (%f,%f,%f)", camera_pos.x, camera_pos.y, camera_pos.z);
		ImGui::Text("Distance: %f", mainCamera_ptr->GetDistance());
		ImGui::Text("Phi: %f deg", glm::degrees(mainCamera_ptr->GetPhi()));
		ImGui::Text("Theta: %f deg", glm::degrees(mainCamera_ptr->GetTheta()));
		ImGui::Text("FOV: %f deg", camera_fov);
		
		ImGui::RadioButton("scroll distance", &camera_scroll_type, 0);
		ImGui::RadioButton("scroll fov", &camera_scroll_type, 1);
		ImGui::RadioButton("scroll theta", &camera_scroll_type, 2);
		ImGui::RadioButton("scroll phi", &camera_scroll_type, 3);

		ImGui::Checkbox("split camera", &split_camera);

		ImGui::Text("Vehicle velocity: (%f,%f,%f)", vehicleVelocity.x, vehicleVelocity.y, vehicleVelocity.z);

	}
private:
	CameraEntity* mainCamera_ptr = nullptr;
};

//might have to move to Camera.cpp to declutter later
//void Game::CalculateCameraPanning(float current_xpos, float current_ypos) {
//	//right now it snaps since the input manager only records the final mouse position rather than delta change
//	float screen_width = window->getFrameBufferSize().first;
//	float screen_height = window->getFrameBufferSize().second;
//	float aspect_ratio = screen_width / screen_height;
//	//get relative position based on center of screen
//	float xscreen = (current_xpos / (screen_width / 2)) - 1;
//	float yscreen = (current_ypos / (screen_height / 2)) - 1;
//
//	//float cam_distance = camera->GetDistance();
//
//	//apply some aspect ratio to the movement
//	if (screen_width > screen_height) {
//		xscreen = ((xscreen) * aspect_ratio) / cam_distance;
//		yscreen = (yscreen) / cam_distance;
//	}
//	else {
//		xscreen = (xscreen) / cam_distance;
//		yscreen = ((yscreen) * aspect_ratio) / cam_distance;
//	}
//	if (first_time_held_right_click == false) {
//		first_time_held_right_click = true;
//		previous_mouse_position = glm::dvec2(xscreen, -yscreen);
//	}
//
//	//float deltaX = (xscreen - previous_mouse_position.x) * cam_distance;
//	//float deltaY = (yscreen + previous_mouse_position.y) * cam_distance;
//
//	previous_mouse_position = glm::dvec2(xscreen, -yscreen);
//
//	//apply to camera
//	//camera->ChangeTheta(deltaX);
//	//camera->ChangePhi(deltaY);
//}

Game::Game()
{
	glfwWindowHint(GLFW_SAMPLES, 32);


	//renderer->Init(Camera::Params{});

	inputManager = std::make_shared<InputManager>(
		[this](int width, int height) {
			//glViewport(0, 0, width, height);
			//postProcessor->Resize(window->getFrameBufferSize().first, window->getFrameBufferSize().second);
			renderer->postProcessor->Resize(window->getFrameBufferSize().first, window->getFrameBufferSize().second);
			camera->ChangeAspectRatio((float)window->getFrameBufferSize().first, (float)window->getFrameBufferSize().second);
		}
	); 

	//window = std::make_unique<Window>(1280, 720, "JunkPunk", inputManager); 
	window = std::make_shared<Window>(1280, 720, "JunkPunk", inputManager);
	renderer = std::make_unique<Renderer>(inputManager);

	glfwSwapInterval(1); // Enable vsync to limit fps
	time = std::make_unique<Time>();

	// ------------------ initialize game scene and objects ----------------------

	pScene = std::make_unique<PhysicsScene>();
	
	pScene->InitPhysics();

}

//Game::~Game()
//{
//
//}

// main game function
void Game::Run()
{
	gameScene.insert({ "World", std::make_unique<BaseEntity>()});
	PhysicsEntity* player = gameScene["World"]->addChild<PhysicsEntity>("Player", DrawType::MESH, PhysicsType::VEHICLE, "assets/models/2003_peugeot_hoggar_concept/scene.gltf");

	//camera = player->addChild<CameraEntity>("Camera", DrawType::NO_DRAW, PhysicsType::NO_PHYSICS, CameraParams());
	camera = gameScene["World"]->addChild<CameraEntity>("Camera", DrawType::NO_DRAW, PhysicsType::NO_PHYSICS, CameraParams());

	gameScene["World"]->addChild<PhysicsEntity>("Terrain", DrawType::MESH, PhysicsType::STATIC_MESH, "assets/models/snowy_mountain_-_terrain/scene.gltf");
	gameScene["World"]->getChild("Terrain")->transform.setLocalScale(glm::vec3(500.0f));

	player->transform.setLocalScale(glm::vec3(40.0f));

	camera->transform.setLocalPosition(glm::vec3(0.0f, 2.0f, -2.0f));
	camera->transform.setLocalRotation(glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)));


	gameScene["World"]->addChild<PhysicsEntity>("Rubix", DrawType::MESH, PhysicsType::STATIC_MESH, "assets/models/rubix_2.0/scene.gltf");
	gameScene["World"]->getChild("Rubix")->transform.setLocalScale(glm::vec3(10.0f));
	gameScene["World"]->getChild("Rubix")->transform.setLocalPosition(glm::vec3(5.0f, -20.0f, 0.0f));

	gameScene["World"]->updateSelfAndChild(time->getDeltaTime());

	pScene->InitPhysicsComponentFromEntity(gameScene["World"].get());
	renderer->Init(camera->getCameraParams());

	if (!gameScene["World"])
	{
		std::cout << "World entity empty!" << std::endl;
		return;
	}
	
	// imgui panel for debugging
	ImGuiPanel camera_debug_panel(window);
	auto camera_editor_panel_renderer = std::make_shared<CameraEditorPanelRenderer>(camera);
	camera_debug_panel.setPanelRenderer(camera_editor_panel_renderer);

	// test instance translations
	/*
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
	*/
	// end instance translations 

	// main loop
	while (!window->shouldClose())
	{
		renderer->Clear(0.0f, 0.0f, 0.0f, 1.0f); // clear screen
		time->Update();

		Command command;
		if (inputManager->IsKeyboardButtonDown(GLFW_KEY_W))
		{
			command.throttle = 1.0f;
		}
		if (inputManager->IsKeyboardButtonDown(GLFW_KEY_S))
		{
			//command.brake = 1.0f;
			command.throttle = -1.0f;
		}
		if (inputManager->IsKeyboardButtonDown(GLFW_KEY_A))
		{
			command.steer = 1.0f;
		}
		if (inputManager->IsKeyboardButtonDown(GLFW_KEY_D))
		{
			command.steer = -1.0f;
		}

		// make sure window is focused and controller is connected
		if (inputManager->IsWindowFocused() && inputManager->IsControllerConnected())
		{
			// set vehicle commands based on controller input
			command.throttle = inputManager->GetThrottleValue();
			command.brake = inputManager->GetBrakeValue();
			command.steer = inputManager->GetLStickTurnValue();

			if (inputManager->IsButtonDown(Buttons::JUMP))
			{
				pScene->getVehicle().jump();
			}
			if (inputManager->IsButtonDown(Buttons::POWERUP))
			{
				// not implemented yet
			}
			if (inputManager->IsButtonPressed(Buttons::LEFTROLL))
			{
				// not implemented yet, might need some function to check if we are on the ground or not
			}
			else if (inputManager->IsButtonPressed(Buttons::RIGHTROLL))
			{
				// same here, else if to not roll both ways at once
			}
			if (inputManager->IsButtonPressed(Buttons::PAUSE))
			{
				// not implemented yet
			}

			// camera adjustment
			camera->ChangeTheta(inputManager->GetRStickTurnValueX() * time->getDeltaTime());
			camera->ChangePhi(inputManager->GetRStickTurnValueY() * time->getDeltaTime());
		}

		// send commands to vehicle
		pScene->getVehicle().setCommand(command);
		// simulate physics PhysicsScene
		pScene->Simulate(time->getDeltaTime());

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
			//CalculateCameraPanning(mouse_movement.x, mouse_movement.y);
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
		vehicleVelocity = pScene->getVehicle().getVelocity();


		// update player entity transform from physics vehicle transform
		PxTransform playerTransform = pScene->getVehicle().getTransform();
		glm::vec3 pos = glm::vec3(playerTransform.p.x, playerTransform.p.y, playerTransform.p.z);
		glm::quat rot = glm::quat(playerTransform.q.w, playerTransform.q.x, playerTransform.q.y, playerTransform.q.z);
		player->transform.setLocalPosition(pos);
		player->transform.setLocalRotation(rot);

		camera->transform.setLocalPosition(glm::vec3(pos.x, pos.y + 2.0f, pos.z - 3.0f));
		camera->transform.setLocalRotation(glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

		gameScene["World"]->updateSelfAndChild(time->getDeltaTime()); // updates all entities in the scene

		vehicle_position = player->transform.getGlobalPosition(); // set position for imgui

		// -------------------------- rendering code -----------------------------
		//glViewport(0, 0, width, height); //temporary testing for split camera

		renderer->BeginRender(camera->getCameraParams());
		// shadow pass
		renderer->SetupShadowPass();
		renderer->DrawShadowPass(gameScene["World"].get());
		renderer->EndShadowPass();
		// end shadow pass

		// lighting pass
		renderer->SetupLightingPass();
		renderer->DrawLightingPass(gameScene["World"].get());
		renderer->EndLightingPass();
		// end lighting pass
		
		// draw physics colliders
		renderer->DrawCollisionDebug(pScene->GetRenderBuffer());

		// post processing pass
		renderer->SetupPostProcessingPass();
		renderer->DrawPostProcessingPass();
		renderer->EndPostProcessingPass();
		// end post processing

		camera_debug_panel.render();// render imgui camera debugger

		renderer->EndRender();

		window->swapBuffers();
		glfwPollEvents();
	}

	Cleanup();
}

void Game::Cleanup()
{
	pScene->Cleanup();

	std::cout << "Game cleaned up and exited successfully." << std::endl;
}


void Game::DrawGameObjectsInstanced(const std::vector<glm::mat4> modelMatrices, std::shared_ptr<BaseEntity> entity)
{
	//renderer->DrawEntityInstanced(camera->GetViewProjectionMatrix(), defaultInstanceShader, entity, modelMatrices);
}
