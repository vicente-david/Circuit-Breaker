#pragma once 
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "GameState.h"
#include "ecs/System.h"
#include "ParticleEmitter.h"
#include "ShaderProgram.h"


class ParticleSystem : public System {

public:

	void init();
	void update(GameState& game, const double dt);

	static std::shared_ptr<ParticleSystem> registerSystem(std::shared_ptr<Coordinator>& coord);

	void addParticleBurst(Particle particle, unsigned int spawnNum);

	glm::mat4* proj; //projection from rendering system


private:
	std::unique_ptr<ParticleEmitter> atkDmgEmitter;
	std::vector<std::unique_ptr<ParticleEmitter>> emitterList;
	std::unique_ptr<ShaderProgram> shader;
	

};