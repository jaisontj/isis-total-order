#ifndef SEQ_PROVIDER_H
#define SEQ_PROVIDER_H

#include<iostream>
#include<atomic>

class SeqProvider {
	private:
		SeqProvider();
		std::atomic<uint32_t> sequence;
		void set_sequence(uint32_t new_seq);
		uint32_t get_sequence();
	public:
		static SeqProvider& get_instance();
		SeqProvider(SeqProvider const&) = delete;
		void operator=(SeqProvider const&) = delete;
		void update_sequence_if_greater(uint32_t new_seq);
		uint32_t increment_sequence();
};

#endif
