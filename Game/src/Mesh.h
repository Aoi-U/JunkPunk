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


	const std::vector<Vertex>& getVertices() const { return vertices; }
	const std::vector<GLuint>& getIndices() const { return indices; }

	void BindVao() const { vao.Bind(); }
	void UnbindVao() { vao.Unbind(); }

	void setMaterialIndex(unsigned int index) { materialIndex = index; }
	unsigned int getMaterialIndex() const { return materialIndex; }

private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	unsigned int materialIndex{};

	VAO vao;
};
