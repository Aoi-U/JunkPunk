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
		transform.computeModelMatrix(parent->transform.getModelMatrix());
	else
		transform.computeModelMatrix();
	for (auto&& child : children)
	{
		child->forceUpdateSelfAndChild(deltaTime);
	}
}

void CameraEntity::ChangeAspectRatio(float width, float height)
{
	this->width = width;
	this->height = height;
	aspectRatio = width / height;
}

void CameraEntity::Update(float deltaTime)
{
	// smooth tracking function
}

void CameraEntity::ChangeFov(float newFov)
{
	fov = newFov;
}

void CameraEntity::ChangeTheta(float deltaTheta)
{

}

void CameraEntity::ChangePhi(float deltaPhi)
{

}

void CameraEntity::ChangeRadius(float deltaRadius)
{

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
