#define _BSD_SOURCE
#define NUM_ARGS 3

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 4061

int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	// Create a TCP socket.
	int sock = socket(AF_INET , SOCK_STREAM , 0);

	// Specify an address to connect to (we use the local host or 'loop-back' address).
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(SERVER_PORT);
	address.sin_addr.s_addr = inet_addr(argv[1]);

	// Connect it.
	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) == 0) {

		// TODO: Write data to socket.
		char buffer[256];
		memset(buffer,'\0',sizeof(buffer));
		strcpy(buffer,argv[2]);
		strcat(buffer,",");
		strcat(buffer,argv[3]);
		int b = send(sock, (const void*) buffer, sizeof(buffer), NULL);
		// TODO: Read the data sent back.
		memset(buffer,'\0',sizeof(buffer));
		recv(sock, buffer, sizeof(buffer), NULL);
		printf("%s",buffer);
		// Close the socket.
		close(sock);

	} else {

		perror("Connection failed!");
	}
}
