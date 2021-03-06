#ifndef MESSAGE_DISPATCHER_H
#define MESSAGE_DISPATCHER_H

#include<iostream>
#include<atomic>
#include<mutex>
#include<vector>
#include<queue>
#include<ctime>

#include "NetworkDataTypes.h"
#include "MessageInfo.h"

class MessageDispatcher {
	private:
		MessageDispatcher();
		std::mutex m;
		std::atomic<bool> is_dispatching;
		std::queue<MessageInfo> mqueue;

		MessageInfo get_queue_front();
		void pop_queue();
		bool is_queue_empty();

	public:
		static MessageDispatcher& get_instance();
		MessageDispatcher(MessageDispatcher const&) = delete;
		void operator=(MessageDispatcher const&) = delete;
		void dispatch_messages();
		void add_message_to_queue(NetworkMessage *message, size_t message_size, std::string hostname, std::string port, int retry_count = 0);
		void add_message_to_queue(NetworkMessage *message, size_t message_size, std::vector<std::string> hostnames, std::string port);
};

#endif
