/********************************************************
 * 				      Andrew Murdoch	
 * 					    100707816
 *                      client.c
 *
 *  This program interacts with the server.
 *******************************************************/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "lib.h"

// This function handles output from server
void *recieve(void *sockID) {
	char *message;
	int clientSocket = *((int *) sockID);

	while ((message = readString(clientSocket))) {
		if (message == NULL) {
			close(clientSocket);
		} else {
			printf("%s", message);
			free(message);
		}
	}

	printf("You disconnected from server!\n");
	close(clientSocket);
	exit(1);
}

int main(int argc, char **argv) {
	struct addrinfo hints;
	struct addrinfo *addr;
	struct sockaddr_in *addrinfo;
	int rc;
	int sock;
	char buffer[512];
	char *ret;
	char *username;
	int port = 55555;
	// Get username
	if (argc < 2) {
		printf("Must input username as argument!\n");
		exit(0);
	}
	username = argv[1];
	printf("You chose \"%s\" as ur username.\n", username);

	/*
	 *  clear the hints structure to zero
	 */
	memset(&hints, 0, sizeof(hints));

	/*
	 *  want a stream on a compatible interface
    */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;
	/*
	 *  localhost is the name of the current computer
	 */
	rc = getaddrinfo("localhost", NULL, &hints, &addr);
	if (rc != 0) {
		printf("Host name lookup failed: %s\n", gai_strerror(rc));
		exit(1);
	}

	/*
	 *  use the first result from getaddrinfo
	 */
	addrinfo = (struct sockaddr_in *) addr->ai_addr;

	sock = socket(addrinfo->sin_family, addr->ai_socktype, addr->ai_protocol);
	if (sock < 0) {
		printf("Can't create socket\n");
		exit(1);
	}

	/*
	 *  specify the port number
	 */
	addrinfo->sin_port = htons(port);

	rc = connect(sock, (struct sockaddr *) addrinfo, addr->ai_addrlen);
	if (rc != 0) {
		printf("Can't connect to server\n");
		exit(1);
	}

	/*
	 *  free the results returned by get addrinfo
	 */
	freeaddrinfo(addr);

	/*
	 *  Loop reading input from stdin, send the line to
	 *  to the echo server, and then read the results
	 *  from the server.
	 */
	printf("Connection established to port %d ............\n", port);

	// Create seperate process (thread) for recieving data from server using receive function
	pthread_t thread;
	pthread_create(&thread, NULL, recieve, (void *) &sock);

	// Send username to server
	writeString(sock, username);

	// Handle client input
	while ((ret = fgets(buffer, 512, stdin))) {
		/*
		 *  check for user entering end of file
		 */
		if (ret == NULL) {
			shutdown(sock, SHUT_WR);
			break;
		} else {
			writeString(sock, buffer);
		}
	}

	return 0;
}