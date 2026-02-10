#include <unordered_map>
#include <memory>
#include "ComponentArray.cpp"
// will manage addition and removal of components between the various component arrays
// note: every component does have it's own id or bit in the signature

using ComponentType = size_t;

class ComponentManager {
public:
	template<typename T>
	void registerComponent();

	template<typename T>
	ComponentType getComponentType();

	template<typename T>
	void addComponent(Entity entity, T component);

	template<typename T>
	void removeComponent(Entity entity);

	template<typename T>
	T& getComponent(Entity entity);

	void entityDestroyed(Entity entity);


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
	std::shared_ptr<ComponentArray<T>> getComponentArray();

};