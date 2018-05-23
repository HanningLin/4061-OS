/*
 * VCforStudents.c
 *
 *  Created on: Feb 2, 2018
 *      Author: ayushi
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "makeargv.h"

#define MAX_NODES 100


//Function signatures

/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Total Allocated Nodes
 * About parseInput: parseInput is supposed to
 * 1) Open the Input File [There is a utility function provided in utility handbook]
 * 2) Read it line by line : Ignore the empty lines [There is a utility function provided in utility handbook]
 * 3) Call parseInputLine(..) on each one of these lines
 ..After all lines are parsed(and the DAG created)
 4) Assign node->"prog" ie, the commands that each of the nodes has to execute
 For Leaf Nodes: ./leafcounter <arguments> is the command to be executed.
 Please refer to the utility handbook for more details.
 For Non-Leaf Nodes, that are not the root node(ie, the node which declares the winner):
 ./aggregate_votes <arguments> is the application to be executed. [Refer utility handbook]
 For the Node which declares the winner:
 This gets run only once, after all other nodes are done executing
 It uses: ./find_winner <arguments> [Refer utility handbook]
 */
int parseInput(char *filename, node_t *n){
 //open the file, point to it with document
 FILE *document;
 document=fopen(filename, "r");//read only
 size_t len = 1024;
 ssize_t read;
 char *buf = malloc(1024*sizeof(char));
 if (buf == NULL) {
  printf("MALLOC FAILURE");
  _exit(1);
 }

 while ((read = getline(&buf, &len, document)) !=-1) {
        printf("%s", buf);
 }
 fclose(document);
 return 5;//why return 5?
}


void buildtree(char *filename,node_t *n){
//use second line of input.txt to bulid the tree
FILE *document;
document=fopen(filename, "r");//read only
size_t len = 1024;
ssize_t read;
char *buf = malloc(1024*sizeof(char));
if (buf == NULL) {
 printf("MALLOC FAILURE");
 _exit(1);
}
int linecounter=0;
while ((read = getline(&buf, &len, document)) != -1) {
  linecounter++;
  if(linecounter==2){
    // split the strings with space to find all node
    char **nodename;
    int num_node;
    num_node=makeargv(buf,' ',&nodename);
    for(int i=0;i<num_node;i++){

      printf("%s\n",*((*nodename)+i));
    }

  }

}
fclose(document);
return 5;//why return 5?
}
/**Function : parseInputLine
 * Arguments: 's' - Line to be parsed
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Region Nodes allocated
 * About parseInputLine: parseInputLine is supposed to
 * 1) Split the Input file [Hint: Use makeargv(..)]
 * 2) Recognize the line containing information of
 * candidates(You can assume this will always be the first line containing data).
 * You may want to store the candidate's information
 * 3) Recognize the line containing "All Nodes"
 * (You can assume this will always be the second line containing data)
 * 4) All the other lines containing data, will show how to connect the nodes together
 * You can choose to do this by having a pointer to other nodes, or in a list etc-
 * */
int parseInputLine(char *s, node_t *n);

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes: parseInputLine is supposed to
 * If the node passed has children, fork and execute them first
 * Please note that processes which are independent of each other
 * can and should be running in a parallel fashion
 * */
void execNodes(node_t *n);


int main(int argc, char **argv){

	//Allocate space for MAX_NODES to node pointer
	struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

	//call parseInput
	int num = buildtree(argv[1], mainnodes);


	//Call execNodes on the root node


	return 0;
}
