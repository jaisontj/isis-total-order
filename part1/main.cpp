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

#include "../lib/DataMessageQueue.cpp"
#include "../lib/SeqProvider.cpp"
#include "../lib/helpers.h"
#include "../lib/socket_helpers.h"
#include "../lib/DataMessageSeqTracker.cpp"

using namespace std;

uint32_t ID;
vector<FileLineContent> file_content;
CommandArgs c_args;
SeqProvider seq_provider;
DataMessageQueue message_queue;
DataMessageSeqTracker data_message_proposal_tracker;

void send_message_to_hosts(
		void *message,
		size_t message_size
		) {
	const char *port = c_args.port;
	for (auto const& line: file_content) {
		string hostname = line.content;
		send_message_to_host(hostname.c_str(), port, message, message_size);
	}
}

void print(DataMessage *message) {
	debug_print("DataMessage:Sender->" + to_string(message->sender)
			+ " MessageID->" + to_string(message->msg_id)
			+ " Data->" + to_string(message->data));
}

void print(AckMessage *message) {
	debug_print("AckMessage: Proposer->" + to_string(message->proposer)
			+ " MessageID->" + to_string(message->msg_id)
			+ " ProposedSequence->" + to_string(message->proposed_seq));
}

void print(SeqMessage *seq_msg) {
	debug_print("SeqMessage:  MessageID->" + to_string(seq_msg->msg_id)
			+ " MessageSenderID->" + to_string(seq_msg->sender)
			+ " FinalSequence->" + to_string(seq_msg->final_seq)
			+ " SeqProposerID->" + to_string(seq_msg->final_seq_proposer));
}


void handle_data_message(DataMessage *message) {
	string hostname = get_hostname_from_id(file_content, message->sender);
	if (hostname == "") {
		cout<<"Received DataMessage from unknown sender. Ignoring"<<endl;
		return;
	}
	//Add it to unordered queue
	message_queue.add_undeliverable(*message);
	//send ack with last_seq+1
	AckMessage ack = {
		.type = 2,
		.sender = message->sender,
		.msg_id = message->msg_id,
		.proposed_seq = seq_provider.increment_sequence(),
		.proposer = ID
	};
	debug_print("Received DataMessage-------------------");
	print(message);
	debug_print("Sending ACK----------------------------");
	print((AckMessage *) &ack);

	int sent_bytes = send_message_to_host(hostname.c_str(), c_args.port, (void *) &ack, sizeof ack);
	if (sent_bytes == -1) {
		//TODO: HANDLE
		cout<<"Failed to send ack to sender for DataMessage"<<endl;
	}
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
	debug_print("Recieved AckMessage---------------------");
	print(message);
	if (data_message_proposal_tracker.has_received_all_proposals(message_id)) {
		debug_print("Sending SeqMessage------------------");
		uint32_t final_seq = data_message_proposal_tracker.get_max_proposed_seq(message_id);
		uint32_t final_seq_proposer = data_message_proposal_tracker.get_max_seq_proposer_id(message_id);
		SeqMessage seq_message = {
			.type = 3,
			.sender = message->sender,
			.msg_id = message_id,
			.final_seq = final_seq,
			.final_seq_proposer = final_seq_proposer
		};
		print((SeqMessage *) &seq_message);
		send_message_to_hosts((void *) &seq_message, sizeof seq_message);
		//TODO: what happens if the above message does not reach?
	} else {
		debug_print("Has not received all proposals yet-------------------------------------");
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
	if (should_drop_message()) {
		debug_print("DROPPING NetworkMessage: Type->" + to_string(message->type)
				+ " SenderID->" + to_string(message->sender)
				+ " MessageID->" + to_string(message->msg_id)
				+ " DataOrProposedSequence->" + to_string(message->data_or_seq));
		return;
	}
	simulate_delay_if_needed();
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

void listen_on_socket(int socket_fd) {
	struct sockaddr_storage recv_addr;
	socklen_t recv_addr_len = sizeof(recv_addr);
	NetworkMessage *message = new NetworkMessage();
	int recv_bytes = recvfrom(socket_fd, message, sizeof (NetworkMessage), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
	if (recv_bytes == -1) {
		perror("Listener: Error in receiving message");
		exit(1);
	}
	verbose_print("Listener: packet is " + to_string(recv_bytes) + " long");
	//Handle message in a different thread, to simulate delay
	thread message_handle_thread(handle_network_message, message);
	message_handle_thread.detach();
}

void start_msg_listener(CommandArgs c_args) {
	//Create a socket
	struct addrinfo hints = init_dgram_hints(AI_PASSIVE);
	struct addrinfo *servinfo =  get_addr_info(NULL, c_args.port, &hints);

	SocketInfo s_info = create_first_possible_socket(servinfo, 1);
	int socket_fd = s_info.fd;

	//bind socket to address
	struct addrinfo *p = s_info.addr;
	verbose_print("Listener: Trying to bind socket to address");
	if (::bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
		close(socket_fd);
		perror("Listener: Failed to bind socket. Exiting.");
		exit(0);
	}

	freeaddrinfo(servinfo);

	verbose_print("Listening for messages....");
	while(true) listen_on_socket(socket_fd);
	close(socket_fd);
}


void send_data_messages(uint32_t count) {
	for (uint32_t i = 0; i< count; i++) {
		DataMessage message = {
			.type = 1,
			.sender = ID,
			.msg_id = i+1,
			.data = 1234
		};
		send_message_to_hosts((void *) &message, sizeof message);


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

	message_queue.set_process_id(ID);
	data_message_proposal_tracker.set_max_proposal_count(file_content.size());

	thread listener(start_msg_listener, c_args);
	//TODO: replace this with a way to ensure that all processes are up
	sleep(2);

	send_data_messages(c_args.msg_count);
	//wait for listener thread.
	listener.join();
	return 0;
}
