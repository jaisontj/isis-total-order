#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <limits.h>

#include "custom_types.h"
using namespace std;

bool DEBUG = false;

void debug_print(string message) {
	if (DEBUG) cout<<message<<endl;
}

void show_usage_and_exit() {
	cout<<"Usage: -p port -h hostfile -c count"<<endl;
	exit(0);
}

CommandArgs parse_cmg_args(int argc, char* argv[]) {
	int opt, msg_count, temp_port;
	const char *port = NULL;
	string filepath;
	while((opt = getopt(argc, argv, "p:h:c:d")) != -1) {
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
			case 'd':
				DEBUG = true;
				break;
			default:
				show_usage_and_exit();
				break;
		}
	}

	if(port == NULL || msg_count == -1 || filepath == "")
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

