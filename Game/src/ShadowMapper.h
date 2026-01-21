#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "Camera.h"
#include "Window.h"
#include "Light.h"
#include "Shader.h"

class ShadowMapper
{
public:
	ShadowMapper(std::shared_ptr<Camera> cam, std::shared_ptr<Window> win, std::shared_ptr<Light> light);

	void Init(const std::shared_ptr<Shader> shader, const std::shared_ptr<Shader> shader2);

	void BindShadowMap();

	void UnbindShadowMap();

	void BindDepthMapTexture();

	void BindFramebufferTextures();

	void SetupUBO();

	int GetCascadeCount() const { return static_cast<int>(shadowCascadeLevels.size()); }

	std::vector<float> GetCascadeLevels() const { return shadowCascadeLevels; }
	
	const GLuint SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

private:
	GLuint depthMapFBO;
	GLuint depthMaps;
	GLuint matricesUBO;

	std::shared_ptr<Camera> camera;
	std::shared_ptr<Window> window;
	std::shared_ptr<Light> light;

	std::vector<float> shadowCascadeLevels = { 1000.0 / 50.0f, 1000.0 / 25.0f, 1000.0 / 10.0f, 1000.0 / 3.0f };

	std::vector<glm::mat4> GetLightSpaceMatrices();
	glm::mat4 GetLightSpaceMatrix(const float nearPlane, const float farPlane);
	std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projView);
};