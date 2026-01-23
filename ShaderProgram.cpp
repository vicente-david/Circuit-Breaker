#include "ShaderProgram.h"
#include <vector>
#include <iostream>

ShaderProgram::ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) 
	: vertex(GL_VERTEX_SHADER, vertexPath), fragment(GL_FRAGMENT_SHADER, fragmentPath) {

	id = glCreateProgram();

	// Link program
	attachShader(*this, vertex);
	attachShader(*this, fragment);
	glLinkProgram(id);

	// Delete shaders after linking
	shaderCleanup(vertex, fragment);


	// Error check
	if (!checkLink()) {
		throw std::runtime_error("Shader program linking failure");
		glDeleteProgram(id);
	}
	
}

void attachShader(ShaderProgram& prog, Shader& shad) {
	glAttachShader(prog.id, shad.id);
}

void shaderCleanup(Shader& vert, Shader& frag) {

	glDeleteShader(vert.id);
	glDeleteShader(frag.id);
}

bool ShaderProgram::checkLink() {
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