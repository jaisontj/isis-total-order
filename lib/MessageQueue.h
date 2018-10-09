#ifndef DATA_MESSAGE_QUEUE_H
#define DATA_MESSAGE_QUEUE_H

#include <iostream>
#include <mutex>
#include <vector>

#include "NetworkDataTypes.h"
#include "Message.h"

class MessageQueue {
	private:
		MessageQueue();
		std::mutex m;
		std::vector<Message> mqueue;
		void print_ordered_deliverables();
		void log_mqueue();
		void sort_mqueue();
	public:
		static MessageQueue& get_instance();
		MessageQueue(MessageQueue const&);
		void operator=(MessageQueue const&);
		void add_undeliverable(uint32_t process_id, uint32_t msg_id, uint32_t sender_id, uint32_t final_seq, uint32_t proposer_id);
		void mark_as_deliverable(SeqMessage message);
};

#endif
