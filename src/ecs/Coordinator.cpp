#include "Coordinator.h"

// on initialize create the managers
void Coordinator::Init() {
	componentManager = std::make_unique<ComponentManager>();
	entityManager = std::make_unique<EntityManager>();
	systemManager = std::make_unique<SystemManager>();
}

// entity 

// you run the create entity from the manager
Entity Coordinator::createEntity() {
	return entityManager->createEntity();
}

// with destroy you have to delete the entity and
// it's components, and the systems that use that entity
void Coordinator::destroyEntity(Entity entity) {
	entityManager->destroyEntity(entity);

	componentManager->entityDestroyed(entity);

	systemManager->entityDestroyed(entity);
}

// Component

// register the component
template<typename T>
void Coordinator::registerComponent() {
	componentManager->registerComponent<T>();
}

// add component
template<typename T>
void Coordinator::addComponent(Entity entity, T component) {

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
void Coordinator::removeComponent(Entity entity) {
	componentManager->removeComponent<T>(entity);

	auto signature = entityManager->getSignature(entity);
	signature.set(componentManager->getComponentType<T>(), false);
	entityManager->setSignature(entity, signature);

	systemManager->entitySignatureChanged(entity, signature);
}

template<typename T>
T& Coordinator::getComponent(Entity entity) {
	return componentManager->getComponent<T>(entity);
}

template<typename T>
ComponentType Coordinator::getComponentType() {
	return componentManager->getComponentType<T>();
}


// system
template<typename T>
std::shared_ptr<T> Coordinator::registerSystem() {
	return systemManager->registerSystem<T>();
}

template<typename T>
void Coordinator::setSystemSignature(Signature signature) {
	systemManager->setSignature<T>(signature);
}
