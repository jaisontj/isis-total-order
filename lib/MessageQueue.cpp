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
	Log::v("MessageQueue:: Printing currenty present messages in queue...");
	Log::d("MessageQueue:: Count: " + to_string(mqueue.size()));
	for (Message message: mqueue) {
		Log::v(message.get_as_string());
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

void MessageQueue::process_ordered_deliverables() {
	log_mqueue();
	sort_mqueue();
	while(mqueue.begin() != mqueue.end()) {
		Message msg = mqueue.front();
		if (msg.mstatus != DELIVERABLE) {
			Log::v("MessageQueue:: Head of queue is not marked to be delivered");
			break;
		}
		msg.process_message();
		ordered_queue.push_back(msg);
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

int MessageQueue::mark_as_deliverable(SeqMessage message) {
	lock_guard<mutex> lk(m);
	Log::v("MessageQueue:: Going to mark message as deliverable");
	for (vector<int>::size_type i = 0; i < mqueue.size(); i++) {
		if (mqueue[i].sender_id == message.sender && mqueue[i].msg_id == message.msg_id) {
			mqueue[i].mark_as_deliverable(message.final_seq, message.final_seq_proposer);
			process_ordered_deliverables();
			return 1;
		}
	}
	return -1;
}

bool MessageQueue::has_received_message(uint32_t msg_id, uint32_t sender_id) {
	lock_guard<mutex> lk(m);
	for (auto const &msg: ordered_queue) {
		if (msg.sender_id == sender_id && msg.msg_id == msg_id) {
			return true;
		}
	}
	for (auto const &msg: mqueue) {
		if (msg.sender_id == sender_id && msg.msg_id == msg_id) {
			return true;
		}
	}
	return false;
}

