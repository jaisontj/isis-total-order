#include <string>

using namespace std;

struct CommandArgs {
	int msg_count;
	string port;
	string filename;
};

struct SocketInfo {
	int fd;
	struct addrinfo *addr;
};


