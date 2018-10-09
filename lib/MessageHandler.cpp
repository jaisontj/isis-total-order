#include "MessageHandler.h"
#include "ProcessInfoHelper.h"
#include "SeqProvider.h"
#include "Log.h"
#include "LogHelper.h"
#include "MessageQueue.h"
#include "helpers.h"

MessageHandler::MessageHandler(uint32_t total_proposal_count) {
	proposal_tracker = new DataMessageSeqTracker();
	proposal_tracker->set_max_proposal_count(total_proposal_count);
}

void MessageHandler::handle_data_message(DataMessage *message) {
	ProcessInfo p = ProcessInfoHelper::get_process_info(message->sender);
	if (p.hostname == "") {
		Log::e("Received DataMessage from unknown sender. Ignoring");
		return;
	}

	uint32_t ID = ProcessInfoHelper::SELF.id;

	//send ack with last_seq+1
	AckMessage ack = {
		.type = 2,
		.sender = message->sender,
		.msg_id = message->msg_id,
		.proposed_seq = SeqProvider::get_instance().increment_sequence(),
		.proposer = ID
	};
	//Add it to unordered queue
	MessageQueue::get_instance().add_undeliverable(ID, ack.msg_id, ack.sender, ack.proposed_seq, ack.proposer);

	Log::d("Received DataMessage-------------------");
	log(message);
	Log::d("Sending ACK----------------------------");
	log((AckMessage *) &ack);
	send_message((NetworkMessage *) &ack, sizeof ack, p);
	//TODO: wait for final_seq
	//TODO: What happens if no ack is received?
}

void MessageHandler::handle_ack_message(AckMessage *message) {
	//Add proposed seq for message
	uint32_t message_id = message->msg_id;
	proposal_tracker->handle_sequence_proposal(
			message_id,
			message->proposed_seq,
			message->proposer
			);
	Log::d("Recieved AckMessage---------------------");
	log(message);
	if (proposal_tracker->has_received_all_proposals(message_id)) {
		Log::d("Sending SeqMessage------------------");
		uint32_t final_seq = proposal_tracker->get_max_proposed_seq(message_id);
		uint32_t final_seq_proposer = proposal_tracker->get_max_seq_proposer_id(message_id);
		SeqMessage seq_message = {
			.type = 3,
			.sender = message->sender,
			.msg_id = message_id,
			.final_seq = final_seq,
			.final_seq_proposer = final_seq_proposer
		};
		log((SeqMessage *) &seq_message);
		send_message((NetworkMessage *) &seq_message, sizeof seq_message, ProcessInfoHelper::PROCESS_LIST);
		//TODO: what happens if the above message does not reach?
	} else {
		Log::d("Has not received all proposals yet-------------------------------------");
	}
	//TODO:if no, chill out. Wait....
}

void MessageHandler::handle_seq_message(SeqMessage *seq_msg) {
	//attach the final sequence to the respective received_message
	try {
		MessageQueue::get_instance().mark_as_deliverable(*seq_msg);
		//update last_sequence to this one, if greater
		SeqProvider::get_instance().update_sequence_if_greater(seq_msg->final_seq);
		//TODO: send ack for this seq messsage
	} catch(string m) {
		cout<<"Error with SequenceMessage: "<<m<<endl;
	}
}

void MessageHandler::handle_message(NetworkMessage message) {
	switch(message.type) {
		case 1:
			handle_data_message((DataMessage *) &message);
			return;
		case 2:
			handle_ack_message((AckMessage *) &message);
			return;
		case 3:
			handle_seq_message((SeqMessage *) &message);
			return;
		default:
			Log::e("Received unknown message type from network. Ignoring");
	}
}

