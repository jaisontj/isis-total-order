#include "DeliveryTracker.h"
#include "Log.h"
#include "MessageDispatcher.h"

DeliveryTracker::DeliveryTracker() {}

DeliveryTracker& DeliveryTracker::get_instance() {
	static DeliveryTracker instance;
	return instance;
}

time_t DeliveryTracker::get_expected_delivery_time(NetworkMessage message) {
	time_t current_time = time(NULL);
	switch(message.type) {
		case 2: return current_time + 6*NETWORK_DELAY;
		case 1:
		case 3:
		default: return current_time + 2*2*NETWORK_DELAY;
	}
}

bool DeliveryTracker::are_equal(MessageInfo m1, MessageInfo m2) {
	return m1.message.type == m2.message.type
		&& m1.message.sender == m2.message.sender
		&& m1.message.msg_id == m2.message.msg_id
		&& m1.hostname == m2.hostname
		&& m1.port == m2.port;
}

void DeliveryTracker::track_message(MessageInfo msg, time_t expected_delivery_time) {
	std::lock_guard<std::mutex> lk(m);
	msg.expected_delivery_time = expected_delivery_time;
	undelivered_msgs.push_back(msg);
}

std::vector<MessageInfo> DeliveryTracker::get_undelivered_msgs() {
	std::lock_guard<std::mutex> lk(m);
	return undelivered_msgs;
}

void DeliveryTracker::remove_undelivered_msg(MessageInfo msg) {
	std::lock_guard<std::mutex> lk(m);
	for (std::vector<int>::size_type i = 0; i < undelivered_msgs.size(); i++) {
		MessageInfo msg2 = undelivered_msgs[i];
		if (DeliveryTracker::are_equal(msg, msg2)) {
			undelivered_msgs.erase(undelivered_msgs.begin() + i);
		}
	}
}

void DeliveryTracker::track_delivery() {
	Log::d("Tracking delivery.....");
	while (true) {
		//Go through messages and find messages who exceeded timeout
		for (auto const &msg: get_undelivered_msgs()) {
			time_t current_time = time(NULL);
			if (msg.expected_delivery_time < current_time) {
				if (msg.retry_count == MAX_RETRY_COUNT) {
					std::cout<<"Maybe process listening at "
						<<msg.hostname<<" crashed."
						<<"No response after " <<MAX_RETRY_COUNT<<" retries."
						<<"Exiting application"
						<<std::endl;
					exit(1);
				}
				MessageDispatcher::get_instance()
					.add_message_to_queue((NetworkMessage *) &msg.message,
							msg.message_size,
							msg.hostname,
							msg.port,
							msg.retry_count + 1);
				remove_undelivered_msg(msg);
			}
		}
	}
}

void DeliveryTracker::mark_as_delivered(MessageInfo m) {
	remove_undelivered_msg(m);
}
