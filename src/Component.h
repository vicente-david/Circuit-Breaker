#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// consider these components as data that you attach to entities
// each component will have an id 
// so whichever system needs things about the entity
// the system will have a bit signature that it compares against to grab
// the respective component(s) 

enum class PhysType {
	None,		// doesn't have any physics properties
	StaticMesh, // non-moving object w/ collision box (i.e. track/walls)
	Spark,		// vehicle, in this case, the sparks
	RigidBody	// anything else that has physical properties and moves
};

struct Transform {
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
};

struct Physics {
	PhysType physType = PhysType::None;
};

struct Model {
	Model* model;
};
