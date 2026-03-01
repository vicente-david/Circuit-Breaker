#pragma once
#include <set>
#include <string>

namespace dbug {
enum IgnoreType { BLACK_LIST = false, WHITE_LIST = true };

extern bool printLogs;
extern int minLogSeverity;
extern std::string logPath;
extern std::set<std::string> logIgnoreTags;

extern IgnoreType logListType;
void loggerInit();

// specify the severity of the log
// in general
// 0 = logging info to know whats going on (nothing is wrong)
// 1 = warnings (things might be weird, but not necissarily wrong)
// 2 = err (this shouldn't happen)
// (you can use negative/bigger numbers if you 
// really want something to be hidden or shown)
void log(int level, std::string, ...);
void log(std::string tag, int level, std::string, ...);
// log with default 0 priority and DEF tag
void logd(std::string, ...);

// add a type to the black/whitelist of types to use
void logIgnore(std::string tag);

} // namespace dbug
