#include "ListenerSocket.h"
#include "Log.h"
#include "LogHelper.h"

#include <unistd.h>
#include <netdb.h>
#include <string>

ListenerSocket::ListenerSocket(std::string port) : DgramSocket(AI_PASSIVE, "", port, 1) {
	Log::v("Listener: Trying to bind socket to address");
	if (::bind(this->fd, p->ai_addr, p->ai_addrlen) == -1) {
		close(this->fd);
		perror("Listener: Failed to bind socket");
		throw std::string ("Listener: Failed to bind socket");
	}
	free_serve_info();
}

void ListenerSocket::start_listening(SocketMessageHandler handler) {
	while (true) {
		Log::v("Listening for messages.....");
		struct sockaddr_storage recv_addr;
		socklen_t recv_addr_len = sizeof(recv_addr);
		NetworkMessage *message = new NetworkMessage();
		int recv_bytes = recvfrom(this->fd, message, sizeof (NetworkMessage), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
		if (recv_bytes == -1) {
			perror("Listener: Error in receiving message");
			Log::e("Listener: Error in receiving message");
		}
		Log::v("Listener: packet is " + std::to_string(recv_bytes) + " bytes long");
		Log::d("Received NetworkMessage-----------------");
		log(message);
		handler(message);
	}
}
