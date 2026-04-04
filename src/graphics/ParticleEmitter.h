#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>

// A singular particle
struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 colour;
	float size;
	float life;

};

// A singular particle emitter: point that particles come out of
class ParticleEmitter {
public:

	void ParticleEmitter(unsigned int maxNumParticles) {
		this->maxNumParticles = maxNumParticles;
		init();
	}

	void update();
	void Draw();

	std::vector<Particle> particles; // container of particles

private:
	int FirstUnusedParticle(); // finds the index of last dead (first unused) particle
	void init();

	unsigned int maxNumParticles = 100; // maximum number of particles that can be alive at one time
	unsigned int spawnNumPerFrame = 5; // how many particles to spawn in a frame

	unsigned int lastUsedParticle = 0; // last used particle in the particle container

	// vertices of the particles (instanced, so the particles in this system all share them)
	static const GLfloat vertexBufData[] = {
		 -0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 -0.5f, 0.5f, 0.0f,
		 0.5f, 0.5f, 0.0f,
	};
	GLfloat positionData, colourData;
	GLuint VBO, positionVBO, colourVBO;

};