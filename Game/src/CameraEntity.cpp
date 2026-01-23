#include "CameraEntity.h"

CameraEntity::CameraEntity(const std::string name, DrawType dt, PhysicsType pt, CameraParams params)
	: BaseEntity(name, dt, pt), // no draw, no physics
		aspectRatio(params.width / params.height),
		width(params.width),
		height(params.height),
		fov(params.fov),
		zNear(params.zNear),
		zFar(params.zFar)
{

}

void CameraEntity::updateSelfAndChild(float deltaTime)
{
	if (transform.getIsDirty())
	{
		forceUpdateSelfAndChild(deltaTime);
		return;
	}

	for (auto&& child : children)
	{
		child->updateSelfAndChild(deltaTime);
	}
}

void CameraEntity::forceUpdateSelfAndChild(float deltaTime)
{
	if (parent)
	{
		transform.computeModelMatrix(parent->transform.getModelMatrix());
	}
	else
		transform.computeModelMatrix();
	for (auto&& child : children)
	{
		child->forceUpdateSelfAndChild(deltaTime);
	}
}

void CameraEntity::SetTarget(glm::mat4 target, float dt)
{
	targetMatrix = target;

	glm::vec3 forwardTarget = glm::normalize(glm::vec3((target)[2]));

	float targetTheta = atan2f(forwardTarget.x, forwardTarget.z);

	float deltaTheta = targetTheta - theta;
	deltaTheta = glm::mod(deltaTheta + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();

	// lerp the camera for smooth rotation
	float lerpFactor = glm::clamp(lerpSpeed * dt, 0.0f, 1.0f);
	theta = theta + glm::mix(0.0f, deltaTheta, lerpFactor);

	// make sure theta is clamped between -pi and pi
	theta = glm::mod(theta + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();

	glm::vec3 targetPosition = glm::vec3(target[3]);
	targetPosition.y += 2.0f;

	glm::vec3 localOffset = glm::vec3(0.0f, distance * glm::sin(phi), -distance * glm::cos(phi));
	localOffset.z -= distance; // move camera back a bit
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 offset = glm::vec3(rotation * glm::vec4(localOffset, 0.0f));
	//glm::vec3 newPosition = targetPosition + offset;
	glm::vec3 newPosition = targetPosition + localOffset;
	transform.setLocalPosition(newPosition);


	glm::mat4 viewMatrix = glm::lookAt(newPosition, targetPosition, glm::vec3(0.0f, 1.0f, 0.0f));
	transform.setLocalRotation(glm::quat_cast(viewMatrix));

	std::cout << "Theta: " << theta << "Phi: " << phi << std::endl;

}

void CameraEntity::ChangeAspectRatio(float width, float height)
{
	this->width = width;
	this->height = height;
	aspectRatio = width / height;
}

void CameraEntity::Update(float deltaTime)
{

}

void CameraEntity::ChangeFov(float newFov)
{
	fov = newFov;
}

void CameraEntity::ChangeHorizontal(float deltaTheta)
{
	deltaTheta *= lookSpeed;
	float newTheta = theta + deltaTheta;
	if (newTheta != theta)
	{
		theta = newTheta;
	}
}

void CameraEntity::ChangeVertical(float deltaPhi)
{
	deltaPhi *= lookSpeed;
	float newPhi = glm::clamp(phi + deltaPhi, -glm::pi<float>() * 0.49f, glm::pi<float>() * 0.49f);
	if (newPhi != phi)
	{
		phi = newPhi;
	}
}

void CameraEntity::ChangeRadius(float deltaRadius)
{
	float const newDistance = glm::clamp(distance + deltaRadius, minDistance, maxDistance);
	if (newDistance != distance)
	{
		if (newDistance > maxDistance)
			distance = maxDistance;
		else if (newDistance < minDistance)
			distance = minDistance;
		else
			distance = newDistance;

		// update position based on new distance
		glm::vec3 localOffset = glm::vec3(0.0f, distance * glm::sin(0.0f), -distance * glm::cos(0.0f));
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 offset = glm::vec3(rotation * glm::vec4(localOffset, 0.0f));
		transform.setLocalPosition(offset);
	}
}

CameraParams CameraEntity::getCameraParams() const
{
	CameraParams params;
	params.lerpSpeed = lerpSpeed;
	params.lookSpeed = lookSpeed;
	params.fov = fov;
	params.zNear = zNear;
	params.zFar = zFar;
	params.width = width;
	params.height = height;
	params.viewMatrix = GetViewMatrix();
	params.projectionMatrix = GetProjectionMatrix();
	params.position = transform.getGlobalPosition();
	params.frontVector = transform.getForward();
	params.upVector = transform.getUp();
	params.rightVector = transform.getRight();

	return params;
}
