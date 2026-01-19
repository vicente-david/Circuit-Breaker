#include "shaderProgram.h"
#include <vector>
#include <iostream>

shaderProgram::shaderProgram(const std::string& vertexPath, const std::string& fragmentPath) 
	: vertex(GL_VERTEX_SHADER, vertexPath), fragment(GL_FRAGMENT_SHADER, fragmentPath) {

	id = glCreateProgram();

	// Link program
	attachShader(*this, vertex);
	attachShader(*this, fragment);
	glLinkProgram(id);

	// Error check
	if (!link()) {
		throw std::runtime_error("Shader program linking failure");
		glDeleteProgram(id);
	}
	
}

void attachShader(shaderProgram& prog, shader& shad) {
	glAttachShader(prog.id, shad.id);
}

void shaderCleanup(shaderProgram& prog, shader& vert, shader& frag) {
	glDetachShader(prog.id, vert.id);
	glDetachShader(prog.id, frag.id);

	glDeleteShader(vert.id);
	glDeleteShader(frag.id);
}

bool shaderProgram::link() {
	GLint success;

	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success) {
		// Get log
		GLint logLength;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> errorLog(logLength);
		glGetProgramInfoLog(id, logLength, NULL, errorLog.data());
		std::cout << "ERROR: shader program linking failed. Log: " << errorLog.data() << std::endl;
		return false;
	}
	else {
		std::cout << "Shader program linked successfully." << std::endl;
		return true;
	}
}