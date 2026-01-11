#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;

	vao.Bind();

	VBO vbo(vertices);

	EBO ebo(indices);

	// Links VBO attributes such as coordinates and colors to VAO
	vao.LinkAttributes(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0); // position
	vao.LinkAttributes(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float))); // color
	vao.LinkAttributes(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float))); // normal
	vao.LinkAttributes(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float))); // texCoord

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}