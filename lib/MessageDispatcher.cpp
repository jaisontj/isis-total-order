#include "MessageDispatcher.h"
#include "SenderSocket.h"
#include "Log.h"
#include "LogHelper.h"
#include "DeliveryTracker.h"
#include "SnapshotHandler.h"

#include <unistd.h>
#include <thread>

using namespace std;

MessageDispatcher::MessageDispatcher() {}

MessageDispatcher& MessageDispatcher::get_instance() {
	static MessageDispatcher instance;
	return instance;
}

MessageInfo MessageDispatcher::get_queue_front() {
	lock_guard<mutex> lk(m);
	auto const &m_info = mqueue.front();
	return m_info;
}

void MessageDispatcher::pop_queue() {
	lock_guard<mutex> lk(m);
	mqueue.pop();
}

bool MessageDispatcher::is_queue_empty() {
	lock_guard<mutex> lk(m);
	return mqueue.empty();
}

void MessageDispatcher::dispatch_messages() {
	Log::d("MessageDispatcher:: Starting message dispatch...");
	while(true) {
		if (is_queue_empty()) {
			Log::d("MessageDispatcher:: MessageQueue is empty. Will restart dispatcher when new messages are added.");
			is_dispatching.store(false);
			break;
		}
		auto const &minfo = get_queue_front();
		try {
			SenderSocket socket = SenderSocket(minfo.hostname, minfo.port);
			Log::d("MessageDispatcher:: Sending-> " + get_as_string((NetworkMessage *) &minfo.message) + " Hostname->" + minfo.hostname + " Port->" + minfo.port + " RetryCount->" + to_string(minfo.retry_count));
			//To handle for cases where the listener receives the message BEFORE this thread adds it to the Delivery tracker.
			time_t delivery_time = DeliveryTracker::get_expected_delivery_time(minfo.message);
			DeliveryTracker::get_instance().track_message(minfo, delivery_time);

			int sent_bytes = socket.send((void *) &minfo.message, minfo.message_size);
			//Dont care if it gets sent or not. Either ways, delivery tracker will handle it.
			pop_queue();
			Log::v("MessageDispatcher:: Sent " + to_string(sent_bytes) + " bytes to " + minfo.hostname);
			socket.free_serve_info();
			socket.close_socket();
			if (sent_bytes != -1) {
				SnapshotHandler::get_instance().handle_last_msg_sent(minfo);
			}
		} catch(string m) {
			Log::e("MessageDispatcher:: Unable to create Sender Socket: " + m);
		}
	}
}

void MessageDispatcher::add_message_to_queue(
		NetworkMessage *message,
		size_t message_size,
		string hostname,
		string port,
		int retry_count
		) {
	lock_guard<mutex> lk(m);
	//Copying contents to new pointer
	mqueue.push({*message, message_size, hostname, port, retry_count, time(NULL)});
	if (!is_dispatching.load()) {
		is_dispatching.store(true);
		thread t(&MessageDispatcher::dispatch_messages, this);
		t.detach();
	}
}

void MessageDispatcher::add_message_to_queue(
		NetworkMessage *message,
		size_t message_size,
		vector<string> hostnames,
		string port
		) {
	for (auto const &hostname: hostnames) {
		add_message_to_queue(message, message_size, hostname.c_str(), port);
	}
}

