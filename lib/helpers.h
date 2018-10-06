#include <string>
#include <vector>

#include "custom_types.h"

using namespace std;

void debug_print(string);
void show_usage_and_exit();
CommandArgs parse_cmg_args(int, char* []);
vector<FileLineContent> get_file_content(string);
int get_id_from_file_content(vector<FileLineContent>);
string get_hostname_from_id(vector<FileLineContent>, uint32_t id);
int send_message_to_host(const char *hostname, const char *port, void *message, size_t message_size);
