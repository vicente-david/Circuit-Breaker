#include "EntityManager.h"
#include <iostream>

EntityManager::EntityManager() {
	// initialize queue with usable entities
	for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
		usableIDs.push(entity);
	}
}

Entity EntityManager::createEntity() {

	// to-do: change into asserts
	// apparently that's faster and depending on compiler discarded in release mode
	if (existingEntities == MAX_ENTITIES) { 
		std::cout << "Too many entities" << std::endl;
		return -1; 
	}
	// snag entity id from available ids
	Entity e = usableIDs.front();
	// pop the id from the queue
	usableIDs.pop();
	// increment active entities
	existingEntities++;
	// return the entity
	return e;
}

// destroys specified entity and frees up the id
void EntityManager::destroyEntity(Entity entity) {
	if (entity > MAX_ENTITIES) {
		std::cout << "Entity out of range" << std::endl;
		return;
	}
	// reset the signature 
	entitySignatures[entity].reset();
	// push id back into usable ids
	usableIDs.push(entity);
	// decrement active entity
	existingEntities--;
}

// sets the specified entity's signature
void EntityManager::setSignature(Entity entity, Signature signature) {
	if (entity > MAX_ENTITIES) {
		std::cout << "Entity out of range" << std::endl;
		return;
	}
	entitySignatures[entity] = signature;
}
Signature EntityManager::getSignature(Entity entity) {
	if (entity > MAX_ENTITIES) {
		std::cout << "Entity out of range" << std::endl;
		// empty signature
		return Signature();
	}
	return entitySignatures[entity];
}