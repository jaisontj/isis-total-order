#ifndef HANDSHAKE_TRACKER_H
#define HANDSHAKE_TRACKER_H

#include "NetworkDataTypes.h"
#include <vector>
#include <mutex>

class HandshakeTracker {
	private:
		HandshakeTracker();
		std::mutex m;
	public:
		std::vector<uint32_t> verified_processes;
		static HandshakeTracker& get_instance();
		HandshakeTracker(HandshakeTracker const&) = delete;
		void operator=(HandshakeTracker const&) = delete;
		void handle_handshake(uint32_t sender);
		size_t get_num_verified();
};

#endif
