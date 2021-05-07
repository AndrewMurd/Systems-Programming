/********************************************************
 * 				      Andrew Murdoch	
 * 					    100707816
 *                      server.c
 *
 *    This program creates server to handle clients.
 *******************************************************/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <pthread.h>
#include "lib.h"

// Client data structure
struct client {
	int index;
	int sockID;
	char *username;
};

struct client Client[1024];
pthread_t thread[1024];
int clientCount = 0;
char history[12][512];	// Holds 12 lines of history
int histCount = 0;
int i;

// Concatenate two strings together
char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

// Handle server and client interaction
void *handleNetworking(void *ClientDetail) {
	struct client* clientDetail = (struct client*) ClientDetail;
	int index = clientDetail->index;
	int clientSocket = clientDetail->sockID;
	char *message;

	char *name = Client[index].username;
	char *cat = concat(name, ": ");

	printf("Client %d connected with username \"%s\".\n", index + 1, name);
	
	// Send history to new client
	for (i = 0; i < 12; i++) {
		if (strcmp(history[i], "") != 0) {
			writeString(clientSocket, history[i]);
		}
	}

	while ((message = readString(clientSocket))) {		
		// Create history array
		if (histCount < 12) {
			strcpy(history[histCount], concat(cat, message));
			histCount++;
		} else {
			for (i = 0; i < 12; i++) {
				strcpy(history[i], history[i+1]);
			}
			strcpy(history[11], concat(cat, message));
		}
		// Handle output from client
		if (message == NULL) {
			close(clientSocket);
		} else {
			for (i = 0; i < clientCount; i++) {
				//printf("socket: %d\n", Client[i].sockID);
				if (Client[i].sockID != 0) {
					writeString(Client[i].sockID, concat(cat, message));
				}
			}
			printf("%s", concat(cat, message));
			free(message);
		}
	}

	// Close socket after client disconnects
	close(clientDetail->sockID);
	// Handles client socket array
	for (i = 0; i <= clientCount; i++) {
		if (Client[i].sockID == Client[index].sockID) {
			Client[i].sockID = 0;
			break;
		}
	}
	printf("Client %d has disconnected with username \"%s\".\n", index + 1, name);
}

int main(int argc, char **argv) {
	
	for(i = 0; i < 12; i++) {
		strcpy(history[i], "");
	}

	int sock;
	int i;
	int rc;
	struct sockaddr address;
	socklen_t addrLength = sizeof(address);
	struct addrinfo hints;
	struct addrinfo *addr;
	char *port = "55555";
	int newSock;
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
	if ((rc = getaddrinfo("localhost", port, &hints, &addr))) {
		printf("host name lookup failed: %s\n", gai_strerror(rc));
		exit(1);
	}

	/*
	 *  use the first entry returned by getaddrinfo
	 */
	sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (sock < 0) {
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
	if (rc < 0) {
		printf("Can't bind socket\n");
		exit(1);
	}

	/*
	 *  free results returned by getaddrinfo
	 */
	freeaddrinfo(addr);

	rc = listen(sock, 5);
	if (rc < 0) {
		printf("Listen failed\n");
		exit(1);
	}

	printf("Server started listenting on port %s ...........\n", port);
	// Accept users
	while ((newSock = accept(sock, (struct sockaddr*) &address, &addrLength))) {
		for (i = 0; i <= clientCount; i++) {
			if (Client[i].sockID == 0 || Client[i].sockID == NULL) {
				Client[i].sockID = newSock;
				break;
			}
		}
		Client[i].index = i;

		// Get client username
		Client[i].username = readString(Client[i].sockID);

		// Create thread for each client
		if (pthread_create(&thread[i], NULL, handleNetworking, (void *) &Client[i])){
			printf("Error creating thread for client: %d", Client[i].index);
			exit(1);
		}
		clientCount++;
	}

	if (Client[clientCount].sockID < 0) {
		printf("accept failed");
		exit(1);
	}

	for (i = 0; i < clientCount; i++)
		pthread_join(thread[i], NULL);

	close(sock);
	return 0;
}