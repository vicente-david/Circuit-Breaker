#pragma once
#include "PxRigidDynamic.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// consider these components as data that you attach to entities
// each component will have an id
// so whichever system needs things about the entity
// the system will have a bit signature that it compares against to grab
// the respective component(s)

// note: any chunk of data (aka a class/struct) can be a component!

// this has misc components that don't really fit well in any specific folder
class Transform {
  public:
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 forwardD;
	float oldTiltAngle = 0.f; // the only purpose of this is lerp the mesh render poses
};
