#include "MessageHandler.h"
#include "ProcessInfoHelper.h"
#include "SeqProvider.h"
#include "Log.h"
#include "LogHelper.h"
#include "MessageQueue.h"
#include "helpers.h"
#include "DeliveryTracker.h"
#include "NetworkStatus.h"
#include "HandshakeTracker.h"

#include <thread>

MessageHandler::MessageHandler(uint32_t total_proposal_count) {
	proposal_tracker = new DataMessageSeqTracker();
	proposal_tracker->set_max_proposal_count(total_proposal_count);
}

void MessageHandler::handle_data_message(DataMessage *message) {
	ProcessInfo p = ProcessInfoHelper::get_process_info(message->sender);
	if (p.hostname == "") {
		Log::e("MessageHandler:: Received DataMessage from unknown sender. Ignoring");
		return;
	}

	//If this message is present in unordered or ordered queue. Flag as duplicate.
	bool is_duplicate_msg = MessageQueue::get_instance().has_received_message(message->msg_id, message->sender);

	if (is_duplicate_msg) {
		Log::d("Received duplicate data message. Ignoring.");
		return;
	}

	uint32_t ID = ProcessInfoHelper::SELF.id;

	//Send ack with last_seq+1
	AckMessage ack = {
		.type = 2,
		.sender = message->sender,
		.msg_id = message->msg_id,
		.proposed_seq = SeqProvider::get_instance().increment_sequence(),
		.proposer = ID
	};
	//Add it to unordered queue
	MessageQueue::get_instance().add_undeliverable(ID, ack.msg_id, ack.sender, ack.proposed_seq, ack.proposer);

	Log::i("MessageHandler:: Proposed Seq->" + to_string(ack.proposed_seq) + " MessageID->" + to_string(ack.msg_id) + " SenderID->" + to_string(ack.sender));
	Log::d("MessageHandler:: Received->" + get_as_string((NetworkMessage *) &message));
	Log::d("MessageHandler:: DispatchQueueAddition ->" + get_as_string((NetworkMessage *) &ack));

	send_message((NetworkMessage *) &ack, sizeof ack, p);
}

void MessageHandler::handle_ack_message(AckMessage *message) {
	//Add proposed seq for message
	uint32_t message_id = message->msg_id;
	proposal_tracker->handle_sequence_proposal(message_id, message->proposed_seq, message->proposer);

	Log::d("MessageHandler:: Recieved -> " + get_as_string((NetworkMessage *) message));

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

		Log::d("MessageHandler:: DispatchQueueAddition ->" + get_as_string((NetworkMessage *) &seq_message));

		send_message((NetworkMessage *) &seq_message, sizeof seq_message, ProcessInfoHelper::PROCESS_LIST);
	} else {
		Log::d("MessageHandler:: Has not received all proposals yet");
	}
}

void MessageHandler::handle_seq_message(SeqMessage *seq_msg) {
	//Attach the final sequence to the respective received_message
	ProcessInfo p = ProcessInfoHelper::get_process_info(seq_msg->sender);
	if (p.hostname == "") {
		Log::e("MessageHandler:: Received SeqMessage from unknown sender. Ignoring");
		return;
	}
	int result = MessageQueue::get_instance().mark_as_deliverable(*seq_msg);
	if (result == -1) {
		Log::d("MessageHandler:: Could not find DataMessage for associated SeqMessage in UndeliverableMessageQueue.\n" + get_as_string((NetworkMessage *) seq_msg));
	}
	//Update last_sequence to this one, if greater
	SeqProvider::get_instance().update_sequence_if_greater(seq_msg->final_seq);
	//Send ack for this seq message
	SeqAckMessage seq_ack = {
		.type = 4,
		.sender = seq_msg->sender,
		.msg_id = seq_msg->msg_id,
		.receiver = ProcessInfoHelper::SELF.id
	};
	Log::d("MessageHandler:: DispatchQueueAddition ->" + get_as_string((NetworkMessage *) &seq_ack));
	send_message((NetworkMessage *) &seq_ack, sizeof seq_ack, p);
}

void MessageHandler::handle_message(NetworkMessage message) {
	if (NetworkStatus::SHOULD_DROP_MESSAGE_RANDOM()) {
		Log::i("MessageHandler:: Dropping message-> " + get_as_string((NetworkMessage *) &message));
		return;
	}
	Log::d("MessageHandler:: NetworkStatus: DROPS_MESSAGE-> " + to_string(NetworkStatus::DROPS_MESSAGE) + " DELIVERY_DELAY-> " + to_string(NetworkStatus::DELIVERY_DELAY));
	int delivery_delay = NetworkStatus::DELIVERY_DELAY;
	if (delivery_delay > 0) {
		thread t(&MessageHandler::handle_message_diff_thread, this, message, delivery_delay);
		t.detach();
	} else handle_message_diff_thread(message, 0);
}

void MessageHandler::handle_message_diff_thread(NetworkMessage message, int delay) {
	NetworkStatus::SIMULATE_DELAY(delay);

	if (message.type == 0) {
		HandshakeTracker::get_instance().handle_handshake(message.sender);
		return;
	}
	if (message.type == 1) {
		handle_data_message((DataMessage *) &message);
		return;
	}

	if (message.type == 2) {
		ProcessInfo p = ProcessInfoHelper::get_process_info(message.proposer_id);
		//Received ack. mark data message as delivered.
		DeliveryTracker::get_instance().mark_as_delivered(message, 1, p.hostname, p.port);
		handle_ack_message((AckMessage *) &message);
		return;
	}

	if (message.type == 3) {
		ProcessInfo p = ProcessInfoHelper::get_process_info(message.sender);
		//Received seq. mark ack as delivered.
		DeliveryTracker::get_instance().mark_as_delivered(message, 2, p.hostname, p.port);
		//If sender of this seq is self, then to avoid duplicates, mark, if any, data messages as delivered.
		if (ProcessInfoHelper::SELF.id == message.sender) {
			DeliveryTracker::get_instance().mark_as_delivered(message, 1, p.hostname, p.port);
		}
		handle_seq_message((SeqMessage *) &message);
		return;
	}

	if (message.type == 4) {
		SeqAckMessage* sm = (SeqAckMessage *) &message;
		ProcessInfo p = ProcessInfoHelper::get_process_info(sm->receiver);
		//Received seqack. mark seq delivered.
		DeliveryTracker::get_instance().mark_as_delivered(message, 3, p.hostname, p.port);
		//Inference: I delivered the ack for this datamessage
		DeliveryTracker::get_instance().mark_as_delivered(message, 2, p.hostname, p.port);
		//Inference: I delivered the data message as well.
		DeliveryTracker::get_instance().mark_as_delivered(message, 1, p.hostname, p.port);
		return;
	}

	Log::e("MessageHandler:: Received unknown message from network: " + get_as_string(&message) + " .Ignoring.");
}


