#include<map>
#include<mutex>
#include<algorithm>

#include "MessageQueue.h"
#include "NetworkDataTypes.h"
#include "Log.h"
#include "LogHelper.h"

using namespace std;

MessageQueue::MessageQueue() {}

MessageQueue& MessageQueue::get_instance() {
	static MessageQueue instance;
	return instance;
}

void MessageQueue::log_mqueue() {
	log_line();
	Log::d("Printing currenty present messages in queue...");
	Log::d("Count: " + to_string(mqueue.size()));
	for (Message message: mqueue) {
		Log::d(message.get_as_string());
	}
	log_line();
}

void MessageQueue::sort_mqueue() {
	sort(mqueue.begin(), mqueue.end(), [] (Message const &lhs, Message const &rhs) {
			if (lhs.final_seq == rhs.final_seq) {
			if (lhs.mstatus == rhs.mstatus) {
			return lhs.proposer_id < rhs.proposer_id;
			}
			return lhs.mstatus == UNDELIVERABLE;
			}
			return lhs.final_seq < rhs.final_seq;
			});
}

void MessageQueue::print_ordered_deliverables() {
	log_mqueue();
	sort_mqueue();
	while(mqueue.begin() != mqueue.end()) {
		Message msg = mqueue.front();
		if (msg.mstatus != DELIVERABLE) {
			Log::d("Head of queue is not marked to be delivered");
			break;
		}
		msg.process_message();
		mqueue.erase(mqueue.begin());
	}
}

void MessageQueue::add_undeliverable(
		uint32_t process_id,
		uint32_t msg_id,
		uint32_t sender_id,
		uint32_t final_seq,
		uint32_t proposer_id
		) {
	lock_guard<mutex> lk(m);
	mqueue.push_back(Message(process_id,msg_id, sender_id, final_seq, proposer_id));
}

void MessageQueue::mark_as_deliverable(SeqMessage message) {
	lock_guard<mutex> lk(m);
	Log::d("Going to mark message as deliverable");
	bool marked = false;
	for (vector<int>::size_type i = 0; i < mqueue.size(); i++) {
		if (mqueue[i].sender_id == message.sender && mqueue[i].msg_id == message.msg_id) {
			mqueue[i].mark_as_deliverable(message.final_seq, message.final_seq_proposer);
			marked = true;
			break;
		}
	}
	if (!marked) {
		throw string("Could not find the message to mark. NOT POSSIBLE");
	}
	print_ordered_deliverables();
}

