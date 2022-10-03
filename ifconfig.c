#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	struct ifaddrs *addresses;

	if (getifaddrs(&addresses) == -1) {
		printf("getifaddrs call failed\n");
		return -1;
	}

	struct ifaddrs *address;
	for (address = addresses; address; address = address->ifa_next) {
		int family = address->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6) {
			printf("%s\t", address->ifa_name);						/* adapter name */
			printf("%s\t", family == AF_INET ? "IPv4" : "IPv6");	/* address type */

			char ap[100];
			const int family_size = family == AF_INET ? 
				sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
			getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);

			printf("\t%s\n", ap);
		}
	}

	freeifaddrs(addresses);

	return 0;
}