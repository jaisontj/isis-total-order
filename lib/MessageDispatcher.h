#ifndef MESSAGE_DISPATCHER_H
#define MESSAGE_DISPATCHER_H

#include<iostream>
#include<atomic>
#include<mutex>
#include<vector>
#include<queue>

#include "NetworkDataTypes.h"

struct MessageInfo {
	NetworkMessage message;
	size_t message_size;
	std::string hostname;
	std::string port;
};

class MessageDispatcher {
	private:
		MessageDispatcher();
		std::mutex m;
		std::atomic<bool> is_dispatching;
		int delay = 1;
		static int MAX_DELAY;
		std::queue<MessageInfo> mqueue;

		MessageInfo get_queue_front();
		void pop_queue();
		bool is_queue_empty();

	public:
		static MessageDispatcher& get_instance();
		MessageDispatcher(MessageDispatcher const&) = delete;
		void operator=(MessageDispatcher const&) = delete;
		void dispatch_messages();
		void add_message_to_queue(NetworkMessage *message, size_t message_size, std::string hostname, std::string port);
		void add_message_to_queue(NetworkMessage *message, size_t message_size, std::vector<std::string> hostnames, std::string port);
};

#endif
