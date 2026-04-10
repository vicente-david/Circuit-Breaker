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
	void addParticles(Particle particle, unsigned int spawnNum);

	// vars from rendering system
	int* SCR_WIDTH;
	int* SCR_HEIGHT;
	float* nearPlane;
	float* farPlane;

private:
	std::unique_ptr<ParticleEmitter> atkDmgEmitter;
	std::unique_ptr<ParticleEmitter> boostEmitter;
	std::vector<std::unique_ptr<ParticleEmitter>> emitterList;
	std::unique_ptr<ShaderProgram> shader;
	

};