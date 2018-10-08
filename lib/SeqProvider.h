#ifndef SEQ_PROVIDER_H
#define SEQ_PROVIDER_H

#include<iostream>
#include<atomic>

class SeqProvider {
	private:
		std::atomic<uint32_t> sequence;
		void set_sequence(uint32_t new_seq);
	public:
		uint32_t get_sequence();
		void update_sequence_if_greater(uint32_t new_seq);
		uint32_t increment_sequence();
};

#endif
