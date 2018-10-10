#include "ProcessInfoHelper.h"

#include <fstream>
#include <vector>
#include <unistd.h>

void ProcessInfoHelper::init_from_file(std::string filepath, std::string port) {
	char name[255];
	uint32_t self_id = 0;

	if (gethostname(name, 255) != 0) {
		perror("Unable to get hostname");
		exit(0);
	}

	std::string line_content;
	uint32_t line_num = 0;
	std::vector<ProcessInfo> file_content;
	std::ifstream file (filepath);
	if (file.is_open()) {
		while(getline(file, line_content)) {
			ProcessInfo pi = {
				.hostname = line_content,
				.port = port,
				.id = ++line_num
			};
			file_content.push_back(pi);
			if (line_content == name) self_id = line_num;
		}
		file.close();
	} else {
		std::cout<<"Unable to open file: "<<filepath<<std::endl;
		exit(0);
	}

	if (self_id == 0) {
		std::cout<<"Unable to find hostname in provided file at "<<filepath<<". Please ensure that the hostname is provided in file."<<std::endl;
		exit(0);
	}

	ProcessInfoHelper::SELF = ProcessInfo {
		.hostname = name, .port = port, .id = self_id
	};

	ProcessInfoHelper::PROCESS_LIST = file_content;
}

ProcessInfo ProcessInfoHelper::get_process_info(uint32_t id) {
	for (auto const &process: ProcessInfoHelper::PROCESS_LIST) {
		if (process.id == id) return process;
	}
	return { .hostname = "", .port = "" };
}

std::vector<ProcessInfo> ProcessInfoHelper::get_processes_not_in_list(std::vector<uint32_t> process_ids) {
	std::vector<ProcessInfo> absent_list;
	for (auto const &process: ProcessInfoHelper::PROCESS_LIST) {
		bool isPresent = false;
		for (auto const &process_id: process_ids) {
			if (process.id == process_id) {
				isPresent = true;
				break;
			}
		}
		if (!isPresent)  absent_list.push_back(process);
	}
	return absent_list;
}

