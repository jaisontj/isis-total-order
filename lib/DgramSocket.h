#ifndef SOCKET_CREATOR_H
#define SOCKET_CREATOR_H

#include<string>
#include<arpa/inet.h>

class DgramSocket {

	struct addrinfo *servinfo;
	static struct addrinfo init_dgram_hints(int flags);
	static struct addrinfo* get_addr_info(const char *hostname, const char *port, struct addrinfo *hints);
	static std::string get_socket_ip(addrinfo *p);
	static void *get_in_addr(sockaddr *sa);
	static std::string get_packet_address(sockaddr_storage recv_addr);

	protected:
	int fd;
	struct addrinfo *p;
	std::string hostname;
	std::string port;

	public:
	DgramSocket(int hint_flag, std::string hostname, std::string port, int should_reuse_addr);
	void free_serve_info();
	void close_socket();
};

#endif
