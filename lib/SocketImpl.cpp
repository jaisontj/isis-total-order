#include "SocketImpl.h"
#include "Log.h"

#include <netdb.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

// get sockaddr IPv4 or IPv6
void* SocketImpl::get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string SocketImpl::get_packet_address(sockaddr_storage recv_addr) {
	char s[INET6_ADDRSTRLEN];
	inet_ntop(recv_addr.ss_family, get_in_addr((struct sockaddr *)&recv_addr), s, sizeof(s));
	cout<<"Listener: got packet from "<<s<<endl;
	return s;
}

struct addrinfo SocketImpl::init_dgram_hints(int type, int flags) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	//Can set the below to AF_INET to force IPv4
	//hints.ai_family = AF_INET;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = type;
	if (flags != -1)
		hints.ai_flags = flags;
	return hints;
}

struct addrinfo* SocketImpl::get_addr_info(
		const char *hostname,
		const char *port,
		struct addrinfo *hints) {
	struct addrinfo *servinfo;
	string h_name = hostname == NULL ? "NULL" : hostname;
	Log::v("Getting AddressInfo: Hostname->" + string(h_name) + " Port->" + port);
	int rv = getaddrinfo(hostname, port, hints, &servinfo);
	if (rv != 0) {
		Log::e("getaddrinfo error: " + string(gai_strerror(rv))
				+ " Hostname->" + string(hostname)
				+ " Port->" + string(port));
		throw string("Unable to getaddrinfo");
	}
	return servinfo;
}

string SocketImpl::get_socket_ip(addrinfo *p) {
	char ipstr[INET6_ADDRSTRLEN];
	void *addr = (char *)ipstr;
	inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
	return ipstr;
}

SocketImpl::SocketImpl(
		int type,
		int hint_flag,
		string hostname,
		string port,
		int should_reuse_addr,
		bool should_bind_to_address
		) {
	struct addrinfo hints = init_dgram_hints(type, hint_flag);
	this->servinfo =  get_addr_info(hostname == "" ? NULL : hostname.c_str(), port.c_str(), &hints);
	int socket_fd;
	struct addrinfo *p;
	//Loop through all results to make a socket
	for(p=servinfo; p != NULL; p=p->ai_next) {
		socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socket_fd == -1) {
			perror("Unable to create socket");
			continue;
		}
		break;
	}

	if (socket_fd == -1 || p == NULL) {
		perror("Unable to create socket");
		throw string("Unable to create socket: fd or p were null");
	}

	if (should_reuse_addr == 1) {
		int err = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &should_reuse_addr, sizeof(int));
		if (err) {
			perror("Failed to ");
			throw string("Failed to set socket for reuse");
		}
	}

	Log::v("Socket created with ip: " + get_socket_ip(p));
	this->hostname = hostname;
	this->port = port;
	this->fd = socket_fd;
	this->p = p;

	if (should_bind_to_address) {
		Log::v("SocketImpl:: Trying to bind socket to address");
		if (::bind(this->fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(this->fd);
			perror("SocketImpl:: Failed to bind socket");
			throw std::string ("SocketImpl:: Failed to bind socket");
		}
	}
}

void SocketImpl::free_serve_info() {
	Log::v("Freeing servinfo");
	freeaddrinfo(servinfo);
}

void SocketImpl::close_socket() {
	Log::v("Closing socket: " + to_string(fd));
	close(fd);
}
