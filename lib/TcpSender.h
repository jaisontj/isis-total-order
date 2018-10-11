#ifndef TCP_SENDER_H
#define TCP_SENDER_H

#include "SocketImpl.h"

class TcpSender: public SocketImpl {
	public:
		TcpSender(std::string hostname, std::string port);
		int send(const void *message, size_t message_size);
};

#endif
