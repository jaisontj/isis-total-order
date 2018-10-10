#ifndef LOG_HELPER_H
#define LOG_HELPER_H

#include "NetworkDataTypes.h"
#include "MessageInfo.h"

std::string get_as_string(NetworkMessage *message);
std::string get_as_string(MessageInfo message);
void log_line();

#endif
