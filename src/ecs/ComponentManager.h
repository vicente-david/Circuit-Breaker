#include <unordered_map>
#include <memory>
#include "ComponentArray.h"
// will manage addition and removal of components between the various component arrays
// note: every component does have it's own id or bit in the signature

using ComponentType = size_t;

class ComponentManager {
public:

	template<typename T>
	void registerComponent() {
		const char* typeName = typeid(T).name();

		// assert mComponentTypes.find(typeName) == mComponentTypes.end()  multiple of same component

		componentTypes.insert({ typeName, nextComponentType });
		componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
		nextComponentType++;
	}

	template<typename T>
	ComponentType getComponentType() {
		const char* typeName = typeid(T).name();

		return componentTypes[typeName];
	}

	template<typename T>
	void addComponent(EcsEntity entity, T component) {
		getComponentArray <T>()->insertData(entity, component);
	}

	template<typename T>
	void removeComponent(EcsEntity entity) {
		getComponentArray <T>()->removeData(entity);
	}

	template<typename T>
	T& getComponent(EcsEntity entity) {
		return getComponentArray < T >()->getData(entity);
	}

	void entityDestroyed(EcsEntity entity) {
		// notify each component array that an entity has been destroyed
		// if that component array has that entity, remove it
		for (auto const& pair : componentArrays) {
			auto const& component = pair.second;
			component->entityDestroyed(entity);
		}
	}


private:
	// map from string pointer to component type
	std::unordered_map<const char*, ComponentType> componentTypes{};

	// map from string pointer to component array
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays{};

	// type to be assigned to next registered component starts at 0
	ComponentType nextComponentType{};

	// takes a component of type T
	// returns a pointer to the component array of type T
	template<typename T>
	std::shared_ptr<ComponentArray<T>> getComponentArray() {
		const char* typeName = typeid(T).name();
		// assert component not registered
		return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
	}

};
