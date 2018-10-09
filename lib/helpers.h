#ifndef HELPER_H
#define HELPER_h

#include <string>
#include <vector>

#include "custom_types.h"
#include "NetworkDataTypes.h"
#include "ProcessInfoHelper.h"

using namespace std;

void show_usage_and_exit();
CommandArgs parse_cmg_args(int, char* []);
int get_message_delay();
bool should_drop_message();
void simulate_delay_if_needed();
void send_message(NetworkMessage *message, size_t message_size, vector<ProcessInfo> processes);
void send_message(NetworkMessage *message, size_t message_size, ProcessInfo process);
#endif
