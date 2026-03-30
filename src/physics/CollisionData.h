#pragma once
#include "ecs/EntityManager.h"

enum UserPhysicsType {
	GROUND,
	WALL,
	SPARK,
	FINISH_LINE,
	HEAL,
	TESTING,
};

// all physics actors should have one of these in its userdata so we
// can actually know info about the things colliding, but still have some type
// safety just like extend it for the data needed for each type
class CollisionData {
	public:
	UserPhysicsType type = TESTING;
	Entity entity = -1;
};

