#pragma once

#include <unordered_map>

#include "Window.h"
#include "InputManager.h"
#include "Renderer.h"
#include "SkyBox.h"
#include "Time.h"
//#include "Entity.h"
#include "PhysicsEntity.h"
#include "CameraEntity.h"
#include "PostProcessor.h"
#include "Physics/PhysicsScene.h"
#include "Light.h"
#include "ShadowMapper.h"

class Game
{
public:
	Game();
	~Game() = default;


	void Run();

private:
	enum Buttons
	{
		JUMP			=		0, // A
		POWERUP		=		1, // B
		X					=		2, // X not using yet
		Y					=		3, // Y not using yet
		LEFTROLL	=		8, // LB
		RIGHTROLL	=		9, // RB
		PAUSE			=		12, // START
	};

	std::shared_ptr<InputManager> inputManager; 
	std::shared_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Time> time;

	glm::mat4 cameraTarget{ 0.0 }; // change to car position later

	// list of all game entities
	std::unordered_map<std::string, std::unique_ptr<BaseEntity>> gameScene;
	CameraEntity* camera;
	
	std::unique_ptr<PhysicsScene> pScene;

	void Cleanup();

	// moving to renderer later
	void DrawGameObjectsInstanced(const std::vector<glm::mat4> modelMatrices, std::shared_ptr<BaseEntity> entity);

	// add game related stuff 

	//void CalculateCameraPanning(float current_xpos, float current_ypos);
};