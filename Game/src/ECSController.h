#pragma once

#include <memory>
#include <optional>

#include "Entity/EntityManager.h"
#include "Components/ComponentManager.h"
//#include "Components/Physics.h"
#include "Systems/SystemManager.h"
#include "EventManager.h"

#include "Core/Types.h"

class ECSController
{
public:
	void Init()
	{
		entityManager = std::make_unique<EntityManager>();
		componentManager = std::make_unique<ComponentManager>();
		systemManager = std::make_unique<SystemManager>();
		eventManager = std::make_unique<EventManager>();
	}

	// entity methods

	Entity createEntity()
	{
		return entityManager->CreateEntity();
	}

	void DestroyEntity(Entity entity)
	{
		entityManager->DestroyEntity(entity);

		// notify component and system managers that entity is destroyed
		componentManager->EntityDestroyed(entity);
		systemManager->EntityDestroyed(entity);
	}

	template<typename T>
	void RegisterComponent()
	{
		componentManager->RegisterComponent<T>();
	}
	
	// component methods

	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		componentManager->AddComponent<T>(entity, component);

		Signature signature = entityManager->GetSignature(entity);
		signature.set(componentManager->GetComponentType<T>(), true); // set bit for this component 
		entityManager->SetSignature(entity, signature);

		systemManager->EntitySignatureChanged(entity, signature); // notify system manager that this entitys signature changed
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		//const char* typeName = typeid(T).name();
		//if (typeName == "RigidBody")
		//{
		//	//RigidBody& rigidbodyComponent = GetComponent<RigidBody>(entity);
		//	//rigidbodyComponent.actor->release();
		//}

		componentManager->RemoveComponent<T>(entity);
		Signature signature = entityManager->GetSignature(entity);
		signature.set(componentManager->GetComponentType<T>(), false); // clear bit for this component
		entityManager->SetSignature(entity, signature);

		systemManager->EntitySignatureChanged(entity, signature); // notify system manager that this entitys signature changed
	}

	template<typename T>
	T& GetComponent(Entity entity) // returns a reference to the T component of the entity
	{
		return componentManager->GetComponent<T>(entity);
	}

	template<typename T>
	bool HasComponent(Entity entity) // checks if entity has component T
	{
		ComponentType type = componentManager->GetComponentType<T>();

		Signature signature = entityManager->GetSignature(entity);
		return signature.test(type);
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		return componentManager->GetComponentType<T>();
	}

	// system methods

	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		return systemManager->RegisterSystem<T>();
	}

	template<typename T>
	void SetSystemSignature(Signature signature)
	{
		systemManager->SetSignature<T>(signature);
	}

	// event methods

	void AddEventListener(EventId id, std::function<void(Event&)> const& listener)
	{
		eventManager->AddListener(id, listener);
	}

	void SendEvent(Event& event)
	{
		eventManager->SendEvent(event);
	}

	void SendEvent(EventId id)
	{
		eventManager->SendEvent(id);
	}

	// entity tag methods 
	// added these as helpers to access any entity by a string tag (if it has one assigned)
	void AssignTag(Entity entity, const std::string tag)
	{
		entityManager->AssignTag(entity, tag);
	}

	Entity GetEntityByTag(std::string tag)
	{
		return entityManager->GetEntityByTag(tag);
	}
	

private:
	std::unique_ptr<EntityManager> entityManager;
	std::unique_ptr<ComponentManager> componentManager;
	std::unique_ptr<SystemManager> systemManager;
	std::unique_ptr<EventManager> eventManager;
};