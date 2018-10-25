#ifndef SOCKET_IMPL_H
#define SOCKET_IMPL_H

#include<string>
#include<arpa/inet.h>

class SocketImpl {

	struct addrinfo *servinfo;
	static struct addrinfo init_dgram_hints(int type, int flags);
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
	SocketImpl(int type, int hint_flag, std::string hostname, std::string port, int should_reuse_addr, bool should_bind_to_address);
	void free_serve_info();
	void close_socket();
};

#endif

