#include "HandshakeTracker.h"

HandshakeTracker::HandshakeTracker() {}

HandshakeTracker& HandshakeTracker::get_instance() {
	static HandshakeTracker instance;
	return instance;
}

void HandshakeTracker::handle_handshake(uint32_t sender) {
	std::lock_guard<std::mutex> lk(m);
	for (std::vector<int>::size_type i = 0; i<verified_processes.size(); i++) {
		if (verified_processes[i] == sender) {
			return;
		}
	}
	verified_processes.push_back(sender);
}

size_t HandshakeTracker::get_num_verified() {
	std::lock_guard<std::mutex> lk(m);
	return verified_processes.size();
}
