#include "Entity.h"
#include <queue>

#define MAX_ENTITIES 20

// distributes ID's to entities
// keeps track of which entities are in use and which aren't
class EntityManager {

public:
	EntityManager();
	Entity createEntity();

private:
	// reuse of ids is permissible
	// when entity is destroyed, push that id to the back of the queue
	// when entity is created, use the id at the front of the queue
	std::queue<Entity> usableIDs{};

	// alive entities
	int existingEntities;
	
};