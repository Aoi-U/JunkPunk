#include "RenderSystem.h"

#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Physics.h"
#include "../ECSController.h"

extern ECSController controller;

RenderSystem::RenderSystem()
{
	glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderSystem::Init()
{
	postProcessShader = std::make_shared<Shader>("assets/shaders/postProcess.vert", "assets/shaders/postProcess.frag");
	shadowShader = std::make_shared<Shader>("assets/shaders/shadowMap.vert", "assets/shaders/shadowMap.frag", "assets/shaders/shadowMap.geom");
	defaultShader = std::make_shared<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
	defaultInstanceShader = std::make_shared<Shader>("assets/shaders/defaultInstanced.vert", "assets/shaders/defaultInstanced.frag");
	lightShader = std::make_shared<Shader>("assets/shaders/light.vert", "assets/shaders/light.frag");
	skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
	physicsDebugShader = std::make_shared<Shader>("assets/shaders/colliders.vert", "assets/shaders/colliders.frag");
	textShader = std::make_shared<Shader>("assets/shaders/text.vert", "assets/shaders/text.frag");


	Entity player = controller.GetEntityByTag("VehicleCommands");
	auto& playerTransform = controller.GetComponent<Transform>(player);

	float radius = 5.0f;
	float heightOffset = 1.5f;
	float lerpSpeed = 3.0f;
	float horizontalLookSpeed = 5.0f;
	float verticalLookSpeed = 1.5f;
	float yaw = 0.0f;
	float pitch = 00.0f;
	float fov = 45.0f;
	float zNear = 0.1f;
	float zFar = 1000.0f;
	
	glm::vec3 cameraPos = playerTransform.position + glm::vec3(0.0f, heightOffset, -radius);
	glm::mat4 viewMatrix = glm::lookAt(cameraPos, playerTransform.position + glm::vec3(0.0f, heightOffset, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), 1280 / (float)720, zNear, zFar);

	Entity camera = controller.createEntity();
	controller.AddComponent(camera, ThirdPersonCamera{ radius, heightOffset, lerpSpeed, horizontalLookSpeed, verticalLookSpeed, yaw, pitch, fov, zNear, zFar, 1280, 720, viewMatrix, projectionMatrix });
	controller.AddComponent(camera, Transform{ cameraPos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f) });
	controller.AssignTag(camera, "Camera");

	controller.AddEventListener(Events::Window::RESIZED, [this](Event& e){this->RenderSystem::WindowSizeListener(e); });

	// setup skybox
	skybox = std::make_unique<Skybox>();
	skybox->Init();

	ShaderSetupDefaults();

	// setup text rendering
	text = Text();
	textVAO = VAO();
	textVBO = VBO();
	text.initVAO(&textVAO, &textVBO);
	text.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
	textShader->use();
	textShader->setMat4("u_projection", text.projMat);

	// setup shadow mapper
	light = Light();
	shadowMapper = std::make_unique<ShadowMapper>(screenWidth / (float)screenHeight, zNear, zFar, glm::radians(fov), viewMatrix, light);
	shadowMapper->Init(shadowShader, defaultShader);

	// setup post processor
	postProcessor = std::make_unique<PostProcessor>(1280, 720);

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

void RenderSystem::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderSystem::Update(float fps, const PxRenderBuffer& buffer)
{
	DrawShadowPass();

	DrawLightingPass();

	// draw physics colliders
	DrawCollisionDebug(buffer);

	DrawPostProcessingPass();

	RenderText(std::to_string((int)fps) + " fps", 0.05f, 0.9f, 0.7f, glm::vec3(0.5f, 0.8f, 0.2f), text.charArial);
}


void RenderSystem::DrawShadowPass()
{
	// setup shadow mapping and shaders
	shadowShader->use();
	shadowMapper->SetupUBO();
	glViewport(0, 0, shadowMapper->SHADOW_WIDTH, shadowMapper->SHADOW_HEIGHT);
	shadowMapper->BindShadowMap();
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT); 

	// draw entities
	for (auto& entity : entities)
	{
		auto& renderComp = controller.GetComponent<Render>(entity);
		auto& transformComp = controller.GetComponent<Transform>(entity);
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), transformComp.position);
		glm::mat4 rotation = glm::mat4_cast(transformComp.quatRotation);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), transformComp.scale);
		glm::mat4 model = translation * rotation * scale;

		shadowShader->setMat4("u_model", model);
		for (auto& mesh : renderComp.model->getMeshes())
		{
			mesh.BindVao();
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);
			mesh.UnbindVao();
		}
	}
	shadowMapper->UnbindShadowMap();
	glCullFace(GL_BACK);
	glViewport(0, 0, screenWidth, screenHeight);
}

void RenderSystem::DrawLightingPass()
{
	// set post processing framebuffer
	postProcessor->BindFBO();
	Clear(0.0f, 0.0f, 0.0f, 1.0f);
	
	Entity camera = controller.GetEntityByTag("Camera");
	
	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);

	// create cameras frustum for frustum culling
	glm::vec3 forward = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[2]));
	glm::vec3 right = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[0]));
	glm::vec3 up = glm::normalize(glm::vec3(glm::inverse(tpp.viewMatrix)[1]));
	Frustum frust = CreateFrustum(tpp.zFar, tpp.zNear, tpp.fov, tpp.screenWidth / (float)tpp.screenHeight, -forward, right, up, glm::vec3(glm::inverse(tpp.viewMatrix)[3]));

	// setup uniforms
	glm::vec3 pos = glm::vec3(glm::inverse(tpp.viewMatrix)[3]);
	glm::vec3 lightDir = light.getDirection();

	defaultShader->use();
	defaultShader->setMat4("u_projection", tpp.getProjectionMatrix());
	defaultShader->setMat4("u_view", tpp.viewMatrix);
	defaultShader->setVec3("u_cameraPos", &pos.x);
	defaultShader->setVec3("u_lightDir", &lightDir.x);
	defaultShader->setFloat("u_farPlane", tpp.zFar);
	defaultShader->setInt("u_cascadeCount", shadowMapper->GetCascadeCount());
	for (size_t i = 0; i < shadowMapper->GetCascadeCount(); ++i)
	{
		defaultShader->setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowMapper->GetCascadeLevels()[i]);
	}
	glActiveTexture(GL_TEXTURE6);
	shadowMapper->BindDepthMapTexture();

	// draw entities
	for (auto& entity : entities)
	{
		auto& renderComp = controller.GetComponent<Render>(entity);
		auto& transformComp = controller.GetComponent<Transform>(entity);
		
		// check if entity is visible
		if (!renderComp.boundingVolume->isOnFrustum(frust, transformComp))
		{
			continue;
		}

		glm::mat4 translation = glm::translate(glm::mat4(1.0f), transformComp.position);
		glm::mat4 rotation = glm::mat4_cast(transformComp.quatRotation);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), transformComp.scale);
		glm::mat4 model = translation * rotation * scale;

		defaultShader->setMat4("u_model", model);

		for (auto& mesh : renderComp.model->getMeshes())
		{
			// bind textures
			BindTextures(mesh);
			mesh.BindVao();
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);
			mesh.UnbindVao();
		}
	}

	DrawSkybox();
}

void RenderSystem::DrawPostProcessingPass()
{
	// setup post processing
	postProcessor->Blit();
	postProcessor->Unbind();
	Clear(0.0f, 0.0f, 0.0f, 1.0f);
	postProcessShader->use();

	// render quad
	postProcessor->BindVAO();
	postProcessor->BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderSystem::RenderText(std::string text, float x, float y, float scale, glm::vec3 color, std::map<char, Character> characters)
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
		Character ch = characters[*c];
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

//void Renderer::DrawEntityInstanced(const glm::mat4& projView, std::shared_ptr<Shader> shader, Model* model, const std::vector<glm::mat4>& matrices)
//{
//	shader->use();
//	shader->setMat4("u_projView", projView);
//	shader->setVec3("u_cameraPos", &cameraPos.x);
//	// draw each mesh in the entity's model
//	for (Mesh& mesh : model->getMeshes())
//	{
//		BindTextures(mesh, shader);
//
//		mesh.BindVao();
//		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(matrices.size()));
//
//		mesh.UnbindVao();
//		glActiveTexture(GL_TEXTURE0);
//	}
//}

void RenderSystem::DrawSkybox()
{
	glDepthFunc(GL_LEQUAL); // change depth function for skybox

	Entity camera = controller.GetEntityByTag("Camera");
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
	Entity camera = controller.GetEntityByTag("Camera");
	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);

	glm::mat4 projView = tpp.getProjectionMatrix() * tpp.viewMatrix;

	physicsDebugShader->use();
	physicsDebugShader->setMat4("u_projView", projView);
	
	PxU32 nbLines = renderBuffer.getNbLines();
	PxDebugLine* lines = const_cast<PxDebugLine*>(renderBuffer.getLines());

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

	defaultShader->setBool("hasDiffuseTex", mesh.hasDiffuseTexture());
	defaultShader->setBool("hasSpecularTex", mesh.hasSpecularTexture());
	defaultShader->setBool("hasNormalTex", mesh.hasNormalTexture());
	defaultShader->setBool("hasHeightTex", mesh.hasHeightTexture());

	// bind each texture for the model
	for (unsigned int i = 0; i < mesh.getTextures().size(); i++)
	{
		std::string number;
		std::string name = mesh.getTextures()[i].getType();

		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); // transfer unsigned int to string
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to string
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // transfer unsigned int to string

		//shader->setInt(("material." + name + number).c_str(), i);
		
		defaultShader->setInt((name + number).c_str(), i); // set the texture unit in the shader

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
	defaultShader->use();
	defaultShader->setVec3("u_light.position", &light.getPosition().x);
	defaultShader->setVec3("u_light.ambient", &light.getAmbient().r);
	defaultShader->setVec3("u_light.diffuse", &light.getDiffuse().r);
	defaultShader->setVec3("u_light.specular", &light.getSpecular().r);
	defaultShader->setInt("depthMaps", 6);

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

	Entity camera = controller.GetEntityByTag("Camera");
	auto& tpp = controller.GetComponent<ThirdPersonCamera>(camera);
	shadowMapper->Update(screenWidth / (float)screenHeight, tpp.zNear, tpp.zFar, glm::radians(tpp.fov), tpp.viewMatrix);

	postProcessor->Resize(screenWidth, screenHeight);

	text.projMat = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
}
