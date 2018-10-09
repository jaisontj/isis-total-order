#ifndef NETWORK_STATUS_H
#define NETWORK_STATUS_H

class NetworkStatus {
	public:
		static bool DROPS_MESSAGE;
		static int DELIVERY_DELAY;
		static void SIMULATE_DELAY(int delay);
		static bool SHOULD_DROP_MESSAGE_RANDOM();
};

#endif
