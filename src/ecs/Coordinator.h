#pragma once
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"
#include "debugUtils/Logger.h"
// coordinates between all things of the ecs system
// that is communicate between that three managers
// entity manager, component manager, system manager
// there should only ever be 1 coordinator
// usage of the coordinator would be for example:

// Entity player = coordinator.createEntity();
// coordinator.addComponent<Transform>(player);
// RenderSystem renderSystem = coordinator.registerSystem<RenderSystem>();

// the coordinator is a blackbox for the above

class Coordinator {
public:
	// on initialize create the managers
	void Init() {
		dbug::log("ECS", 0, "Initializing coordinator");
		componentManager = std::make_unique<ComponentManager>();
		entityManager = std::make_unique<EntityManager>();
		systemManager = std::make_unique<SystemManager>();
	}

	// entity 

	// you run the create entity from the manager
	Entity createEntity() {
		return entityManager->createEntity();
	}

	// with destroy you have to delete the entity and
	// it's components, and the systems that use that entity
	void destroyEntity(Entity entity) {
		entityManager->destroyEntity(entity);

		componentManager->entityDestroyed(entity);

		systemManager->entityDestroyed(entity);
	}


	// Component

	// register the component
	template<typename T>
	void registerComponent() {
		componentManager->registerComponent<T>();
	}

	// add component
	template<typename T>
	void addComponent(Entity entity, T component) {
		// first add the component
		componentManager->addComponent<T>(entity, component);

		// get the signature from the entity
		auto signature = entityManager->getSignature(entity);
		// set the signature bit on the entity
		signature.set(componentManager->getComponentType<T>(), true);
		// propogate that change to the entity
		entityManager->setSignature(entity, signature);

		// propogate that change to the systems
		systemManager->entitySignatureChanged(entity, signature);
	}


	// remove a component
	// same as add component but remove
	template<typename T>
	void removeComponent(Entity entity) {
		componentManager->removeComponent<T>(entity);

		auto signature = entityManager->getSignature(entity);
		signature.set(componentManager->getComponentType<T>(), false);
		entityManager->setSignature(entity, signature);

		systemManager->entitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& getComponent(Entity entity) {
		return componentManager->getComponent<T>(entity);
	}

	template<typename T>
	ComponentType getComponentType() {
		return componentManager->getComponentType<T>();
	}


	// register a system
	template<typename T>
	std::shared_ptr<T> registerSystem() {
		return systemManager->registerSystem<T>();
	}

	// set signature of a system
	template<typename T>
	void setSystemSignature(Signature signature) {
		systemManager->setSignature<T>(signature);
	}

private:
	std::unique_ptr<EntityManager> entityManager;
	std::unique_ptr<ComponentManager> componentManager;
	std::unique_ptr<SystemManager> systemManager;
};
