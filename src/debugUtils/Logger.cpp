#include "Logger.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <ios>
#include <string>

namespace dbug {
bool print = true;
int minSeverity = 1;
std::string path = "debugLog.txt";
float startTime;

static void appendLog(int lev, std::string str, va_list args) {
	FILE *file = fopen(path.c_str(), "a");

	if (file == NULL) {
		fprintf(stderr, "Err: Loging file couldn't be opened\n");
		return;
	}

	double t = glfwGetTime();
	fprintf(file, "(Lev: %d) [%6f]: ", lev, t);
	vfprintf(file, (str + "\n").c_str(), args);
	fclose(file);
}


void loggerInit() {
	startTime = glfwGetTime();
	remove(path.c_str());

	log(0, " --== Starting Logging ==--");
}


void log(int level, std::string str, ...) {
	if (level < minSeverity) {
		return;
	}

	va_list args;
	va_start(args, str);

	// str += "\n";
	if (print) {
		va_list printArgs;
		va_copy(printArgs, args);
		std::vprintf(str.c_str(), printArgs);
		std::printf("\n");
		va_end(printArgs);
	}

	appendLog(level, str, args);
	va_end(args);
}

} // namespace dbug
