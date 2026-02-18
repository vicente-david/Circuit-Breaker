#pragma once
#include "EntityManager.h"
#include <set>

// all a system will have is a list of entities
// from that they can do component signature comparison via the coordinator
class System {
public:
	std::set<EcsEntity> entities;
};
