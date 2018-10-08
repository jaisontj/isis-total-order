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

//expected network delay of 1 sec
#define NETWORK_DELAY 1
#define MAX_RETRY_COUNT 2

using namespace std;

LogLevel Log::LOG_LEVEL = NONE;

uint32_t ID;
vector<FileLineContent> file_content;
CommandArgs c_args;
SeqProvider seq_provider;
DataMessageQueue message_queue;
DataMessageSeqTracker data_message_proposal_tracker;
MessageDispatcher message_dispatcher;

void send_message_to_hosts(NetworkMessage *message, size_t message_size) {
	string port = c_args.port;
	for (auto const& line: file_content) {
		string hostname = line.content;
		message_dispatcher.add_message_to_queue(message, message_size, hostname.c_str(), port);
	}
}

void send_message_to_hosts(NetworkMessage *message, size_t message_size, vector<string> hostnames) {
	message_dispatcher.add_message_to_queue(message, message_size, hostnames, c_args.port);
}

void send_message_to_host(NetworkMessage *message, size_t message_size, string hostname) {
	message_dispatcher.add_message_to_queue(message, message_size, hostname, c_args.port);
}

void handle_data_message(DataMessage *message) {
	string hostname = get_hostname_from_id(file_content, message->sender);
	if (hostname == "") {
		cout<<"Received DataMessage from unknown sender. Ignoring"<<endl;
		return;
	}
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
	delete message;
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
		send_message_to_hosts((NetworkMessage *) &seq_message, sizeof seq_message);
		//TODO: what happens if the above message does not reach?
	} else {
		Log::d("Has not received all proposals yet-------------------------------------");
	}
	//TODO:if no, chill out. Wait....
	delete message;
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
	delete seq_msg;
}

void handle_network_message(NetworkMessage *message) {
	switch(message->type) {
		case 1:
			handle_data_message((DataMessage *) message);
			return;
		case 2:
			handle_ack_message((AckMessage *) message);
			return;
		case 3:
			handle_seq_message((SeqMessage *) message);
			return;
		default:
			cout<<"Received unknown message type from network. Ignoring"<<endl;
	}
}

void pretend_proxy(NetworkMessage *message) {
	if (should_drop_message()) {
		Log::d("DROPPING NetworkMessage: Type->" + to_string(message->type)
				+ " SenderID->" + to_string(message->sender)
				+ " MessageID->" + to_string(message->msg_id)
				+ " DataOrProposedSequence->" + to_string(message->data_or_seq));
		return;
	}
	simulate_delay_if_needed();
}


void start_msg_listener(CommandArgs c_args) {
	Log::d("Starting message listener");
	try {
		ListenerSocket listener = ListenerSocket(c_args.port);
		listener.start_listening(&handle_network_message);
		listener.close_socket();
	} catch (string m) {
		Log::e(m);
		start_msg_listener(c_args);
	}
}

void await_acks_for_data_message(DataMessage message, uint32_t number_of_acks, int retry_count = 0) {
	//sleep for 2*NETWORK_DELAY because to and fro, just to be safe, wait 2x that
	sleep(2*2*NETWORK_DELAY);
	//Should have received all acks by now.
	vector<uint32_t> proposers = data_message_proposal_tracker.get_proposers(message.msg_id);
	if (proposers.size() < number_of_acks) {
		if (retry_count == MAX_RETRY_COUNT) {
			cout<<"Not receiving ack for message. Maybe a process has crashed. Exiting."<<endl;
			exit(1);
		}
		//Get list of hostnames who have not ack'd and resend DataMessage
		vector<string> missed_proposers = get_hostnames_not_in_list(file_content, proposers);
		Log::d("Resending DataMessage to missing hosts");
		send_message_to_hosts((NetworkMessage *) &message, sizeof message, missed_proposers);
		await_acks_for_data_message(message, number_of_acks, retry_count + 1);
	}
}

void send_data_messages(uint32_t count) {
	Log::d("Sending data message");
	for (uint32_t i = 0; i< count; i++) {
		DataMessage message = {
			.type = 1,
			.sender = ID,
			.msg_id = i+1,
			.data = 1234
		};

		send_message_to_hosts((NetworkMessage *) &message, sizeof message);

		//expect file_content.size() number of acks within ACK_TIMEOUT
		//thread await_acks_thread(await_acks_for_data_message, message, file_content.size());
		//await_acks_thread.detach();

		//TODO: is this fine?
		//Send messages in 1 second intervals
		sleep(1);

		//TIMEOUT = 5secs
		//sent message, expecting file_content.size() number of acks for it within TIMEOUT time.
		//if dont received within TIMEOUT, resend same message to hosts whose acks are missing.
		////wait for acks and reset timeou
		//
		//Decide on some value of TIMEOUT
		//
		//max time to send message = 1secs
		//meaning acks can be gotten back in 2 seconds tops
		//Timeout for acks => 4secs
		//
		//Logic for datamessage retry
		//Step 1: send messages to all hosts
		//Step 2: start counter until TIMEOUT, and check if all acks have been received.
		//Step 3: if all acks not received,
		////Step 3.1 - Get lets of hosts whose acks are missing
		////Step 3.2 - resend message to hosts whose acks are missing
		////Step 3.3 - start counter until TIMEOUT and check if all acks have been received.
	}
}


int main(int argc, char* argv[]){
	c_args = parse_cmg_args(argc, argv);

	//Read hostfile for a list of hosts and their respective line num
	file_content = get_file_content(c_args.filename);

	//Get process ID for self
	ID = get_id_from_file_content(file_content);
	if (ID == 0) {
		cout<<"Unable to find hostname in provided file at "<<c_args.filename<<". Please ensure that the hostname is provided in file."<<endl;
		return 1;
	}

	data_message_proposal_tracker.set_max_proposal_count(file_content.size());

	thread listener(start_msg_listener, c_args);
	//TODO: replace this with a way to ensure that all processes are up
	sleep(4);

	send_data_messages(c_args.msg_count);
	//wait for listener thread.
	listener.join();
	return 0;
}
