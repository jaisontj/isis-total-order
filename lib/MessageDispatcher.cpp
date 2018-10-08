#include "MessageDispatcher.h"
#include "SenderSocket.h"
#include "Log.h"
#include "LogHelper.h"

#include <unistd.h>
#include <thread>

using namespace std;

int MessageDispatcher::MAX_DELAY = 5;

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
	Log::d("Starting message dispatch...");
	while(true) {
		if (is_queue_empty()) {
			if (delay < MAX_DELAY) delay *= 2;
			Log::v("MessageQueue is empty. Sleeping for " + to_string(delay) + " seconds");
			sleep(delay);
			continue;
		}
		delay = 1;
		auto const &minfo = get_queue_front();
		try {
			SenderSocket socket = SenderSocket(minfo.hostname, minfo.port);
			Log::d("Sending NetworkMessage to socket:");
			log((NetworkMessage *) &minfo.message);
			int sent_bytes = socket.send((void *) &minfo.message, minfo.message_size);
			if (sent_bytes != -1) pop_queue();
			Log::v("Sent " + to_string(sent_bytes) + " bytes to " + minfo.hostname);
			socket.free_serve_info();
			socket.close_socket();
		} catch(string m) {
			Log::e("Unable to create Sender Socket: " + m);
		}
	}
}

void MessageDispatcher::add_message_to_queue(
		NetworkMessage *message,
		size_t message_size,
		string hostname,
		string port
		) {
	lock_guard<mutex> lk(m);
	//Copying contents to new pointer
	mqueue.push({*message, message_size, hostname, port});
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

