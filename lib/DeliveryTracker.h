#ifndef MSG_DELIVERY_TRACKER_H
#define MSG_DELIVERY_TRACKER_H

#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <ctime>
#include <mutex>

#include "NetworkDataTypes.h"
#include "MessageDispatcher.h"

#define PREDICTED_NETWORK_DELAY 2
#define MAX_RETRY_COUNT 5

class DeliveryTracker {
	private:
		std::mutex m;
		std::atomic<bool> is_tracking;
		DeliveryTracker();
		std::vector<MessageInfo> undelivered_msgs;
		std::vector<MessageInfo> get_undelivered_msgs();
		int remove_undelivered_msg(MessageInfo m);
		void track_delivery();
	public:
		static DeliveryTracker& get_instance();
		DeliveryTracker(DeliveryTracker const&) = delete;
		void operator=(DeliveryTracker const&) = delete;
		static time_t get_expected_delivery_time(NetworkMessage message);
		static bool are_equal(MessageInfo m1, MessageInfo m2);
		void track_message(MessageInfo m, time_t expected_delivery_time);
		void mark_as_delivered(MessageInfo m);
		void mark_as_delivered(NetworkMessage m, uint32_t type, std::string hostname, std::string port);
		bool contains_undelivered_messages();
		static void log_status(std::vector<MessageInfo> msgs);
};

#endif
