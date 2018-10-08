#include <string>
#include <vector>

#include "custom_types.h"

using namespace std;

void show_usage_and_exit();
CommandArgs parse_cmg_args(int, char* []);
vector<FileLineContent> get_file_content(string);
int get_id_from_file_content(vector<FileLineContent>);
string get_hostname_from_id(vector<FileLineContent>, uint32_t);
vector<string> get_hostnames_not_in_list(vector<FileLineContent>, vector<uint32_t>);
int get_message_delay();
bool should_drop_message();
void simulate_delay_if_needed();
