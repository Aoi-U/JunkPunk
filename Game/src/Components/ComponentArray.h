#pragma once

#include <array>
#include <cassert>
#include <unordered_map>
#include <functional>

#include "../Core/Types.h"

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void EntityDestroyed(Entity entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray
{
public:
	using OnRemoveCallback = std::function<void(Entity, T&)>;

	void InsertData(Entity entity, T component) // add component to an entity
	{
		assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Failed to add component: Component added to same entity more than once");

		size_t newIndex = size;
		entityToIndexMap[entity] = newIndex;
		indexToEntityMap[newIndex] = entity;
		componentArray[newIndex] = component;
		size++;
	}

	void RemoveData(Entity entity) // remove component from an entity
	{
		assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Failed to remove component: Component not found in entity");

		if (removeCallback) // check if this component type has a remove callback bound
		{
			size_t indexOfRemovedEntity = entityToIndexMap[entity];
			removeCallback(entity, componentArray[indexOfRemovedEntity]); // call the systems remove callback function
		}

		// move element at end into deleted entitys place to stay contiguous
		size_t indexOfRemovedEntity = entityToIndexMap[entity];
		size_t indexOfLastElement = size - 1;
		componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

		// update maps to point to moved elements spot
		Entity entityOfLastElement = indexToEntityMap[indexOfLastElement];
		entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
		indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

		entityToIndexMap.erase(entity);
		indexToEntityMap.erase(indexOfLastElement);

		size--;
	}

	T& GetData(Entity entity) // get component for an entity
	{
		assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Failed to get component: Component not found in entity");

		return componentArray[entityToIndexMap[entity]];
	}

	bool HasData(Entity entity) // check if an entity has this component
	{
		return entityToIndexMap.find(entity) != entityToIndexMap.end();
	}

	void EntityDestroyed(Entity entity) override
	{
		if (entityToIndexMap.find(entity) != entityToIndexMap.end())
		{
			RemoveData(entity);
		}
	}

	void BindOnRemoveCallback(OnRemoveCallback callback)
	{
		removeCallback = callback;
	}

private:
	std::array<T, MAX_ENTITIES> componentArray{}; // holds all components registered to this type
	std::unordered_map<Entity, size_t> entityToIndexMap{}; // maps an entity to its index in the array
	std::unordered_map<size_t, Entity> indexToEntityMap{}; // maps an index in the array to its entity
	size_t size{};

	OnRemoveCallback removeCallback = nullptr;
};