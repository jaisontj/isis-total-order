#include "LogHelper.h"
#include "Log.h"
#include<string>

void log(NetworkMessage *message) {
	Log::d("NetworkMessage: Type->" + std::to_string(message->type)
			+ " SenderID->" + std::to_string(message->sender)
			+ " MessageID->" + std::to_string(message->msg_id)
			+ " DataOrProposedSequence->" + std::to_string(message->data_or_seq));
}

void log(DataMessage *message) {
	Log::d("DataMessage:Sender->" + std::to_string(message->sender)
			+ " MessageID->" + std::to_string(message->msg_id)
			+ " Data->" + std::to_string(message->data));
}

void log(AckMessage *message) {
	Log::d("AckMessage: Proposer->" + std::to_string(message->proposer)
			+ " MessageID->" + std::to_string(message->msg_id)
			+ " ProposedSequence->" + std::to_string(message->proposed_seq));
}

void log(SeqMessage *seq_msg) {
	Log::d("SeqMessage:  MessageID->" + std::to_string(seq_msg->msg_id)
			+ " MessageSenderID->" + std::to_string(seq_msg->sender)
			+ " FinalSequence->" + std::to_string(seq_msg->final_seq)
			+ " SeqProposerID->" + std::to_string(seq_msg->final_seq_proposer));
}

void log_line() {
	Log::d("-----------------------------------------------------------------------------------------------");
}
