#pragma once

#include <string>

#include <vector>
#include <glad/glad.h>

#include "VAO.h"
#include "EBO.h"
#include "Texture.h"

// https://www.youtube.com/watch?v=NUZF_5RKfS4 
class Mesh
{
public:
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures);

	void Cleanup();

	const std::vector<Vertex>& getVertices() const { return vertices; }
	const std::vector<GLuint>& getIndices() const { return indices; }

	void BindVao() const { vao.Bind(); }
	void UnbindVao() { vao.Unbind(); }

	std::vector<Texture>& getTextures() { return textures; }

private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	VAO vao;
	VBO vbo;
	EBO ebo;

	void SetupMesh();
};
