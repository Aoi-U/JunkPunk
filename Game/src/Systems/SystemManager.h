#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>

#include "System.h"
#include "../Core/Types.h"

class SystemManager
{
public:
	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		const char* typeName = typeid(T).name();
		assert(systems.find(typeName) == systems.end() && "Failed to register system: System already registered");

		// create pointer to the system and return it so it can be used 
		auto system = std::make_shared<T>();
		systems.insert({ typeName, system });
		
		return system;
	}

	template<typename T>
	void SetSignature(Signature signature)
	{
		const char* typeName = typeid(T).name();
		assert(systems.find(typeName) != systems.end() && "Failed to set system signature: System not registered before use");

		signatures.insert({ typeName, signature });
	}

	void EntityDestroyed(Entity entity)
	{
		// erase destroyed entity from all systems
		for (auto const& [_, system] : systems)
		{
			system->entities.erase(entity);
		}
	}

	void EntitySignatureChanged(Entity entity, Signature entitySignature)
	{
		// notify each system that an entitys signature changed
		for (auto const& [type, system] : systems)
		{
			auto const& systemSignature = signatures[type];

			if ((entitySignature & systemSignature) == systemSignature) // entity signature matches system signature so insert into set
			{
				system->entities.insert(entity);
			}
			else // entity signature does not match system signature so remove from set
			{
				system->entities.erase(entity);
			}
		}
	}

private:
	std::unordered_map<const char*, Signature> signatures{}; // map from system type string pointer to a signature
	std::unordered_map<const char*, std::shared_ptr<System>> systems{}; // map from system type string pointer to a system pointer
};