#pragma once

#include <unordered_map>

#include "Window.h"
#include "InputManager.h"
#include "Systems/RenderSystem.h"
#include "Systems/CameraControlSystem.h"
#include "Systems/LevelLoaderSystem.h"
#include "Systems/PhysicsSystem.h"
#include "Systems/VehicleControlSystem.h"
#include "Core/Time.h"

class Game
{
public:
	Game();
	~Game() = default;


	void Run();

private:
	std::unique_ptr<Window> window;
	
	// systems
	std::shared_ptr<RenderSystem> renderSystem;
	std::shared_ptr<CameraControlSystem> camControlSystem;
	std::shared_ptr<PhysicsSystem> physicsSystem;
	std::shared_ptr<LevelLoaderSystem> loaderSystem;
	std::shared_ptr<VehicleControlSystem> vehicleControlSystem;


	std::unique_ptr<Time> time;

	std::shared_ptr<Gamepad> gamepad;
	

	void Cleanup();

	// add game related stuff 

};