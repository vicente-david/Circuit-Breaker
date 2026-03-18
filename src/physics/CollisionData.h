#pragma once
#include "ecs/EntityManager.h"

enum UserPhysicsType {
	GROUND,
	SPARK_BODY,
	SPARK_GROUND,
	FINISH_LINE,
	HEAL,
	TESTING,
};

// all physics actors should have one of these in its userdata so we
// can actually know info about the things colliding, but still have some type
// safety just like extend it for the data needed for each type
struct CollisionData {
	UserPhysicsType type = TESTING;
	Entity entity = -1;
};

