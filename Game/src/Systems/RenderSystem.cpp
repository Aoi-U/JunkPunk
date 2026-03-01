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
	camera = controller.GetEntityByTag("Camera");
	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);
	zNear = tpp.zNear;
	zFar = tpp.zFar;
	fov = tpp.fov;
	viewMatrix = tpp.viewMatrix;
	shadowMapper->Update(screenWidth / (float)screenHeight, zNear, zFar, glm::radians(fov), viewMatrix);
	bananaIconTexture = std::make_unique<Texture>("banana.png");
	bananaIconTexture->Load("assets/UI");
	boostIconTexture = std::make_unique<Texture>("flash.png");
	boostIconTexture->Load("assets/UI");

	float uiVertices[] = {
		// positions    // texcoords
		-1.0f,  1.0f,    0.0f, 1.0f,
		-1.0f, -1.0f,    0.0f, 0.0f,
		 1.0f, -1.0f,    1.0f, 0.0f,

		-1.0f,  1.0f,    0.0f, 1.0f,
		 1.0f, -1.0f,    1.0f, 0.0f,
		 1.0f,  1.0f,    1.0f, 1.0f
	};

	glGenVertexArrays(1, &uiVAO);
	glGenBuffers(1, &uiVBO);

	glBindVertexArray(uiVAO);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uiVertices), uiVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
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

void RenderSystem::Update(float fps, const PxRenderBuffer& buffer)
{
	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);
	glm::vec3 forward = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[2]));
	glm::vec3 right = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[0]));
	glm::vec3 up = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[1]));
	glm::vec3 pos = glm::vec3(glm::inverse(tpp.viewMatrix)[3]);
	Frustum frust = CreateFrustum(tpp.zFar, tpp.zNear, glm::radians(tpp.fov), tpp.screenWidth / (float)tpp.screenHeight, -forward, right, up, glm::vec3(glm::inverse(tpp.viewMatrix)[3]));

	DrawShadowPass(frust);

	DrawLightingPass(frust, tpp, pos);

	DrawSkybox();

	particleRenderSystem->Update(tpp);

	// draw physics colliders
	DrawCollisionDebug(buffer);

	DrawPostProcessingPass();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RenderPowerupUI();
	glEnable(GL_DEPTH_TEST);
	
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
	glCullFace(GL_FRONT); 

	std::unordered_map<Model*, std::vector<glm::mat4>> instancedModels;
	shadowShader->setBool("u_isInstanced", false);


	// draw entities
	for (auto& entity : entities)
	{
		auto& renderComp = controller.GetComponent<Render>(entity);
		auto& transformComp = controller.GetComponent<Transform>(entity);

		//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), transformComp.position);
		//modelMatrix = modelMatrix * glm::mat4_cast(transformComp.quatRotation);
		//modelMatrix = glm::scale(modelMatrix, transformComp.scale);
		glm::mat3 rot = glm::mat3_cast(transformComp.quatRotation);
		rot[0] *= transformComp.scale.x;
		rot[1] *= transformComp.scale.y;
		rot[2] *= transformComp.scale.z;
		glm::mat4 modelMatrix(rot);
		modelMatrix[3] = glm::vec4(transformComp.position, 1.0f);

		bool isVisible = renderComp.boundingVolume->isOnFrustum(frust, modelMatrix);

	
		// only using culling for instanced models in shadow pass 
		if (renderComp.isInstanced)
		{
			if (isVisible)
			{
				instancedModels[renderComp.model.get()].push_back(modelMatrix);
			}
			continue;
		}

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
		
		/*glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), transformComp.position);
		modelMatrix = modelMatrix * glm::mat4_cast(transformComp.quatRotation);
		modelMatrix = glm::scale(modelMatrix, transformComp.scale);*/
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

	Mesh* previousMesh = nullptr;
	for (auto& [model, matrices] : instancedModels) // loop through each unique model that is instanced
	{
		for (Mesh& mesh : model->getMeshes()) // instance render each mesh in the model
		{
			if (previousMesh != &mesh) // only bind textures if the mesh is different from the previous one since instanced meshes will share the same texture
			{
				// bind textures
				BindTextures(mesh);
				previousMesh = &mesh;
			}

			//mesh.UpdateInstanceBuffer(matrices);
			//mesh.SetupInstanceMesh();
			mesh.BindVao();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(matrices.size()));
			mesh.UnbindVao();
		}
	}

	//std::cout << "Culled entities this frame: " << cullCount << std::endl;
}

void RenderSystem::DrawPostProcessingPass()
{
	// setup post processing
	postProcessor->Blit();
	postProcessor->Unbind();
	Clear(0.0f, 0.0f, 0.0f, 1.0f);
	postProcessShader->use();
	// set tint uniform
	postProcessShader->setVec3("u_tintColor", &tintColor.x);

	// render quad
	postProcessor->BindVAO();
	postProcessor->BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);
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

void RenderSystem::DrawSkybox()
{
	glDepthFunc(GL_LEQUAL); // change depth function for skybox

	auto& cameraComp = controller.GetComponent<ThirdPersonCamera>(camera);

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

void RenderSystem::DrawCollisionDebug(const PxRenderBuffer& renderBuffer)
{
	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);

	glm::mat4 projView = tpp.getProjectionMatrix() * tpp.viewMatrix;

	physicsDebugShader->use();
	physicsDebugShader->setMat4("u_projView", projView);
	
	PxU32 nbLines = renderBuffer.getNbLines();
	const PxDebugLine* lines = renderBuffer.getLines();

	if (nbLines == 0)
		return;

	if (nbLines > maxDebugLines)
	{
		std::cout << "Too many debug lines. Some lines will not be rendered" << std::endl;
		std::cout << "	Attemping to render " << nbLines << " lines." << std::endl;
		nbLines = maxDebugLines;
	}

	glBindBuffer(GL_ARRAY_BUFFER, debugVbo);
	GLfloat* vertexData = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	for (PxU32 i = 0; i < nbLines; i++)
	{
		const PxDebugLine& line = lines[i];
		glm::vec3 col0 = glm::vec3(
			((line.color0 >> 16) & 0xFF) / 255.0f,
			((line.color0 >> 8) & 0xFF) / 255.0f,
			(line.color0 & 0xFF) / 255.0f
		);
		glm::vec3 col1 = glm::vec3(
			((line.color1 >> 16) & 0xFF) / 255.0f,
			((line.color1 >> 8) & 0xFF) / 255.0f,
			(line.color1 & 0xFF) / 255.0f
		);
		// first vertex
		vertexData[i * 12 + 0] = line.pos0.x;
		vertexData[i * 12 + 1] = line.pos0.y;
		vertexData[i * 12 + 2] = line.pos0.z;
		vertexData[i * 12 + 3] = col0.x;
		vertexData[i * 12 + 4] = col0.y;
		vertexData[i * 12 + 5] = col0.z;

		// second vertex
		vertexData[i * 12 + 6] = line.pos1.x;
		vertexData[i * 12 + 7] = line.pos1.y;
		vertexData[i * 12 + 8] = line.pos1.z;
		vertexData[i * 12 + 9] = col1.x;
		vertexData[i * 12 + 10] = col1.y;
		vertexData[i * 12 + 11] = col1.z;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindVertexArray(debugVao);
	glDrawArrays(GL_LINES, 0, nbLines * 2);
	glBindVertexArray(0);
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
	if (!(camera == MAX_ENTITIES))
	{
		auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);

		zNear = tpp.zNear;
		zFar = tpp.zFar;
		fov = tpp.fov;
		viewMatrix = tpp.viewMatrix;
	}

	shadowMapper->Update(screenWidth / (float)screenHeight, zNear, zFar, glm::radians(fov), viewMatrix);

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

void RenderSystem::drawUI(GLuint textureID,
	float x0_px, float y0_px,
	float x1_px, float y1_px,
	int layerIndex)
{
	postProcessShader->use();
	float screenW = (float)screenWidth;
	float screenH = (float)screenHeight;

	auto pxToNdcX = [&](float x) { return (x / screenW) * 2.0f - 1.0f; };
	auto pxToNdcY = [&](float y) { return (y / screenH) * 2.0f - 1.0f; };

	float x0 = pxToNdcX(x0_px);
	float y0 = pxToNdcY(y0_px);
	float x1 = pxToNdcX(x1_px);
	float y1 = pxToNdcY(y1_px);

	float vertices[] = {
	x0, y1,  0.0f, 0.0f,
	x0, y0,  0.0f, 1.0f,
	x1, y0,  1.0f, 1.0f,

	x0, y1,  0.0f, 0.0f,
	x1, y0,  1.0f, 1.0f,
	x1, y1,  1.0f, 0.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glBindVertexArray(uiVAO);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}

void RenderSystem::RenderPowerupUI() {
	Entity player = controller.GetEntityByTag("VehicleCommands");

	if (!controller.HasComponent<Powerup>(player))
		return;

	auto& p = controller.GetComponent <Powerup>(player);

	GLuint textureID = 0;

	if (p.type == 1)
		textureID = boostIconTexture->getID();
	else if (p.type == 2)
		textureID = bananaIconTexture->getID();

	if (textureID == 0)
		return;

	float iconSize = 96.0f;
	float margin = 20.0f;

	float x0 = (screenWidth * 0.5f) - (iconSize * 0.5f);
	float y0 = screenHeight - margin - iconSize;
	float x1 = x0 + iconSize;
	float y1 = y0 + iconSize;

	drawUI(textureID, x0, y0, x1, y1, 5);
}