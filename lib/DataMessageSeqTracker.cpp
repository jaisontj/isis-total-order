#include "DataMessageSeqTracker.h"
#include <algorithm>

using namespace std;

ProposalTracker::ProposalTracker (uint32_t max_possible_proposals) {
	max_proposals = max_possible_proposals;
	max_proposed_seq = 0;
	max_seq_proposer_id = 0;
}

void ProposalTracker::handle_seq_proposal(uint32_t sequence, uint32_t proposer_id) {
	if (sequence > max_proposed_seq || (sequence == max_proposed_seq && max_seq_proposer_id > proposer_id)) {
		max_proposed_seq = sequence;
		max_seq_proposer_id = proposer_id;
	}
	//Only add to vector if its a new proposer.
	if (find(proposers.begin(), proposers.end(), proposer_id) == proposers.end()) {
		proposers.push_back(proposer_id);
	}
}

vector<uint32_t> ProposalTracker::get_proposers() { return proposers; }

bool ProposalTracker::has_received_all_proposals() { return proposers.size() == max_proposals; }

uint32_t ProposalTracker::get_max_proposed_seq() { return max_proposed_seq; }

uint32_t ProposalTracker::get_max_seq_proposer_id() { return max_seq_proposer_id; }



void DataMessageSeqTracker::set_max_proposal_count(uint32_t count) {
	max_proposal_count = count;
}

void DataMessageSeqTracker::handle_sequence_proposal(
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

bool DataMessageSeqTracker::has_received_all_proposals(uint32_t message_id) {
	lock_guard<mutex> lk(m);
	if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) return false;
	return msg_proposal_map[message_id].has_received_all_proposals();
}

uint32_t DataMessageSeqTracker::get_max_proposed_seq(uint32_t message_id) {
	lock_guard<mutex> lk(m);
	if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) return 0;
	return msg_proposal_map[message_id].get_max_proposed_seq();
}

uint32_t DataMessageSeqTracker::get_max_seq_proposer_id(uint32_t message_id) {
	lock_guard<mutex> lk(m);
	if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) return 0;
	return msg_proposal_map[message_id].get_max_seq_proposer_id();
}

vector<uint32_t> DataMessageSeqTracker::get_proposers(uint32_t message_id) {
	lock_guard<mutex> lk(m);
	if (msg_proposal_map.find(message_id) == msg_proposal_map.end()) return {};
	return msg_proposal_map[message_id].get_proposers();
}

