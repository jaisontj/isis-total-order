#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <thread>

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

struct addrinfo* get_addr_info(const char *hostname, const char *port, struct addrinfo *hints) {
	struct addrinfo *servinfo;
	string h_name = hostname == NULL ? "NULL" : hostname;
	debug_print("Getting AddressInfo: Hostname->" + string(h_name) + " Port->" + port);
	int rv = getaddrinfo(hostname, port, hints, &servinfo);
	if (rv != 0) {
		std::cout<<"getaddrinfo error: "<<gai_strerror(rv)<<std::endl;
		exit(1);
	}
	return servinfo;
}

string get_socket_ip(addrinfo *p) {
	char ipstr[INET6_ADDRSTRLEN];
	void *addr = (char *)ipstr;
	inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
	return ipstr;
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

	if (socket_fd == -1 || p == NULL) {
		perror("Unable to create socket");
		exit(1);
	}

	if (should_reuse_addr == 1) {
		int err = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &should_reuse_addr, sizeof(int));
		if (err) {
			perror("Failed to ");
			exit(1);
		}
	}

	debug_print("Socket created with ip: " + get_socket_ip(p));

	return (SocketInfo) { socket_fd, p };
}

string& trim_string(string &str) {
	const string &trim_chars = "\t\n\f\v\r ";
	//trim from the left
	str.erase(0, str.find_first_not_of(trim_chars));
	//trim from the right
	str.erase(str.find_last_not_of(trim_chars) + 1);
	return str;
}

int send_message_to_host(
		const char *hostname,
		const char *port,
		void *message,
		size_t message_size
		) {
	if (should_drop_message()) {
		debug_print("Dropping message...");
		return 16;
	}

	int delay = get_message_delay();
	debug_print("Delay amount: " + to_string(delay));
	sleep(delay);
	string h_name = hostname == NULL ? "NULL" : hostname;
	trim_string(h_name);
	if (h_name.empty()) {
		debug_print("Received empty hostname. Ignoring....");
		return -1;
	}
	struct addrinfo hints = init_dgram_hints();
	struct addrinfo *servinfo = get_addr_info(hostname, port, &hints);

	SocketInfo s_info = create_first_possible_socket(servinfo, 1);
	int socket_fd = s_info.fd;
	struct addrinfo *p = s_info.addr;
	int sent_bytes = sendto(socket_fd, message, message_size, 0, p->ai_addr, p->ai_addrlen);
	freeaddrinfo(servinfo);
	close(socket_fd);
	return sent_bytes;
}

string get_packet_address(sockaddr_storage recv_addr) {
	char s[INET6_ADDRSTRLEN];
	inet_ntop(recv_addr.ss_family, get_in_addr((struct sockaddr *)&recv_addr), s, sizeof(s));
	cout<<"Listener: got packet from "<<s<<endl;
	return s;
}


