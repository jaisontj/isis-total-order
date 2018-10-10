#include "LogHelper.h"
#include "Log.h"
#include<string>

std::string get_as_string(NetworkMessage *message) {
	return "NetworkMessage: Type->" + std::to_string(message->type)
		+ " SenderID->" + std::to_string(message->sender)
		+ " MessageID->" + std::to_string(message->msg_id)
		+ " ThirdVariable->" + std::to_string(message->data_or_seq)
		+ " FourthVariable->" + std::to_string(message->proposer_id);
}

void log_line() {
	Log::d("-----------------------------------------------------------------------------------------------");
}

std::string get_as_string(MessageInfo info) {
	return "MessageInfo: " + get_as_string(&info.message)
		+ "MessageSize->" + std::to_string(info.message_size) + " Hostname->" + info.hostname + " Port->" + info.port + " RetryCount->" + std::to_string(info.retry_count) + " Expected DeliveryTime->" + std::to_string(info.expected_delivery_time);
}
