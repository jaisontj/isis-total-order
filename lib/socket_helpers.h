struct addrinfo init_dgram_hints(int);
struct addrinfo init_dgram_hints();

void *get_in_addr(struct sockaddr *);
struct addrinfo *get_addr_info(const char*, const char *, struct addrinfo *);
struct socket_info create_first_possible_socket(struct addrinfo *, int);
