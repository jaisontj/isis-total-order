#include "MessageHandler.h"
#include "ProcessInfoHelper.h"
#include "SeqProvider.h"
#include "Log.h"
#include "LogHelper.h"
#include "MessageQueue.h"
#include "helpers.h"
#include "DeliveryTracker.h"
#include "NetworkStatus.h"

#include <thread>

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

	Log::i("Proposed Seq: " + to_string(ack.proposed_seq));
	Log::v("Received->" + get_as_string(message));
	Log::v("Sending ->" + get_as_string((AckMessage *) &ack));
	send_message((NetworkMessage *) &ack, sizeof ack, p);
}

void MessageHandler::handle_ack_message(AckMessage *message) {
	//Add proposed seq for message
	uint32_t message_id = message->msg_id;
	proposal_tracker->handle_sequence_proposal(
			message_id,
			message->proposed_seq,
			message->proposer
			);
	Log::d("Recieved -> " + get_as_string(message));
	if (proposal_tracker->has_received_all_proposals(message_id)) {
		uint32_t final_seq = proposal_tracker->get_max_proposed_seq(message_id);
		uint32_t final_seq_proposer = proposal_tracker->get_max_seq_proposer_id(message_id);
		SeqMessage seq_message = {
			.type = 3,
			.sender = message->sender,
			.msg_id = message_id,
			.final_seq = final_seq,
			.final_seq_proposer = final_seq_proposer
		};
		Log::d("Sending ->" + get_as_string((SeqMessage *) &seq_message));
		send_message((NetworkMessage *) &seq_message, sizeof seq_message, ProcessInfoHelper::PROCESS_LIST);
		//TODO: what happens if the above message does not reach?
	} else {
		Log::d("Has not received all proposals yet");
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
		Log::e("Error with SequenceMessage: " + m + "\n" + get_as_string(seq_msg));
	}
}

void MessageHandler::handle_message(NetworkMessage message) {
	if (NetworkStatus::SHOULD_DROP_MESSAGE_RANDOM()) {
		Log::i("Dropping message-> " + get_as_string((NetworkMessage *) &message));
		return;
	}
	Log::d("NetworkStatus: DROPS_MESSAGE-> " + to_string(NetworkStatus::DROPS_MESSAGE) + " DELIVERY_DELAY-> " + to_string(NetworkStatus::DELIVERY_DELAY));
	int delivery_delay = NetworkStatus::DELIVERY_DELAY;
	if (delivery_delay > 0) {
		thread t(&MessageHandler::handle_message_diff_thread, this, message, delivery_delay);
		t.detach();
	} else handle_message_diff_thread(message, 0);
}

void MessageHandler::handle_message_diff_thread(NetworkMessage message, int delay) {
	NetworkStatus::SIMULATE_DELAY(delay);
	if (message.type == 1) {
		handle_data_message((DataMessage *) &message);
		return;
	}

	if (message.type == 2) {
		ProcessInfo p = ProcessInfoHelper::get_process_info(message.proposer_id);
		NetworkMessage m = message;
		m.type = 1;
		MessageInfo m_info = MessageInfo {
			.message = m,
				.message_size = sizeof message,
				.hostname = p.hostname,
				.port = p.port,
				.retry_count = 0,
				.expected_delivery_time = time(NULL)
		};
		DeliveryTracker::get_instance().mark_as_delivered(m_info);
		handle_ack_message((AckMessage *) &message);
		return;
	}

	if (message.type == 3) {
		ProcessInfo p = ProcessInfoHelper::get_process_info(message.sender);
		NetworkMessage m = message;
		m.type = 2;
		MessageInfo m_info = MessageInfo {
			.message = m,
				.message_size = sizeof message,
				.hostname = p.hostname,
				.port = p.port,
				.retry_count = 0,
				.expected_delivery_time = time(NULL)
		};
		DeliveryTracker::get_instance().mark_as_delivered(m_info);
		handle_seq_message((SeqMessage *) &message);
		return;
	}

	Log::e("Received unknown message type from network. Ignoring");
}


