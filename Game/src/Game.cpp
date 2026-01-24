#include "Game.h"

#include "glm/glm.hpp"

#include "ImGuiTest.h"
#include "ImGuiPanel.h"

#include "Components/Render.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Player.h"
#include "Components/Physics.h"

#include "ECSController.h"


//#include "Entity.h"

static glm::vec3 vehicle_position = glm::vec3(0.0f, 0.0f, 00.0f);
static glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 0.0f);
static int camera_scroll_type = 0;
static float camera_fov = 90.0f;
static bool split_camera = false;
static glm::dvec2 previous_mouse_position;
static bool first_time_held_right_click = false;
static PxVec3 vehicleVelocity = PxVec3(0.0f, 0.0f, 0.0f);

// Define a global ECSController instance so systems can access it
ECSController controller;

Game::Game()
{
	//controller = ECSController();
	controller.Init();
	glfwWindowHint(GLFW_SAMPLES, 32);


	window = std::make_unique<Window>(1280, 720, "JunkPunk");
	loaderSystem = std::make_unique<LevelLoaderSystem>();
	time = std::make_unique<Time>();
	gamepad = std::make_shared<Gamepad>(1); // initialize gamepad at index 0
	glfwSwapInterval(1); // Enable vsync to limit fps


	// register components
	controller.RegisterComponent<Render>();
	controller.RegisterComponent<Transform>();
	controller.RegisterComponent<ThirdPersonCamera>();
	controller.RegisterComponent<Player>();
	controller.RegisterComponent<PhysicsBody>();
	controller.RegisterComponent<RigidBody>();
	controller.RegisterComponent<StaticModel>();
	controller.RegisterComponent<VehicleBody>();
	controller.RegisterComponent<Trigger>();

	
	// register systems
	loaderSystem = controller.RegisterSystem<LevelLoaderSystem>();
	{
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
		signature.set(controller.GetComponentType<Player>());
		controller.SetSystemSignature<VehicleControlSystem>(signature);
	}

	loaderSystem->LoadLevel();
	vehicleControlSystem->Init(gamepad);
	camControlSystem->Init(gamepad);
	renderSystem->Init();
	physicsSystem->Init();
}

//Game::~Game()
//{
//
//}

// main game function
void Game::Run()
{

	// main loop
	while (!window->shouldClose())
	{
		renderSystem->Clear(0.0f, 0.0f, 0.0f, 1.0f); // clear screen
		time->Update();

		vehicleControlSystem->Update();


		int totalUpdates = 0;
		while (time->accumulator >= time->deltaTime)
		{
			physicsSystem->Update(time->deltaTime);
			time->accumulator -= time->deltaTime;
			time->totalTime += time->deltaTime;
			totalUpdates++;
			//std::cout << "Total Physics Updates this frame: " << totalUpdates << std::endl;
		}
		

		camControlSystem->Update(time->deltaTime);

		renderSystem->Update(time->fps(), physicsSystem->GetRenderBuffer()); // render physics debug data
		
		window->swapBuffers();
		glfwPollEvents();
	}

	Cleanup();
}

void Game::Cleanup()
{

	std::cout << "Game cleaned up and exited successfully." << std::endl;
}
