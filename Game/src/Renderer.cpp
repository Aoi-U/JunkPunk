#include "Renderer.h"

Renderer::Renderer(std::shared_ptr<InputManager> inputMgr)
	: inputManager(inputMgr)
{
	glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
	glEnable(GL_CULL_FACE);

	/*glEnable(GL_POLYGON_OFFSET_FILL);
	float factor = 1.0f;
	float units = 1.0f;
	glPolygonOffset(factor, units);*/

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//Renderer::~Renderer()
//{
//
//}

void Renderer::Init()
{
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

}

void Renderer::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::DrawEntity(const glm::mat4& proj, const glm::mat4& view, std::shared_ptr<Shader> shader, Entity& entity)
{	
	// draw each mesh in the entity's model
	for (Mesh& mesh : entity.getModel()->getMeshes())
	{
		DrawMesh(mesh, proj, view, shader);
	}
}

void Renderer::DrawEntityShadow(Entity& entity)
{	
	// draw each mesh in the entity's model
	for (Mesh& mesh : entity.getModel()->getMeshes())
	{
		mesh.BindVao();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);

		mesh.UnbindVao();
	}
}

void Renderer::DrawEntityInstanced(const glm::mat4& projView, std::shared_ptr<Shader> shader, Entity& entity, const std::vector<glm::mat4>& translations)
{
	shader->use();
	shader->setMat4("u_projView", projView);
	shader->setVec3("u_cameraPos", &cameraPos.x);
	// draw each mesh in the entity's model
	for (Mesh& mesh : entity.getModel()->getMeshes())
	{
		BindTextures(mesh, shader);

		mesh.BindVao();
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(translations.size()));

		mesh.UnbindVao();
		glActiveTexture(GL_TEXTURE0);
	}
}

void Renderer::DrawSkybox(const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Skybox> skybox)
{
	glDepthFunc(GL_LEQUAL); // change depth function for skybox

	shader->use();
	shader->setMat4("u_projView", projView);

	// bind skybox VAO and cubemap texture
	skybox->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetCubemapTexture());
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	skybox->Unbind();

	glDepthFunc(GL_LESS); // reset depth function
}

void Renderer::DrawQuad(std::shared_ptr<Shader> shader, std::shared_ptr<PostProcessor> postProcessor)
{
	shader->use();
	postProcessor->BindVAO();
	postProcessor->BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::DrawCollisionDebug(const glm::mat4& projView, std::shared_ptr<Shader> shader, const PxRenderBuffer& renderBuffer)
{
	shader->use();
	shader->setMat4("u_projView", projView);
	/*
	PxU32 nbLines = renderBuffer.getNbLines();
	PxDebugLine* lines = const_cast<PxDebugLine*>(renderBuffer.getLines());

	if (nbLines == 0)
		return;

	if (nbLines > maxDebugLines)
	{
		nbLines = maxDebugLines;
		std::cout << "Too many debug lines. Some lines will not be rendered" << std::endl;
	}

	glBindBuffer(GL_ARRAY_BUFFER, debugVbo);

	*/
	for (PxU32 i = 0; i < renderBuffer.getNbLines(); i++)
	{
		const PxDebugLine& line = renderBuffer.getLines()[i];
		
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

		GLfloat lineVertices[] = {
			line.pos0.x, line.pos0.y, line.pos0.z, col0.x, col0.y, col0.z,
			line.pos1.x, line.pos1.y, line.pos1.z, col1.x, col1.y, col1.z
		};

		GLuint lineIndices[] = {
			0, 1
		};

		GLuint lineVAO, lineVBO, lineEBO;
		glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &lineVBO);
		glGenBuffers(1, &lineEBO);

		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lineIndices), lineIndices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glDeleteVertexArrays(1, &lineVAO);
		glDeleteBuffers(1, &lineVBO);
		glDeleteBuffers(1, &lineEBO);	
	}
}

void Renderer::DrawMesh(Mesh& mesh, const glm::mat4& proj, const glm::mat4& view, std::shared_ptr<Shader> shader)
{
	BindTextures(mesh, shader);

	mesh.BindVao();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);

	mesh.UnbindVao();
	//glActiveTexture(GL_TEXTURE0);
}

void Renderer::BindTextures(Mesh& mesh, std::shared_ptr<Shader> shader)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	shader->setBool("hasDiffuseTex", mesh.hasDiffuseTexture());
	shader->setBool("hasSpecularTex", mesh.hasSpecularTexture());
	shader->setBool("hasNormalTex", mesh.hasNormalTexture());
	shader->setBool("hasHeightTex", mesh.hasHeightTexture());

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
		if (shader)
			shader->setInt((name + number).c_str(), i); // set the texture unit in the shader

		//glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		mesh.getTextures()[i].Bind(GL_TEXTURE0 + i);
	}
}



