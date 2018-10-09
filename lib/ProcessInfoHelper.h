#ifndef PROCESS_INFO_HELPER_H
#define PROCESS_INFO_HELPER_H

#include <iostream>
#include <vector>
#include <string>

struct ProcessInfo {
	std::string hostname;
	std::string port;
	uint32_t id;
};

class ProcessInfoHelper {
	public:
		static std::vector<ProcessInfo> PROCESS_LIST;
		static ProcessInfo SELF;
		static void init_from_file(std::string filename, std::string port);
		static std::vector<ProcessInfo> get_processes_not_in_list(std::vector<uint32_t> process_ids);
		static ProcessInfo get_process_info(uint32_t id);
};

#endif
