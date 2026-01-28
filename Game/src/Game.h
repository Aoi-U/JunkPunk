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
#include "Core/Time.h"

class Game
{
public:
	Game();
	~Game() = default;

	void Run();

private:
	std::shared_ptr<Gamepad> gamepad;
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


	void Cleanup();

	// add game related stuff 

};