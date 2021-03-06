#include "Message.h"
#include <fstream>

Message::Message(
		uint32_t process_id,
		uint32_t msg_id,
		uint32_t sender_id,
		uint32_t final_seq,
		uint32_t proposer_id) {
	this->process_id = process_id;
	this->msg_id = msg_id;
	this->sender_id = sender_id;
	this->final_seq = final_seq;
	this->proposer_id = proposer_id;
	this->mstatus = UNDELIVERABLE;
}

void Message::process_message() {
	std::cout<<get_as_string();
	std::ofstream file;
	std::string filename = "Process-" + std::to_string(process_id) + ".txt";
	file.open(filename, std::ios::app);
	file<<get_as_string_without_process_id()<<std::endl;
	file.close();
}

std::string Message::get_as_string() {
	return std::to_string(process_id)
		+ ": " + get_as_string_without_process_id() + "\n";
}

std::string Message::get_as_string_without_process_id() {
	return "Processed message "  + std::to_string(msg_id)
		+ " from sender " + std::to_string(sender_id)
		+ " with seq (" + std::to_string(final_seq) + ", " + std::to_string(proposer_id) + ")";
}


void Message::mark_as_deliverable(uint32_t final_seq, uint32_t proposer) {
	this->mstatus = DELIVERABLE;
	this->final_seq = final_seq;
	this->proposer_id = proposer;
}


