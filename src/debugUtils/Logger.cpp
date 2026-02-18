#include "Logger.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <set>
#include <string>

namespace dbug {
bool printLogs = true;
int minLogSeverity = 1;
std::string logPath = "debugLog.txt";
float startTime;
std::set<std::string> ignoreTypes;

IgnoreType logIgnoreType = BLACK_LIST;

static void appendLog(std::string type, int lev, std::string str,
					  va_list args) {
	FILE *file = fopen(logPath.c_str(), "a");

	if (file == NULL) {
		fprintf(stderr, "Err: Loging file couldn't be opened\n");
		return;
	}

	double t = glfwGetTime();
	fprintf(file, "%s (Lev: %d) [%6f]: ", type.c_str(), lev, t);
	vfprintf(file, (str + "\n").c_str(), args);
	fclose(file);
}

void logIgnore(std::string type) {
	ignoreTypes.emplace(type);
}
void loggerInit() {
	startTime = glfwGetTime();
	remove(logPath.c_str());

	log(0, " --== Starting Logging ==--");
}

void vlog(std::string type, int level, std::string str, va_list args) {
	if (level < minLogSeverity) {
		return;
	}

	if ((ignoreTypes.count(type) == 0) == logIgnoreType) {
		return;
	}

	if (printLogs) {
		va_list printArgs;
		va_copy(printArgs, args);
		std::vprintf((type+": "+str).c_str(), printArgs);
		std::printf("\n");
		va_end(printArgs);
	}

	appendLog(type, level, str, args);
	va_end(args);
}
// specify the severity of the log
// in general
// 0 = logging info to know whats going on (nothing is wrong)
// 1 = warnings (things might be weird, but not necissarily wrong)
// 2 = err (this shouldn't happen)
// (you can use negative/bigger numbers if you 
// really want something to be hidden or shown)
void log(std::string type, int level, std::string str, ...) {
	va_list args;
	va_start(args, str);
	vlog(type, level, str, args);
	va_end(args);
}
void log(int level, std::string str, ...) {
	va_list args;
	va_start(args, str);
	vlog("GEN", level, str, args);
	va_end(args);
}
void logd(std::string str, ...) {
	va_list args;
	va_start(args, str);
	vlog("GEN", 0, str, args);
	va_end(args);
}

} // namespace dbug
