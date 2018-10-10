#ifndef LISTENER_SOCKET_H
#define LISTENER_SOCKET_H

#include "SocketImpl.h"
#include "NetworkDataTypes.h"
#include "MessageHandler.h"

class ListenerSocket: public SocketImpl {
	public:
		ListenerSocket(std::string port);
		void start_listening(MessageHandler &handler);
};


#endif
