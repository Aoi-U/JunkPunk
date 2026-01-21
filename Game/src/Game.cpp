#include "Game.h"

#include "glm/glm.hpp"

#include "ImGuiTest.h"
#include "ImGuiPanel.h"

#include "Entity.h"
#include "Renderer.h"


static glm::vec3 vehicle_position = glm::vec3(0.0f, 0.0f, 00.0f);
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
	CameraEditorPanelRenderer(std::shared_ptr<Camera> mainCamera) : mainCamera_ptr(mainCamera) {}
	virtual void render() override {
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Vehicle position: (%f,%f,%f)", vehicle_position.x, vehicle_position.y, vehicle_position.z);
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
	std::shared_ptr<Camera> mainCamera_ptr = nullptr;
};

//might have to move to Camera.cpp to declutter later
void Game::CalculateCameraPanning(float current_xpos, float current_ypos) {
	//right now it snaps since the input manager only records the final mouse position rather than delta change
	float screen_width = window->getFrameBufferSize().first;
	float screen_height = window->getFrameBufferSize().second;
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
			//glViewport(0, 0, width, height);
			postProcessor->Resize(window->getFrameBufferSize().first, window->getFrameBufferSize().second);
			camera->ChangeAspectRatio((float)window->getFrameBufferSize().first / (float)window->getFrameBufferSize().second);
		}
	); 

	//window = std::make_unique<Window>(1280, 720, "JunkPunk", inputManager); 
	window = std::make_shared<Window>(1280, 720, "JunkPunk", inputManager);
	postProcessor = std::make_shared<PostProcessor>(window->getFrameBufferSize().first, window->getFrameBufferSize().second);

	glfwSwapInterval(1); // Enable vsync to limit fps

	renderer = std::make_shared<Renderer>(inputManager);

	time = std::make_unique<Time>();

	camera = std::make_shared<Camera>(cameraTarget, Camera::Params{}, camera_fov, (float)window->getFrameBufferSize().first / (float)window->getFrameBufferSize().second); // replace with actual values later

	skybox = std::make_shared<Skybox>();

	// initialize light properties
	light = std::make_shared<Light>();
	shadowMapper = std::make_shared<ShadowMapper>(camera, window, light);

	// initialize shaders
	postProcessShader = std::make_shared<Shader>("assets/shaders/postProcess.vert", "assets/shaders/postProcess.frag");
	shadowShader = std::make_shared<Shader>("assets/shaders/shadowMap.vert", "assets/shaders/shadowMap.frag", "assets/shaders/shadowMap.geom");
	defaultShader = std::make_shared<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
	defaultInstanceShader = std::make_shared<Shader>("assets/shaders/defaultInstanced.vert", "assets/shaders/defaultInstanced.frag");
	lightShader = std::make_shared<Shader>("assets/shaders/light.vert", "assets/shaders/light.frag");
	skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
	physicsShader = std::make_shared<Shader>("assets/shaders/colliders.vert", "assets/shaders/colliders.frag");


	// ------------------ initialize game scene and objects ----------------------

	
	// test car model 
	//player = std::make_shared<Model>("assets/models/2003_peugeot_hoggar_concept/scene.gltf");
	player = std::make_unique<Entity>("Player", "assets/models/2003_peugeot_hoggar_concept/scene.gltf");
	player->transform.setLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	player->transform.setLocalScale(glm::vec3(40.0f));
	player->updateSelfAndChild();
	// end test car model

	
	//staticGameObjects.insert({ "Terrain", std::make_unique<Entity>("Terrain", "assets/models/snowy_mountain_-_terrain/scene.gltf" }));
	staticGameObjects.insert({ "Terrain", std::make_unique<Entity>("Terrain", "assets/models/snowy_mountain_-_terrain/scene.gltf") });
	staticGameObjects["Terrain"]->transform.setLocalScale(glm::vec3(500.0f));
	staticGameObjects["Terrain"]->updateSelfAndChild();

	//// test rubix model
	//staticGameObjects.emplace_back("Rubix", std::make_unique<Entity>("Rubix", "assets/models/rubix_2.0/scene.gltf"));
	staticGameObjects.insert({ "Rubix", std::make_unique<Entity>("Rubix1", "assets/models/rubix_2.0/scene.gltf")});
	staticGameObjects["Rubix"]->addChild("Rubix2", "assets/models/rubix_2.0/scene.gltf");
	staticGameObjects["Rubix"]->getChild("Rubix2")->transform.setLocalScale(glm::vec3(0.5f));
	staticGameObjects["Rubix"]->getChild("Rubix2")->transform.setLocalPosition(glm::vec3(3.0f, -2.0f, 0.0f));

	staticGameObjects["Rubix"]->transform.setLocalPosition(glm::vec3(50.0f, -20.0f, 0.0f));
	staticGameObjects["Rubix"]->transform.setLocalScale(glm::vec3(5.0f));
	staticGameObjects["Rubix"]->updateSelfAndChild();
	

	pScene = std::make_unique<PhysicsScene>();
	
	pScene->InitPhysics();
	for (const auto& [_, entity] : staticGameObjects)
	{
		pScene->InitPhysicsComponentFromEntity(entity.get());
	}
}

//Game::~Game()
//{
//
//}

// main game function
void Game::Run()
{
	renderer->Init();
	skybox->Init(); // load and process skybox 
	shadowMapper->Init(shadowShader, defaultShader);
	ShaderSetup(); // set up shaders

	//grass = Entity(Model("assets/models/single_grass/PhysicsScene.gltf"), glm::mat4(1.0f));

	//pScene.InitPhysics(staticGameObjects)
	
	
	

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

			// camera adjustment
			camera->ChangeTheta(inputManager->GetRStickTurnValueX() * time->getDeltaTime());
			camera->ChangePhi(inputManager->GetRStickTurnValueY() * time->getDeltaTime());
		}

		// send commands to vehicle
		pScene->getVehicle().setCommand(command);
		// simulate physics PhysicsScene
		pScene->Simulate(time->getDeltaTime());

		for (auto& [_, entity] : staticGameObjects)
		{
			entity->updateSelfAndChild();
		}

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
		vehicleVelocity = pScene->getVehicle().getVelocity();

		glm::mat4 playerModelMatrix = player->transform.getModelMatrix();
		camera->Update(time->getDeltaTime(), playerModelMatrix);


		for (auto& [_, entity] : staticGameObjects)
		{
			entity->updateSelfAndChild();
		}

		// update player and camera
		PxTransform playerTransform = pScene->getVehicle().getTransform();
		glm::vec3 pos = glm::vec3(playerTransform.p.x, playerTransform.p.y, playerTransform.p.z);
		glm::quat rot = glm::quat(playerTransform.q.w, playerTransform.q.x, playerTransform.q.y, playerTransform.q.z);
		player->transform.setLocalPosition(pos);
		player->transform.setLocalRotation(rot);
		player->updateSelfAndChild();

		vehicle_position = glm::vec3(pos); // set position for imgui

		// -------------------------- rendering code -----------------------------
		renderer->SetCamera(camera->GetPosition());

		//glViewport(0, 0, width, height); //temporary testing for split camera

		// shadow pass
		shadowShader->use();
		shadowMapper->SetupUBO();
		glViewport(0, 0, shadowMapper->SHADOW_WIDTH, shadowMapper->SHADOW_HEIGHT);
		shadowMapper->BindShadowMap();
		//shadowMapper->BindFramebufferTextures();
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderShadowScene();
		shadowMapper->UnbindShadowMap();
		// end shadow pass

		// lighting pass
		//glViewport(0, 0, window->getWindowSize().x, window->getWindowSize().y);
		glViewport(0, 0, window->getFrameBufferSize().first, window->getFrameBufferSize().second);
		postProcessor->BindFBO();
		renderer->Clear(0.0f, 0.0f, 0.0f, 1.0f);
		RenderScene();
		// end lighting pass
		
		// draw physics colliders
		renderer->DrawCollisionDebug(camera->GetViewProjectionMatrix(), physicsShader, pScene->GetRenderBuffer());

		// post processing pass
		postProcessor->Blit();
		postProcessor->Unbind();
		renderer->Clear(0.0f, 0.0f, 0.0f, 1.0f);
		renderer->DrawQuad(postProcessShader, postProcessor);
		// end post processing

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

	shadowShader->use();
	shadowShader->setInt("depthMaps", 6);

	// setup default shader
	defaultShader->use();
	defaultShader->setVec3("u_light.position", &light->getPosition().x);
	defaultShader->setVec3("u_light.ambient", &light->getAmbient().r);
	defaultShader->setVec3("u_light.diffuse", &light->getDiffuse().r);
	defaultShader->setVec3("u_light.specular", &light->getSpecular().r);
	defaultShader->setInt("depthMaps", 6);

	// setup light shader
	lightShader->use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, light->getPosition());
	lightShader->setMat4("u_model", model);
	

	// setup skybox shader
	skyboxShader->use();
	skyboxShader->setInt("u_skybox", 0);
}

void Game::Cleanup()
{
	//grass.Cleanup();

	postProcessor->Cleanup();

	postProcessShader->Delete();
	shadowShader->Delete();
	defaultShader->Delete();
	defaultInstanceShader->Delete();
	lightShader->Delete();
	skyboxShader->Delete();
	skybox->Delete();
	physicsShader->Delete();

	pScene->Cleanup();

	std::cout << "Game cleaned up and exited successfully." << std::endl;
}

void Game::RenderShadowScene()
{
	glCullFace(GL_FRONT); // reduce peter panning

	int temp = 0;
	for (auto& [_, entity] : staticGameObjects)
	{
		Frustum frust = CreateFrustum(camera);
		
		entity->drawSelfAndChild(frust, renderer, shadowShader, true, temp);
	}

	shadowShader->setMat4("u_model", player->transform.getModelMatrix());
	player->drawSelfAndChild(CreateFrustum(camera), renderer, shadowShader, true, temp);

	glCullFace(GL_BACK);
}

void Game::RenderScene()
{
	glm::mat4 proj = camera->GetProjectionMatrix();
	glm::mat4 view = camera->GetViewMatrix();
	glm::vec3 cameraPos = camera->GetPosition();

	defaultShader->use();
	defaultShader->setMat4("u_view", camera->GetViewMatrix());
	defaultShader->setMat4("u_projection", camera->GetProjectionMatrix());
	defaultShader->setVec3("u_cameraPos", &cameraPos.x);
	defaultShader->setVec3("u_lightDir", &light->getDirection().x);
	defaultShader->setFloat("u_farPlane", camera->GetFarClipPlane());
	defaultShader->setInt("u_cascadeCount", shadowMapper->GetCascadeCount());
	for (size_t i = 0; i < shadowMapper->GetCascadeCount(); ++i)
	{
		defaultShader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowMapper->GetCascadeLevels()[i]);
	}

	glActiveTexture(GL_TEXTURE6);
	shadowMapper->BindDepthMapTexture();
	defaultShader->setInt("depthMaps", 6);
	
	Frustum frust = CreateFrustum(camera);

	int numDrawed = 0;
	for (auto& [_, entity] : staticGameObjects)
	{
		entity->drawSelfAndChild(frust, renderer, defaultShader, false, numDrawed);
	}

	defaultShader->setMat4("u_model", player->transform.getModelMatrix());
	player->drawSelfAndChild(frust, renderer, defaultShader, false, numDrawed);
	
	glm::mat4 projView = camera->GetProjectionMatrix() * glm::mat4(glm::mat3(camera->GetViewMatrix())); // remove translation from the view matrix
	renderer->DrawSkybox(projView, skyboxShader, skybox);
}

void Game::DrawGameObjectsInstanced(const std::vector<glm::mat4> modelMatrices, std::shared_ptr<Entity> entity)
{
	//renderer->DrawEntityInstanced(camera->GetViewProjectionMatrix(), defaultInstanceShader, entity, modelMatrices);
}
