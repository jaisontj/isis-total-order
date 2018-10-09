#include "LogHelper.h"
#include "Log.h"
#include<string>

std::string get_as_string(NetworkMessage *message) {
	return "NetworkMessage: Type->" + std::to_string(message->type)
		+ " SenderID->" + std::to_string(message->sender)
		+ " MessageID->" + std::to_string(message->msg_id)
		+ " DataOrProposedSequence->" + std::to_string(message->data_or_seq);
}

std::string get_as_string(DataMessage *message) {
	return "DataMessage:Sender->" + std::to_string(message->sender)
		+ " MessageID->" + std::to_string(message->msg_id)
		+ " Data->" + std::to_string(message->data);
}

std::string get_as_string(AckMessage *message) {
	return "AckMessage: Proposer->" + std::to_string(message->proposer)
		+ " MessageID->" + std::to_string(message->msg_id)
		+ " ProposedSequence->" + std::to_string(message->proposed_seq);
}

std::string get_as_string(SeqMessage *seq_msg) {
	return "SeqMessage:  MessageID->" + std::to_string(seq_msg->msg_id)
		+ " MessageSenderID->" + std::to_string(seq_msg->sender)
		+ " FinalSequence->" + std::to_string(seq_msg->final_seq)
		+ " SeqProposerID->" + std::to_string(seq_msg->final_seq_proposer);
}

void log_line() {
	Log::d("-----------------------------------------------------------------------------------------------");
}

std::string get_as_string(MessageInfo info) {
	return "MessageInfo: " + get_as_string(&info.message)
		+ "MessageSize->" + std::to_string(info.message_size) + " Hostname->" + info.hostname + " Port->" + info.port + " RetryCount->" + std::to_string(info.retry_count) + " Expected DeliveryTime->" + std::to_string(info.expected_delivery_time);
}
