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
//Create a new node in the DAF
node *create(char *name, node * next, node * prev, int count)
{
    //Use malloc to allocate space on the heap
    node *new_node = (node *) malloc(sizeof(node));
    if (new_node == NULL) {
        printf("Error creating a new node.\n");
        _exit(0);
    }
    //set the vote, name, pointer to next node and previous node
    new_node->count = count;
    strcpy(new_node->name, name);
    new_node->next = next;
    new_node->prev = prev;
    return new_node;
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
//Create the path of the output file
//Input path of a directory and output path of the output file
void getFilePath(char *path, char *newpath)
{
    char delim[] = "/";
    int num;
    int totalnum = strlen(path);
    char **argvp;
    if ((num = makeargv(path, delim, &argvp)) == -1) {
        fprintf(stderr, "failed to parse the string you given\n");
        return 1;
    }
    //add "/<name of the directory>.txt" to the path
    strcpy(newpath, path);
    strcat(newpath, "/");
    strcat(newpath, argvp[num - 1]);
    strcat(newpath, ".txt");
    return 0;
}
//function of Aggregate_Votes
void aggr(char *path)
{
    FILE *wr_document;//file descriptor of writing document
    FILE *wr_document_sym;//file descriptor of writing document, especially for symbolic link
    FILE *rd_document;//file descriptor of reading document
    int num;
    int num2;
    char filename[MAX_STR_SIZE];
    char rd_filename[MAX_STR_SIZE];
    int i;
    char argtemp[MAX_STR_SIZE];//temp string
    char argtemp2[MAX_STR_SIZE];//temp string
    char stroutput[MAX_STR_SIZE];//output
    char path_subdir[MAX_STR_SIZE]; //path of sub directory
    char path_symlink[MAX_STR_SIZE];//path used especially for sub directory
    char writepath_symlink[MAX_STR_SIZE];
    memset(writepath_symlink, '\0', sizeof(writepath_symlink));//initial the string with \0
    char tmp_symlink[MAX_STR_SIZE];
    memset(tmp_symlink, '\0', sizeof(writepath_symlink));//initial the string with \0
    char buf_symlink[MAX_STR_SIZE];
    memset(buf_symlink, '\0', sizeof(buf_symlink));//initial the string with \0
    char pointto_symlink[MAX_STR_SIZE];
    char **argvp;//used in makeargv
    char **argvp_symlink;
    char **argvp2_symlink;
    char **name_and_count;
    char *buf = malloc(MAX_STR_SIZE * sizeof(char));
    int flag;

    if (buf == NULL) {
        perror("MALLOC FAILURE\n");
        _exit(1);
    }
    DIR *dir;
    struct dirent *entry;
//open directory
    if (!(dir = opendir(path))) {
        printf("Current directory open failed!\n");
        perror(NULL);
        _exit(1);
    }
    node *tmp_node = root;
    int nodenum = 0;            //the total number of nodes
    getFilePath(path, filename);//get the path of output file
    printf("%s\n", filename);   //required output
    remove(filename);//remove the exist file
    //Traversal all the entry in the directory
    while ((entry = readdir(dir)) != NULL) {
      //if the entry is a symbolic link
        if (entry->d_type == DT_LNK) {
            if ((num = makeargv(path, "/", &argvp_symlink)) == -1) {
                perror(NULL);
                return 1;
            }
            strcpy(path_symlink, path);
            strcat(path_symlink, "/");
            strcat(path_symlink, entry->d_name);
            readlink(path_symlink, buf_symlink, sizeof(buf_symlink));//get the path that the symbolic link point to
            if ((num2 = makeargv(buf_symlink, "/", &argvp2_symlink)) == -1) {
                perror(NULL);
                return 1;
            }
            strcpy(pointto_symlink, argvp2_symlink[num2 - 1]);//pointto_symlink is the name of the directory the symlink point to
            flag = 0;
            for (i = 0; i < num; i++) {
                if (strcmp(pointto_symlink, argvp_symlink[i]) == 0) {//if there is a loop
                    sprintf(stroutput, "There is a cycle from %s to %s.\n", argvp_symlink[num - 1], pointto_symlink);
                }
                // find the path of Who_Won, used later to write "There is a loop......"
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
            //open the Who_Won file and write "There is a loop......"
            wr_document_sym = fopen(writepath_symlink, "a");
            if (wr_document_sym == NULL)
            {
                printf("File open and create failed!\n");
                perror(NULL);
                _exit(1);
            }
            fwrite(stroutput, 1, strlen(stroutput), wr_document_sym);
            fclose(wr_document_sym);
            continue;
        }
        if (entry->d_type == DT_DIR) {  //if the entry is a directory
            //ignore the "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            size_t len = MAX_STR_SIZE;
            ssize_t read;
            //create the path of the output file in the subdirectory
            strcpy(rd_filename, path);
            strcat(rd_filename, "/");
            strcat(rd_filename, entry->d_name);
            strcpy(path_subdir, rd_filename);   //path_subdir is the path of subdirectory used to check wheather is a leaf or not.
            strcat(rd_filename, "/");
            strcat(rd_filename, entry->d_name);
            strcat(rd_filename, ".txt\0");

            pid_t p1 = fork();
            if (p1 < 0) {
                fprintf(stderr, "Error: Unable to fork.\n");
                exit(EXIT_FAILURE);
            }
            if (p1 == 0) {
                //this is a child process
                //block the stdout of chlld process
                int devNull = open("/dev/null", O_WRONLY);
                   if(devNull == -1){
                   fprintf(stderr,"Error in open('/dev/null',0)\n");
                   exit(EXIT_FAILURE);
                   }
                   if(dup2(devNull, STDOUT_FILENO) == -1) {
                   fprintf(stderr,"Error in dup2(devNull, STDOUT_FILENO)\n");
                   exit(EXIT_FAILURE);
                   }
                if (has_sub_dir(path_subdir)) { //if sub directory is a leaf node, then it will execute the Leaf_Counter program
                    execl("./Aggregate_Votes", "./Aggregate_Votes", path_subdir, (char *)NULL);

                } else {        //if sub directory is not a leaf node
                    execl("./Leaf_Counter", "Leaf_Counter", path_subdir, (char *)NULL);
                }
            }
            //read file and extract parent process
            else {              // this is parent process
                wait(p1);       //wait for the child process to end

                // creat and open the result txt file

                wr_document = fopen(filename, "a");     //open output file, wr_document used to write the result
                if (wr_document == NULL)
                {
                    printf("File open and create failed!\n");
                    perror(NULL);
                    _exit(1);
                }
                rd_document = fopen(rd_filename, "r"); //open reading file, rd_document used to read the result of it's sub directory
                if(rd_document==NULL)
                {

                    printf("File:%s open failed!\n", rd_filename);
                    perror(NULL);
                    _exit(1);
                }
                while ((read = getline(&buf, &len, rd_document)) != -1) { //read reading file line by line.
                    //split the reading file
                    if ((num = makeargv(buf, ",", &argvp)) == -1) {
                        perror(NULL);
                        return 1;
                    }
                    for (i = 0; i < num; i++) {
                        strcpy(argtemp, argvp[i]);
                        //find the name and count number
                        if ((num2 = makeargv(argtemp, ":", &name_and_count)) == -1) {
                            perror(NULL);
                            return 1;
                        }
                        if (num2 != 2) {
                            printf("Find name and count failed!\n");
                            exit(1);
                        }
                        // make node
                        flag = 0;       //used to know wheather there is the node already exsit
                        tmp_node = root;
                        while (tmp_node->next != NULL) {
                            tmp_node = tmp_node->next;
                            if (strcmp(tmp_node->name, name_and_count[0]) == 0) {//if node already exist
                                tmp_node->count = tmp_node->count + atoi(name_and_count[1]);//change the count
                                flag = 1;
                                break;
                            }
                        }
                        if (flag == 0) {//if node doesn't exist, make a new node
                            tmp_node->next = create(name_and_count[0], NULL, tmp_node, atoi(name_and_count[1]));
                        }
                    }
                    break;      //only read one line
                }
            }

        }
    }
    //write the reasult to output file
    tmp_node = root;
    tmp_node = tmp_node->next;
    strcpy(stroutput, tmp_node->name);
    strcat(stroutput, ":");
    sprintf(argtemp, "%d", tmp_node->count);
    strcat(stroutput, argtemp);
    fwrite(stroutput, 1, strlen(stroutput), wr_document);
    while (tmp_node->next != NULL) {
        tmp_node = tmp_node->next;
        strcpy(stroutput, ",");
        strcat(stroutput, tmp_node->name);
        strcat(stroutput, ":");
        sprintf(argtemp, "%d", tmp_node->count);
        strcat(stroutput, argtemp);
        fwrite(stroutput, 1, strlen(stroutput), wr_document);
    }
    fwrite("\n", 1, 1, wr_document);
    free(buf);//free the heap
    if (wr_document != NULL) {
        fclose(wr_document);
    }
    if (rd_document != NULL) {
        fclose(rd_document);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Wrong number of args, expected 1, given %d\n", argc - 1);
        exit(1);
    }
    root = create("ROOT_NODE", NULL, NULL, 0);//create the ROOT_NODE
    node *tmp_node = root;
    if (!(has_sub_dir(argv[1]))) {      //if we call the program on a leaf node, then it will execute the Leaf_Counter program
        execl("./Leaf_Counter", "Leaf_Counter", argv[1], (char *)NULL);
        //execute leafcounter here waiting for leafcounter to be done
    } else {
        aggr(argv[1]);
    }
    //free all the node in the
    while (tmp_node->next != NULL) {
        tmp_node = tmp_node->next;
    }
    node *freenode = tmp_node;
    while (tmp_node->prev != NULL) {
        freenode = tmp_node;
        tmp_node = tmp_node->prev;
        free(freenode);
    }
    free(root);
    return 0;
}
