#define _BSD_SOURCE
#define NUM_ARGS 2

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
#define MAX_CONNECTIONS 100

struct bankAccount {

	double balance;
	pthread_mutex_t* mutex;
};

struct threadArg {

	int accountNumber;
	double change;
	int clientfd;
	struct bankAccount * account;
};

void threadFunction(void* args) {

	struct threadArg* arg = args;

	// TODO: Alter the specified account;
	struct bankAccount * bank=arg->account;
	int index=arg->accountNumber;
	(bank+index)->balance += arg->change;
	// TODO: Write back to client.
	char buffer[256];
	memset(buffer,'\0',sizeof(buffer));
	sprintf(buffer,"New Balance=%f\n",(bank+index)->balance);
	int b = send(arg->clientfd, (const void*) buffer, sizeof(buffer), NULL);
	// Close client connection.
	close(arg->clientfd);

	//free(args);
}
//function splits your string with specified delimiter
int makeargv(const char *s, const char *delimiters, char ***argvp)
{
// this function splits your string with specified delimiter
    int error;
    int i;
    int numtokens;
    const char *snew;
    char *t;
		char *lasts;

    if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {

        errno = EINVAL;
        return -1;

    }

    *argvp = NULL;              // already assigned as a new var, just blanking out

    snew = s + strspn(s, delimiters);

    if ((t = malloc(strlen(snew) + 1)) == NULL)
        return -1;

    strcpy(t, snew);

    numtokens = 0;

    if (strtok_r(t, delimiters,&lasts) != NULL)  // count number of tokens in s
        for (numtokens = 1; strtok_r(NULL, delimiters,&lasts) != NULL; numtokens++) ;

    // create arg array for pointers to tokens
    if ((*argvp = malloc((numtokens + 1) * sizeof(char *))) == NULL) {
        error = errno;
        free(t);
        errno = error;
        return -1;
    }
    // insert pointers to tokens into the arg array
    if (numtokens == 0)
        free(t);

    else {
        strcpy(t, snew);
        **argvp = strtok_r(t, delimiters,&lasts);
        for (i = 1; i < numtokens; i++)
            *((*argvp) + i) = strtok_r(NULL, delimiters,&lasts);
    }

    *((*argvp) + numtokens) = NULL;     // put in final NULL pointer

    return numtokens;
}

int main(int argc, char** argv) {
	if (argc > NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	// Create a TCP socket.
	int sock = socket(AF_INET , SOCK_STREAM , 0);
	// Bind it to a local address.
	struct sockaddr_in servAddress;
	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(SERVER_PORT);
	servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress));
	// We must now listen on this port.
	listen(sock, MAX_CONNECTIONS);
	// TODO: Init bank accounts.
	int bank_size=atoi(argv[1]);
	struct bankAccount *bank=malloc(bank_size*sizeof(struct bankAccount));

	// TODO: Init threads.
	int thread_num=atoi(argv[2]);
	struct threadArg* p=malloc(thread_num*sizeof(struct threadArg));
	int numtokens;
	pthread_t pool[thread_num];
	char delim[] = ",";
	char **argvp;
	int index;
	int value;
	int i=0;
	// A server typically runs infinitely, with some boolean flag to terminate.
	while (1) {
		// Now accept the incoming connections.
		struct sockaddr_in clientAddress;

		socklen_t size = sizeof(struct sockaddr_in);

		int clientfd = accept(sock, (struct sockaddr*) &clientAddress, &size);
		// TODO: Read request from the socket.
		char buffer[256];
		memset(buffer,'\0',sizeof(buffer));
		recv(clientfd, buffer, sizeof(buffer), NULL);
		//makeargv buffer
		numtokens = makeargv(buffer, delim, &argvp);
		index=atoi(argvp[0]);
		if(i<thread_num){
			(p+i)->accountNumber=index;
			(p+i)->change=atof(argvp[1]);
			(p+i)->clientfd=clientfd;
			(p+i)->account=bank;
			// TODO: Run a thread to handle the request.
			(bank+index)->mutex=(pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
			pthread_mutex_init((bank+index)->mutex,NULL);
			pthread_mutex_lock((bank+index)->mutex);
			pthread_create(&pool[i],NULL,threadFunction,(p+i));
			pthread_join(pool[i],NULL);
			pthread_mutex_unlock((bank+index)->mutex);
			i++;
			i=i%2;
		}
		else{
			printf("Too many concurrent connections\n");
		}
		}
		// Close the socket.
		close(sock);
	}
