#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <utility>

#include "Transform.h"

enum class DrawType
{
	NO_DRAW,
	MESH,
	INSTANCED_MESH,
	SKYBOX,
	QUAD
};

enum class PhysicsType
{
	NO_PHYSICS,
	STATIC_MESH,
	VEHICLE,
	RIGID_BODY,
	KINEMATIC,
	TRIGGER
};


class BaseEntity
{
public:
	BaseEntity() = default;
	BaseEntity(const std::string name, DrawType drawType = DrawType::NO_DRAW, PhysicsType physicsType = PhysicsType::NO_PHYSICS);

	virtual ~BaseEntity() = default;

	virtual void setType(DrawType newDrawType, PhysicsType newPhysicsType);

	// add a child entity to this entity
	template<typename T, typename... TArgs>
	T* addChild(const std::string name, DrawType dt, PhysicsType pt, TArgs&&... args)
	{
		auto newChild = std::make_unique<T>(name, dt, pt, args...);
		std::string type = "";
		switch (pt)
		{
			case PhysicsType::NO_PHYSICS:
			type = "NO_PHYSICS";
			break;
			case PhysicsType::STATIC_MESH:
				type = "STATIC_MESH";
				break;
			case PhysicsType::VEHICLE:
				type = "VEHICLE";
				break;
			case PhysicsType::RIGID_BODY:
				type = "RIGID_BODY";
				break;
			case PhysicsType::KINEMATIC:
				type = "KINEMATIC";
				break;
			case PhysicsType::TRIGGER:
				type = "TRIGGER";
				break;
		}
		std::cout << "Added child entity: " << name << " to parent entity: " << this->name << " with physics type: " << type << std::endl;

		T* newChildPtr = newChild.get();
		children.emplace_back(std::move(newChild));
		children.back()->parent = this;

		return newChildPtr;
	}

	virtual void Init() {}

	virtual void updateSelfAndChild(float deltaTime);

	virtual void forceUpdateSelfAndChild(float deltaTime);

	BaseEntity* getChild(const std::string& name);

	const std::string name;

	std::vector<std::unique_ptr<BaseEntity>> children; // scene graph
	BaseEntity* parent = nullptr;
	Transform transform;

	DrawType drawType = DrawType::NO_DRAW;
	PhysicsType physicsType = PhysicsType::NO_PHYSICS;

private:
};