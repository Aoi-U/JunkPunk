#pragma once

#include <memory>
#include <PxPhysicsAPI.h>

#include "glm/glm.hpp"
#include "../Core/Model.h"
#include "../Core/BoundingVolumes.h"
#include "../Core/Vehicle/MainVehicle.h"

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
	glm::vec3 linearVelocity;
	glm::vec3 angularVelocity;
};

struct StaticBody
{
	PxRigidStatic* actor = nullptr;
	std::shared_ptr<Model> collisionMesh;
};

struct VehicleBody
{
	EngineDriveVehicle* vehicle = nullptr;
	glm::vec3 linearVelocity;
	glm::vec3 angularVelocity;
};

// trigger boxes will not be rendered on the screen so you must set the eVISUALIZATION flag on and view it in PVD
struct Trigger
{
	PxRigidStatic* actor;
	float width; // half extents of the width in x direction
	float length; // half extents of the length in z direction
	float height; // half extents of height in y direction

};