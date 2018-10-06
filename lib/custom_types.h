#include <string>

using namespace std;

struct FileLineContent {
	string content;
	uint32_t num;
};

struct CommandArgs {
	int msg_count;
	const char *port;
	string filename;
};

struct SocketInfo {
	int fd;
	struct addrinfo *addr;
};


