#pragma once
#include "PxRigidDynamic.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// consider these components as data that you attach to entities
// each component will have an id 
// so whichever system needs things about the entity
// the system will have a bit signature that it compares against to grab
// the respective component(s) 

enum class PhysType1 {
	None,		// doesn't have any physics properties
	StaticMesh, // non-moving object w/ collision box (i.e. track/walls)
	Spark,		// vehicle, in this case, the sparks
	RigidBody	// anything else that has physical properties and moves
};

struct Physics1 {
	PhysType1 physType = PhysType1::None;
};


class Transform
{
public:
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 forwardD;
};
