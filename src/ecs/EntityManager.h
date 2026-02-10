#pragma once
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
using Entity = int;

// distributes ID's to entities
// keeps track of which entities are in use and which aren't
class EntityManager {

public:
	EntityManager();
	Entity createEntity();
	void destroyEntity(Entity entity);
	void setSignature(Entity entity, Signature signature);
	Signature getSignature(Entity entity);

private:
	// reuse of ids is permissible
	// when entity is destroyed, push that id to the back of the queue
	// when entity is created, use the id at the front of the queue
	std::queue<Entity> usableIDs{};

	// array of signatures, index corresponds to entity id
	// Signature is a type-alias for std::bitset<MAX_COMPONENTS>
	std::array<Signature, MAX_ENTITIES> entitySignatures{};
	// signature just means what components that entity has
	// it is just a bitset 
	// ex: 101 could correspond to having the transform component, not having a physics type, and having a model component


	// alive entities
	int existingEntities = 0;
	
};