#ifndef SENDER_SOCKET_H
#define SENDER_SOCKET_H

#include "DgramSocket.h"

class SenderSocket: public DgramSocket {
	public:
		SenderSocket(std::string hostname, std::string port);
		int send(void *message, size_t message_size);
};


#endif
