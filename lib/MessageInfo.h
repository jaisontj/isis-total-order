#ifndef MESSAGE_INFO_H
#define MESSAGE_INFO_H

#include "NetworkDataTypes.h"

struct MessageInfo {
	NetworkMessage message;
	size_t message_size;
	std::string hostname;
	std::string port;
	int retry_count;
	time_t expected_delivery_time;
};



#endif
