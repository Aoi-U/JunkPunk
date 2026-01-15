#include <iostream>

#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices)
	: vertices(vertices), indices(indices), vbo(vertices), ebo(indices)
{
	//SetupMesh();
}

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures)
	: vertices(vertices), indices(indices), textures(textures), vbo(vertices), ebo(indices)
{
	//SetupMesh();
}

void Mesh::Cleanup()
{
	vbo.Delete();
	ebo.Delete();
	vao.Delete();

	for (auto& texture : textures)
	{
		texture.Delete();
	}
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

void Mesh::SetupInstanceMesh()
{
	vao.Bind();
	// set attribute pointers for matrix (4 times vec4)
	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	vao.Unbind();
}
