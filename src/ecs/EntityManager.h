#pragma once
#include "debugUtils/Logger.h"
#include <cstdio>
#include <queue>
#include <bitset>
#include <array>
#include <iostream>
// all entity manager does is to
// manage the creation and deletion of entities
// this means that giving an entity an id, or adding an id back into usable ids
// it also means setting that entity's signature, or grabbing that

// IT DOES NOT ADD COMPONENTS 



// to-do: move these to cmake
#define MAX_ENTITIES 2000
// this constant is also in entity.h
#define MAX_COMPONENTS 32

using Signature = std::bitset<MAX_COMPONENTS>;
using EcsEntity = int;

// distributes ID's to entities
// keeps track of which entities are in use and which aren't
class EntityManager {

public:

	EntityManager() {
		// initialize queue with usable entities
		for (EcsEntity entity = 0; entity < MAX_ENTITIES; entity++) {
			usableIDs.push(entity);
		}
	}

	EcsEntity createEntity() {

		// to-do: change into asserts
		// apparently that's faster and depending on compiler discarded in release mode
		dbug::log("ECS",0, "Creating entity %d/%d", existingEntities, MAX_ENTITIES);
		if (existingEntities == MAX_ENTITIES) {
			std::cout << "Too many entities" << std::endl;
			return -1;
		}
		// snag entity id from available ids
		EcsEntity e = usableIDs.front();
		// pop the id from the queue
		usableIDs.pop();
		// increment active entities
		existingEntities++;
		// return the entity
		return e;
	}

	// destroys specified entity and frees up the id
	void destroyEntity(EcsEntity entity) {
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
	void setSignature(EcsEntity entity, Signature signature) {
		if (entity > MAX_ENTITIES) {
			std::cout << "Entity out of range" << std::endl;
			return;
		}
		entitySignatures[entity] = signature;
	}

	Signature getSignature(EcsEntity entity) {
		if (entity > MAX_ENTITIES) {
			std::cout << "Entity out of range" << std::endl;
			// empty signature
			return Signature();
		}
		return entitySignatures[entity];
	}

private:
	// reuse of ids is permissible
	// when entity is destroyed, push that id to the back of the queue
	// when entity is created, use the id at the front of the queue
	std::queue<EcsEntity> usableIDs{};

	// array of signatures, index corresponds to entity id
	// Signature is a type-alias for std::bitset<MAX_COMPONENTS>
	std::array<Signature, MAX_ENTITIES> entitySignatures{};
	// signature just means what components that entity has
	// it is just a bitset 
	// ex: 101 could correspond to having the transform component, not having a physics type, and having a model component


	// alive entities
	int existingEntities = 0;
	
};
