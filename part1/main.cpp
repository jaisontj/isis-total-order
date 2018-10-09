#include<iostream>
#include<cstring>
#include<vector>
#include<unistd.h>
#include<thread>

#include "../lib/MessageQueue.h"
#include "../lib/SeqProvider.h"
#include "../lib/helpers.h"
#include "../lib/DataMessageSeqTracker.h"
#include "../lib/ListenerSocket.h"
#include "../lib/Log.h"
#include "../lib/LogHelper.h"
#include "../lib/ProcessInfoHelper.h"
#include "../lib/MessageHandler.h"

//expected network delay of 1 sec
#define NETWORK_DELAY 1
#define MAX_RETRY_COUNT 2

using namespace std;

LogLevel Log::LOG_LEVEL = ERROR;
vector<ProcessInfo> ProcessInfoHelper::PROCESS_LIST;
ProcessInfo ProcessInfoHelper::SELF;


void start_msg_listener(CommandArgs c_args, MessageHandler *handler) {
	Log::d("Starting message listener");
	try {
		ListenerSocket listener = ListenerSocket(c_args.port);
		listener.start_listening(*handler);
		listener.close_socket();
	} catch (string m) {
		Log::e(m);
		start_msg_listener(c_args, handler);
	}
}

void send_data_messages(uint32_t count) {
	Log::d("Sending data message");
	for (uint32_t i = 0; i< count; i++) {
		DataMessage message = {
			.type = 1,
			.sender = ProcessInfoHelper::SELF.id,
			.msg_id = i+1,
			.data = 1234
		};

		send_message((NetworkMessage *) &message, sizeof message, ProcessInfoHelper::PROCESS_LIST);

		//await ack for message

		//Send messages in 1 second intervals
		sleep(1);
	}
}


int main(int argc, char* argv[]) {
	CommandArgs c_args = parse_cmg_args(argc, argv);

	//Read hostfile for a list of hosts and their respective line num
	ProcessInfoHelper::init_from_file(c_args.filename, c_args.port);

	MessageHandler handler = MessageHandler(ProcessInfoHelper::PROCESS_LIST.size());

	thread listener(start_msg_listener, c_args, &handler);
	//TODO: replace this with a way to ensure that all processes are up
	sleep(4);

	send_data_messages(c_args.msg_count);
	//wait for listener thread.
	listener.join();
	return 0;
}
