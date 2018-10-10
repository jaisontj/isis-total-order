#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "NetworkDataTypes.h"
#include "DataMessageSeqTracker.h"

class MessageHandler {
	private:
		DataMessageSeqTracker *proposal_tracker;
		void handle_data_message(DataMessage *message);
		void handle_ack_message(AckMessage *message);
		void handle_seq_message(SeqMessage *seq_msg);
		void handle_seq_ack_message(SeqAckMessage *seq_ack_msg);
		void handle_message_diff_thread(NetworkMessage m, int delay);
	public:
		MessageHandler(uint32_t total_proposal_count);
		void handle_message(NetworkMessage message);
};

#endif
