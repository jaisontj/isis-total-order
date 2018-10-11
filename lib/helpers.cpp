#include <iostream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <limits.h>
#include <cstdlib>
#include <ctime>

#include "helpers.h"
#include "Log.h"
#include "NetworkDataTypes.h"
#include "ProcessInfoHelper.h"
#include "MessageDispatcher.h"
#include "NetworkStatus.h"

using namespace std;

string& trim_string(string &str) {
	const string &trim_chars = "\t\n\f\v\r ";
	//trim from the left
	str.erase(0, str.find_first_not_of(trim_chars));
	//trim from the right
	str.erase(str.find_last_not_of(trim_chars) + 1);
	return str;
}

void show_usage_and_exit() {
	cout<<"Usage: -p port -h hostfile -c count"<<endl;
	exit(0);
}

CommandArgs parse_cmg_args(int argc, char* argv[]) {
	int opt, msg_count, temp_port, x = -1;
	string port = "";
	string filepath;
	while((opt = getopt(argc, argv, "p:h:c:v:d:lx:")) != -1) {
		switch(opt) {
			case 'p':
				temp_port = atoi(optarg);
				if (temp_port < 1024 || temp_port > 65535) {
					cout<<"The value for port (p) must be a valid number between 1024 and 65535"<<endl;
					exit(0);
				}
				port = optarg;
				break;
			case 'h':
				filepath = optarg;
				break;
			case 'c':
				msg_count = atoi(optarg);
				break;
			case 'v':
				if (strcmp(optarg, "debug") == 0) Log::LOG_LEVEL = DEBUG;
				if (strcmp(optarg, "verbose") == 0) Log::LOG_LEVEL = VERBOSE;
				if (strcmp(optarg, "error") == 0) Log::LOG_LEVEL = ERROR;
				if (strcmp(optarg, "info") == 0) Log::LOG_LEVEL = INFO;
				break;
			case 'd':
				NetworkStatus::DELIVERY_DELAY = atoi(optarg);
				break;
			case 'l':
				NetworkStatus::DROPS_MESSAGE = true;
				break;
			case 'x':
				x = atoi(optarg);
				break;
			default:
				show_usage_and_exit();
				break;
		}
	}

	if(port == "" || msg_count == -1 || filepath == "")
		show_usage_and_exit();
	return (CommandArgs) { msg_count, port, filepath, x };
}

void send_message(NetworkMessage *message, size_t message_size, vector<ProcessInfo> processes) {
	for (auto const& process: processes) {
		string hostname = process.hostname;
		string port = process.port;
		MessageDispatcher::get_instance().add_message_to_queue(message, message_size, hostname.c_str(), port);
	}
}

void send_message(NetworkMessage *message, size_t message_size, ProcessInfo process) {
	MessageDispatcher::get_instance().add_message_to_queue(message, message_size, process.hostname, process.port);
}





