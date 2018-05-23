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

int max = 0;
char winner[MAX_STR_SIZE];

//See leaf counter
void parse_top(char *can_data)
{
    char delim[] = ":";
    int num;
    int totalnum = strlen(can_data);
    char **argvp;
    ////printf("path is %s\ntotalnum is %d\n",path,totalnum);
    if ((num = makeargv(can_data, delim, &argvp)) == -1) {
        fprintf(stderr, "failed to parse the string you given\n");
    }
    if (atoi(argvp[1]) > max) { //if this canidate has more the the prev max
        max = atoi(argvp[1]);   //set the max equal to this amount
        strcpy(winner, argvp[0]);       //Save this guy as the front runner
    }
}

//finds the winner by breaking apart
//the output of aggr votes
void find_winner(char *line)
{
    char delim[] = ",";
    int num;
    int totalnum = strlen(line);
    char **argvp;
    ////printf("path is %s\ntotalnum is %d\n",path,totalnum);
    if ((num = makeargv(line, delim, &argvp)) == -1) {
        fprintf(stderr, "failed to parse the string you given\n");
    }
    //printf("Num is %d\n",num);
    int i = 0;
    while (i < num) {
        parse_top(argvp[i]);
        i++;
    }
}

//Wrapper for find_winner
//makes sure we dont try to decipher cycle text
void parse_line(char *line)
{
    if (!strstr(line, "There is a cycle") != NULL) {    //looking for a substring
        find_winner(line);
    }

}

//Read the aggr vote file, and get ready to parse
void read_file(char *path, char *old_path)
{
    FILE *document;
    document = fopen(path, "rw");

    if (document == NULL) {
        fprintf(stderr, "%s open failed!\n", path);
        _exit(1);
    }

    size_t len = MAX_STR_SIZE;
    ssize_t read;
    char *buf = malloc(MAX_STR_SIZE * sizeof(char));    //set aside a chunk of memory for the buffer
    if (buf == NULL) {
        perror("MALLOC FAILURE\n");
        _exit(1);
    }
    int not_who = 0;
    FILE *out;
    if (!strstr(path, "Who_Won/Who_Won.txt") != NULL) {
        not_who = 1;
        out = fopen(old_path, "w+");
        if (out == NULL) {
            perror("Bad File Path");
        }

    }
    //for each line in the file, parse it
    while ((read = getline(&buf, &len, document)) != -1) {
        if (not_who) {
            //if we arent in the root, copy it over to who_won
            fprintf(out, "%s\n", trimwhitespace(buf));
        }
        parse_line(trimwhitespace(buf));
    }
    //close both files
    fclose(document);
    if (not_who) {
        fclose(out);
    }
}

//See aggr votes
void getFilePath(char *path, char *newpath)
{
    char delim[] = "/";
    int num;
    int totalnum = strlen(path);
    char **argvp;
    char **name_and_count;
    //printf("path is %s\ntotalnum is %d\n",path,totalnum);
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

//wrapper function for aggr votes
char *run_aggr(char *path)
{
    pid_t pid = fork();         //fork because we need to exec
    char *output_path = malloc(MAX_STR_SIZE);;
    strcpy(output_path, path);
    strcat(output_path, "/Who_Won.txt");

    if (pid == 0) {
        //int file = open(output_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
        close(1);               //dont print on stdout
        execl("./Aggregate_Votes", "./Aggregate_Votes", path, (char *)NULL);
    }
    int status;
    if (wait(&status) == -1) {
        perror("Child has failed!");
        _exit(1);
    }                           //wait for the child
    //printf("Output path is: %s\n",output_path);
    return output_path;
}

int main(int argc, char **argv)
{
    char paths[MAX_STR_SIZE];
    memset(paths, '\0', sizeof(paths));
    if (argc == 1) {
        //strcpy(paths,"Who_Won");
        getcwd(paths, sizeof(paths));
        //printf("paths is %s\n",paths);
    } else if (argc != 2) {
        //printf("Wrong number of args, expected 1, given %d\n", argc - 1);
        exit(1);
    } else {
        strcpy(paths, argv[1]);
    }
    //node* tmp_node = root;
    char path[MAX_STR_SIZE];
    strcpy(path, run_aggr(paths));
    //printf("%s\n", path);
    char paths_new[MAX_STR_SIZE];
    getFilePath(paths, paths_new);
    //printf("paths is %s\n",paths_new);
    read_file(paths_new, path);
    FILE *doc;
    printf("%s\n",paths_new); //use this if we shouldnt call everything who_won.txt
    doc = fopen(path, "a");
    if (doc == NULL) {
        //printf("%s open failed!\n",path);
        _exit(1);
    }

    fprintf(doc, "Winner:%s\n", winner);
    fclose(doc);
    if (strcmp(paths_new, path)) {
        FILE *doc1;
        doc1 = fopen(paths_new, "a");
        if (doc1 == NULL) {
            //printf("%s open failed!\n",path);
            _exit(1);
        }

        fprintf(doc1, "Winner:%s\n", winner);
        fclose(doc1);
    }
}
