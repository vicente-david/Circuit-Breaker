#pragma once
#include <string>
#include "Transform.h"
#include "Model.h"
#include <bitset>

#define MAX_COMPONENTS 5

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

	// theoretically an entity only needs an id 
	// id to identify entity 
	int id;
	// signature to identify which components are in use (temporary)
	std::bitset<MAX_COMPONENTS> signature;
};