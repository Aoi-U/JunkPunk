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

void Renderer::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/*
void Renderer::DrawEntity(const glm::mat4& proj, const glm::mat4& view, std::shared_ptr<Shader> shader, std::shared_ptr<Entity> entity)
{	
	// draw each mesh in the entity's model
	for (Mesh& mesh : entity->getMeshes())
	{
		DrawMesh(mesh, shader);
	}
}
*/

/*
void Renderer::DrawEntityShadow(std::shared_ptr<Entity> entity)
{	
	// draw each mesh in the entity's model
	for (Mesh& mesh : entity->getMeshes())
	{
		mesh.BindVao();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);

		mesh.UnbindVao();
	}
}
*/


void Renderer::DrawEntityInstanced(const glm::mat4& projView, std::shared_ptr<Shader> shader, std::shared_ptr<Model> model, const std::vector<glm::mat4>& matrices)
{
	shader->use();
	shader->setMat4("u_projView", projView);
	shader->setVec3("u_cameraPos", &cameraPos.x);
	// draw each mesh in the entity's model
	for (Mesh& mesh : model->getMeshes())
	{
		BindTextures(mesh, shader);

		mesh.BindVao();
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(matrices.size()));

		mesh.UnbindVao();
		glActiveTexture(GL_TEXTURE0);
	}
}

void Renderer::DrawSkybox(const glm::mat4& projView, const std::shared_ptr<Shader> shader, std::shared_ptr<Skybox> skybox)
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

void Renderer::DrawQuad(const std::shared_ptr<Shader> shader, std::shared_ptr<PostProcessor> postProcessor)
{
	shader->use();
	postProcessor->BindVAO();
	postProcessor->BindTexture();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::DrawCollisionDebug(const glm::mat4& projView, const std::shared_ptr<Shader> shader, const PxRenderBuffer& renderBuffer)
{
	shader->use();
	shader->setMat4("u_projView", projView);
	
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

void Renderer::DrawMesh(Mesh& mesh, const std::shared_ptr<Shader> shader)
{
	if (shader)
		BindTextures(mesh, shader);

	mesh.BindVao();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);

	mesh.UnbindVao();
	//glActiveTexture(GL_TEXTURE0);
}

void Renderer::DrawModel(std::shared_ptr<Model> model, const std::shared_ptr<Shader> shader)
{
	for (Mesh& mesh : model->getMeshes())
	{
		DrawMesh(mesh, shader);
	}
}

void Renderer::DrawModelShadow(std::shared_ptr<Model> model)
{
	for (Mesh& mesh : model->getMeshes())
	{
		mesh.BindVao();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.getIndices().size()), GL_UNSIGNED_INT, 0);
		mesh.UnbindVao();
	}
}

void Renderer::BindTextures(Mesh& mesh, const std::shared_ptr<Shader> shader)
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



