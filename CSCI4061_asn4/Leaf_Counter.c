/*login: tessm061 lin00116
* date: 03/09/18
* name: Tyler Tessmann, Hanning Lin */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_STR_SIZE 1024
#define DT_DIR 4
#define DT_LNK 10

//A simple node structure
typedef struct node {
    int count;                  //Number of votes for the canidate
    char name[MAX_STR_SIZE];    //Name of canidate
    struct node *next;          //Next canidate node
    struct node *prev;          // previous canidate node
} node;                         //declare node public

node *root;                     // make root node public, so all functions can enter DAG

//Parses the given full path
//See documentation in AGGR votes
void getFilePath(char *path, char *newpath)
{
    char delim[] = "/";
    int num;
    int totalnum = strlen(path);
    char **argvp;
    char **name_and_count;
    if ((num = makeargv(path, delim, &argvp)) == -1) {
        fprintf(stderr, "failed to parse the string you given\n");
        return 1;
    }
    strcpy(newpath, path);
    strcat(newpath, "/");
    strcat(newpath, argvp[num - 1]);
    strcat(newpath, ".txt");
    return 0;
}

//Create a new node in the DAF
node *create(char *name, node * next, node * prev)
{
    //Use malloc to allocate space on the heap
    node *new_node = (node *) malloc(sizeof(node));
    if (new_node == NULL) {
        fprintf(stderr, "Error creating a new node.\n");
        _exit(0);
    }
    //Give it one vote
    new_node->count = 1;
    //set the name
    strcpy(new_node->name, name);
    new_node->next = next;
    new_node->prev = prev;
    // printf("Made a new node called %s\n",name);
    return new_node;
}

//Navigate the dag, and print out each name
//and count in the predefined format
void print_results(char *path, char *name)
{
    char filename[MAX_STR_SIZE];
    getFilePath(path, filename);
    printf("%s\n", filename);
    FILE *fp;
    fp = fopen(filename, "w");  //OPEN with write permissions
    if (fp == NULL) {
        perror("Bad File Path");
    }
    chmod(filename, 0700);
    int i = 0;
    char tmp[MAX_STR_SIZE];
    char output[MAX_STR_SIZE] = "";
    //char* output;
    node *cur_node = root;
    if (cur_node->next != NULL) {
        cur_node = cur_node->next;
        strcat(output, cur_node->name);
        strcat(output, ":");
        sprintf(tmp, "%d", cur_node->count);
        strcat(output, tmp);
        i++;
    }
    while (cur_node->next != NULL) {
        cur_node = cur_node->next;
        strcat(output, ",");
        strcat(output, cur_node->name);
        strcat(output, ":");
        sprintf(tmp, "%d", cur_node->count);
        strcat(output, tmp);
        i++;
    }
    strcat(output,"\n");
    fprintf(fp, "%s", output);
    int stat = fclose(fp);      //Close the file, flushes output
    if (stat != 0) {
        perror("Bad File Close");
    }

}

void canidate_counter(char *vote)
{
    node *tmp_node = root;
    //while the node isnt the right node, and while there are more to search
    while (strcmp(vote, tmp_node->name) && tmp_node->next != NULL) {
        tmp_node = tmp_node->next;
    }
    if (strcmp(vote, tmp_node->name) == 0) {    //if the current node is correct
        strcpy(tmp_node->name, vote);
        tmp_node->count = tmp_node->count + 1;
    } else {                    //no node exists, so make one and link it in
        tmp_node->next = create(vote, NULL, tmp_node);
    }
}

//check if this directory has a sub_directory
//Just iterates through the dir, looking for types
int has_sub_dir(char *path)
{
    DIR *dir = opendir(path);
    struct dirent *ent;
    int i = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;
        if (ent->d_type == DT_DIR) {
            return 1;
        }
    }
    return 0;
}

//Loop finder is same as Agg Votes
//Look there for documentation
void find_loop(char *path)
{
    DIR *dir;
    FILE *wr_document;
    int i;
    int flag;
    int num;
    int num2;
    struct dirent *entry;
    char **argvp_symlink;
    char **argvp2_symlink;
    char path_symlink[MAX_STR_SIZE];
    char writepath_symlink[MAX_STR_SIZE];
    memset(writepath_symlink, '\0', sizeof(writepath_symlink));
    char tmp_symlink[MAX_STR_SIZE];
    memset(tmp_symlink, '\0', sizeof(writepath_symlink));
    char buf_symlink[MAX_STR_SIZE];
    memset(buf_symlink, '\0', sizeof(buf_symlink));
    char stroutput[MAX_STR_SIZE];
    char pointto_symlink[MAX_STR_SIZE];
    if (!(dir = opendir(path))) {
        printf("Current directory open failed!\n");
        perror(NULL);
        _exit(1);
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_LNK) {
            if ((num = makeargv(path, "/", &argvp_symlink)) == -1) {
                perror(NULL);
                return 1;
            }
            strcpy(path_symlink, path);
            strcat(path_symlink, "/");
            strcat(path_symlink, entry->d_name);
            readlink(path_symlink, buf_symlink, sizeof(buf_symlink));
            if ((num2 = makeargv(buf_symlink, "/", &argvp2_symlink)) == -1) {
                perror(NULL);
                return 1;
            }
            strcpy(pointto_symlink, argvp2_symlink[num2 - 1]);
            flag = 0;
            for (i = 0; i < num; i++) {
                if (strcmp(pointto_symlink, argvp_symlink[i]) == 0) {
                    sprintf(stroutput, "There is a cycle from %s to %s.\n", argvp_symlink[num - 1], pointto_symlink);
                }
                if (strcmp(argvp_symlink[i], "Who_Won") == 0) {
                    for (int j = 0; j < i + 1; j++) {
                        strcat(writepath_symlink, argvp_symlink[j]);
                        strcat(writepath_symlink, "/");
                    }
                    strcat(writepath_symlink, "Who_Won.txt");
                    if (strcmp(argvp_symlink[0], "home") == 0) {
                        strcpy(tmp_symlink, "/");
                        strcat(tmp_symlink, writepath_symlink);
                        strcpy(writepath_symlink, tmp_symlink);
                    }
                }
            }
            wr_document = fopen(writepath_symlink, "a");
            if (wr_document == NULL)    //read and write
            {
                printf("File open and create failed!\n");
                perror(NULL);
                _exit(1);
            }
            fwrite(stroutput, 1, strlen(stroutput), wr_document);
            fclose(wr_document);
            continue;
        }
    }
}

//initial code for trying to open file
void file_opener(char *path)
{
    FILE *document;
    char filename[MAX_STR_SIZE];
    strcpy(filename, path);
//    strcat(filename, "County_1.votes");
//    if (has_sub_dir(path)) {    //if it has a sub_dir, it isnt a leaf
//        printf("Not a leaf node.\n");
//        _exit(1);
   // }
    //if there is no votes.txt, not a leaf
    if (access(filename, R_OK) == -1) { //Try to access with READ permissions
        printf("Not a leaf node.\n");
        _exit(1);
    }
    //sort the input file
    // printf("filename is:%s\n",filename);
    pid_t pid = fork();
    if (pid == 0) {
        execlp("/usr/bin/sort", "sort", "-o", filename, filename, (char *)NULL);
        perror("EXECLP to sort failed");
    } else if (pid > 0)
        wait(pid);
    else
        perror(NULL);
    //open doc
    document = fopen(filename, "r");    //read only

    if (document == NULL) {
        perror(NULL);
        printf("votes.txt open failed!\n");
        exit(1);
    }

    size_t len = MAX_STR_SIZE;
    ssize_t read;
    char *buf = malloc(MAX_STR_SIZE * sizeof(char));    //set aside a chunk of memory for the buffer
    if (buf == NULL) {
        perror("MALLOC FAILURE\n");
        _exit(1);
    }
    int linecounter = 0;        //keep a running count of what line we are on
    while ((read = getline(&buf, &len, document)) != -1) {
        if (sizeof(trimwhitespace(buf)) != 0)   //make sure it isnt blank
            canidate_counter(trimwhitespace(buf));
    }
    free(buf);                  //prevent mem leaks
    char name[MAX_STR_SIZE];
    char **argvp;
    makeargv(path, "/", &argvp);
    int i = 0;
    while (argvp[i] != 0) {
        strcpy(name, argvp[i]);
        i++;
    }
    print_results(path, name);
}

//Function to traverse the nodes,
//freeing all of the nodes
void free_nodes(node * root)
{
    while (root->next != NULL) {
        root = root->next;
        free(root->prev);

    }
    free(root);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Wrong number of args, expected 1, given %d", argc - 1);
        exit(1);
    }
    //find_loop(argv[1]);
    root = create("ROOT_NODE", NULL, NULL);
    file_opener(argv[1]);
    free_nodes(root);
}
