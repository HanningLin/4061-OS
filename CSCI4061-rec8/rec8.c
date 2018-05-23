#define _BSD_SOURCE
#define NUM_ARGS 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

struct threadArgs {

	int** A;
	int** B;
	int** C;
	int n;
	int row;

};

// Create a matrix of size n. DO NOT NEED TO CHANGE
int** createMatrix(int n) {

	int** result = (int**) malloc(sizeof(int*) * n);
	for (int i=0; i < n; ++i) result[i] = (int*) malloc(sizeof(int) * n);

	return result;
}

// Fill the matrix with random values. DO NOT NEED TO CHANGE
void fillRandom(int** matrix, int n) {

	for (int i=0; i < n; ++i) {
		for (int j=0; j < n; ++j) {
			matrix[i][j] = rand();
		}
	}
}

// Add the values contianed in a single row of A and B into the same row in C.
void matrixRowAdd(struct threadArgs* arg) {//arg is the structure that pass in, it has the number of the row in it
	for (int i=0;i<(arg->n);i++){
		(arg->C)[arg->row][i]=(arg->A)[arg->row][i]+(arg->B)[arg->row][i];
		//printf("C is %d\nA is %d\nB is %d\n",(arg->C)[arg->row][i],(arg->A)[arg->row][i],(arg->B)[arg->row][i]);
	}

	// TODO: Your code here.
}
void singleThread(struct threadArgs* arg){
	for (int i=0;i<(arg->n);i++){
		matrixRowAdd(&arg[i]);
	}
}
int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}
	srand(time(NULL));
	int n = atoi(argv[1]);
	int i=0;
	// Create argument matrices.
	int** A = createMatrix(n);
	int** B = createMatrix(n);
	int** C = createMatrix(n);

	// Fill args randomly.
	fillRandom(A, n);
	fillRandom(B, n);


	// Perform matrix addition in one thread.

	struct threadArgs args[n];
	for(i=0;i<n;i++){
		args[i].A = A;
		args[i].B = B;
		args[i].C = C;
		args[i].n = n;
		args[i].row = i;
	}
	long t1 = time(NULL);
	// TODO: Your code here.
	pthread_t threadSingle;
	int ret_thrdSingle;
	int tmpSingle	;
	void *retvalSingle;
	ret_thrdSingle=pthread_create(&threadSingle,NULL,(void *)&singleThread,&args);
	tmpSingle = pthread_join(threadSingle, &retvalSingle);
	long t2 = time(NULL);
	printf("Time (in sec) to preform single thread add = %d\n", (t2 - t1));

	// Perform matrix addition with multiple (n) threads.
	t1 = time(NULL);
	pthread_t thread[n];
	int ret_thrd[n];
	int tmp[n];
	void *retval;
for(i=0;i<n;i++){
	ret_thrd[i]=pthread_create(&thread[i],NULL,(void *)&matrixRowAdd,&args[i]);
	if (ret_thrd[i]!=0){
		fprintf(stderr, "failed to create thread!\n");
		exit(1);
	}
}
for(i=0;i<n;i++){
	tmp[i] = pthread_join(thread[i], &retval);
}

	// TODO: Your code here.
	//multithread goes here
        t2 = time(NULL);
        printf("Time (in sec) to perform multithreaded add = %d\n", (t2 - t1));
}
