#pragma once 
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "ecs/System.h"

class ParticleSystem : public System {

public:
	ParticleSystem();
	void init();

	static std::shared_ptr<ParticleSystem> registerSystem(std::shared_ptr<Coordinator>& coord);

private:

};