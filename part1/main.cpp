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
#include "../lib/DeliveryTracker.h"
#include "../lib/HandshakeTracker.h"

using namespace std;

LogLevel Log::LOG_LEVEL = NONE;
vector<ProcessInfo> ProcessInfoHelper::PROCESS_LIST;
ProcessInfo ProcessInfoHelper::SELF;
bool NetworkStatus::DROPS_MESSAGE;
int NetworkStatus::DELIVERY_DELAY;


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

void wait_for_all_processes() {
	Log::i("Main:: performing handshake with all processes");
	//type 0 identifies as a handshake message.
	DataMessage message = {
		.type = 0,
		.sender = ProcessInfoHelper::SELF.id,
		.msg_id = 0,
		.data = 0
	};
	//DeliveryTracker will handle retries.
	while (HandshakeTracker::get_instance().get_num_verified() != ProcessInfoHelper::PROCESS_LIST.size()) {
		vector<uint32_t> verified_ps = HandshakeTracker::get_instance().verified_processes;
		vector<ProcessInfo> unverified_ps = ProcessInfoHelper::get_processes_not_in_list(verified_ps);
		send_message((NetworkMessage *) &message, sizeof message, unverified_ps);
		sleep(1);
	};
	Log::i("Main:: Handshake complete");
}


int main(int argc, char* argv[]) {
	CommandArgs c_args = parse_cmg_args(argc, argv);
	//Read hostfile for a list of hosts and their respective line num
	ProcessInfoHelper::init_from_file(c_args.filename, c_args.port);
	MessageHandler handler = MessageHandler(ProcessInfoHelper::PROCESS_LIST.size());
	thread listener(start_msg_listener, c_args, &handler);
	sleep(5);
	send_data_messages(c_args.msg_count);
	//wait for listener thread.
	listener.join();
	return 0;
}
