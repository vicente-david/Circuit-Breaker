#pragma once
#include <string>

namespace dbug {
extern bool print;
extern int minSeverity;
extern std::string path;

void loggerInit();


// specify the severity of the log
// in general
// 0 = logging info to know whats going on (nothing is wrong)
// 1 = warnings (things might be weird, but not necissarily wrong)
// 2 = err (this shouldn't happen)
// template <typename... Args>
void log(int level, std::string, ...);
} // namespace dbug
