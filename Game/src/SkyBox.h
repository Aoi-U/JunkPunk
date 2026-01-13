#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include "stb/stb_image.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

//std::vector<Vertex> skyboxVertices = {
//	{{-1.0f, -1.0f,  1.0f}, {}, {}, {}},
//	{{ 1.0f, -1.0f,  1.0f}, {}, {}, {}},
//	{{ 1.0f, -1.0f, -1.0f}, {}, {}, {}},
//	{{-1.0f, -1.0f, -1.0f}, {}, {}, {}},
//	{{-1.0f,  1.0f,  1.0f}, {}, {}, {}},
//	{{ 1.0f,  1.0f,  1.0f}, {}, {}, {}},
//	{{ 1.0f,  1.0f, -1.0f}, {}, {}, {}},
//	{{-1.0f,  1.0f, -1.0f}, {}, {}, {}}
//};
//
//std::vector<GLuint> skyboxIndices = {
//	// right
//	1, 2, 6,
//	6, 5, 1,
//	// left
//	0, 4, 7,
//	7, 3, 0,
//	// top
//	4, 5, 6,
//	6, 7, 4,
//	// bottom
//	0, 3, 2,
//	2, 1, 0,
//	// back
//	0, 1, 5,
//	5, 4, 0,
//	// front
//	3, 7, 6,
//	6, 2, 3
//};

class Skybox
{
public:
	Skybox();

	void Init();
	
	GLuint GetCubemapTexture() const { return cubemapTexture; }
	void Bind();
	void Unbind();
	void Delete();
private:
	std::unique_ptr<VAO> vao;
	std::unique_ptr<VBO> vbo;
	std::unique_ptr<EBO> ebo;

	std::vector<std::string> faces{
		"assets/skybox/right.jpg",
		"assets/skybox/left.jpg",
		"assets/skybox/top.jpg",
		"assets/skybox/bottom.jpg",
		"assets/skybox/front.jpg",
		"assets/skybox/back.jpg"
	};

	GLuint cubemapTexture{};
};
