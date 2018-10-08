#ifndef LOG_HELPER_H
#define LOG_HELPER_H

#include "NetworkDataTypes.h"

void log(NetworkMessage *message);
void log(DataMessage *message);
void log(AckMessage *message);
void log(SeqMessage *message);
void log_line();

#endif
