#ifndef TCP_LISTENER_H
#define TCP_LISTENER_H

#include "SocketImpl.h"
#include "Marker.h"

typedef void (*TcpMessageHandler)(Marker);
class TcpListener: public SocketImpl {
	public:
		TcpListener(std::string port);
		void start_listening(TcpMessageHandler handler);
};

#endif
