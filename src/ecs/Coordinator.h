#pragma once
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"
// coordinates between all things of the ecs system
// that is communicate between that three managers
// entity manager, component manager, system manager
// there should only ever be 1 coordinator
// usage of the coordinator would be for example:

// Entity player = coordinator.CreateEntity();
// coordinator.AddComponent<Transform>(player);
// RenderSystem renderSystem = coordinator.RegisterSystem<RenderSystem>();

// the coordinator is a blackbox for the above

class Coordinator {
public:
	void Init();

	// entity 
	Entity createEntity();
	void destroyEntity(Entity entity);

	// Component
	template<typename T>
	void registerComponent();

	template<typename T>
	void addComponent(Entity entity, T component);

	template<typename T>
	void removeComponent(Entity entity);

	template<typename T>
	T& getComponent(Entity entity);

	template<typename T>
	ComponentType getComponentType();


	// system
	template<typename T>
	std::shared_ptr<T> registerSystem();

	template<typename T>
	void setSystemSignature(Signature signature);

private:
	std::unique_ptr<ComponentManager> componentManager;
	std::unique_ptr<EntityManager> entityManager;
	std::unique_ptr<SystemManager> systemManager;
};