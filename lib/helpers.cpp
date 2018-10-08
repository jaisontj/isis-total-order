#include <iostream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <limits.h>
#include <cstdlib>
#include <ctime>

#include "custom_types.h"
#include "Log.h"

using namespace std;

bool DROP_MESSAGE = false;
int delay_amount = 0;

string& trim_string(string &str) {
	const string &trim_chars = "\t\n\f\v\r ";
	//trim from the left
	str.erase(0, str.find_first_not_of(trim_chars));
	//trim from the right
	str.erase(str.find_last_not_of(trim_chars) + 1);
	return str;
}

int get_message_delay() {
	return delay_amount;
}

void simulate_delay_if_needed() {
	int delay = get_message_delay();
	Log::d("Delay in handling message: " + to_string(delay));
	sleep(delay);
}

bool should_drop_message() {
	if (DROP_MESSAGE) {
		//decide randomly
		srand(time(0));
		return rand() % 2 == 0;
	}
	return false;
}

void show_usage_and_exit() {
	cout<<"Usage: -p port -h hostfile -c count"<<endl;
	exit(0);
}

CommandArgs parse_cmg_args(int argc, char* argv[]) {
	int opt, msg_count, temp_port;
	string port = "";
	string filepath;
	while((opt = getopt(argc, argv, "p:h:c:vd:l")) != -1) {
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
				Log::LOG_LEVEL = DEBUG;
				break;
			case 'd':
				delay_amount = atoi(optarg);
				break;
			case 'l':
				DROP_MESSAGE = true;
				break;
			default:
				show_usage_and_exit();
				break;
		}
	}

	if(port == "" || msg_count == -1 || filepath == "")
		show_usage_and_exit();
	return (CommandArgs) { msg_count, port, filepath };
}


