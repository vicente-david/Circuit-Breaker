#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>


Shader::Shader(GLenum type, const std::string& path) : type(type), path(path) {

	// Create shader
	id = glCreateShader(type);

	if (!compile()) {
		throw std::runtime_error("Shader compilation failure");
	}

}

bool Shader::compile() {
	// Read from source file
	std::string shaderString;
	std::ifstream file(path);
 	file.exceptions(std::ifstream::failbit | std::ifstream::badbit); // Enable exceptions


	try {
		std::stringstream stream;

		// Read file's buffer contents into stream
		stream << file.rdbuf();

		file.close();

		// Convert to string
		shaderString = stream.str();

	}
	catch (std::ifstream::failure& e) {
		std::cout << "ERROR: shader file not successfully read at: " << path << std::endl;
	}

	// Compile shader
	const GLchar* shaderCode = shaderString.c_str();
	glShaderSource(id, 1, &shaderCode, NULL);
	glCompileShader(id);

	// Error check
	GLint success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) {
		// Get log
		GLint logLength;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> errorLog(logLength);
		glGetShaderInfoLog(id, logLength, NULL, errorLog.data());
		std::cout << "ERROR: shader compilation failed. Log: " << errorLog.data() << std::endl;
	}

	return success;
}