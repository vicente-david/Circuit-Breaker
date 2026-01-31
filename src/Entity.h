#pragma once
#include <string>
#include "Transform.h"
#include "Model.h"

// class to store types of physics for each object
enum class PhysType {
	None,		// doesn't have any physics properties
	StaticMesh, // non-moving object w/ collision box (i.e. track/walls)
	Spark,		// vehicle, in this case, the sparks
	RigidBody	// anything else that has physical properties and moves
};

class Entity
{
public:
	std::string name;
	PhysType physType = PhysType::None;
	Model* model;
	Transform* transform;
};