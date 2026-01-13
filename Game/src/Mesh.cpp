#include <iostream>

#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices)
	: vertices(vertices), indices(indices), vbo(vertices), ebo(indices)
{
	SetupMesh();
}

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures)
	: vertices(vertices), indices(indices), textures(textures), vbo(vertices), ebo(indices)
{
	SetupMesh();
}

void Mesh::SetupMesh()
{
	vao.Bind();
	vbo.Bind();
	ebo.Bind();

	// Links VBO attributes such as coordinates and colors to VAO
	vao.LinkAttributes(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0); // position
	vao.LinkAttributes(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float))); // color
	vao.LinkAttributes(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float))); // normal
	vao.LinkAttributes(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float))); // texCoord

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}
