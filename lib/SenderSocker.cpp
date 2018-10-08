#include<netdb.h>
#include<string>
#include<iostream>

#include"SenderSocket.h"
#include "Log.h"

SenderSocket::SenderSocket(std::string hostname, std::string port): DgramSocket(-1, hostname, port, 1) {
	Log::v("Created Sender Socket: Hostname->" + hostname + " Port->" + port);
}

int SenderSocket::send(void *message, size_t message_size) {
	return sendto(this->fd, message, message_size, 0, p->ai_addr, p->ai_addrlen);
}

