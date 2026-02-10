#include "ComponentManager.h"

template<typename T>
void ComponentManager::registerComponent() {
	const char* typeName = typeid(T).name();

	// assert mComponentTypes.find(typeName) == mComponentTypes.end()  multiple of same component

	componentTypes.insert({ typeName, nextComponentType });
	componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
	nextComponentType++;
}

template<typename T>
ComponentType ComponentManager::getComponentType() {
	const char* typeName = typeid(T).name();

	return componentTypes[typeName];
}

template<typename T>
void ComponentManager::addComponent(Entity entity, T component) {
	ComponentManager::getComponentArray <T>()->insertData(entity, component);
}

template<typename T>
void ComponentManager::removeComponent(Entity entity) {
	ComponentManager::getComponentArray <T>()->removeData(entity);
}

template<typename T>
T& ComponentManager::getComponent(Entity entity) {
	return ComponentManager::getComponentArray < T >()->getData(entity);
}

void ComponentManager::entityDestroyed(Entity entity) {
	// notify each component array that an entity has been destroyed
	// if that component array has that entity, remove it
	for (auto const& pair : componentArrays) {
		auto const& component = pair.second;
		component->entityDestroyed(entity);
	}
}

template<typename T>
std::shared_ptr<ComponentArray<T>> ComponentManager::getComponentArray() {
	const char* typeName = typeid(T).name();
	// assert component not registered
	return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
}