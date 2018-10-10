#include "NetworkStatus.h"
#include "Log.h"
#include <unistd.h>

void NetworkStatus::SIMULATE_DELAY(int delay) {
	Log::d("Simulating delay: " + std::to_string(delay));
	sleep(delay);
}

bool NetworkStatus::SHOULD_DROP_MESSAGE_RANDOM() {
	if (DROPS_MESSAGE) {
		srand(time(0));
		return rand() % 3 == 0;
	}
	return false;
}


