#ifndef LOG_H
#define LOG_H

#include<string>

enum LogLevel { NONE, DEBUG, VERBOSE, INFO, ERROR };

class Log {
	public:
		static LogLevel LOG_LEVEL;
		static void v(std::string message);
		static void d(std::string message);
		static void e(std::string message);
		static void i(std::string message);
};

#endif
