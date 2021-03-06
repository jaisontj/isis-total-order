#ifndef NETWORK_DATA_TYPES_H
#define NETWORK_DATA_TYPES_H

#include <iostream>

struct NetworkMessage {
	uint32_t type;
	uint32_t sender;
	uint32_t msg_id;
	uint32_t data_or_seq;
	uint32_t proposer_id;
};

struct DataMessage {
	uint32_t type; // must be equal to 1
	uint32_t sender; // the sender’s id
	uint32_t msg_id; // the identifier of the message generated by the sender
	uint32_t data; // a dummy integer
};

struct AckMessage {
	uint32_t type; // must be equal to 2
	uint32_t sender; // the sender of the DataMessage
	uint32_t msg_id; // the identifier of the DataMessage generated by the sender
	uint32_t proposed_seq; // the proposed sequence number
	uint32_t proposer; // the process id of the proposer
};

struct SeqMessage {
	uint32_t type; // must be equal to 3
	uint32_t sender; // the sender of the DataMessage
	uint32_t msg_id; // the identifier of the DataMessage generated by the sender
	uint32_t final_seq; // the final sequence number selected by the sender
	uint32_t final_seq_proposer; // the process id of the proposer who proposed the final_seq
};

struct SeqAckMessage {
	uint32_t type;
	uint32_t sender;
	uint32_t msg_id;
	uint32_t receiver;
};

#endif
