#include <iostream>

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


