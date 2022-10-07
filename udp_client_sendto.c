#include "sock_comm.h"

int main(int argc, char const *argv[])
{
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d)) {
		fprintf(stderr, "Failed to initialize.\n");
		return 1;
	}
#endif

	if (argc < 3) {
		fprintf(stderr, "usage: udp_client_sendto hostname port\n");
		return 1;
	}	

	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;
	struct addrinfo *peer_address;
	if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Remote address is: ");
	char address_buf[100];
	char service_buf[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
		address_buf, sizeof(address_buf),
		service_buf, sizeof(service_buf),
		NI_NUMERICHOST | NI_NUMERICSERV);
	printf("%s %s\n", address_buf, service_buf);

	printf("Creating socket...\n");
	SOCKET socket_peer;
	socket_peer = socket(peer_address->ai_family,
		peer_address->ai_socktype, peer_address->ai_protocol);
	if (!ISAVALIDSOCKET(socket_peer)) {
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	const char *message = "Hello World";
	printf("Sending: %s\n", message);
	int bytes_sent = sendto(socket_peer, message, strlen(message),
		0,
		peer_address->ai_addr, peer_address->ai_addrlen);
	printf("Send %d bytes.\n", bytes_sent);

	freeaddrinfo(peer_address);
	CLOSESOCKET(socket_peer);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Finished.\n");

	return 0;
}