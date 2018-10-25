#include "TcpListener.h"
#include "Log.h"
#include "Marker.h"

#include <unistd.h>
#include <netdb.h>
#include <string>

using namespace std;

TcpListener::TcpListener(std::string port) : SocketImpl(SOCK_STREAM, AI_PASSIVE, "", port, 1, true) {
	free_serve_info();
}

void TcpListener::start_listening(TcpMessageHandler handler) {

	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number

	int newfd;        // newly accept()ed socket descriptor

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);
	//Listen
	//Hold upto 10 pending connections queue
	if (listen(this->fd, 10) == -1) {
		perror("TcpListener:: listen");
		throw string("TcpListener Error");
	}

	//add listener to the master set
	FD_SET(this->fd, &master);

	//Keep track of the biggest file descriptor
	//Defaults to the one creator with this class
	fdmax = this->fd;

	while(true) {
		//Copy master socket
		read_fds = master;
		Log::v("TcpListener:: Waiting for activity on sockets");
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("TcpListener:: select");
			throw string("TcpListener:: Error at select");
		}
		Log::v("TcpListner: Activity.....");
		//Go through existing connections, check if there is data to be read.
		for (int i = 0; i<= fdmax;i++) {
			if (FD_ISSET(i, &read_fds)) {
				//New Connection
				if (i == this->fd) {
					struct sockaddr_storage remoteaddr;
					socklen_t addrlen = sizeof remoteaddr;
					newfd = accept(this->fd, (sockaddr *) &remoteaddr, &addrlen);
					if (newfd == -1) {
						perror("TcpListener: accept");
						Log::e("TcpListener: error getting new field descriptor");
					} else {
						//Add the new fd to master set
						FD_SET(newfd, &master);
						//Update max fd value
						if (newfd > fdmax) {
							fdmax = newfd;
						}
						Log::d("TcpListener:: New Connection!");
					}
				} else {
					//Received data.
					Marker *m = new Marker();
					int recv_bytes = recv(i, m, sizeof (Marker), 0);
					if (recv_bytes <= 0) {
						if (recv_bytes == 0) {
							Log::i("Socket " + to_string(i) + " hung up");
						} else {
							perror("TcpListener:: recv error");
						}
						//Close socket
						close(i);
						//Clear/remove from master set.
						FD_CLR(i, &master);
					} else if (recv_bytes > 0) {
						//Handle data!
						handler(*m);
					}
				}
			}
		}
	}
}

