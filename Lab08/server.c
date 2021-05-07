/***********************************************
 *
 *                server.c
 *
 *  A data gram based echo server.
 **********************************************/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	int sock;
	int arg1, arg2;
	int i;
	int rc;
	struct sockaddr address;
	socklen_t addrLength = sizeof(address);
	struct addrinfo hints;
	struct addrinfo *addr;
	char *message;

	/*
	 *  set the hints structure to zero
	 */
	memset(&hints, 0, sizeof(hints));

	/*
	 *  want a datagram, also address that will accept all
	 *  connections on this host
	 */
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	if((rc = getaddrinfo(NULL, "4321", &hints, &addr))) {
		printf("host name lookup failed: %s\n", gai_strerror(rc));
		exit(1);
	}

	/*
	 *  use the first entry returned by getaddrinfo
	 */
	sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if(sock < 0) {
		printf("Can't create socket\n");
		exit(1);
	}

	/*
	 *  want to be able to reuse the address right after
	 *  the socket is closed.  Otherwise must wait for 2 minutes
	 */
	i = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	rc = bind(sock, addr->ai_addr, addr->ai_addrlen);
	if(rc < 0) {
		printf("Can't bind socket\n");
		exit(1);
	}

	/*
	 *  free results returned by getaddrinfo
	 */
	freeaddrinfo(addr);
	/*
	 *  loop receiving a string from client and echoing it back
	 */
	while(1) {
		/*
		 *  read message from client and respond
		 */
		recvfrom(sock, (char*) &arg1, sizeof(arg1), 0, (struct sockaddr*) &address,  &addrLength);
		recvfrom(sock, (char*) &arg2, sizeof(arg2), 0, (struct sockaddr*) &address,  &addrLength);
		printf("Data from client: %d %d\n", arg1, arg2);
		arg1 = arg1+arg2;
		sendto(sock, (char *) &arg1, sizeof(arg1), 0, 
				(const struct sockaddr*) &address, addrLength);

		// recvfrom(sock, (char*) &message, sizeof(message), 0, (struct sockaddr*) &address,  &addrLength);
		// if(message != NULL) {
		// 	printf("Message from client: %s\n", message);
		// 	free(message);
		// } else {
		// 	printf("Error receiving message from client\n");
		// }
	}
	close(sock);
	exit(0);
}
