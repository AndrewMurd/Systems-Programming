 /*********************************************
 *                server.c
 *
 *  A more robust server that receives two
 *  integers from a client and returns their
 *  sum.  In return it gets a nice message
 *  from the client.
 **********************************************/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "lib.h"

int main(int argc, char **argv) {
	int sock, conn;
	int i;
	int rc;
	struct sockaddr address;
	socklen_t addrLength = sizeof(address);
	struct addrinfo hints;
	struct addrinfo *addr;
	int len;
	int arg1, arg2;
	int ret;
	char *message;

	struct data data;
	arg1 = data.arg1;
	arg2 = data.arg2;
	ret = data.result;

	/*
	 *  set the hints structure to zero
	 */
	memset(&hints, 0, sizeof(hints));

	/*
	 *  want a stream, also address that will accept all
	 *  connections on this host
	 */
	hints.ai_socktype = SOCK_STREAM;
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

	rc = listen(sock, 5);
	if(rc < 0) {
		printf("Listen failed\n");
		exit(1);
	}

	/*
	 *  accept an arbitrary number of connections in a loop
	 */
	while((conn = accept(sock, (struct sockaddr*) &address, &addrLength))
		>= 0) {

		ret = receiveData(conn, &arg1);
		ret = receiveData(conn, &arg2);
		printf("Data from client: %d %d\n", arg1, arg2);
		
		arg1 = arg1 + arg2;
		sendData(conn, arg1, arg2);

		message = readString(conn);
		if(message != NULL) {
			printf("Message from client: %s\n", message);
			free(message);
		} else {
			printf("Error receiving message from client\n");
		}
		close(conn);
	}

	close(sock);
	exit(0);
}