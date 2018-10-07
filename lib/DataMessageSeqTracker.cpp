#include <iostream>
#include <map>
#include <mutex>

using namespace std;

class ProposalTracker {
	uint32_t max_proposed_seq;
	uint32_t max_seq_proposer_id;
	uint32_t proposal_count;
	uint32_t max_proposals;

	public:
	ProposalTracker() {}
	ProposalTracker (uint32_t max_possible_proposals) {
		max_proposals = max_possible_proposals;
		max_proposed_seq = 0;
		proposal_count = 0;
		max_seq_proposer_id = 0;
	}

	void handle_seq_proposal(uint32_t sequence, uint32_t proposer_id) {
		if (sequence > max_proposed_seq) {
			max_proposed_seq = sequence;
			max_seq_proposer_id = proposer_id;
		}
		proposal_count++;
	}

	bool has_received_all_proposals() {
		return proposal_count == max_proposals;
	}

	uint32_t get_max_proposed_seq() {
		return max_proposed_seq;
	}

	uint32_t get_max_seq_proposer_id() {
		return max_seq_proposer_id;
	}
};



class DataMessageSeqTracker {
	mutex m;
	uint32_t max_proposal_count;
	map<uint32_t, ProposalTracker> msg_proposal_map;

	public:

	void set_max_proposal_count(uint32_t count) { max_proposal_count = count; }
	void handle_sequence_proposal(
			uint32_t message_id,
			uint32_t proposed_seq,
			uint32_t proposer_id
			) {
		lock_guard<mutex> lk(m);
		if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) {
			msg_proposal_map[message_id] = ProposalTracker(max_proposal_count);
		}
		msg_proposal_map[message_id].handle_seq_proposal(proposed_seq, proposer_id);
	}

	bool has_received_all_proposals(uint32_t message_id) {
		lock_guard<mutex> lk(m);
		if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) return false;
		return msg_proposal_map[message_id].has_received_all_proposals();
	}

	uint32_t get_max_proposed_seq(uint32_t message_id) {
		lock_guard<mutex> lk(m);
		if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) return 0;
		return msg_proposal_map[message_id].get_max_proposed_seq();
	}

	uint32_t get_max_seq_proposer_id(uint32_t message_id) {
		lock_guard<mutex> lk(m);
		if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) return 0;
		return msg_proposal_map[message_id].get_max_seq_proposer_id();
	}
};
