#pragma once

#include "VAO.h"
#include "EBO.h"

// https://www.youtube.com/watch?v=NUZF_5RKfS4 
class Mesh
{
public:
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);

	const std::vector<Vertex>& getVertices() { return vertices; }
	const std::vector<GLuint>& getIndices() { return indices; }

	void BindVao() { vao.Bind(); }

private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	VAO vao;
};
