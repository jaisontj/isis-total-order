#include<iostream>
#include<map>
#include<unistd.h>
#include<fstream>
#include<string>
#include<cstring>
#include<vector>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<thread>

#include "../lib/DataMessageQueue.h"
#include "../lib/SeqProvider.h"
#include "../lib/helpers.h"
#include "../lib/DataMessageSeqTracker.h"
#include "../lib/ListenerSocket.h"
#include "../lib/MessageDispatcher.h"
#include "../lib/Log.h"
#include "../lib/LogHelper.h"
#include "../lib/ProcessInfoHelper.h"


//expected network delay of 1 sec
#define NETWORK_DELAY 1
#define MAX_RETRY_COUNT 2

using namespace std;

LogLevel Log::LOG_LEVEL = NONE;
vector<ProcessInfo> ProcessInfoHelper::PROCESS_LIST;
ProcessInfo ProcessInfoHelper::SELF;

CommandArgs c_args;
DataMessageQueue message_queue;
DataMessageSeqTracker data_message_proposal_tracker;
MessageDispatcher message_dispatcher;
SeqProvider seq_provider;

void send_message_to_processes(NetworkMessage *message, size_t message_size, vector<ProcessInfo> processes) {
	string port = c_args.port;
	for (auto const& process: processes) {
		string hostname = process.hostname;
		message_dispatcher.add_message_to_queue(message, message_size, hostname.c_str(), port);
	}
}

void send_message_to_host(NetworkMessage *message, size_t message_size, string hostname) {
	message_dispatcher.add_message_to_queue(message, message_size, hostname, c_args.port);
}

void handle_data_message(DataMessage *message) {
	ProcessInfo p = ProcessInfoHelper::get_process_info(message->sender);
	string hostname = p.hostname;
	if (hostname == "") {
		Log::e("Received DataMessage from unknown sender. Ignoring");
		return;
	}

	uint32_t ID = ProcessInfoHelper::SELF.id;

	//send ack with last_seq+1
	AckMessage ack = {
		.type = 2,
		.sender = message->sender,
		.msg_id = message->msg_id,
		.proposed_seq = seq_provider.increment_sequence(),
		.proposer = ID
	};
	//Add it to unordered queue
	message_queue.add_undeliverable(ID, ack.msg_id, ack.sender, ack.proposed_seq, ack.proposer);

	Log::d("Received DataMessage-------------------");
	log(message);
	Log::d("Sending ACK----------------------------");
	log((AckMessage *) &ack);
	send_message_to_host((NetworkMessage *) &ack, sizeof ack, hostname.c_str());
	//TODO: wait for final_seq
	//TODO: What happens if no ack is received?
}

void handle_ack_message(AckMessage *message) {
	//Add proposed seq for message
	uint32_t message_id = message->msg_id;
	data_message_proposal_tracker.handle_sequence_proposal(
			message_id,
			message->proposed_seq,
			message->proposer
			);
	Log::d("Recieved AckMessage---------------------");
	log(message);
	if (data_message_proposal_tracker.has_received_all_proposals(message_id)) {
		Log::d("Sending SeqMessage------------------");
		uint32_t final_seq = data_message_proposal_tracker.get_max_proposed_seq(message_id);
		uint32_t final_seq_proposer = data_message_proposal_tracker.get_max_seq_proposer_id(message_id);
		SeqMessage seq_message = {
			.type = 3,
			.sender = message->sender,
			.msg_id = message_id,
			.final_seq = final_seq,
			.final_seq_proposer = final_seq_proposer
		};
		log((SeqMessage *) &seq_message);
		send_message_to_processes((NetworkMessage *) &seq_message, sizeof seq_message, ProcessInfoHelper::PROCESS_LIST);
		//TODO: what happens if the above message does not reach?
	} else {
		Log::d("Has not received all proposals yet-------------------------------------");
	}
	//TODO:if no, chill out. Wait....
}

void handle_seq_message(SeqMessage *seq_msg) {
	//attach the final sequence to the respective received_message
	try {
		message_queue.mark_as_deliverable(*seq_msg);
		//update last_sequence to this one, if greater
		seq_provider.update_sequence_if_greater(seq_msg->final_seq);
		//TODO: send ack for this seq messsage
	} catch(string m) {
		cout<<"Error with SequenceMessage: "<<m<<endl;
	}
}

void handle_message(NetworkMessage message) {
	switch(message.type) {
		case 1:
			handle_data_message((DataMessage *) &message);
			return;
		case 2:
			handle_ack_message((AckMessage *) &message);
			return;
		case 3:
			handle_seq_message((SeqMessage *) &message);
			return;
		default:
			Log::e("Received unknown message type from network. Ignoring");
	}
}

void start_msg_listener(CommandArgs c_args) {
	Log::d("Starting message listener");
	try {
		ListenerSocket listener = ListenerSocket(c_args.port);
		listener.start_listening(&handle_message);
		listener.close_socket();
	} catch (string m) {
		Log::e(m);
		start_msg_listener(c_args);
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

		send_message_to_processes((NetworkMessage *) &message, sizeof message, ProcessInfoHelper::PROCESS_LIST);

		//TODO: is this fine?
		//Send messages in 1 second intervals
		sleep(1);
	}
}


int main(int argc, char* argv[]){
	c_args = parse_cmg_args(argc, argv);

	//Read hostfile for a list of hosts and their respective line num
	ProcessInfoHelper::init_from_file(c_args.filename);

	data_message_proposal_tracker.set_max_proposal_count(ProcessInfoHelper::PROCESS_LIST.size());

	thread listener(start_msg_listener, c_args);
	//TODO: replace this with a way to ensure that all processes are up
	sleep(4);

	send_data_messages(c_args.msg_count);
	//wait for listener thread.
	listener.join();
	return 0;
}
