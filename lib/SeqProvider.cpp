#include<iostream>
#include<mutex>

using namespace std;


class SeqProvider {
	private:
		mutex m;
		uint32_t sequence;

	public:

		uint32_t get_sequence() {
			lock_guard<mutex> lk(m);
			return sequence;
		}

		void set_sequence(uint32_t new_seq) {
			lock_guard<mutex> lk(m);
			this->sequence = new_seq;
		}

		void update_sequence_if_greater(uint32_t new_seq) {
			if (new_seq > get_sequence()) set_sequence(new_seq);
		}
};
