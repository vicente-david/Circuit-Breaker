#include "ParticleEmitter.h"

// vertices of the particles (instanced, so the particles in this system all share them)
static const GLfloat vertexBufData[] = {
	 -0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 -0.5f, 0.5f, 0.0f,
	 0.5f, 0.5f, 0.0f,
};

ParticleEmitter::ParticleEmitter(unsigned int maxNumParticles)
 {
	 this->maxNumParticles = maxNumParticles;
	 //particles.resize(maxNumParticles);
	 lastUsedParticle = 0;
	 init();
	}

void ParticleEmitter::init() {
	// Buffer creation
	GLuint VBO, positionVBO, colourVBO;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufData), vertexBufData, GL_STATIC_DRAW);

	// VBO containing positions and sizes of particles
	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, maxNumParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// VBO containing colours of particles
	glGenBuffers(1, &colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glBufferData(GL_ARRAY_BUFFER, maxNumParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	
	this->VBO = VBO;
	this->positionVBO = positionVBO;
	this->colourVBO = colourVBO;
}

/*
* Update particles in this system
*/
void ParticleEmitter::update(const double dt) {
	int particleCount = 0;

	if (particles.empty()) {
		// no alive particles, nothing to do
		return;
	}
	positionData.resize(maxNumParticles * 4 * sizeof(GLfloat));
	colourData.resize(maxNumParticles * 4 * sizeof(GLfloat));

	for (int i = 0; i < particles.size(); i++) {
		Particle& p = particles[i];

		if (p.life > 0.0f) {

			// update particle position based on collision velocity
			// TODO: make it behave better
			p.position += p.velocity * ((float)dt * 2.f);

			// fill GPU buffer
			positionData[4 * particleCount + 0] = p.position.x;
			positionData[4 * particleCount + 1] = p.position.y;
			positionData[4 * particleCount + 2] = p.position.z;
			positionData[4 * particleCount + 3] = p.size;

			colourData[4 * particleCount + 0] = p.colour.x;
			colourData[4 * particleCount + 1] = p.colour.y;
			colourData[4 * particleCount + 2] = p.colour.z;
			colourData[4 * particleCount + 3] = p.colour.w;

			p.life -= dt;
			particleCount++;
		}
		else {
			particles.erase(particles.begin() + i);
			i--;
		}
	}
	// update buffers
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, maxNumParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // buffer orphaning
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount * sizeof(GLfloat) * 4, positionData.data());

	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glBufferData(GL_ARRAY_BUFFER, maxNumParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // buffer orphaning
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount * sizeof(GLfloat) * 4, colourData.data());

}

void ParticleEmitter::Draw(const ShaderProgram& shader) {
	shader.use();
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad -> 1

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles.size());
}

/*
* Find the first particle in the container that is not in use (life < 0)
*/
int ParticleEmitter::FirstUnusedParticle() {

	// check from the last used particle
	for (int i = lastUsedParticle; i < maxNumParticles; i++) {
		if (particles.at(i).life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// no unused were found, check others
	for (int i = 0; i < lastUsedParticle; i++) {
		if (particles.at(i).life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// if all have been used, override the first
	return 0;
}
