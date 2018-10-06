#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>

#include "helpers.h"

using namespace std;

struct addrinfo init_dgram_hints(int flags) {
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));

	//Can set the below to AF_INET to force IPv4
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if (flags != -1)
		hints.ai_flags = flags;

	return hints;
}

struct addrinfo init_dgram_hints() {
	return init_dgram_hints(-1);
}

// get sockaddr IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct addrinfo *get_addr_info(const char* hostname, const char *port, struct addrinfo *hints) {
	struct addrinfo *servinfo;
	string h_name = hostname == NULL ? "NULL" : string(hostname);
	debug_print("\nget_addr_info: \n\tTrying to resolve for: Hostname: " + h_name + "  Port: " + port);
	int rv = getaddrinfo(hostname, port, hints, &servinfo);
	if (rv != 0) {
		std::cout<<"getaddrinfo error: "<<gai_strerror(rv)<<std::endl;
	}
	return servinfo;
}


struct SocketInfo create_first_possible_socket(struct addrinfo *servinfo, int should_reuse_addr) {
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

	if (should_reuse_addr == 1) {
		int err = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &should_reuse_addr, sizeof(int));
		if (err) {
			perror("Failed to ");
			exit(1);
		}
	}

	if (p != NULL) {
		char ipstr[INET6_ADDRSTRLEN];
		void *addr = (char *)ipstr;
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		cout<<"Socket created with ip: "<<ipstr<<endl;
	}
	return (SocketInfo) { socket_fd, p };
}
