#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <map>
#include <memory>
#include <iostream>
#include "VAO.h"
#include "VBO.h"

struct Character
{
	unsigned int textID; // ID handle of texture
	glm::ivec2 size; // size of glyph
	glm::ivec2 bearing; // offset from baseline
	unsigned int Advance; // offset to next glyph
};

class Text
{
public:
	Text();

	void initVAO(VAO* vao, VBO* vbo);

	std::map<char, Character> initFont(const char* font);

	glm::mat4 projMat = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
	std::map<char, Character> charArial;
};
