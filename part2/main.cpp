#include<iostream>
#include<cstring>
#include<vector>
#include<unistd.h>
#include<thread>

#include "../lib/helpers.h"
#include "../lib/ListenerSocket.h"
#include "../lib/Log.h"
#include "../lib/ProcessInfoHelper.h"
#include "../lib/MessageHandler.h"
#include "../lib/NetworkStatus.h"
#include "../lib/TcpListener.h"
#include "../lib/SnapshotHandler.h"

using namespace std;

LogLevel Log::LOG_LEVEL = NONE;
vector<ProcessInfo> ProcessInfoHelper::PROCESS_LIST;
ProcessInfo ProcessInfoHelper::SELF;
bool NetworkStatus::DROPS_MESSAGE;
int NetworkStatus::DELIVERY_DELAY;
int SnapshotHandler::X = -1;

void start_msg_listener(CommandArgs c_args, MessageHandler *handler) {
	Log::d("Starting message listener");
	try {
		ListenerSocket listener = ListenerSocket(c_args.port);
		//Blocks
		listener.start_listening(*handler);
		listener.close_socket();
	} catch (string m) {
		Log::e(m);
		//Re-try on error
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
		//send to all processes, including self.
		send_message((NetworkMessage *) &message, sizeof message, ProcessInfoHelper::PROCESS_LIST);
		//Send messages in 1 second intervals
		sleep(1);
	}
}

void tcp_message_handler(Marker m) {
	SnapshotHandler::get_instance().handle_marker(m);
}

void start_tcp_listener(CommandArgs c_args) {
	try {
		TcpListener listener = TcpListener(c_args.port);
		listener.start_listening(&tcp_message_handler);
		listener.close_socket();
	} catch (string m) {
		Log::e(m);
		start_tcp_listener(c_args);
	}
}

int main(int argc, char* argv[]) {
	CommandArgs c_args = parse_cmg_args(argc, argv);
	ProcessInfoHelper::init_from_file(c_args.filename, c_args.port);
	if (c_args.x > 0)
		SnapshotHandler::get_instance().X = c_args.x;
	thread tcp_listener(start_tcp_listener, c_args);
	MessageHandler handler = MessageHandler(ProcessInfoHelper::PROCESS_LIST.size());
	thread listener(start_msg_listener, c_args, &handler);
	sleep(5);
	send_data_messages(c_args.msg_count);
	listener.join();
	tcp_listener.join();
	return 0;
}
