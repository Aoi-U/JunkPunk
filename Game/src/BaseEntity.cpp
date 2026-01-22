#include "BaseEntity.h"

BaseEntity::BaseEntity(const std::string name, DrawType drawType, PhysicsType physicsType) 
	: name(name), drawType(drawType), physicsType(physicsType)
{

}

void BaseEntity::setType(DrawType newDrawType, PhysicsType newPhysicsType)
{
	drawType = newDrawType;
	physicsType = newPhysicsType;
}

void BaseEntity::updateSelfAndChild(float deltaTime)
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

void BaseEntity::forceUpdateSelfAndChild(float deltaTime)
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

BaseEntity* BaseEntity::getChild(const std::string& name)
{
	for (auto& child : children)
	{
		if (child->name == name)
		{
			return child.get();
		}
	}

	std::cout << "Child " << name << " not found" << std::endl;
	return nullptr;
}