#include "Camera.h"

Camera::Camera(glm::mat4& target, Params const& params) 
	: target(&target)
	, distance(params.defaultDistance)
	, minDistance(params.minDistance)
	, maxDistance(params.maxDistance)
	, theta(params.defaultTheta)
	, phi(params.defaultPhi)
{
	defaultTheta = params.defaultTheta;
	defaultPhi = params.defaultPhi;
	defaultDistance = params.defaultDistance;
}

glm::mat4 Camera::GetViewMatrix()
{
	UpdateViewMatrix();
	return viewMatrix;
}

glm::vec3 Camera::GetPosition()
{
	UpdateViewMatrix();
	return position;
}
float Camera::GetDistance()
{
	UpdateViewMatrix();
	return distance;
}
float Camera::GetTheta()
{
	UpdateViewMatrix();
	return theta;
}
float Camera::GetPhi()
{
	UpdateViewMatrix();
	return phi;
}

void Camera::UpdateViewMatrix()
{
	if (isDirty == true)
	{
		isDirty = false;

		auto const hRot = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0f, 1.0f, 0.0f));
		auto const vRot = glm::rotate(glm::mat4(1.0f), phi, glm::vec3(1.0f, 0.0f, 0.0f ));

		//_position = glm::vec3(hRot * vRot * glm::vec4{ Math::ForwardVec3, 0.0f }) * _distance;
		position = glm::vec3(hRot * vRot * glm::vec4{glm::vec3(0.0f, 0.0f, 1.0f), 0.0f }) * distance + glm::vec3((*target)[3]);

		auto center = glm::vec3((*target)[3]);
		viewMatrix = glm::lookAt(position, center, glm::vec3(0.0f, 1.0f, 0.0f));
	}
}

void Camera::ChangeTheta(float const deltaTheta)
{
	auto newTheta = theta + deltaTheta;
	if (newTheta != theta)
	{
		theta = newTheta;
		isDirty = true;
	}
}

void Camera::ChangePhi(float const deltaPhi)
{
	float const newPhi = glm::clamp(phi + deltaPhi, -glm::pi<float>() * 0.49f, glm::pi<float>() * 0.49f);
	if (newPhi != phi)
	{
		isDirty = true;
		phi = newPhi;
	}
}

void Camera::ChangeRadius(float const deltaRadius)
{
	float const newDistance = glm::clamp(distance + deltaRadius, minDistance, maxDistance);
	if (newDistance != distance)
	{
		isDirty = true;
		distance = newDistance;
	}
}
void Camera::Reset()
{
	Params params{};
	distance = params.defaultDistance;
	minDistance = params.minDistance;
	maxDistance = params.maxDistance;
	float lerpFactor = 0.1f; // adjust for faster/slower reset
	theta = glm::mix(theta, defaultTheta, lerpFactor);
	phi = glm::mix(phi, defaultPhi, lerpFactor);
	distance = glm::mix(distance, defaultDistance, lerpFactor);
	isDirty = true;
}