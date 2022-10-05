#include "sock_comm.h"
#include <ctype.h>

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
	hints.ai_socktype = SOCK_STREAM;
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

	printf("Listening...\n");
	if (listen(socket_listen, 10) < 0) {
		fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	/* Handling socket events */
	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	SOCKET max_socket = socket_listen;

	printf("Waiting for connections...\n");

	while (1) {
		fd_set reads;
		reads = master;
		if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
			fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
			return 1;
		}

		SOCKET i;
		for (i = 1; i <= max_socket; ++i) {
			if (FD_ISSET(i, &reads)) {
				if (i == socket_listen) { /* ready to accept() a new connection */
					struct sockaddr_storage client_address;
					socklen_t client_len = sizeof(client_address);
					SOCKET socket_client = accept(socket_listen,
						(struct sockaddr*) &client_address, &client_len);
					if (!ISAVALIDSOCKET(socket_client)) {
						fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
						return 1;
					}

					FD_SET(socket_client, &master);
					if (socket_client > max_socket)
						max_socket = socket_client;

					char address_buf[100];
					getnameinfo((struct sockaddr*) &client_address, client_len,
						address_buf, sizeof(address_buf), 0, 0, NI_NUMERICHOST);
					printf("New connection from %s\n", address_buf);

				} else { /* a socket is ready to recv() call */
					printf("Reading from a peer...\n");

					char read[1024];
					int bytes_received = recv(i, read, sizeof(read), 0);
					if (bytes_received < 1) {
						printf("The peer got disconnected.\n");
						FD_CLR(i, &master);
						CLOSESOCKET(i);
						continue;
					}

					printf("Got message: %.*s", bytes_received, read);

					int j;
					for (j = 1; j <= max_socket; ++j) {
						if (FD_ISSET(j, &master)) {
							if (j == socket_listen || j == i) {
								continue;
							}

							send(j, read, bytes_received, 0);
						}
					}
				}
			}
		}
	}

	printf("Closing listening socket...\n");
	CLOSESOCKET(socket_listen);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Finished.\n");

	return 0;
}