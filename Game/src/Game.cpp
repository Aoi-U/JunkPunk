#include "Game.h"

#include <glm/glm.hpp>

#include "Components/Render.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Player.h"
#include "Components/Physics.h"
#include "Components/Obstacle.h"
#include "Components/Particles.h"
#include "Components/Powerup.h"
#include "Components/AiDriver.h"

#include "ECSController.h"
#include "Core/Types.h"


static int camera_scroll_type = 0;
static bool split_camera = false;
static bool first_time_held_right_click = false;
bool playerWon = false;
bool aiWon = false;
// -------------------------
// For logging player position
// position logging
//float posLogTimer = 0.0f;
//const float POS_LOG_INTERVAL = 1.0f; // 500 ms
// -------------------------
float winTimer = 0.0f;
const float WIN_DELAY = 5.0f;
float fadeAlpha = 0.0f;
Entity playerEntity;

int numPlayers = 1;

// Define a global ECSController instance so systems can access it
ECSController controller;
GameState currentStateGlobal = GameState::STARTMENU;


//Setup ImGui panel for camera, putting it here to quick access
class CameraEditorPanelRenderer : public ImGuiPanelRendererInterface {
public:
	//CameraEditorPanelRenderer(){}
	CameraEditorPanelRenderer()
	{
		controller.AddEventListener(Events::Window::SCROLLED, [this](Event& e) { ScrollEventListener(e); });
	}
	void setCamera(ThirdPersonCamera* camera, Transform* transform)
	{
		mainCamera_ptr = camera;
		cameraTransform_ptr = transform;
	}

	void Reset()
	{
		mainCamera_ptr = nullptr;
		cameraTransform_ptr = nullptr;
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
	ThirdPersonCamera* mainCamera_ptr = nullptr;
	Transform* cameraTransform_ptr = nullptr;

	void ScrollEventListener(Event& e)
	{
		std::cout << "Scroll event received in camera panel" << std::endl;
		double xoffset = e.GetParam<double>(Events::Window::Scrolled::XOFFSET);
		double yoffset = e.GetParam<double>(Events::Window::Scrolled::YOFFSET);

		if (mainCamera_ptr)
		{
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
	}

};
std::shared_ptr<CameraEditorPanelRenderer> cameraPanelRenderer; // testing

// some sources that explain ecs
// https://austinmorlan.com/posts/entity_component_system/ // the ecs i wrote is based on this article so best to read this to understand how it works
// https://www.youtube.com/watch?v=dEdFM0uQpA0

Game::Game()
{
	controller.Init();
	//glfwWindowHint(GLFW_SAMPLES, 32);


	window = std::make_shared<Window>(1280, 720, "JunkPunk");
	//gamepad = std::make_shared<Gamepad>(1); // initialize gamepad
	for (int i = 0; i < numPlayers; i++)
	{
		gamepads.push_back(std::make_shared<Gamepad>(i + 1)); // initialize all connected gamepads
	}

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
	controller.RegisterComponent<Powerup>();
	controller.RegisterComponent<CheckPoint>();
	controller.RegisterComponent<PlayerController>();
	controller.RegisterComponent<AiDriver>();

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
		signature.set(controller.GetComponentType<PlayerController>());
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

	menuSystem = controller.RegisterSystem<MenuSystem>();
	{
		Signature signature;
		controller.SetSystemSignature<MenuSystem>(signature);
	}

	pauseSystem = controller.RegisterSystem<PauseSystem>();
	{
		Signature signature;
		controller.SetSystemSignature<PauseSystem>(signature);
	}

	aiSystem = controller.RegisterSystem<AiSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<AiDriver>());
		signature.set(controller.GetComponentType<Transform>());
		signature.set(controller.GetComponentType<VehicleCommands>());
		signature.set(controller.GetComponentType<VehicleBody>());
		controller.SetSystemSignature<AiSystem>(signature);
	}


	audioSystem->Init();
	vehicleControlSystem->Init(gamepads);
	camControlSystem->Init(gamepads);
	menuSystem->Init(gamepads[0]);
	pauseSystem->Init(gamepads[0]);
	aiSystem->Init();

	controller.AddEventListener(Events::GameState::NEW_STATE, [this](Event& e) { this->ChangeGameStateListener(e); });
	controller.AddEventListener(Events::Window::INPUT, [this](Event& e) { this->KeyboardInputListener(e); });
	controller.AddEventListener(Events::Physics::TRIGGER_ENTER, [this](Event& e) { this->TriggerEnterListener(e); });
}

// main game function
void Game::Run()
{
	// temporary background music. add separate menu bgm, game bgm, etc in their proper states
	Event event(Events::Audio::PLAY_SOUND);
	event.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/jazz-background-music-325355.mp3");
	event.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3{ 0.0f, 0.0f, 0.0f });
	event.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, -50.0f);
	controller.SendEvent(event);

	// imgui panel for debugging
	camera_debug_panel = std::make_unique<ImGuiPanel>(window);
	cameraPanelRenderer = std::make_shared<CameraEditorPanelRenderer>();
	camera_debug_panel->setPanelRenderer(cameraPanelRenderer);


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
		{
			Entity player = playerEntity;
			// -----------------------------------------------------------------------------------
			// FOr logging player position every 0.5 seconds, can be removed later, just for testing
			//posLogTimer += time->frameTime;
			//if (posLogTimer >= POS_LOG_INTERVAL)
			//{
			//	posLogTimer -= POS_LOG_INTERVAL; // preserve remainder instead of resetting to zero

			//	if (controller.HasComponent<Transform>(player))
			//	{
			//		const auto& t = controller.GetComponent<Transform>(player);
			//		std::cout << "glm::vec3("
			//			<< t.position.x << ", "
			//			<< t.position.y << ", "
			//			<< t.position.z << ")\n";
			//	}
			//	else
			//	{
			//		std::cout << "Player transform not available\n";
			//	}
			//}
			// -----------------------------------------------------------------------------------
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
			menuSystem->RenderWinText();
			camera_debug_panel->render(); // render debug panel
			aiSystem->Update(time->frameTime); // update ai drivers
			aiSystem->RenderDebugWaypoints(); // render ai waypoints for debugging


			if (playerWon) {
				winTimer += time->frameTime;
				if (winTimer >= 1.0f) {
					fadeAlpha += time->frameTime * 0.25f;
					fadeAlpha = glm::clamp(fadeAlpha, 0.0f, 1.0f);
					menuSystem->RenderFadeOverlay(fadeAlpha);
				}
				if (winTimer >= WIN_DELAY) {
					Event event(Events::GameState::NEW_STATE);
					event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::ENDMENU);
					controller.SendEvent(event);
				}
			}

			if (aiWon) {
				winTimer += time->frameTime;
				if (winTimer >= 1.0f) {
					fadeAlpha += time->frameTime * 0.25f;
					fadeAlpha = glm::clamp(fadeAlpha, 0.0f, 1.0f);
					menuSystem->RenderFadeOverlay(fadeAlpha);
				}
				if (winTimer >= WIN_DELAY) {
					Event event(Events::GameState::NEW_STATE);
					event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::ENDMENU);
					controller.SendEvent(event);
				}
			}
			if (controller.HasComponent<Powerup>(player)) {
				auto& p = controller.GetComponent<Powerup>(player);
				if (p.active) {
					p.elapsed += time->frameTime;
					if (p.elapsed >= p.duration) {
						controller.RemoveComponent<Powerup>(player);
						std::cout << "Powerup ended" << std::endl;
					}
				}
			}
			audioSystem->Update();

			if (gamepads[0]->GetButtonDown(Buttons::PAUSE))
			{
				Event event(Events::GameState::NEW_STATE);
				event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::PAUSED);
				controller.SendEvent(event);
			}
			if (controller.HasComponent<Powerup>(player)) {
				auto& p = controller.GetComponent<Powerup>(player);
				if (gamepads[0]->GetButtonDown(Buttons::POWERUP) && !p.active) {
					p.active = true;
					p.elapsed = 0.0f;
					std::cout << "Boost Used" << std::endl;
				}
			}

			break;
		}
		case PAUSED:
			renderSystem->Update(time->fps(), physicsSystem->GetRenderBuffer());
			pauseSystem->Update();
			camera_debug_panel->render();
			audioSystem->Update();

			break;
		case STARTMENU:
			// render main menu
			menuSystem->Update();
			audioSystem->Update();
			break;

		case ENDMENU:
			renderSystem->Update(time->fps(), physicsSystem->GetRenderBuffer());
			menuSystem->RenderEndScreen();
			break;
		}

		//gamepad->RefreshState();
		//gamepad->Update();
		for (auto& gamepad : gamepads)
		{
			gamepad->RefreshState();
			gamepad->Update();
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
	std::cout << "Changing game state to " << state << std::endl;

	switch (state)
	{
	case::GameState::STARTMENU:
	{
		controller.Reset();
		physicsSystem->Cleanup();
		menuSystem->Reset();
		renderSystem->Reset();

		time->Pause();
		currentState = state;
		currentStateGlobal = state;
		playerWon = false;
		aiWon = false;
		winTimer = 0.0f;
		fadeAlpha = 0.0f;
		break;
	}
	case::GameState::GAME:
	{
		if (currentState == GameState::STARTMENU) // set up the world when coming from the main menu
		{
			loaderSystem->LoadLevel();
			renderSystem->Init();
			physicsSystem->Init();

			playerEntity = controller.GetEntityByTag("VehicleCommands");

			// testing
			Entity cameraEntity = controller.GetEntityByTag("Camera");
			ThirdPersonCamera& mainCamera = controller.GetComponent<ThirdPersonCamera>(cameraEntity);
			Transform& cameraTransform = controller.GetComponent<Transform>(cameraEntity);
			cameraPanelRenderer->setCamera(&mainCamera, &cameraTransform);
			// end testing
		}

		time->Unpause();
		currentState = state;
		currentStateGlobal = state;
		break;
	}
	case::GameState::ENDMENU:
	{
		// clear the scene or draw some menu on top of game screen?
		time->Pause();
		currentState = state;
		currentStateGlobal = state;

		break;
	}
	case::GameState::PAUSED:
	{
		pauseSystem->Reset();
		// should not clear anything, just stop physics simulation
		time->Pause();
		currentState = state;
		currentStateGlobal = state;

		break;
	}
	case::GameState::RESTART:
	{
		controller.Reset();
		physicsSystem->Cleanup();


		loaderSystem->LoadLevel();
		renderSystem->Init();
		physicsSystem->Init();
		time->Unpause();
		winTimer = 0.0f;
		fadeAlpha = 0.0f;
		playerWon = false;
		aiWon = false;

		currentState = GameState::GAME;
		currentStateGlobal = GameState::GAME;

		break;
	}
	}
}

void Game::KeyboardInputListener(Event& e)
{
	int keyRecieve = e.GetParam<int>(Events::Window::Input::KEY);
	int action = e.GetParam<bool>(Events::Window::Input::ACTION);
	char key = static_cast<char>(keyRecieve);

	Entity player = playerEntity;

	if (currentState == GameState::GAME)
	{
		if (key == Keys::KEY_PAUSE && action == true)
		{
			Event event(Events::GameState::NEW_STATE);
			event.SetParam<GameState>(Events::GameState::New_State::STATE, GameState::PAUSED);
			controller.SendEvent(event);
		}
		if (controller.HasComponent<Powerup>(player)) {
			auto& p = controller.GetComponent<Powerup>(player);
			if (key == Keys::KEY_USE && action == true && !p.active) {
				p.active = true;
				p.elapsed = 0.0f;
				std::cout << "Boost Used" << std::endl;
			}
		}
	}
}

void Game::TriggerEnterListener(Event& e)
{
	Entity triggerEntity = e.GetParam<Entity>(Events::Physics::Trigger_Enter::ENTITY_ONE);
	Entity otherEntity = e.GetParam<Entity>(Events::Physics::Trigger_Enter::ENTITY_TWO);

	Entity finishLine = controller.GetEntityByTag("FinishLine");
	if (triggerEntity == finishLine) {
		//playerWon = true;
		//fadeAlpha = 0.0f;
		//winTimer = 0.0f;
		//std::cout << "you win!" << std::endl;
		if (!playerWon && controller.HasComponent<PlayerController>(otherEntity)) {
			playerWon = true;
			fadeAlpha = 0.0f;
			winTimer = 0.0f;
			std::cout << "you win!" << std::endl;
		}
		else if (!aiWon && controller.HasComponent<AiDriver>(otherEntity)) {
			aiWon = true;
			fadeAlpha = 0.0f;
			winTimer = 0.0f;
			std::cout << "AI wins!" << std::endl;
		}
		return;
	}
	else if (controller.HasComponent<Powerup>(triggerEntity)) {
		Entity player = playerEntity;
		auto pickup = controller.GetComponent<Powerup>(triggerEntity);
		controller.AddComponent(player, pickup);
		std::cout << "Boost collected!" << std::endl;
		controller.DestroyEntity(triggerEntity);
	}
	else if (controller.HasComponent<CheckPoint>(triggerEntity))
	{
		Event e(Events::Checkpoint::REACHED);
		e.SetParam<Entity>(Events::Checkpoint::Reached::PLAYER_ENTITY, otherEntity);
		e.SetParam<Entity>(Events::Checkpoint::Reached::CHECKPOINT_ENTITY, triggerEntity);
		controller.SendEvent(e);

		controller.DestroyEntity(triggerEntity);
	}
}
