#include "Game.h"

#include "glm/glm.hpp"

#include "ImGuiTest.h"
#include "ImGuiPanel.h"

#include "Components/Render.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Player.h"
#include "Components/Physics.h"
#include "Components/Obstacle.h"
#include "Components/Particles.h"

#include "ECSController.h"
#include "Core/Types.h"


static int camera_scroll_type = 0;
static bool split_camera = false;
static bool first_time_held_right_click = false;

// Define a global ECSController instance so systems can access it
ECSController controller;


//Setup ImGui panel for camera, putting it here to quick access 
class CameraEditorPanelRenderer : public ImGuiPanelRendererInterface {
public:
	//CameraEditorPanelRenderer(){}
	CameraEditorPanelRenderer(ThirdPersonCamera* mainCamera, Transform* cameraTransform) : mainCamera_ptr(mainCamera), cameraTransform_ptr(cameraTransform) 
	{
		controller.AddEventListener(Events::Window::SCROLLED, [this](Event& e) { ScrollEventListener(e); });
	}
	virtual void render() override {
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Position: (%f,%f,%f)", cameraTransform_ptr->position.x, cameraTransform_ptr->position.y, cameraTransform_ptr->position.z);
		ImGui::Text("Distance: %f", mainCamera_ptr->radius);
		ImGui::Text("Phi: %f deg", glm::degrees(mainCamera_ptr->pitch));
		ImGui::Text("Theta: %f deg", glm::degrees(mainCamera_ptr->yaw));
		ImGui::Text("FOV: %f deg", mainCamera_ptr->fov);

		ImGui::RadioButton("scroll distance", &camera_scroll_type, 0);
		ImGui::RadioButton("scroll fov", &camera_scroll_type, 1);
		ImGui::RadioButton("scroll theta", &camera_scroll_type, 2);
		ImGui::RadioButton("scroll phi", &camera_scroll_type, 3);

	}

private:
	ThirdPersonCamera* mainCamera_ptr;
	Transform* cameraTransform_ptr;

	void ScrollEventListener(Event& e)
	{
		std::cout << "Scroll event received in camera panel" << std::endl;
		double xoffset = e.GetParam<double>(Events::Window::Scrolled::XOFFSET);
		double yoffset = e.GetParam<double>(Events::Window::Scrolled::YOFFSET);

		switch (camera_scroll_type)
		{
		case 0:
			mainCamera_ptr->radius = glm::clamp(mainCamera_ptr->radius + static_cast<float>(yoffset) * 0.1f, 1.0f, 10.0f);
			break;
		case 1:
			mainCamera_ptr->fov = glm::clamp(mainCamera_ptr->fov + static_cast<float>(yoffset) * 0.5f, 30.0f, 120.0f);
			break;
		case 2:
			mainCamera_ptr->yaw += static_cast<float>(yoffset) * 0.1f;
			break;
		case 3:
			mainCamera_ptr->pitch = glm::clamp(mainCamera_ptr->pitch + static_cast<float>(yoffset) * 0.1f, glm::radians(-89.0f), glm::radians(89.0f));
			break;
		}
	}

};

// some sources that explain ecs
// https://austinmorlan.com/posts/entity_component_system/ // the ecs i wrote is based on this article so best to read this to understand how it works 
// https://www.youtube.com/watch?v=dEdFM0uQpA0 

Game::Game()
{
	controller.Init();
	//glfwWindowHint(GLFW_SAMPLES, 32);


	window = std::make_shared<Window>(1280, 720, "JunkPunk");
	gamepad = std::make_shared<Gamepad>(1); // initialize gamepad 
	
	time = std::make_unique<Time>();
	glfwSwapInterval(1); // Enable vsync to limit fps


	// register components (you must register components first to use them so just register all components here)
	controller.RegisterComponent<Render>();
	controller.RegisterComponent<Transform>();
	controller.RegisterComponent<ThirdPersonCamera>();
	controller.RegisterComponent<VehicleCommands>();
	controller.RegisterComponent<PhysicsBody>();
	controller.RegisterComponent<RigidBody>();
	controller.RegisterComponent<StaticBody>();
	controller.RegisterComponent<VehicleBody>();
	controller.RegisterComponent<Trigger>();
	controller.RegisterComponent<MovingObstacle>();
	controller.RegisterComponent<ParticleEmitter>();

	// register systems (you must register systems before setting component signatures) 
	loaderSystem = controller.RegisterSystem<LevelLoaderSystem>();
	{
		// signatures basically just tell the system which components to look for in an entity
		// an entity must have all the components in the signature to be added to the systems entity list
		// meaning, an entities component list must be a superset of the systems signature to be added to the system (an entity can have more components than a system requires)
		// ex:
		//	system signature: [Transform, Render]
		//	entity A components: [Transform, Render, PhysicsBody] -> added to system, it has both Transform AND Render
		//	entity B components: [Transform] -> not added to system, it is missing Render
		Signature signature;
		controller.SetSystemSignature<LevelLoaderSystem>(signature);
	}

	camControlSystem = controller.RegisterSystem<CameraControlSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<ThirdPersonCamera>());
		signature.set(controller.GetComponentType<Transform>());
		controller.SetSystemSignature<CameraControlSystem>(signature);
	}

	renderSystem = controller.RegisterSystem<RenderSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<Render>());
		signature.set(controller.GetComponentType<Transform>());
		
		controller.SetSystemSignature<RenderSystem>(signature);
	}
	
	physicsSystem = controller.RegisterSystem<PhysicsSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<Transform>());
		signature.set(controller.GetComponentType<PhysicsBody>()); 
		controller.SetSystemSignature<PhysicsSystem>(signature);
	}

	vehicleControlSystem = controller.RegisterSystem<VehicleControlSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<VehicleCommands>());
		controller.SetSystemSignature<VehicleControlSystem>(signature);
	}

	audioSystem = controller.RegisterSystem<AudioSystem>();
	{
		Signature signature;
		controller.SetSystemSignature<AudioSystem>(signature);
	}

	particleSystem = controller.RegisterSystem<ParticleSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<ParticleEmitter>());
		controller.SetSystemSignature<ParticleSystem>(signature);
	}

	std::shared_ptr<ParticleRenderSystem> particleRenderSystem = controller.RegisterSystem<ParticleRenderSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<ParticleEmitter>());
		controller.SetSystemSignature<ParticleRenderSystem>(signature);
	}

	menuRenderSystem = controller.RegisterSystem<MenuRenderSystem>();
	{
		Signature signature;
		controller.SetSystemSignature<MenuRenderSystem>(signature);
	}


	loaderSystem->LoadLevel();
	audioSystem->Init();
	vehicleControlSystem->Init(gamepad);
	camControlSystem->Init(gamepad);
	renderSystem->Init(particleRenderSystem);
	physicsSystem->Init();
	particleSystem->Init();

	controller.AddEventListener(Events::GameState::NEW_STATE, [this](Event& e) { this->ChangeGameStateListener(e); });
}

// main game function
void Game::Run()
{
	Event event(Events::Audio::PLAY_SOUND);
	event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/jazz-background-music-325355.mp3");
	event.SetParam<Vector3>(Events::Audio::Play_Sound::POSITION, Vector3{ 0.0f, 0.0f, 0.0f });
	event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, -10.0f);
	controller.SendEvent(event);


	// imgui panel for debugging
	ImGuiPanel camera_debug_panel(window);
	// get main camera and transform to pass to panel
	Entity cameraEntity = controller.GetEntityByTag("Camera");
	ThirdPersonCamera& mainCamera = controller.GetComponent<ThirdPersonCamera>(cameraEntity);
	Transform& cameraTransform = controller.GetComponent<Transform>(cameraEntity);
	std::shared_ptr<CameraEditorPanelRenderer> cameraPanelRenderer = std::make_shared<CameraEditorPanelRenderer>(&mainCamera, &cameraTransform);
	camera_debug_panel.setPanelRenderer(cameraPanelRenderer);

	// when creating an entity that needs physics during runtime, add all necessary components first
	// then create and send Events::Physics::CREATE_ACTOR event with the entity created as the parameter
	// so the physics system can properly create the actor into the scene

  // main loop
	while (!window->shouldClose())
	{
		time->Update();
		
		switch (currentState)
		{
		case GAME:
			// physics update first to prevent twitching/jittering objects
			while (time->accumulator >= time->deltaTime)
			{
				physicsSystem->Update(time->deltaTime);
				time->accumulator -= time->deltaTime;
				time->totalTime += time->deltaTime;
			}

			vehicleControlSystem->Update(); // handle vehicle controls

			camControlSystem->Update(time->frameTime); // handle camera controls

			particleSystem->Update(time->frameTime); // update particles

			renderSystem->Update(time->fps(), physicsSystem->GetRenderBuffer()); // render physics debug data
		
			camera_debug_panel.render(); // render debug panel

			break;

		case PAUSED:
			renderSystem->Update(time->fps(), physicsSystem->GetRenderBuffer());
			// render some pause menu
			camera_debug_panel.render();
			break;
		case STARTMENU:
			// render main menu
			break;
		}

		window->swapBuffers();
		glfwPollEvents();
	}

	Cleanup();
}

void Game::Cleanup()
{

}

void Game::ChangeGameStateListener(Event& e)
{
	auto state = e.GetParam<GameState>(Events::GameState::New_State::STATE);

	switch (state)
	{
	case::GameState::STARTMENU:
		controller.ClearAllEntities();
		physicsSystem->Cleanup();

		currentState = state;
		break;
	case::GameState::GAME:
		// need to keep track of previous state so these arnt called when going from pause to resume
		loaderSystem->LoadLevel(); 
		physicsSystem->Init();
		// -----------------------

		currentState = state;
		break;
	case::GameState::ENDMENU:
		// clear the scene or draw some menu on top of game screen?
		currentState = state;
		break;
	case::GameState::PAUSED:
		// should not clear anything, just stop regular rendering and simulation
		currentState = state;
		break;
	case::GameState::SETTINGS:
		break;
	}
}
