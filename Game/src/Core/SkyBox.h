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
