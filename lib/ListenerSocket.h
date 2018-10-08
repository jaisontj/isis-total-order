#ifndef LISTENER_SOCKET_H
#define LISTENER_SOCKET_H

#include "DgramSocket.h"
#include "NetworkDataTypes.h"

typedef void (*SocketMessageHandler)(NetworkMessage message);

class ListenerSocket: public DgramSocket {
	public:
		ListenerSocket(std::string port);
		void start_listening(SocketMessageHandler handler);
};


#endif
