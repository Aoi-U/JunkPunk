#pragma once

#include "BaseEntity.h"
#include "Transform.h"

struct CameraParams
{
	float lerpSpeed = 4.0f; // how fast the camera returns to default position
	float lookSpeed = 6.0f; // how fast the camera rotates based on input

	float fov = 90.0f;
	float zNear = 0.1f;
	float zFar = 1000.0f;
	float width = 1280.0f;
	float height = 720.0f;

	glm::mat4 viewMatrix{};
	glm::mat4 projectionMatrix{};
	glm::vec3 position{};

	glm::vec3 frontVector{ 0.0f, 0.0f, -1.0f };
	glm::vec3 upVector{ 0.0f, 1.0f, 0.0f };
	glm::vec3 rightVector{ 1.0f, 0.0f, 0.0f };
};

class CameraEntity : public BaseEntity
{
public:
	CameraEntity(const std::string name, DrawType dt, PhysicsType pt, CameraParams params);

	//void Init() override;

	void updateSelfAndChild(float deltaTime) override;

	void forceUpdateSelfAndChild(float deltaTime) override;

	void ChangeAspectRatio(float width, float height);

	void Update(float deltaTime);

	void ChangeAspectRatio(float newAspectRatio);

	void ChangeFov(float newFov);

	void ChangeTheta(float deltaTheta);

	void ChangePhi(float deltaPhi);

	void ChangeRadius(float deltaRadius);

	// probably need some function to lerp between current and new position for smooth camera movement

	void Reset();

	CameraParams getCameraParams() const;

	glm::mat4 GetViewProjectionMatrix() const { return GetProjectionMatrix() * GetViewMatrix(); }

	glm::mat4 GetProjectionMatrix() const { return glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar); }

	glm::mat4 GetViewMatrix() const 
	{ 
		glm::mat4 vehicleMatrix = glm::mat4(1.0f);
		vehicleMatrix[0] = glm::vec4(transform.getRight(), 0.0f);
		vehicleMatrix[1] = glm::vec4(transform.getUp(), 0.0f);
		vehicleMatrix[2] = glm::vec4(-transform.getForward(), 0.0f);
		vehicleMatrix[3] = glm::vec4(transform.getGlobalPosition(), 1.0f);

		return glm::inverse(vehicleMatrix);
	}

	glm::vec3 GetPosition() const { return transform.getGlobalPosition(); }

	glm::vec3 GetFrontVector() const { return glm::normalize(transform.getForward()); }

	glm::vec3 GetUpVector() const { return glm::normalize(transform.getUp()); }

	glm::vec3 GetRightVector() const { return glm::normalize(transform.getRight()); }

	float GetDistance() const { return transform.getLocalPosition().z; }

	float GetTheta() const
	{
		glm::vec3 dir = glm::normalize(transform.getLocalPosition());
		return glm::atan(dir.x, -dir.z);
	}

	float GetPhi() const
	{
		glm::vec3 dir = glm::normalize(transform.getLocalPosition());
		return glm::asin(dir.y);
	}

	float GetFarClipPlane() const { return zFar; }

	float GetNearClipPlane() const { return zNear; }

	float GetFov() const { return fov; }

	float GetAspectRatio() const { return aspectRatio; }

private:

	float lerpSpeed = 4.0f; // how fast the camera returns to default position
	float lookSpeed = 6.0f; // how fast the camera rotates based on input
	float minDistance = 2.0f;
	float maxDistance = 10.0f;

	float aspectRatio;
	float width;
	float height;


	float fov;
	float zNear = 0.1f;
	float zFar = 1000.0f;


};