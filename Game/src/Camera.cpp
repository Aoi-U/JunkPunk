#include "Camera.h"

Camera::Camera(glm::mat4& target, Params const& params, float fov, float aspectRatio) 
	: target(&target)
	, distance(params.defaultDistance)
	, minDistance(params.minDistance)
	, maxDistance(params.maxDistance)
	, theta(params.defaultTheta)
	, phi(params.defaultPhi)
	, lerpSpeed(params.lerpSpeed)
	, lookSpeed(params.lookSpeed)
	, fov(fov)
	, aspectRatio(aspectRatio)
{
	defaultTheta = params.defaultTheta;
	defaultPhi = params.defaultPhi;
	defaultDistance = params.defaultDistance;
}

void Camera::Update(float deltaTime, glm::mat4& newTarget)
{
	target = &newTarget;

	glm::vec3 forward = glm::normalize(glm::vec3((*target)[2]));
	float targetTheta = atan2f(forward.x, forward.z); 

	float deltaTheta = targetTheta - theta; 
	deltaTheta = glm::mod(deltaTheta + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>(); 

	// lerp the camera for smooth rotation
	float lerpFactor = glm::clamp(lerpSpeed * deltaTime, 0.0f, 1.0f);
	theta = theta + glm::mix(0.0f, deltaTheta, lerpFactor);

	// make sure theta is clamped between -pi and pi
	theta = glm::mod(theta + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();

	isDirty = true;
}


void Camera::ChangeAspectRatio(float newAspectRatio)
{
	aspectRatio = newAspectRatio; 
	isDirty = true;
}

void Camera::ChangeFov(float newFov)
{
	fov = newFov;
	isDirty = true;
}

glm::mat4 Camera::GetProjectionMatrix()
{
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
	return projectionMatrix;
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

		glm::vec3 targetPosition = glm::vec3((*target)[3]);
		targetPosition.y += 2.0f; // adjust height of camera target

		glm::vec3 localOffset = glm::vec3(0.0f, distance * glm::sin(phi), -distance * glm::cos(phi));
		localOffset.z -= 1.0f; // move camera back a bit
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 offset = glm::vec3(rotation * glm::vec4(localOffset, 0.0f));
		targetCameraPosition = targetPosition + offset;

		position = targetCameraPosition;

		viewMatrix = glm::lookAt(position, targetPosition, glm::vec3(0.0f, 1.0f, 0.0f));
	}
}

/*
Frustum Camera::CreateFrustum()
{
	Frustum frustum;
	const float halfVSide = zFar * tanf(glm::radians(fov) * 0.5f);
	const float halfHSide = halfVSide * aspectRatio;
	const glm::vec3 front = glm::normalize(-glm::vec3(viewMatrix[2]));
	const glm::vec3 frontMultFar = zFar * front;

	glm::vec3 right = glm::normalize(glm::vec3(viewMatrix[0]));

	frustum.near = { position + zNear * front, front };
	frustum.far = { position + frontMultFar, -front };
	frustum.right = { position, glm::cross(frontMultFar - right * halfHSide, upVector) };
	frustum.left = { position, glm::cross(upVector, frontMultFar + right * halfHSide) };
	frustum.top = { position, glm::cross(right, frontMultFar - upVector * halfHSide) };
	frustum.bottom = { position, glm::cross(frontMultFar + upVector * halfVSide, right) };

	return frustum;
}
*/

void Camera::ChangeTheta(float deltaTheta)
{
	deltaTheta *= lookSpeed;
	auto newTheta = theta + deltaTheta;
	if (newTheta != theta)
	{
		theta = newTheta;
		isDirty = true;
	}
}

void Camera::ChangePhi(float deltaPhi)
{
	deltaPhi *= lookSpeed;
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

glm::mat4 Camera::GetViewProjectionMatrix()
{
	return GetProjectionMatrix() * GetViewMatrix();
}
