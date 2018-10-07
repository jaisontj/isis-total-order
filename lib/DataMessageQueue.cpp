#include<map>
#include<mutex>
#include<iostream>

#include "NetworkDataTypes.h"
using namespace std;

class DeliverableMessage {
	uint32_t process_id;
	uint32_t msg_id;
	uint32_t sender_id;
	uint32_t final_seq;
	uint32_t proposer_id;

	public:
	DeliverableMessage() {}
	DeliverableMessage(uint32_t process_id, uint32_t msg_id, uint32_t sender_id, uint32_t final_seq, uint32_t proposer_id) {
		this->process_id = process_id;
		this->msg_id = msg_id;
		this->sender_id = sender_id;
		this->final_seq = final_seq;
		this->proposer_id = proposer_id;
	}

	void process_message() {
		cout<<process_id
			<<": Processed message "<<msg_id
			<<" from sender "<<sender_id
			<<" with seq ("<<final_seq<<", "<<proposer_id<<")"
			<<endl;
	}
};

class DataMessageQueue {
	mutex m;
	uint32_t process_id;
	uint32_t last_delivered_seq;
	map<string, DataMessage> undeliverables;
	map<uint32_t, DeliverableMessage> deliverables;

	string get_undeliverable_message_id(uint32_t sender, uint32_t msg_id) {
		return to_string(sender) + to_string(msg_id);
	}

	void print_ordered_deliverables() {
		//Print starting from last_seq until ordered seq
		while(deliverables.find(last_delivered_seq + 1) != deliverables.end()) {
			deliverables[last_delivered_seq + 1].process_message();
			++last_delivered_seq;
		}
	}

	public:

	void set_process_id(uint32_t id) {
		process_id = id;
	}

	void add_undeliverable(DataMessage message) {
		lock_guard<mutex> lk(m);
		string message_id = get_undeliverable_message_id(message.sender, message.msg_id);
		undeliverables[message_id] = message;
	}

	void mark_as_deliverable(SeqMessage message) {
		lock_guard<mutex> lk(m);
		string message_id = get_undeliverable_message_id(message.sender, message.msg_id);
		if (undeliverables.find(message_id) == undeliverables.end())
			throw string("Cannot find message in undeliverables");

		uint32_t final_seq = message.final_seq;
		//Unused because m.data is not needed.
		//DataMessage m = undeliverables[message_id];
		DeliverableMessage dm = DeliverableMessage(
				process_id,
				message.msg_id,
				message.sender,
				final_seq,
				message.final_seq_proposer
				);

		if (deliverables.find(final_seq) != deliverables.end()) {
			throw string("Deliverable already contains another message with same sequence");
		}

		deliverables[final_seq] = dm;
		print_ordered_deliverables();
	}
};

