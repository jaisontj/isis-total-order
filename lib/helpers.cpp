#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <limits.h>
#include <cstdlib>
#include <ctime>
#include <mutex>

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
				Log::LOG_LEVEL = VERBOSE;
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

vector<FileLineContent> get_file_content(string filepath) {
	string line_content;
	uint32_t line_num = 0;
	vector<FileLineContent> file_content;
	ifstream file (filepath);
	if (file.is_open()) {
		while(getline(file, line_content)) {
			FileLineContent fc = {
				.content = line_content,
				.num = ++line_num
			};
			file_content.push_back(fc);
		}
		file.close();
	} else {
		cout<<"Unable to open file: "<<filepath<<endl;
		exit(0);
	}
	return file_content;
}

string get_hostname() {
	char hostname[_POSIX_HOST_NAME_MAX];
	if (gethostname(hostname, _POSIX_HOST_NAME_MAX) != 0) {
		perror("Unable to get hostname");
		exit(0);
	}
	return hostname;
}

int get_id_from_file_content(vector<FileLineContent> file_content) {
	for (auto const& line: file_content) {
		if (line.content == get_hostname()) {
			return line.num;
		}
	}
	return 0;
}

/**Assumption: id is the same as linenum on file**/
string get_hostname_from_id(vector<FileLineContent> file_content, uint32_t id) {
	for (auto const &line: file_content) {
		if (line.num == id) return line.content;
	}
	return "";
}

vector<string> get_hostnames_not_in_list(vector<FileLineContent> file_content, vector<uint32_t> host_ids) {
	vector<string> absent_list;
	for (auto const &line: file_content) {
		bool isPresent = false;
		for (auto const &host_id: host_ids) {
			if (line.num == host_id) {
				isPresent = true;
				break;
			}
		}
		if (!isPresent)  absent_list.push_back(line.content);
	}
	return absent_list;
}

