#include "RenderSystem.h"

#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/Particles.h"
#include "../ECSController.h"

extern ECSController controller;

RenderSystem::RenderSystem()
{
	glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	postProcessShader = std::make_shared<Shader>("assets/shaders/postProcess.vert", "assets/shaders/postProcess.frag");
	shadowShader = std::make_shared<Shader>("assets/shaders/shadowMap.vert", "assets/shaders/shadowMap.frag", "assets/shaders/shadowMap.geom");
	defaultShader = std::make_shared<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
	defaultInstanceShader = std::make_shared<Shader>("assets/shaders/defaultInstanced.vert", "assets/shaders/defaultInstanced.frag");
	lightShader = std::make_shared<Shader>("assets/shaders/light.vert", "assets/shaders/light.frag");
	skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
	physicsDebugShader = std::make_shared<Shader>("assets/shaders/colliders.vert", "assets/shaders/colliders.frag");
	textShader = std::make_shared<Shader>("assets/shaders/text.vert", "assets/shaders/text.frag");
	uiShader = std::make_shared<Shader>("assets/shaders/ui.vert", "assets/shaders/ui.frag");

	controller.AddEventListener(Events::Window::RESIZED, [this](Event& e){this->RenderSystem::WindowSizeListener(e); });
	controller.AddEventListener(Events::GameState::NEW_STATE, [this](Event& e) {this->RenderSystem::ChangeGameStateListener(e); });

	// setup skybox
	skybox = std::make_unique<Skybox>();
	skybox->Init();
	ShaderSetupDefaults();
	// setup text rendering
	fonts = Text();
	textVAO = VAO();
	textVBO = VBO();
	fonts.initVAO(&textVAO, &textVBO);
	fonts.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
	textShader->use();
	textShader->setMat4("u_projection", fonts.projMat);

	particleRenderSystem = controller.RegisterSystem<ParticleRenderSystem>();
	{
		Signature signature;
		signature.set(controller.GetComponentType<ParticleEmitter>());
		controller.SetSystemSignature<ParticleRenderSystem>(signature);
	}

	// setup post processor
	postProcessor = std::make_unique<PostProcessor>(1280, 720);


	shadowMapper = std::make_unique<ShadowMapper>(1280 / (float)720, 0.1f, 800.0f, glm::radians(45.0f), glm::mat4(1.0f), light);
	shadowMapper->Init(shadowShader, defaultInstanceShader);
	light = Light();

	// setup debug line buffers
	glGenVertexArrays(1, &debugVao);
	glGenBuffers(1, &debugVbo);
	glBindVertexArray(debugVao);
	glBindBuffer(GL_ARRAY_BUFFER, debugVbo);

	glBufferData(GL_ARRAY_BUFFER, maxDebugLines * 2 * sizeof(GLfloat) * 6, nullptr, GL_DYNAMIC_DRAW); // line: 2 vertices, 3 position 3 color
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);
	// end setup debug line buffers			
}

void RenderSystem::Init()
{
	// setup shadow mapper
	camera = controller.GetEntityByTag("Camera1");
	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);
	
	viewMatrix = tpp.viewMatrix;
	shadowMapper->Update(screenWidth / (float)screenHeight, tpp.zNear, tpp.zFar, glm::radians(tpp.fov), viewMatrix);

	bananaIconTexture = std::make_unique<Texture>("banana.png");
	bananaIconTexture->Load("assets/UI");
	boostIconTexture = std::make_unique<Texture>("flash.png");
	boostIconTexture->Load("assets/UI");
	bombIconTexture = std::make_unique<Texture>("bomb.png");
	bombIconTexture->Load("assets/UI");

	uiVAO = VAO();
	uiVBO = VBO();
	uiVAO.Bind();
	uiVBO.Bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	uiVAO.Unbind();
	uiVBO.Unbind();

	uiShader->use();
	uiShader->setMat4("u_projection", fonts.projMat);
	uiShader->setInt("u_texture", 0);
}

void RenderSystem::Reset()
{
	camera = MAX_ENTITIES;
}

void RenderSystem::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderSystem::Update(float fps)
{
	// Clear the default screen buffer ONCE at the start of the frame
	Clear(0.0f, 0.0f, 0.0f, 1.0f);

	int numPlayers = cameraEntities.size();
	for (int i = 0; i < numPlayers; i++)
	{
		auto& tpp = controller.GetComponent<ThirdPersonCamera>(cameraEntities[i]);

		// Calculate split-screen viewport
		int vx = 0, vy = 0, vw = screenWidth, vh = screenHeight;
		if (numPlayers == 2) {
			// Horizontal split-screen (Top / Bottom)
			vw = screenWidth;
			vh = screenHeight / 2;
			vy = (i == 0) ? vh : 0; // Player 1 on top, Player 2 on bottom
		}
		else if (numPlayers > 2) {
			// 4-way split-screen
			vw = screenWidth / 2;
			vh = screenHeight / 2;
			vx = (i % 2) * vw;
			vy = (i < 2) ? vh : 0;
		}

		float aspect = vw / (float)vh;

		// Update camera aspect ratio for proper projection matrix calculation
		tpp.screenWidth = (unsigned int)vw;
		tpp.screenHeight = (unsigned int)vh;

		glm::vec3 forward = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[2]));
		glm::vec3 right = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[0]));
		glm::vec3 up = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[1]));
		glm::vec3 pos = glm::vec3(glm::inverse(tpp.viewMatrix)[3]);
		Frustum frust = CreateFrustum(tpp.zFar, tpp.zNear, glm::radians(tpp.fov), aspect, -forward, right, up, glm::vec3(glm::inverse(tpp.viewMatrix)[3]));

		shadowMapper->Update(aspect, tpp.zNear, tpp.zFar, glm::radians(tpp.fov), tpp.viewMatrix);
		DrawShadowPass(frust);

		// DrawLightingPass uses its own postProcessor FBO that cleans itself
		DrawLightingPass(frust, tpp, pos);
		DrawSkybox(tpp);
		particleRenderSystem->Update(tpp);

		// Draw the finished FBO to the current player's quadrant on the screen
		DrawPostProcessingPass(vx, vy, vw, vh);
		RenderPowerupUI(tpp.playerEntity, vx, vy, vw, vh);
	}

	// Reset back to full viewport for debug text, UI overlays, etc.
	glViewport(0, 0, screenWidth, screenHeight);

	
	int f = static_cast<int>(fps);
	std::string text = "fps: " + std::to_string(f);
	RenderText(text, 0.05f, 0.9f, 0.7f, glm::vec3(0.5f, 0.8f, 0.2f));
}

void RenderSystem::DrawShadowPass(const Frustum& frust)
{
	// setup shadow mapping and shaders
	shadowShader->use();
	shadowMapper->SetupUBO();
	shadowMapper->BindShadowMap();
	glViewport(0, 0, shadowMapper->SHADOW_WIDTH, shadowMapper->SHADOW_HEIGHT);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_FRONT); 
	glCullFace(GL_BACK);

	std::unordered_map<Model*, std::vector<glm::mat4>> instancedModels;
	shadowShader->setBool("u_isInstanced", false);

	// draw entities
	for (auto& entity : entities)
	{
		auto& renderComp = controller.GetComponent<Render>(entity);
		auto& transformComp = controller.GetComponent<Transform>(entity);

		glm::mat3 rot = glm::mat3_cast(transformComp.quatRotation);
		rot[0] *= transformComp.scale.x;
		rot[1] *= transformComp.scale.y;
		rot[2] *= transformComp.scale.z;
		glm::mat4 modelMatrix(rot);
		modelMatrix[3] = glm::vec4(transformComp.position, 1.0f);

		if (renderComp.isInstanced)
		{
			if (renderComp.boundingVolume->isOnFrustum(frust, modelMatrix))
			{
				instancedModels[renderComp.model.get()].push_back(modelMatrix);
			}
			continue;
		}

		// render non-instanced entities normally
		shadowShader->setMat4("u_model", modelMatrix);
		for (auto& mesh : renderComp.model->getMeshes())
		{
			mesh.BindVao();
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);
			mesh.UnbindVao();
		}
	}

	// draw instanced models
	shadowShader->setBool("u_isInstanced", true);
	for (auto& [model, matrices] : instancedModels)
	{
		for (auto& mesh : model->getMeshes())
		{
			mesh.UpdateInstanceBuffer(matrices);
			mesh.SetupInstanceMesh();
			mesh.BindVao();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(matrices.size()));
			mesh.UnbindVao();
		}
	}

	shadowMapper->UnbindShadowMap();
	glCullFace(GL_BACK);
	glViewport(0, 0, screenWidth, screenHeight);
}

void RenderSystem::DrawLightingPass(const Frustum& frust, const ThirdPersonCamera& tpp, glm::vec3 pos)
{
	//int cullCount = 0;
	// set post processing framebuffer
	postProcessor->BindFBO();
	Clear(0.0f, 0.0f, 0.0f, 1.0f);
		
	// setup uniforms
	glm::vec3 lightDir = light.getDirection();

	defaultInstanceShader->use();
	defaultInstanceShader->setMat4("u_projection", tpp.getProjectionMatrix());
	defaultInstanceShader->setMat4("u_view", tpp.viewMatrix);
	defaultInstanceShader->setVec3("u_cameraPos", &pos.x);
	defaultInstanceShader->setVec3("u_lightDir", &lightDir.x);
	defaultInstanceShader->setFloat("u_farPlane", tpp.zFar);
	defaultInstanceShader->setInt("u_cascadeCount", shadowMapper->GetCascadeCount());
	for (size_t i = 0; i < shadowMapper->GetCascadeCount(); ++i)
	{
		defaultInstanceShader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowMapper->GetCascadeLevels()[i]);
	}
	glActiveTexture(GL_TEXTURE6);
	shadowMapper->BindDepthMapTexture();

	std::unordered_map<Model*, std::vector<glm::mat4>> instancedModels;
	defaultInstanceShader->setBool("u_isInstanced", false);

	// draw entities
	for (auto& entity : entities)
	{
		auto& renderComp = controller.GetComponent<Render>(entity);
		auto& transformComp = controller.GetComponent<Transform>(entity);

		glm::mat3 rot = glm::mat3_cast(transformComp.quatRotation);
		rot[0] *= transformComp.scale.x;
		rot[1] *= transformComp.scale.y;
		rot[2] *= transformComp.scale.z;
		glm::mat4 modelMatrix(rot);
		modelMatrix[3] = glm::vec4(transformComp.position, 1.0f);

		bool isVisible = renderComp.boundingVolume->isOnFrustum(frust, modelMatrix);

		if (isVisible)
		{
			if (renderComp.isInstanced)
			{
				instancedModels[renderComp.model.get()].push_back(modelMatrix);
				continue;
			}

			
			defaultInstanceShader->setBool("u_useFlatColor", renderComp.useFlatColor);
			defaultInstanceShader->setVec3("u_flatColor", &renderComp.flatColor.x);

			// render non-instanced entities normally
			defaultInstanceShader->setMat4("u_model", modelMatrix);

			for (auto& mesh : renderComp.model->getMeshes())
			{
				// bind textures
				BindTextures(mesh);
				mesh.BindVao();
				glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);
				mesh.UnbindVao();
			}
		}
		//cullCount++;
	}

	// draw instanced entities
	defaultInstanceShader->setBool("u_isInstanced", true);
	defaultInstanceShader->setBool("u_useFlatColor", false); // instanced path uses normal material flow

	Mesh* previousMesh = nullptr;
	for (auto& [model, matrices] : instancedModels)
	{
		for (Mesh& mesh : model->getMeshes())
		{
			if (previousMesh != &mesh)
			{
				BindTextures(mesh);
				previousMesh = &mesh;
			}

			mesh.UpdateInstanceBuffer(matrices);
			mesh.SetupInstanceMesh();

			mesh.BindVao();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(matrices.size()));
			mesh.UnbindVao();
		}
	}

	//std::cout << "Culled entities this frame: " << cullCount << std::endl;
}

void RenderSystem::DrawPostProcessingPass(int vx, int vy, int vw, int vh)
{
	// setup post processing
	postProcessor->Blit();
	postProcessor->Unbind(); // return to default framebuffer

	// Apply viewport specific to this player's screen quadrant
	glViewport(vx, vy, vw, vh);

	// Remov clear step here! We don't want to erase other players' quadrants.

	postProcessShader->use();
	// set tint uniform
	postProcessShader->setVec3("u_tintColor", &tintColor.x);

	// render quad
	postProcessor->BindVAO();
	postProcessor->BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderSystem::DrawUI(Texture* tex, float x0_px, float y0_px, float x1_px, float y1_px, int layerIndex)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uiShader->use();
	glm::vec4 tempColor = glm::vec4(1.0f);
	uiShader->setVec4("u_color", &tempColor.x);
	uiShader->setInt("u_useTexture", 1);
	uiShader->setMat4("u_projection", fonts.projMat);

	float vertices[] = {
		x0_px, y1_px,  0.0f, 0.0f,
		x0_px, y0_px,  0.0f, 1.0f,
		x1_px, y0_px,  1.0f, 1.0f,
		x0_px, y1_px,  0.0f, 0.0f,
		x1_px, y0_px,  1.0f, 1.0f,
		x1_px, y1_px,  1.0f, 0.0f
	};

	tex->Bind(GL_TEXTURE0);
	uiVAO.Bind();
	uiVBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	uiVAO.Unbind();
	tex->Unbind();
	glEnable(GL_DEPTH_TEST);
}

void RenderSystem::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	textShader->use();
	textShader->setVec3("textColor", &color.x);
	textShader->setMat4("u_projection", fonts.projMat);
	glActiveTexture(GL_TEXTURE0);
	textVAO.Bind();

	// iterate through all characters
	std::string::const_iterator c;

	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = fonts.charArial[*c];
		float xPos = x + ch.bearing.x * scale;
		float yPos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;	

		// update vbo for each character
		float vertices[6][4] =
		{
			{xPos,			yPos + h, 0.0f, 0.0f},
			{xPos,			yPos,			0.0f, 1.0f},
			{xPos + w,	yPos,			1.0f, 1.0f},

			{xPos,			yPos + h, 0.0f, 0.0f},
			{xPos + w,	yPos,			1.0f, 1.0f},
			{xPos + w,	yPos + h,	1.0f, 0.0f}
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textID);
		// update content of VBO memory
		textVBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		textVBO.Unbind();
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// advance cursors for next glyph (advance is number of 1/64 pixels
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}

	textVAO.Unbind();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystem::RenderPowerupUI(Entity player, int vx, int vy, int vw, int vh)
{
	if (!controller.HasComponent<Powerup>(player))
		return;

	auto& p = controller.GetComponent<Powerup>(player);

	Texture* tex = nullptr;

	if (p.type == 1)
		tex = boostIconTexture.get();
	else if (p.type == 2)
		tex = bananaIconTexture.get();
	else if (p.type == 3)
		tex = bombIconTexture.get();

	if (tex == nullptr)
		return;

	float iconSize = 96.0f;
	float margin = 20.0f;

	float x0 = (screenWidth * 0.5f) - (iconSize * 0.5f);
	float y0 = screenHeight - margin - iconSize;
	float x1 = x0 + iconSize;
	float y1 = y0 + iconSize;

	DrawUI(tex, x0, y0, x1, y1, 5);
}

void RenderSystem::DrawSkybox(const ThirdPersonCamera& cameraComp)
{
	glDepthFunc(GL_LEQUAL); // change depth function for skybox

	glm::mat4 projView = cameraComp.getProjectionMatrix() * glm::mat4(glm::mat3(cameraComp.viewMatrix)); // remove translation from view matrix

	skyboxShader->use();
	skyboxShader->setMat4("u_projView", projView);

	// bind skybox VAO and cubemap texture
	skybox->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetCubemapTexture());
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	skybox->Unbind();

	glDepthFunc(GL_LESS); // reset depth function
}

void RenderSystem::BindTextures(Mesh& mesh)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	// set uniforms
	defaultInstanceShader->setBool("hasDiffuseTex", mesh.hasDiffuseTexture());
	defaultInstanceShader->setBool("hasSpecularTex", mesh.hasSpecularTexture());
	defaultInstanceShader->setBool("hasNormalTex", mesh.hasNormalTexture());
	defaultInstanceShader->setBool("hasHeightTex", mesh.hasHeightTexture());

	// bind each texture for the model
	std::vector<Texture>& textures = mesh.getTextures();
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		const std::string& type = textures[i].getType();
		const char* uniformName = nullptr;
		if (type == "texture_diffuse")
		{
			if (diffuseNr == 1) uniformName = "texture_diffuse1";
			else if (diffuseNr == 2) uniformName = "texture_diffuse2";
			else if (diffuseNr == 3) uniformName = "texture_diffuse3";
			diffuseNr++;
		}
		else if (type == "texture_specular")
		{
			if (specularNr == 1) uniformName = "texture_specular1";
			else if (specularNr == 2) uniformName = "texture_specular2";
			else if (specularNr == 3) uniformName = "texture_specular3";
			specularNr++;
		}
		else if (type == "texture_normal")
		{
			if (normalNr == 1) uniformName = "texture_normal1";
			else if (normalNr == 2) uniformName = "texture_normal2";
			else if (normalNr == 3) uniformName = "texture_normal3";
			normalNr++;

		}
		else if (type == "texture_height")
		{
			if (heightNr == 1) uniformName = "texture_height1";
			else if (heightNr == 2) uniformName = "texture_height2";
			else if (heightNr == 3) uniformName = "texture_height3";
			heightNr++;
		}

		if (uniformName)
		{
			defaultInstanceShader->setInt(uniformName, i); // set the texture unit in the shader
		}
	
		mesh.getTextures()[i].Bind(GL_TEXTURE0 + i); // activate and bind texture
	}
}

void RenderSystem::ShaderSetupDefaults()
{
	// post processor
	postProcessShader->use();
	postProcessShader->setInt("screenTexture", 0);

	shadowShader->use();
	shadowShader->setInt("depthMaps", 6);

	// setup default shader
	defaultInstanceShader->use();
	defaultInstanceShader->setVec3("u_light.position", &light.getPosition().x);
	defaultInstanceShader->setVec3("u_light.ambient", &light.getAmbient().r);
	defaultInstanceShader->setVec3("u_light.diffuse", &light.getDiffuse().r);
	defaultInstanceShader->setVec3("u_light.specular", &light.getSpecular().r);
	defaultInstanceShader->setInt("depthMaps", 6);

	glm::vec3 white(1.0f, 1.0f, 1.0f);
	defaultInstanceShader->setBool("u_useFlatColor", false);
	defaultInstanceShader->setVec3("u_flatColor", &white.x);

	// setup light shader
	lightShader->use();
	glm::mat4 model = glm::translate(glm::mat4(1.0f), light.getPosition());
	lightShader->setMat4("u_model", model);

	// setup skybox shader
	skyboxShader->use();
	skyboxShader->setInt("u_skybox", 0);
}

void RenderSystem::WindowSizeListener(Event& e)
{
	screenWidth = e.GetParam<unsigned int>(Events::Window::Resized::WIDTH);
	screenHeight = e.GetParam<unsigned int>(Events::Window::Resized::HEIGHT);
	//if (!(camera == MAX_ENTITIES))
	//{
	//	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);
	//	shadowMapper->Update(screenWidth / (float)screenHeight, tpp.zNear, tpp.zFar, glm::radians(tpp.fov), tpp.viewMatrix);
	//}

	postProcessor->Resize(screenWidth, screenHeight);

	fonts.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
}

void RenderSystem::ChangeGameStateListener(Event& e)
{
	GameState newState = e.GetParam<GameState>(Events::GameState::New_State::STATE);
	if (newState == GameState::PAUSED)
	{
		tintColor = glm::vec3(0.3f, 0.3f, 0.3f);
	}
	else if (newState == GameState::GAME ||  newState == GameState::RESTART)
	{
		tintColor = glm::vec3(1.0f, 1.0f, 1.0f);
	}
}

void RenderSystem::RenderDistanceToFinish(float distance)
{
	int d = static_cast<int>(distance);
	std::string text = "Distance to finish: " + std::to_string(d) + "m";
	RenderText(text, 20.0f, screenHeight - 40.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
}