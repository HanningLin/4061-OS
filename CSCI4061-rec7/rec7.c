#define _BSD_SOURCE
#define NUM_ARGS 2

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

struct matrix {

	int x;
	int y;
	int** data;
};

struct matrix* newMatrix(int x, int y) {

	//int data[x][y];//stack lost
	int **data=(int **)malloc(sizeof(int*)*x);
	for(int i=0;i<x;i++)
		data[i]=(int*)malloc(y*sizeof(int));
	struct matrix*m=(struct matrix*)malloc(sizeof(struct matrix));
	m->x=x;
	m->y=y;
	m->data=data;
	//m={x,y,data};// stack lost
	return m;
	//!!!need to be free in main!!!
}

void diagAdd(struct matrix* m, int c) {

	int d = (rand() % m->y);
	for (int i=0; (d+i < m->y)&(i<m->x); ++i) {
		m->data[i][d+i] += c;
	}
}

struct matrix* matrixMultiply(struct matrix** matrices, int start, int end) {

	//struct matrix* mat = {matrices[start]->x, matrices[start]->y, matrices[start]->data};
	struct matrix* mat= (struct matrix*)malloc(sizeof(struct matrix));
	mat->x=matrices[start]->x;
	mat->y=matrices[start]->y;
	mat->data=matrices[start]->data;
	for (int i=start+1; i < end; ++i) {
		for (int j=0; j < matrices[i]->x; ++j) {
			for (int k=0; k < matrices[i]->y; ++k) {
				for (int l=0; l < matrices[i]->x; ++l) {
					mat->data[j][k] += mat->data[j][l] * matrices[i]->data[l][k];
				}
			}
		}
	}

	return mat;
}

int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	int max = atoi(argv[1]);
	int duration = atoi(argv[2]);
	int start = time(NULL);

	srand(time(NULL));

	while (time(NULL) < start + duration) {

		struct matrix* matrices[max];
		for (int i=0; i < max; ++i) {

			// Initialize the matrices.
			matrices[i] = newMatrix(256, 256);
			for (int j=0; j < 10; ++j) diagAdd(matrices[i], (int)(rand() % 256));
		}

		struct matrix* m = matrixMultiply(matrices, 0, max);

		printf("First element in matrix = %d\n", m->data[0][0]);
		//free
		for(int i=0;i<max;i++)
		{
			for(int j=0;j<256;j++){
				free(matrices[i]->data[j]);
			}
			free(matrices[i]->data);
			free(matrices[i]);
		}
		free(m);
		}


}
