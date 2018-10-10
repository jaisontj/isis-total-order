#ifndef SENDER_SOCKET_H
#define SENDER_SOCKET_H

#include "SocketImpl.h"

class SenderSocket: public SocketImpl{
	public:
		SenderSocket(std::string hostname, std::string port);
		int send(void *message, size_t message_size);
};


#endif
