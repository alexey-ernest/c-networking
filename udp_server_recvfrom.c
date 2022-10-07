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

	printf("Configuring local address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address;
	getaddrinfo(0, "8080", &hints, &bind_address);

	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family,
		bind_address->ai_socktype, bind_address->ai_protocol);
	if (!ISAVALIDSOCKET(socket_listen)) {
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Binding socket to local address...\n");
	if (bind(socket_listen, 
		bind_address->ai_addr, bind_address->ai_addrlen)) {
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);

	/* Receive data from peers */
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	char read[1024];
	int bytes_received = recvfrom(socket_listen,
		read, sizeof(read),
		0,
		(struct sockaddr*) &client_address, &client_len);

	printf("Received (%d bytes): %.*s\n", bytes_received, bytes_received, read);

	printf("Remote address is: ");
	char address_buf[100];
	char service_buf[100];
	getnameinfo((struct sockaddr*) &client_address, client_len,
		address_buf, sizeof(address_buf),
		service_buf, sizeof(service_buf),
		NI_NUMERICHOST | NI_NUMERICSERV);
	printf("%s %s\n", address_buf, service_buf);

	CLOSESOCKET(socket_listen);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Finished.\n");

	return 0;
}