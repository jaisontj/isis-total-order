#include "SeqProvider.h"

SeqProvider::SeqProvider() {}

SeqProvider& SeqProvider::get_instance() {
	static SeqProvider instance;
	return instance;
}

void SeqProvider::set_sequence(uint32_t new_seq) {
	this->sequence.store(new_seq);
}

uint32_t SeqProvider::get_sequence() {
	return sequence.load();
}

void SeqProvider::update_sequence_if_greater(uint32_t new_seq) {
	if (new_seq > get_sequence()) set_sequence(new_seq);
}

uint32_t SeqProvider::increment_sequence() {
	set_sequence(get_sequence() + 1);
	return get_sequence();
}
