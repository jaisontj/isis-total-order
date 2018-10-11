#ifndef SNAPSHOT_HANDLER_H
#define SNAPSHOT_HANDLER_H

#include "MessageInfo.h"
#include "Marker.h"

#include <iostream>
#include <vector>
#include <atomic>
#include <mutex>

class SnapshotHandler {
	private:
		SnapshotHandler();
		std::mutex m;
		std::vector<std::string> marker_received_processes;
		std::atomic<int> num_messages_sent;
		std::atomic<int> last_seq_sent;
		std::atomic<bool> has_saved_state;
		void initiate_snapshot();
		void send_marker_to_all();
		void save_state();
		void mark_process(std::string message);
	public:
		static SnapshotHandler& get_instance();
		SnapshotHandler(SnapshotHandler const&) = delete;
		void operator=(SnapshotHandler const&) = delete;
		static int X;
		void handle_last_msg_sent(MessageInfo m);
		void handle_marker(Marker m);
};

#endif
