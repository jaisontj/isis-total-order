#ifndef DATA_MESSAGE_SEQ_TRACKER_H
#define DATA_MESSAGE_SEQ_TRACKER_H

#include <iostream>
#include <map>
#include <mutex>
#include <vector>

class ProposalTracker {
	private:
		uint32_t max_proposed_seq;
		uint32_t max_seq_proposer_id;
		std::vector<uint32_t> proposers;
		uint32_t max_proposals;

	public:
		ProposalTracker() {}
		ProposalTracker (uint32_t max_possible_proposals);
		void handle_seq_proposal(uint32_t sequence, uint32_t proposer_id);
		std::vector<uint32_t> get_proposers();
		bool has_received_all_proposals();
		uint32_t get_max_proposed_seq();
		uint32_t get_max_seq_proposer_id();
};



class DataMessageSeqTracker {
	private:
		std::mutex m;
		uint32_t max_proposal_count;
		std::map<uint32_t, ProposalTracker> msg_proposal_map;

	public:
		void set_max_proposal_count(uint32_t count);
		void handle_sequence_proposal(uint32_t message_id, uint32_t proposed_seq, uint32_t proposer_id);
		bool has_received_all_proposals(uint32_t message_id);
		uint32_t get_max_proposed_seq(uint32_t message_id);
		uint32_t get_max_seq_proposer_id(uint32_t message_id);
		std::vector<uint32_t> get_proposers(uint32_t message_id);
};

#endif
