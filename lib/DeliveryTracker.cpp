#include "DeliveryTracker.h"
#include "Log.h"
#include "MessageDispatcher.h"
#include "LogHelper.h"

#include<thread>

DeliveryTracker::DeliveryTracker() {}

DeliveryTracker& DeliveryTracker::get_instance() {
	static DeliveryTracker instance;
	return instance;
}

time_t DeliveryTracker::get_expected_delivery_time(NetworkMessage message) {
	time_t current_time = time(NULL);
	switch(message.type) {
		case 2: return current_time + 6*PREDICTED_NETWORK_DELAY;
		case 1:
		case 3:
		default: return current_time + 2*2*PREDICTED_NETWORK_DELAY;
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
	if (msg.message.type == 4 || msg.message.type == 0) {
		Log::d("DeliveryTracker: Not tracking SeqAckMessage or Handshake delivery. Ignoring call.");
		return;
	}
	std::lock_guard<std::mutex> lk(m);
	msg.expected_delivery_time = expected_delivery_time;
	undelivered_msgs.push_back(msg);
	if (!is_tracking.load()) {
		is_tracking.store(true);
		std::thread t(&DeliveryTracker::track_delivery, this);
		t.detach();
	}
	Log::d("DeliveryTracker::Added new message to track delivery");
	DeliveryTracker::log_status(undelivered_msgs);
}

std::vector<MessageInfo> DeliveryTracker::get_undelivered_msgs() {
	std::lock_guard<std::mutex> lk(m);
	return undelivered_msgs;
}

int DeliveryTracker::remove_undelivered_msg(MessageInfo msg) {
	std::lock_guard<std::mutex> lk(m);
	for (std::vector<int>::size_type i=0; i<undelivered_msgs.size(); i++) {
		MessageInfo msg2 = undelivered_msgs[i];
		if (DeliveryTracker::are_equal(msg, msg2)) {
			undelivered_msgs.erase(undelivered_msgs.begin() + i);
			return 1;
		}
	}
	return -1;
}

void DeliveryTracker::track_delivery() {
	Log::d("DeliveryTracker:: Tracking.....");
	while (true) {
		//Go through messages and find messages who exceeded timeout
		std::vector<MessageInfo> tracked_messages = get_undelivered_msgs();
		if (tracked_messages.size() == 0) {
			Log::i("DeliveryTracker:: No more messages to track. Will restart once more messages are added.");
			is_tracking.store(false);
			return;
		}
		for (auto const &msg: get_undelivered_msgs()) {
			time_t current_time = time(NULL);
			if (msg.expected_delivery_time < current_time) {
				if (msg.retry_count == MAX_RETRY_COUNT) {
					Log::e("DeliveryTracker::Maybe process listening at " + msg.hostname + " crashed. No response after " + std::to_string(MAX_RETRY_COUNT)
							+ " retries. Message->" + get_as_string((NetworkMessage *) &msg.message)
							+ ". Exiting application");
					exit(1);
				}
				Log::d("DeliveryTracker::The following message has exceeded timeout from: hostname->" + msg.hostname
						+ " at port->" + msg.port
						+ " RetryCount->" + std::to_string(msg.retry_count)
						+ "Message-> " + get_as_string((NetworkMessage *) &msg.message)
						+ "\n Resending.....");
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
	Log::d("DeliveryTracker:: Going to remove message->" + get_as_string(m));
	int result = remove_undelivered_msg(m);
	if (result == -1) {
		Log::d("DeliveryTracker:: Could not find message to be marked as delivered->" + get_as_string(m) + ". Possible duplicate message received. This is OK.");
		return;
	}
	Log::d("DeliveryTracker:: The following message has been delivered to: hostname->" + m.hostname
			+ " at port->" + m.port
			+ " Message->" + get_as_string(&m.message));
}

void DeliveryTracker::mark_as_delivered(NetworkMessage m, uint32_t type, std::string hostname, std::string port) {
	m.type = type;
	MessageInfo m_info = {
		.message = m,
		.message_size = sizeof m,
		.hostname = hostname,
		.port = port,
		.retry_count = 0,
		.expected_delivery_time = time(NULL)
	};
	mark_as_delivered(m_info);
}

bool DeliveryTracker::contains_undelivered_messages() {
	return undelivered_msgs.size() == 0;
}

void DeliveryTracker::log_status(std::vector<MessageInfo> msgs) {
	std::string l = " Messages: \n";
	for (auto const &m: msgs) {
		l += get_as_string(m) + "\n";
	}
	Log::i("DeliveryTracker status: Count: " + std::to_string(msgs.size())
			+ " Current Time: " + std::to_string(time(NULL)));
	Log::v(l);
}

