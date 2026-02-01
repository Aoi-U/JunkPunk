#pragma once

#include <array>
#include <cassert>
#include <queue>
#include <string>
#include <unordered_map>

#include "../Core/Types.h"

class EntityManager
{
public:
	EntityManager()
	{
		for (Entity entity = 0; entity < MAX_ENTITIES; entity++)
		{
			availableEntities.push(entity);
		}
	}

	Entity CreateEntity()
	{
		assert(livingEntityCount < MAX_ENTITIES && "Failed to create entity: Maximum number of entities reached");

		Entity id = availableEntities.front();
		availableEntities.pop();
		livingEntityCount++;

		return id;
	}

	void DestroyEntity(Entity entity)
	{
		assert(entity < MAX_ENTITIES && "Failed to destroy entity: Entity out of range");

		signatures[entity].reset();

		if (entityToTagMap.find(entity) != entityToTagMap.end())
		{
			std::string tag = entityToTagMap[entity];
			tagToEntityMap.erase(tag);
			entityToTagMap.erase(entity);
		}

		availableEntities.push(entity); // mark entity ID available
		livingEntityCount--;
	}

	void SetSignature(Entity entity, Signature signature)
	{
		assert(entity < MAX_ENTITIES && "Failed to set entity signature: Entity out of range");

		signatures[entity] = signature;
	}

	Signature GetSignature(Entity entity)
	{
		assert(entity < MAX_ENTITIES && "Failed to get entity signature: Entity out of range");

		return signatures[entity];
	}

	void AssignTag(Entity entity, const std::string tag)
	{
		assert(entity < MAX_ENTITIES && "Failed to assign tag: Entity out of range");
		//assert(tagToEntityMap.find(tag) == tagToEntityMap.end() && "Failed to assign tag: Tag already in use");

		if (entityToTagMap.find(entity) != entityToTagMap.end()) // remove this entitys old tag if it exists
		{
			tagToEntityMap.erase(entityToTagMap[entity]);
		}

		tagToEntityMap[tag] = entity;
		entityToTagMap[entity] = tag;
	}

	Entity GetEntityByTag(std::string tag)
	{
		if (tagToEntityMap.find(tag) == tagToEntityMap.end())
		{
			assert(false && "Failed to get entity by tag: Tag does not exist");
			return MAX_ENTITIES;
		}

		return tagToEntityMap[tag];
	}

	std::string GetTagByEntity(Entity entity)
	{
		if (entityToTagMap.find(entity) == entityToTagMap.end())
		{
			assert(false && "Failed to get tag by entity: Entity has no tag assigned");
			return "";
		}
		return entityToTagMap[entity];
	}
	
	bool HasTag(std::string tag)
	{
		return tagToEntityMap.find(tag) != tagToEntityMap.end();
	}

	void Reset()
	{
		livingEntityCount = 0;

		std::queue<Entity> empty;
		std::swap(availableEntities, empty);

		for (Entity entity = 0; entity < MAX_ENTITIES; entity++)
		{
			availableEntities.push(entity);
		}

		for (auto& signature : signatures)
		{
			signature.reset();
		}

		tagToEntityMap.clear();
		entityToTagMap.clear();
	}

private:
	std::queue<Entity> availableEntities{}; // queue of unused entity IDs
	std::array<Signature, MAX_ENTITIES> signatures{}; //  array of entity signatures
	uint32_t livingEntityCount{}; // total entities in use
	std::unordered_map<std::string, Entity> tagToEntityMap; // assigns names to entities
	std::unordered_map<Entity, std::string> entityToTagMap;

};