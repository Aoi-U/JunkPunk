#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"

class ShadowMapper
{
public:
	ShadowMapper();

	void Init();

	void BindShadowMap();

	void UnbindShadowMap();

	void BindDepthMapTexture();

	std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
	
	const GLuint SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

private:
	GLuint depthMapFBO;
	GLuint depthMap;
	GLuint depthMaps;

	std::vector<float> shadowCascadeLevels;

};