#include "ShadowMapper.h"

ShadowMapper::ShadowMapper(float aspect, float near, float far, float fovY, glm::mat4 view, Light light)
	: depthMapFBO(0), depthMaps(0), matricesUBO(0), aspectRatio(aspect), nearPlane(near), farPlane(far), fovY(fovY), view(view), light(light)
{

}

void ShadowMapper::Init(const std::shared_ptr<Shader> shader, const std::shared_ptr<Shader> shader2)
{
	// make depth map fbo
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMaps);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, int(shadowCascadeLevels.size()) + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMaps, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete: " << status << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	const unsigned int shader1ID = shader->getID();
	const unsigned int shader2ID = shader2->getID();

	GLuint uboIndex = glGetUniformBlockIndex(shader1ID, "LightSpaceMatrices");
	GLuint uboIndex2 = glGetUniformBlockIndex(shader2ID, "LightSpaceMatrices");
	glUniformBlockBinding(shader->getID(), uboIndex, 0);
	glUniformBlockBinding(shader2->getID(), uboIndex2, 0);

	if (uboIndex == GL_INVALID_INDEX)
	{
		std::cout << "ERROR: LightSpaceMatrices UBO not found in shadow shader!" << std::endl;
	}
	if (uboIndex2 == GL_INVALID_INDEX)
	{
		std::cout << "ERROR: LightSpaceMatrices UBO not found in default shader!" << std::endl;
	}

	glGenBuffers(1, &matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ShadowMapper::Update(float aspect, float near, float far, float fovY, glm::mat4 view)
{
	this->aspectRatio = aspect;
	this->nearPlane = near;
	this->farPlane = far;
	this->fovY = fovY;
	this->view = view;

	shadowCascadeLevels = { far / 50.0f, far / 25.0f, far / 10.0f, far / 2.0f };
}

void ShadowMapper::BindShadowMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glEnable(GL_DEPTH_TEST);
}

void ShadowMapper::UnbindShadowMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glEnable(GL_DEPTH_TEST);
}

void ShadowMapper::BindDepthMapTexture()
{
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMaps);
}

void ShadowMapper::BindFramebufferTextures()
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMaps, 0);
}

void ShadowMapper::SetupUBO()
{
	const auto lightMatrices = GetLightSpaceMatrices();

	glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
	for (size_t i = 0; i < lightMatrices.size(); ++i)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

glm::mat4 ShadowMapper::GetLightSpaceMatrix(const float nearPlane, const float farPlane)
{
	const auto proj = glm::perspective(fovY, aspectRatio, nearPlane, farPlane);
	const std::vector<glm::vec4> corners = GetFrustumCornersWorldSpace(proj * view);

	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();

	const auto lightView = glm::lookAt(center + light.getDirection(), center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		const auto trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	constexpr float zMult = 10.0f;
	if (minZ < 0)
	{
		minZ *= zMult;
	}
	else
	{
		minZ /= zMult;
	}
	if (maxZ < 0)
	{
		maxZ /= zMult;
	}
	else
	{
		maxZ *= zMult;
	}

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

std::vector<glm::mat4> ShadowMapper::GetLightSpaceMatrices()
{
	std::vector<glm::mat4> lightSpaceMatrices;
	for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			lightSpaceMatrices.push_back(GetLightSpaceMatrix(nearPlane, shadowCascadeLevels[i]));
		}
		else if (i < shadowCascadeLevels.size())
		{
			lightSpaceMatrices.push_back(GetLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
		}
		else
		{
			lightSpaceMatrices.push_back(GetLightSpaceMatrix(shadowCascadeLevels[i - 1], farPlane));
		}
	}
	return lightSpaceMatrices;
}

std::vector<glm::vec4> ShadowMapper::GetFrustumCornersWorldSpace(const glm::mat4& projView)
{
	const auto inv = glm::inverse(projView);

	std::vector<glm::vec4> frustumCorners;
	frustumCorners.reserve(8);

	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}
