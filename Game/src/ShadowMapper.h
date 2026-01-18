#pragma once

#include "glad/glad.h"
#include <iostream>

class ShadowMapper
{
public:
	ShadowMapper();

	void Init();

	void BindShadowMap();

	void UnbindShadowMap();

	void BindDepthMapTexture();
	
	const GLuint SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

private:
	GLuint depthMapFBO;
	GLuint depthMap;

};