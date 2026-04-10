#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include "ShaderProgram.h"

// A singular particle
struct Particle {
	glm::vec3 position;
	glm::vec4 colour;
	float size;
	float life;

	glm::vec3 dir;
	float cameraDist; // squared distance to camera (needed for sorting translucent particles)

	bool operator<(const Particle& that) const {
		// reverse order
		return this->cameraDist > that.cameraDist;
	}

};

// A singular particle emitter: point that particles come out of
class ParticleEmitter {
public:

	ParticleEmitter(unsigned int maxNumParticles);

	void update(const double dt, const float sizeFactor, glm::vec3 cameraPos);
	void Draw(const ShaderProgram& shader);

	std::vector<Particle> particles; // container of particles
	unsigned int maxNumParticles = 100; // maximum number of particles that can be alive at one time

private:
	int FirstUnusedParticle(); // finds the index of last dead (first unused) particle
	void init();
	void SortParticles();

	unsigned int spawnNumPerFrame = 5; // how many particles to spawn in a frame
	unsigned int lastUsedParticle = 0; // last used particle in the particle container

	std::vector<GLfloat> positionData;
	std::vector<GLfloat> colourData;
	GLuint VBO, positionVBO, colourVBO;
	GLuint VAO;

};
