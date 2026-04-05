#include "ParticleSystem.h"
#include "CameraComp.h"
#include <glm/gtc/type_ptr.hpp>


std::shared_ptr<ParticleSystem>ParticleSystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	// register system
	auto system = coord->registerSystem<ParticleSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<CameraComp>());
	coord->setSystemSignature<ParticleSystem>(sig);

	return system;
}

void ParticleSystem::init() {
	// init particle shader program
	shader = std::make_unique<ShaderProgram>("shaders/particle.vert", "shaders/particle.frag");

	// init particle emitters
	atkDmgEmitter = std::make_unique<ParticleEmitter>(100);
	//emitterList.push_back(atkDmgEmitter);
}

void ParticleSystem::update(GameState& game) {
	for (auto& entity : entities) {
		auto& camera = game.coordinator->getComponent<CameraComp>(entity);
		glm::mat4 view = camera.GetViewMatrix();
		// camera's up and right vectors in world space 
		glm::vec3 camUp = { view[0][1], view[1][1], view[2][1] };
		glm::vec3 camRight = { view[0][0], view[1][0], view[2][0] };
		glm::mat4 VPMatrix = *proj * view;

		shader->use();
		glUniform3f(glGetUniformLocation(shader->id, "cameraUp"), camUp.x, camUp.y, camUp.z);
		glUniform3f(glGetUniformLocation(shader->id, "cameraRight"), camRight.x, camRight.y, camRight.z);
		glUniformMatrix4fv(glGetUniformLocation(shader->id, "VP"), 1, GL_FALSE, glm::value_ptr(VPMatrix));
		atkDmgEmitter->update();
		atkDmgEmitter->Draw(*shader);
	}
	

}

// this is not a great way of doing this but its simple I guess
// This function is called when there is damage dealt by a spark to another spark and adds particles to render
// using the atkDmgEmitter
void ParticleSystem::addParticleBurst(Particle& particle, unsigned int spawnNum) {
	
	// Add specified number of the given particle to the particle list of the emitter
	for (int i = 0; i < spawnNum; i++) {
		// check to ensure that we don't use too many particles at once
		// for this emitter this shouldn't really be reached
		if (atkDmgEmitter->particles.size() < atkDmgEmitter->maxNumParticles) {
			atkDmgEmitter->particles.push_back(particle);
		}
		else {
			dbug::log("PARTICLES", 2, "Adding atkDmgEmitter particles failed, exceeded max particles in list");
		}
	}
}