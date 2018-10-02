#include<iostream>
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

#include "../lib/helpers.h"
#include "../lib/socket_helpers.h"

using namespace std;

#define BUFFERSIZE 1024

void listen_on_socket(int socket_fd) {
	debug_print("Listener: Waiting for message....");
	struct sockaddr_storage recv_addr;
	socklen_t recv_addr_len = sizeof(recv_addr);
	char buffer[BUFFERSIZE];
	int recv_bytes = recvfrom(socket_fd, buffer, BUFFERSIZE-1, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
	if (recv_bytes == -1) {
		perror("Listener: Error in receiving message");
		exit(1);
	}

	char s[INET6_ADDRSTRLEN];
	inet_ntop(recv_addr.ss_family, get_in_addr((struct sockaddr *)&recv_addr), s, sizeof(s));
	cout<<"Listener: got packet from "<<s<<endl;
	cout<<"Listener: packet is "<<recv_bytes<<" long"<<endl;
	cout<<"Listener: packet contains: "<<buffer<<endl;
	buffer[recv_bytes] = '\0';
}

void start_msg_listener(command_args c_args) {
	//Create a socket
	debug_print("Listener: Trying to create a socket");
	debug_print("Listener: Hostname: NULL Port: " + string(c_args.port));
	struct addrinfo hints = init_dgram_hints(AI_PASSIVE);
	struct addrinfo *servinfo =  get_addr_info(NULL, c_args.port, &hints);

	struct socket_info s_info = create_first_possible_socket(servinfo, 1);
	int socket_fd = s_info.fd;
	if (socket_fd == -1) {
		perror("Listener: Failed to create listen socket");
		exit(0);
	}
	debug_print("Listener: Successfully created socket to listen");

	struct addrinfo *p = s_info.addr;
	//bind socket to address
	debug_print("Listener: Trying to bind socket to address");
	if (::bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
		close(socket_fd);
		perror("Listener: Failed to bind socket. Exiting.");
		exit(0);
	}
	debug_print("Listener: Binding socket successful");

	freeaddrinfo(servinfo);

	while(true) listen_on_socket(socket_fd);

	close(socket_fd);
}

void send_message_to_hosts(vector<FileLineContent> file_content, const char *port, const char *message) {
	for (auto const& line: file_content) {
		string hostname = line.content;
		struct addrinfo hints = init_dgram_hints();
		struct addrinfo *servinfo = get_addr_info(hostname.c_str(), port, &hints);

		struct socket_info s_info = create_first_possible_socket(servinfo, 1);
		int socket_fd = s_info.fd;
		if (socket_fd == -1) {
			perror("Talker: Failed to create socket");
			exit(0);
		}

		struct addrinfo *p = s_info.addr;
		int sent_bytes = sendto(socket_fd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen);
		if (sent_bytes == -1) {
			perror("Talker: Failed to send message");
			exit(1);
		}

		freeaddrinfo(servinfo);

		debug_print("Talker: Sent " + to_string(sent_bytes) + " to " + hostname);
		close(socket_fd);
	}
}

int main(int argc, char* argv[]){
	struct command_args c_args = parse_cmg_args(argc, argv);

	//Read hostfile for a list of hosts and their respective line num
	vector<FileLineContent> file_content = get_file_content(c_args.filename);

	//Get process ID for self
	int ID = get_id_from_file_content(file_content);
	if (ID == -1) {
		cout<<"Unable to find hostname in provided file at "<<c_args.filename<<". Please ensure that the hostname is provided in file."<<endl;
		return 1;
	}
	debug_print("Self ID: " + to_string(ID));

	thread listener(start_msg_listener, c_args);
	send_message_to_hosts(file_content, c_args.port, "Test Message 1");
	//wait for listener thread.
	listener.join();
	return 0;
}
