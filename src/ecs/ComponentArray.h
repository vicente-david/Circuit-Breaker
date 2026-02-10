#include "EntityManager.h"
#include <array>
#include <unordered_map>
// this is probably the bread and butter of the ECS system

// component array implementation from https://austinmorlan.com/posts/entity_component_system/

// essentially what we're doing is making a "packed" array
// an array that never has missing data
// this is necessary because we want to avoid if(entity exists) checks
// because we'll have a lot of entities
// the above and the fact that we want each entity's components to be valid at all times

// the implementation below is the following:
// we have a map that maps an entity to an array index
// we also have a map that maps an array index to an entity (useful for shifting entity index)
// on entity deletion we take last valid entity data and move it to where the deletion occured

// we'll eventually have a list of all component arrays
// we want to notify all of them when an entity is destroyed (so it can update accordingly)
// So if we keep a list of their common interfaces so we can call entity destroyed on them
class IComponentArray {
public:
	// this is a virtual destructor
	// this is a c++ gimmick where if you have a virtual destructor
	// you can delete a derived class by a base class pointer
	// essentially does proper cleanup
	virtual ~IComponentArray() = default;

	virtual void entityDestroyed(Entity entity) = 0;
};


// T here is just any component
// Ex: Transform component contains Position, Rotation, and Scale  
// you could create ComponentArray<Transform> 
// you have all Transform components of every entity that has the Transform component
template<typename T>
class ComponentArray : public IComponentArray {
public:

	
	void insertData(Entity entity, T component) {
		// to-do: asserts
		// they're basically compiler only if statements, they disappear in release mode
		// assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to same entity more than once.");

		// new index is right after the last spot (since everything is assumed to be tightly packed)
		size_t newIndex = validEntries;
		// update maps
		entityToIndex[entity] = newIndex;
		indexToEntity[newIndex] = entity;
		// add the component
		componentArray[newIndex] = component;
		// update valid entries
		validEntries++;

	}

	// replacing the current last element into the to be deleted element spot
	void removeData(Entity entity) {
		// todo: assert (removal of nonexistent component)

		// whatever we are removing, note the index
		size_t removedIndex = entityToIndex[entity];
		// current last index
		size_t lastIndex = validEntries - 1;

		// replace the data of the removed entity, with that of the last entity
		componentArray[removedIndex] = componentArray[lastIndex];

		// update maps
		Entity lastEntity = indexToEntity[lastIndex];
		entityToIndex[lastEntity] = removedIndex;
		indexToEntity[removedIndex] = lastEntity;

		// update entries
		validEntries--;

	}

	// gets the data of a component
	T& getData(Entity entity) {
		// assert: non existent entity

		return componentArray[entityToIndex[entity]];
	}

	// if an entity was destroyed 
	void entityDestroyed(Entity entity) override{

		// this if statement just means if the entity was found destroy it
		if (entityToIndex.find(entity) != entityToIndex.end()) {
			removeData(entity);
		}
	}

private:
	// this is the tightly packed array of a component T for all entities that have T as a component
	// ex: Transform component array
	std::array<T, MAX_ENTITIES> componentArray;

	// map entity id to index in above array
	std::unordered_map<Entity, size_t> entityToIndex;

	// map index to entity id in above array
	std::unordered_map<size_t, Entity> indexToEntity;
	
	// how many valid entries are in the array
	size_t validEntries;
};