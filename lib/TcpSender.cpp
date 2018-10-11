#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>


#include "TcpSender.h"
#include "Log.h"

TcpSender::TcpSender(std::string hostname, std::string port): SocketImpl(SOCK_STREAM, -1, hostname, port, -1, false) {
	Log::v("TcpSender:: Created Tcp Sender Socket: Hostname->" + hostname + " Port->" + port);
	Log::v("TcpSender:: Trying to connect socket to address");
	if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
		close(fd);
		throw std::string("Unable to connect socket to address");
	}
}

int TcpSender::send(const void *message, size_t message_size) {
	return ::send(this->fd, message, message_size, 0);
}
