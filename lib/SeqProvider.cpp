#include<iostream>
#include<atomic>

using namespace std;


class SeqProvider {
	private:
		atomic<uint32_t> sequence;
		void set_sequence(uint32_t new_seq) {
			this->sequence.store(new_seq);
		}
	public:

		uint32_t get_sequence() {
			return sequence.load();
		}

		void update_sequence_if_greater(uint32_t new_seq) {
			if (new_seq > get_sequence()) set_sequence(new_seq);
		}

		uint32_t increment_sequence() {
			set_sequence(get_sequence() + 1);
			return get_sequence();
		}
};
