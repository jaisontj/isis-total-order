#include <string>
#include <vector>

#include "custom_types.h"

using namespace std;

void debug_print(string);
void show_usage_and_exit();
struct command_args parse_cmg_args(int, char* []);
vector<FileLineContent> get_file_content(string);
int get_id_from_file_content(vector<FileLineContent>);
