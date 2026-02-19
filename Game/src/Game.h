#pragma once

#include "Window.h"
#include "InputManager.h"
#include "Systems/RenderSystem.h"
#include "Systems/CameraControlSystem.h"
#include "Systems/LevelLoaderSystem.h"
#include "Systems/PhysicsSystem.h"
#include "Systems/VehicleControlSystem.h"
#include "Systems/AudioSystem.h"
#include "Systems/ParticleSystem.h"
#include "Systems/ParticleRenderSystem.h"
#include "Systems/MenuSystem.h"
#include "Systems/PauseSystem.h"
#include "Core/Time.h"

#include "ImGuiPanel.h"

class Game
{
public:
	Game();
	~Game() = default;

	void Run();

private:
	//std::shared_ptr<Gamepad> gamepad;
	std::vector<std::shared_ptr<Gamepad>> gamepads; // list of gamepads 
	std::shared_ptr<Window> window;
	std::unique_ptr<Time> time;

	
	// systems
	std::shared_ptr<RenderSystem> renderSystem;
	std::shared_ptr<CameraControlSystem> camControlSystem;
	std::shared_ptr<PhysicsSystem> physicsSystem;
	std::shared_ptr<LevelLoaderSystem> loaderSystem;
	std::shared_ptr<VehicleControlSystem> vehicleControlSystem;
	std::shared_ptr<AudioSystem> audioSystem;
	std::shared_ptr<ParticleSystem> particleSystem;
	std::shared_ptr<ParticleRenderSystem> particleRenderSystem;
	std::shared_ptr<MenuSystem> menuSystem;
	std::shared_ptr<PauseSystem> pauseSystem;


	void Cleanup();

	// add game related stuff 
	void ChangeGameStateListener(Event& e);
	void KeyboardInputListener(Event& e);
	void TriggerEnterListener(Event& e);

	std::unique_ptr<ImGuiPanel> camera_debug_panel; // testing
	
	// current game state
	GameState currentState = GameState::STARTMENU;
};