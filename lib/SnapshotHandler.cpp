#include "SnapshotHandler.h"
#include "ProcessInfoHelper.h"
#include "TcpSender.h"
#include "Log.h"
#include "MessageQueue.h"
#include "Marker.h"

#include <vector>

SnapshotHandler::SnapshotHandler() {}

SnapshotHandler& SnapshotHandler::get_instance() {
	static SnapshotHandler instance;
	return instance;
}

void SnapshotHandler::handle_last_msg_sent(MessageInfo m) {
	if (m.message.type == 1) {
		num_messages_sent.store(num_messages_sent + 1);
	}
	if (m.message.type == 2 || m.message.type == 3) {
		last_seq_sent.store(m.message.data_or_seq);
	}
	if (num_messages_sent == SnapshotHandler::X && !has_saved_state) {
		initiate_snapshot();
	}
}

void SnapshotHandler::mark_process(std::string message) {
	std::lock_guard<std::mutex> lk(m);
	bool is_present = false;
	for (auto &process: marker_received_processes) {
		if (process == message) {
			is_present = true;
			break;
		}
	}
	if (!is_present) {
		marker_received_processes.push_back(message);
	}

	Log::i("Number of processes that received marker: " + std::to_string(marker_received_processes.size()));

	if (marker_received_processes.size() == ProcessInfoHelper::PROCESS_LIST.size() - 1) {
		std::cout<<"#################SNAPSHOT DONE"<<std::endl;
	}
}

void SnapshotHandler::handle_marker(Marker m) {
	if (m.type != 1 || m.sender > ProcessInfoHelper::PROCESS_LIST.size()){
		Log::i("SnapshotHandler:: Received message on TCP. But it was not a marker.");
		return;
	}
	Log::i("SnapshotHandler:: Received marker from " + std::to_string(m.sender));
	mark_process(std::to_string(m.sender));
	if (!has_saved_state) {
		initiate_snapshot();
	}
}

void SnapshotHandler::initiate_snapshot() {
	if (has_saved_state) {
		Log::e("Already saved state. Only ONE Process can initiate snapshot. Ignoring.");
		return;
	}
	std::cout<<"######################TAKING SNAPSHOT"<<std::endl;
	//save state
	save_state();
	//Send marker to everyone via tcp
	send_marker_to_all();
}
void SnapshotHandler::send_marker_to_all() {
	std::vector<ProcessInfo> processes = ProcessInfoHelper::get_all_other_processes();
	std::vector<ProcessInfo> unmarked_processes;
	for (auto &process: processes) {
		bool is_present = false;
		for (auto &mp: marker_received_processes) {
			if (mp == std::to_string(process.id)) {
				is_present = true;
				break;
			}
		}
		if (!is_present) {
			unmarked_processes.push_back(process);
		}
	}
	std::vector<uint32_t> sent_list;
	while(sent_list.size() < unmarked_processes.size()) {
		for (auto const &process: processes) {
			bool connected = false;
			for (auto const &pid: sent_list) {
				if (pid == process.id) {
					connected = true;
					break;
				}
			}
			if (connected) {
				continue;
			}
			std::string hostname = process.hostname;
			std::string port = process.port;
			try {
				TcpSender sender = TcpSender(hostname, port);
				Marker m = { .type = 1, .sender = ProcessInfoHelper::SELF.id};
				Log::i("Sending marker to : " + hostname + " at port: " + port + "Marker: " + std::to_string(m.sender));
				sender.send((void *) &m, sizeof m);
				sender.free_serve_info();
				//sender.close_socket();
				sent_list.push_back(process.id);
			} catch(std::string m) {
				Log::e(m);
			}
		}
		if (sent_list.size() == processes.size()) {
			Log::i("Sent marker to all processes");
		} else {
			Log::i("Sent marker to " + std::to_string(sent_list.size()) + " processes");
		}
	}
}

void SnapshotHandler::save_state() {
	has_saved_state = true;
	std::cout<<"---------------------------------------------------"<<std::endl;
	std::cout<<"Printing Process State:"<<std::endl;
	std::cout<<"Last sent sequence: "<<last_seq_sent<<std::endl;
	std::cout<<"Unordered Messages: "<<std::endl;
	std::vector<Message> unordered_mgs = MessageQueue::get_instance().get_unordered_messages();
	std::cout<<"Count: "<<unordered_mgs.size()<<std::endl;
	for (auto &msg: unordered_mgs) {
		std::cout<<msg.get_as_string_without_process_id()<<std::endl;
	}
	std::cout<<"Ordered Messages: "<<std::endl;
	std::vector<Message> ordered_msgs = MessageQueue::get_instance().get_ordered_messages();
	std::cout<<"Count: "<<ordered_msgs.size()<<std::endl;
	for (auto &msg: ordered_msgs) {
		std::cout<<msg.get_as_string_without_process_id()<<std::endl;
	}
	std::cout<<"---------------------------------------------------"<<std::endl;
}

