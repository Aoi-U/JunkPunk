#pragma once

#include <memory>
#include <PxPhysicsAPI.h>

#include "glm/glm.hpp"
#include "../Core/Model.h"
#include "../Core/BoundingVolumes.h"

using namespace physx;

struct PhysicsBody
{

};

struct RigidBody
{
	PxRigidDynamic* actor = nullptr;
	std::shared_ptr<Model>collisionMesh;
	std::shared_ptr<AABB> boundingVolume;
	float mass;
	bool useGravity;
	bool isKinematic;
	glm::vec3 linearVelocity;
	glm::vec3 angularVelocity;
};

struct StaticModel
{
	PxRigidStatic* actor = nullptr;
	std::shared_ptr<Model> collisionMesh;
};

struct VehicleBody
{
	glm::vec3 linearVelocity;
	glm::vec3 angularVelocity;
};

struct Trigger
{
	float radius;
};