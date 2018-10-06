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
#include "../lib/ProposalTracker.cpp"

using namespace std;

uint32_t ID;
vector<FileLineContent> file_content;
CommandArgs c_args;
SeqProvider seq_provider;
map<uint32_t, ProposalTracker> msg_proposal_map;
DataMessageQueue message_queue;

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

void handle_data_message(DataMessage *message) {
	cout<<"Received Data Message: Data is "<<message->data<<endl;
	//Add it to unordered queue
	message_queue.add_undeliverable(*message);
	//send ack with last_seq+1
	AckMessage ack = {
		.type = 2,
		.sender = message->sender,
		.msg_id = message->msg_id,
		.proposed_seq = seq_provider.get_sequence() + 1,
		.proposer = ID
	};
	string hostname = get_hostname_from_id(file_content, message->sender);
	if (hostname == "") {
		cout<<"Received DataMessage from unknown sender. Ignoring"<<endl;
		return;
	}
	int sent_bytes = send_message_to_host(hostname.c_str(), c_args.port, (void *) &ack, sizeof ack);
	if (sent_bytes == -1) {
		//TODO: HANDLE
		cout<<"Failed to send ack to sender for DataMessage"<<endl;
	}
	//TODO: wait for final_seq
	//TODO: What happens if no ack is received?
}

void handle_ack_message(AckMessage *message) {
	cout<<"Received Ack Message:"
		<<" Proposer->"<<message->proposer
		<<" MessageID->"<<message->msg_id
		<<" ProposedSequence->"<<message->proposed_seq
		<<endl;
	//Add proposed seq for message
	uint32_t message_id = message->msg_id;
	if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) {
		msg_proposal_map[message_id] = ProposalTracker(file_content.size());
	}
	msg_proposal_map[message_id].handle_seq_proposal(message->proposed_seq, message->proposer);
	//check if all acks have been received
	if (msg_proposal_map[message_id].has_received_all_proposals()) {
		//if yes, get max seq and send Seq message to all
		SeqMessage seq_message = {
			.type = 3,
			.sender = message->sender,
			.msg_id = message_id,
			.final_seq = msg_proposal_map[message_id].get_max_proposed_seq(),
			.final_seq_proposer = msg_proposal_map[message_id].get_max_seq_proposer_id()
		};
		send_message_to_hosts((void *) &seq_message, sizeof seq_message);
		//TODO: what happens if the above message does not reach?
	}
	//TODO:if no, chill out. Wait....
}

void handle_seq_message(SeqMessage *seq_msg) {
	cout<<"Received SeqMessage: "
		<<" MessageID->"<<seq_msg->msg_id
		<<" MessageSenderID->"<<seq_msg->sender
		<<" FinalSequence->"<<seq_msg->final_seq
		<<" SeqProposerID->"<<seq_msg->final_seq_proposer
		<<endl;
	//attach the final sequence to the respective received_message
	debug_print("Marking message as deliverable");
	seq_provider.set_sequence(seq_msg->final_seq);
	message_queue.mark_as_deliverable(*seq_msg);
	debug_print("DONE Marking message as deliverable");
	//update last_sequence to this one, if greater
	//TODO: send ack for this seq messsage
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

void listen_on_socket(int socket_fd) {
	struct sockaddr_storage recv_addr;
	socklen_t recv_addr_len = sizeof(recv_addr);
	NetworkMessage *message = new NetworkMessage();
	int recv_bytes = recvfrom(socket_fd, message, sizeof (NetworkMessage), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
	if (recv_bytes == -1) {
		perror("Listener: Error in receiving message");
		exit(1);
	}
	debug_print("Listener: packet is " + to_string(recv_bytes) + " long");
	handle_network_message(message);
	delete message;
}

void start_msg_listener(CommandArgs c_args) {
	//Create a socket
	struct addrinfo hints = init_dgram_hints(AI_PASSIVE);
	struct addrinfo *servinfo =  get_addr_info(NULL, c_args.port, &hints);

	SocketInfo s_info = create_first_possible_socket(servinfo, 1);
	int socket_fd = s_info.fd;

	//bind socket to address
	struct addrinfo *p = s_info.addr;
	debug_print("Listener: Trying to bind socket to address");
	if (::bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
		close(socket_fd);
		perror("Listener: Failed to bind socket. Exiting.");
		exit(0);
	}

	freeaddrinfo(servinfo);

	debug_print("Listening for messages....");
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

	thread listener(start_msg_listener, c_args);
	//TODO: replace this with a way to ensure that all processes are up
	sleep(2);

	send_data_messages(c_args.msg_count);
	//wait for listener thread.
	listener.join();
	return 0;
}
