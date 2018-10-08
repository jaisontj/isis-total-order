#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>

enum MessageStatus { DELIVERABLE, UNDELIVERABLE };

class Message {
	public:
		uint32_t process_id;
		uint32_t msg_id;
		uint32_t final_seq;
		uint32_t sender_id;
		uint32_t proposer_id;
		MessageStatus mstatus = UNDELIVERABLE;

		Message(uint32_t process_id, uint32_t msg_id, uint32_t sender_id, uint32_t final_seq, uint32_t proposer_id);
		void mark_as_deliverable(uint32_t final_seq, uint32_t proposer);
		void process_message();
		std::string get_as_string();
};

#endif
