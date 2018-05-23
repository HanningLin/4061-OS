/*login: tessm061 lin00116
* date: 02/18/18
* name: Tyler Tessmann, Hanning Lin */

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
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_NODES 50
#define MAX_STR_SIZE 1024
#define MAX_CANS 20

char num_can[MAX_STR_SIZE];
const char *visited[MAX_NODES];
int j = 1;
char **can_names;

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
void parseInput(char *filename, node_t * n, int *numofnodes)
{
    //open the file, point to it with document
    FILE *document;
    if (access(filename, R_OK) == -1) { //Try to access with READ permissions
        printf("Cannot find or read input file!!!\n");
        _exit(1);
    }
    document = fopen(filename, "r");    //read only
    perror(NULL);
    size_t len = MAX_STR_SIZE;
    ssize_t read;
    char *buf = malloc(MAX_STR_SIZE * sizeof(char));    //set aside a chunk of memory for the buffer
    if (buf == NULL) {
        perror("MALLOC FAILURE\n");
        _exit(1);
    }
    int linecounter = 0;        //keep a running count of what line we are on
    while ((read = getline(&buf, &len, document)) != -1) {
        if (buf[0] == '#') {    //skip comment lines
            printf("Skipping comment %s \n", buf);
        } else {
            *numofnodes = parseInputLine(buf, linecounter, n);  //parse each line, getting the number of nodes
            linecounter++;
        }
    }

    if (*numofnodes < 2) {      //if we only have who_won, we have no voting districts
        printf("No voting Regions\n");
        _exit(1);
    }
    visited[0] = "Who_Won";
    if (findDAG_Loop()) {       //Extra Credit, if there is a loop, quit!
        printf("There is a loop! \n");
        _exit(1);
    }
    for (int i = 0; i < *numofnodes; i++) {
        n[i].num_descendant = n[i].num_children;        //init num_descendant. will be changed by funtion findRoot()
    }
    int status = fclose(document);           //close the document, we are done with it
    if(status != 0){
	perror("fclose ERROR");
    }
    free(buf);                  //also free the buffer from memory
    perror(NULL);
}

/**Function: makeNodes
*Argument 'numtokens' -number of the nodes
*node is  the address of the first nodes of the node list which contain all the nodes
**/
void makeNodes(int numtokens, node_t * node)
{
    int i;
    for (i = 0; i < numtokens; i++) {
        strcpy(node[i].name, "No name now");
        node[i].num_children = 0;
        node[i].status = 0;
        node[i].num_descendant = 0;//init the node
        printf("Setting children to 0 \n");
    }
}

/**Funtion: changeNodeName
*Change the name of the empty node list that was created by funtion makeNodes
**/
void changeNodeName(node_t * node, int i, char *name_i)
{
    strcpy(node[i].name, trimwhitespace(name_i));// change the name of empty node
}

/**Funtion: creatTree
*change node.children, node.num_children
**/
void creatTree(node_t * node, char *fathername, char *childname, int numofnodes)
{
    int i;
    for (i = 0; i < numofnodes; i++) {
        if (!strcmp(node[i].name, fathername)) {
            for (int j = 0; j < numofnodes; j++) {
                if (!strcmp(node[j].name, childname)) {
                    node[i].children[node[i].num_children] = j;//add children to mother node
                    node[i].num_children++;//add the number of child parent nodes have
                    break;
                }
            }
            break;
        }
    }
}

/**Function:NumOfDescendant
* num is the num of node that calculate
*flag is used to know weather all the child is caculated
*This funtion is used to find how many descendant that a node have
**/
int NumOfDescendant(node_t * node, int num)     //used to calculate num_descendant of each node
{
    int i;                      //counter
    for (i = 0; i < node[num].num_children; i++) {
        if (!node[node[num].children[i]].status)        //child's num_descendant has not been caculated
        {
            node[num].num_descendant = node[num].num_descendant + NumOfDescendant(node, node[num].children[i]);
        } else                  //child's num_descendant has not been caculated
        {
            node[num].num_descendant = node[num].num_descendant + node[node[num].children[i]].num_descendant;
        }
    }
    node[num].status = 1;
    return node[num].num_descendant;
}
/**Function:FindRoot
*This funtion is used to find the sequence number of the root node in the array of nodes
**/
int findRoot(node_t * node, int numofnodes)
{
    int num_of_parent = -1;
    int max_num_of_descendent = 0;
    int n;
    for (int i = 0; i < numofnodes; i++) {
        n = NumOfDescendant(node, i);
        if (n > max_num_of_descendent) {// the node that have the most number of descendants is the root node
            max_num_of_descendent = n;
            num_of_parent = i;
        }
    }
    return num_of_parent;
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

int parseInputLine(char *buf, int linecounter, node_t * mainnode)
{
    char delim[] = " \t";
    int i;                      //the current number of lines
    char **argvp;
    int numtokens;
    char *text = buf;
    int numofnodes;
    if ((numtokens = makeargv(text, delim, &argvp)) == -1) {
        fprintf(stderr, "failed to parse the string you given:%s\n", text);
        return 1;
    }
    if (linecounter == 0) {     //this is the first line of input
        if (isdigit(text[0])) { //if it isnt a number, something is wrong.
            strcpy(num_can, argvp[0]);  //Set the number of canidates to our global varible
            char buffer[MAX_STR_SIZE] = "";
            for (i = 1; i < numtokens; i++) {
                char tmp[MAX_STR_SIZE];
                strcpy(tmp, argvp[i]);
                strcat(tmp, " ");
                strcat(buffer, tmp);
            }
            makeargv(buffer, " ", &can_names);  //make and array of canidate names
        } else {
            printf("error, input not formatted right\n");
            _exit(1);
        }

    } else if (linecounter == 1) {      //if it is the second line of input
        if (1)                  //add more file checking later if needed, no issues found as of now
        {
            numofnodes = numtokens;
            makeNodes(numofnodes, mainnode);
            for (i = 0; i < numtokens; i++) {
                changeNodeName(mainnode, i, argvp[i]);
            }
        } else {
            printf("error, input not formatted right\n");
            _exit(1);
        }
    } else {                    //attach children
        for (i = 2; i < numtokens; i++) {
            visited[j] = argvp[i];
            j++;
            creatTree(mainnode, trimwhitespace(argvp[0]), trimwhitespace(argvp[i]), numofnodes);
        }
    }
    return numofnodes;
}

/**Function : execNodes
 * */
void execNodes(int root_id, node_t * firstnode)
{
    int i = 0;
    while (firstnode[root_id].num_children > i) {       //while the node we are calling has more children
        int id = firstnode[root_id].children[i];
        if (firstnode[id].num_children == 0) {  //If the child has no children, it is a leaf node
            pid_t pid = fork(); //fork and attempt to use leafcounter
            if (pid < 0) {      //pid is only <0 when errors happen
                perror("ERROR: Fork Failure\n");
                _exit(1);
            } else if (pid > 0) {       //PID is >0 for parents
                //wait(NULL);
		int status;
		if(wait(&status) == -1){
		perror("Child has failed!");	
		_exit(1);
		}
            } else {            //PID is 0, meaning it is a child
                char inputF[MAX_STR_SIZE];      //start constructing the array to feed into execvp
                char inputFtmp[MAX_STR_SIZE];
                strcpy(inputF, firstnode[id].name);
                strcpy(inputFtmp, inputF);
                char outputF[MAX_STR_SIZE];
                strcpy(outputF, prepend(inputFtmp, "Output_"));
                char *argarr[MAX_CANS + 5];     // = { "leafcounter", inputF, outputF, "4", "A", "B", "C", "D", (char *)NULL };
                argarr[0] = "leafcounter";
                argarr[1] = inputF;
                argarr[2] = outputF;
                argarr[3] = num_can;
                int j = 0;
                while (can_names[j] != NULL) {
                    argarr[4 + j] = can_names[j];
                    j++;
                }
                execvp("./leafcounter", argarr);        //Call to exec, child will die after it finishes.
                perror("execvp failed");        //This only exucutes if execvp fails
            }
        } else {                //This is when the child node has more children
            pid_t pida = fork();        //fork, and have the child go through execNodes
            if (pida > 0) {     //if it is a parent, wait
                //wait(NULL);
		int status;
		if(wait(&status) == -1){
                perror("Child has failed!");
                _exit(1);
                }
            } else if (pida < 0) {
                perror("ERROR: Fork Failure\n");
                _exit(1);
            } else {            // have the child execNodes
                execNodes(id, firstnode);
            }
        }
        i++;                    //go to next child
    }
    i = 0;                      //reset i for future use
    char *argarr[MAX_NODES];    //start making array to aggregate votes
    char str[MAX_STR_SIZE];
    sprintf(str, "%d", firstnode[root_id].num_children);
    argarr[0] = "aggregate_votes";
    argarr[1] = str;
    while (firstnode[root_id].num_children > i) {       //for each child, make another input file to feed into the aggregate_votes program
        int id = firstnode[root_id].children[i];
        printf("Outfile name %s \n", firstnode[id].name);
        char inputF[MAX_STR_SIZE];
        char inputFtmp[MAX_STR_SIZE];
        strcpy(inputF, firstnode[id].name);
        strcpy(inputFtmp, inputF);
        char outputF[MAX_STR_SIZE];
        char add[MAX_STR_SIZE];
        strcpy(add, "Output_");
        strcat(add, inputFtmp);
        argarr[i + 2] = prepend(firstnode[id].name, "Output_");
        printf("Adding %s into array at %d\n", argarr[i + 1], (i + 1));
        i++;
    }
    i--;
    char inputFi[MAX_STR_SIZE];
    char inputFtmpi[MAX_STR_SIZE];
    strcpy(inputFi, firstnode[root_id].name);
    strcpy(inputFtmpi, inputFi);
    char outputFi[MAX_STR_SIZE];
    char adda[MAX_STR_SIZE];
    strcpy(adda, "Output_");
    strcat(adda, inputFtmpi);
    argarr[i + 3] = adda;       //add in the output file
    argarr[i + 4] = num_can;    //add in the number of canidates
    int j = 0;
    char tmp[MAX_STR_SIZE];
    while (can_names[j] != NULL) {      //add in the canidate names
        argarr[i + 5 + j] = can_names[j];
        j++;
    }
    argarr[i + j + 5] = NULL;   //NULL terminate it
    if (!strcmp(argarr[i + 3], "Output_Who_Won")) {     //if the node is called Who_Won, then call who_won instead of aggregate_votes
        execvp("./find_winner", argarr);
    } else {                    //if it isnt the final node, just do the aggregate_votes
        execvp("./aggregate_votes", argarr);
    }
    perror("execvp failed because ");   //Either one exucutes, but it only gets here if an execvp call fails
}
/**Function:findDAG_Loop
*this is the function to find a cycle in the DAG
*If there are nodes that have a child which is also their ancestor,
*there is a loop in the tree.
**/
int findDAG_Loop()//extra credit
{
    int i = 0;
    while (i < j) {
        int ii = 0;
        while (ii < j) {
            if (!strcmp(visited[i], visited[ii]) && i != ii) {
                return 1;
            }
            ii++;
        }
        i++;
    }
    printf("No loop \n");
    return 0;
}

int main(int argc, char **argv)
{
    //Allocate space for MAX_NODES to node pointer
    struct node *firstnode = (struct node *)malloc(sizeof(struct node) * MAX_NODES);
    if (argc != 2) {            //if there isnt 2 args, fail now and show proper usage
        printf("Usage: %s Program\n", argv[0]);
        printf("Try ./%s input.txt\n", argv[0]);
        return -1;              //exit in failure
    }
    int numofnodes;
    parseInput(argv[1], firstnode, &numofnodes);        //store the number of nodes in numofnodes
    int num_of_root = findRoot(firstnode, numofnodes);  //get the ID of the root node
    execNodes(num_of_root, firstnode);  //start exucuting at root node
    free(firstnode);            //all done, free the DAG
    return 0;
}
