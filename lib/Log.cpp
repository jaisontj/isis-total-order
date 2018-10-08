#include "Log.h"
#include <string>
#include <iostream>

void Log::v(std::string message) {
	if (LOG_LEVEL == VERBOSE)
		std::cout<<message<<std::endl;
}

void Log::d(std::string message) {
	if (LOG_LEVEL == DEBUG || LOG_LEVEL == VERBOSE)
		std::cout<<message<<std::endl;
}

void Log::e(std::string message) {
	if (LOG_LEVEL == ERROR || LOG_LEVEL == DEBUG || LOG_LEVEL == VERBOSE)
		std::cout<<message<<std::endl;
}
