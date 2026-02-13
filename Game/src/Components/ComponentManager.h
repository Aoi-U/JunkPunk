#pragma once

#include <any>
#include <memory>
#include <unordered_map>

#include "ComponentArray.h"
#include "../Core/Types.h"

class ComponentManager
{
public:
	template<typename T>
	void RegisterComponent()
	{
		const char* typeName = typeid(T).name();
		assert(componentTypes.find(typeName) == componentTypes.end() && "Failed to register component: Component already registered");

		componentTypes.insert({ typeName, nextComponentType });
		componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

		nextComponentType++;
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		const char* typeName = typeid(T).name();
		assert(componentTypes.find(typeName) != componentTypes.end() && "Failed to get component type: Component not registered before use");

		return componentTypes[typeName];
	}

	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		GetComponentArray<T>()->InsertData(entity, component);
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		GetComponentArray<T>()->RemoveData(entity);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return GetComponentArray<T>()->GetData(entity);
	}

	// notify each component array that an entity has been destroyed
	void EntityDestroyed(Entity entity)
	{
		for (auto const& [_, component] : componentArrays)
		{
			component->EntityDestroyed(entity);
		}
	}

	void Reset()
	{
		for (auto const& [_, component] : componentArrays)
		{
			component->Reset();
		}
	}

	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray()
	{
		const char* typeName = typeid(T).name();

		assert(componentTypes.find(typeName) != componentTypes.end() && "Failed to get component array: Component not registered before use");

		return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
	}

private:
	std::unordered_map<const char*, ComponentType> componentTypes{};
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays{};

	ComponentType nextComponentType{};

};