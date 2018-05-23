/*login: tessm061 lin00116
* date: 04/30/18
* name: Tyler Tessmann, Hanning Lin
*Extra credit [YES]*/

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
#include <libgen.h>

//Custom global varibles for use in multiple client functions
char *server_IP;
int server_port;

int max_data = 0;

int sock_g;

char canidate_names[4096][4096] = { NULL };
int canidate_votes[4096] = { NULL };;

//Given function
char *trimwhitespace(char *str)
{
    char *end;
    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)              // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
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

    if (strtok_r(t, delimiters, &lasts) != NULL)        // count number of tokens in s
        for (numtokens = 1; strtok_r(NULL, delimiters, &lasts) != NULL; numtokens++) ;

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
        **argvp = strtok_r(t, delimiters, &lasts);
        for (i = 1; i < numtokens; i++)
            *((*argvp) + i) = strtok_r(NULL, delimiters, &lasts);
    }

    *((*argvp) + numtokens) = NULL;     // put in final NULL pointer

    return numtokens;
}

//eod of copy
//takes in a single canidate name, and places the vote for them
void count_vote(char *name)
{
    trimwhitespace(name);
//  printf("Counting %s\n",name);
    int i = 0;

    while (canidate_votes[i] != 0 && strcmp(canidate_names[i], name)) {
//    printf(" '%s' is not '%s' \n",canidate_names[i], name);  
        i++;
    }
    if (canidate_votes[i] == 0) {
        //create new slot
        strcpy(canidate_names[i], name);
        canidate_votes[i] = 1;
    } else {
//    printf(" '%s' is '%s' \n",canidate_names[i], name);
        canidate_votes[i]++;
    }
}

//Opens a vote file, and calls count vote for every line
void leafcounter(char *file)
{
    //printf("Starting to count votes in file: %s\n",file);
    //char* out;
    //strcat(file,".out");
    FILE *fp;
    char *vote = malloc(1024);
    size_t len = 0;
    ssize_t read;
    //printf("Opening file\n");
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        //     fprintf(stdout, "Current working dir: %s\n", cwd);
        strcat(cwd, "/");
    strcat(cwd, file);
//              fprintf(stdout, "Opening: %s\n", cwd);
    trimwhitespace(cwd);
    fp = fopen(cwd, "r");
    //perror("Opening file ");
    while ((read = getline(&vote, &len, fp)) != -1) {
//                      printf("Read line\n");
        char *tmp;
        tmp = malloc(1024);
        strcpy(tmp, vote);
        count_vote(tmp);
    }
    fclose(fp);
//              printf("Canidate %s had %d votes\n",canidate_names[0],canidate_votes[0]);
}

//caps a request with whitespace, so odd characters don't come in
char *cap_req(char *req)
{
    for (int i = 0; i < max_data; i++) {
        strcat(req, " ");
    }
    return req;
}

//prints the current votes counted into a socket
void print_file_into_data(char *id, char *region, char *code)
{
    int i = 0;
    char votes[4096];
    strcpy(votes, "");
    while (canidate_votes[i] != 0) {
//      printf("can votes\n");
//      printf("CODE is %s\n",code);
        strcat(code, canidate_names[i]);
        strcat(votes, canidate_names[i]);
//      printf("CODE is %s\n",code);
        strcat(code, ":");
        strcat(votes, ":");
//      printf("CODE is %s\n",code);
        char tmp[4096];
        sprintf(tmp, "%d", canidate_votes[i]);
//      printf("tmp is %s\n",tmp);
        strcat(code, tmp);
        strcat(votes, tmp);
//      printf("CODE is %s\n",votes);
        strcat(code, ",");
        strcat(votes, ",");
        i++;
    }

    memset(canidate_votes, 0, 4096);
    memset(canidate_names, NULL, 4096);
    code[strlen(code) - 1] = '\0';
    votes[strlen(votes) - 1] = '\0';
    strcat(code, "      ");
    //printf("CODE is %s\n",code);
    if (strlen(code) > max_data) {
        max_data = strlen(code);
    }
    cap_req(code);
    write_all(sock_g, code, strlen(code));
    //Sending request to server: AV County_1 B:2,A:2
    fprintf(stdout, "Sending request to server: %s %s %s\n", id, trimwhitespace(region), votes);
}

//pads a string with up to 15 spaces for region name
char *StringPad(char *str)
{
//printf("string pad: '%s'\n",str);
    int padLen = 15 - strlen(str);      // Calc Padding length

    for (int i = 0; i < padLen; i++) {
        strcat(str, " ");
    }

    printf("\n");
    return str;
}

//Function call for adding votes
void Add_Votes(char *name, char *file)
{
    printf("\n");
    char init[4096];
    char *tmp;
    tmp = malloc(4096);
    strcpy(tmp, name);
    printf("\n");
    trimwhitespace(tmp);
    printf("\n");
    StringPad(tmp);
    printf("\n");
    strcpy(init, "AV;");
    printf("\n");
    strcat(init, tmp);
    printf("\n");
    strcat(init, ";");
    printf("\n");
    //fprintf(stdout, "Sending request to server: AV %s (null)\n",trimwhitespace(tmp));
//  strcat(init,";");
    leafcounter(file);
    //strcat(file,".out");
    print_file_into_data("AV", tmp, init);
    //  write_all(sock_g,code,sizeof(code));
}

//Function call for open polls
void Open_Polls(char *name)
{
    printf("\n");
    char init[4096];
    char *tmp;
    tmp = malloc(4096);

    strcpy(tmp, name);
    trimwhitespace(tmp);
    StringPad(tmp);
    strcpy(init, "OP;");
    printf("\n");
    strcat(init, tmp);          //fox 
    printf("\n");
    strcat(init, ";");
    cap_req(init);
    //init[strlen(init)] = '\0';
    printf("\n");
    fprintf(stdout, "Sending request to server: OP %s (null)\n", trimwhitespace(tmp));
    write_all(sock_g, init, strlen(init));
    //write_all(sock_g,"OP: we are here\0",sizeof("OP: we are here\0"))
}

//Function call for counting votes
void Count_Votes(char *name)
{
    printf("\n");
    char init[4096];
    char *tmp;
    tmp = malloc(4096);

    strcpy(tmp, name);
    trimwhitespace(tmp);
    StringPad(tmp);
    strcpy(init, "CV;");
    printf("\n");
    strcat(init, tmp);
    printf("\n");
    strcat(init, ";");
    cap_req(init);
    printf("\n");
    fprintf(stdout, "Sending request to server: CV %s (null)\n", trimwhitespace(tmp));
    write_all(sock_g, init, strlen(init));
}

//Function call for returning the winner
void Return_Winner(char *name)
{
    printf("\n");
    char init[4096];
    char *tmp;
    tmp = malloc(4096);

    strcpy(tmp, name);
    trimwhitespace(tmp);
    StringPad(tmp);
    strcpy(init, "RW;");
    cap_req(init);
    fprintf(stdout, "Sending request to server: RW (null) (null)\n", init);
    write_all(sock_g, init, strlen(init));
}

//function to write everything in string to a socket
//checks return of write to make sure
void write_all(int socket, char *str, int len)
{
    int ret;
    while (len > 0) {
        ret = write(socket, str, len + 1);
        str += ret;
        len -= ret;
    }
}

//handles server response
void handle_response(char *buffer_r)
{

}

//waits for server to send response
void getResponse()
{

    char buffer[1];
    char buffer_r[4096];
    //strcpy(buffer_r,"");
    int done = 0;
    int i = 0;
    while (!done) {
        read(sock_g, buffer, 1);
        //strcat(buffer_r,buffer);
        buffer_r[i] = buffer[0];
        //printf("Char:'%c'\n",buffer[0]);
        if (buffer[0] == ';') {
            buffer_r[i] = ' ';
        }

        if (buffer[0] == '\0') {
            done = 1;
        }
        i++;
    }
    if (!strcmp(buffer_r, "SC ")) {
        fprintf(stdout, "Received response from server: SC (null)\n");
    } else {
        fprintf(stdout, "Received response from server: %s\n", buffer_r);
    }
    handle_response(buffer_r);
    //Received response from server: SC Winner:B 
}

//Function call for adding a new region
void Add_Region(char *parent, char *region)
{
    printf("\n");
    char init[4096];

    char *tmp;
    tmp = malloc(4096);
    strcpy(tmp, parent);
    printf("\n");
    trimwhitespace(tmp);
    printf("\n");
    StringPad(tmp);
    printf("\n");

    char *tmp1;
    tmp1 = malloc(4096);
    strcpy(tmp1, region);
    printf("\n");
    trimwhitespace(tmp1);
    printf("\n");
    //StringPad(tmp1);
    printf("\n");

    strcpy(init, "AR;");
    printf("\n");
    strcat(init, tmp);
    printf("\n");
    strcat(init, ";");
    printf("\n");
    strcat(init, tmp1);
    printf("\n");
    cap_req(init);
    fprintf(stdout, "Sending request to server: AR %s %s\n", trimwhitespace(tmp), trimwhitespace(tmp1));
    write_all(sock_g, init, strlen(init));

}

//Function call for closing polls
void Close_Polls(char *name)
{
    printf("\n");
    char init[4096];
    char *tmp;
    tmp = malloc(4096);

    strcpy(tmp, name);
    trimwhitespace(tmp);
    StringPad(tmp);
    strcpy(init, "CP;");
    printf("\n");
    strcat(init, tmp);
    printf("\n");
    strcat(init, ";       ");
    printf("\n");
    cap_req(init);
    fprintf(stdout, "Sending request to server: CP %s (null)\n", trimwhitespace(tmp));
    write_all(sock_g, init, strlen(init));
}

//Function call for removing votes
void Remove_Votes(char *name, char *file)
{
    printf("\n");
    char init[4096];
    char *tmp;
    tmp = malloc(4096);

    strcpy(tmp, name);
    trimwhitespace(tmp);
    StringPad(tmp);
    strcpy(init, "RV;");
    printf("\n");
    strcat(init, tmp);
    printf("\n");
    strcat(init, ";");
    printf("\n");
//  fprintf(stdout, "Sending request to server: RV %s (null)\n",trimwhitespace(tmp));
//  strcat(init,";");
    leafcounter(file);
    //strcat(file,".out");
    print_file_into_data("RV", tmp, init);
    //  write_all(sock_g,code,sizeof(code));

}

//Takes in a command, and matches it with a function call
void dispatchLine(char *str)
{
    //printf("DISPATCH: %s",str);
    //makeargv
    char delim[] = " ";
    int flag = 0;
    char **argvp;
    int numtokens;
    if ((numtokens = makeargv(str, delim, &argvp)) == -1) {
        fprintf(stderr, "failed to parse the string you given:%s\n", str);
        return 1;               //argvp[0] is the parent node.
    }
    char *region;
    char *file;
    char *cmd;
    region = malloc(1024);
    file = malloc(1024);
    cmd = malloc(1024);
    cmd = argvp[0];
    if (numtokens > 1)
        region = argvp[1];
    if (numtokens > 2)
        file = argvp[2];
    trimwhitespace(cmd);

    if (strcmp("Open_Polls", cmd) == 0)
        Open_Polls(region);
    else if (strcmp("Add_Votes", cmd) == 0)
        Add_Votes(region, file);
    else if (strcmp("Count_Votes", cmd) == 0)
        Count_Votes(region);
    else if (strcmp("Close_Polls", cmd) == 0)
        Close_Polls(region);
    else if (strcmp("Return_Winner", cmd) == 0)
        Return_Winner(region);
    else if (strcmp("Remove_Votes", cmd) == 0)
        Remove_Votes(region, file);
    else if (strcmp("Add_Region", cmd) == 0)
        Add_Region(region, file);
    else {
        fprintf(stderr, "ERR, no match of '%s' to 'Return_Winner'\n", argvp[0]);
    }
}

//Main driver, reads the input file, calling dispatch on every line
int main(int argc, char **argv)
{

    if (argc < NUM_ARGS + 1 || argc > NUM_ARGS + 1) {

        printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
        exit(1);
    }
    char *tmp;
    char *tmp1;
    tmp = strdup(argv[1]);
    tmp1 = strdup(argv[1]);
    //printf("Path is %s\n",tmp);

    dirname(tmp);
    basename(tmp1);

    fprintf(stdout, "./%s/\n", tmp);

    strcpy(tmp1, basename(tmp1));
    chdir(tmp);
    server_port = atoi(argv[3]);
    //printf("Dir is %s\n",tmp);

    server_IP = malloc(1024);
    server_IP = argv[2];

    // Create a TCP socket.
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // Specify an address to connect to (we use the local host or 'loop-back' address).
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);
    address.sin_addr.s_addr = inet_addr(server_IP);

    // Connect it.
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == 0) {

        // Buffer for data.
        char buffer[256];
        fprintf(stdout, "Initiated connection with server at %s:%d\n", server_IP, server_port);
        sock_g = sock;
        // Open the file.
        FILE *fp;
        char *vote = malloc(1024);
        size_t len = 0;
        ssize_t read;

        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        //fprintf(stdout, "Current working dir: %s\n", cwd);
        strcat(cwd, "/");
        strcat(cwd, tmp1);
//              fprintf(stdout, "Opening: %s\n", cwd);
        fp = fopen(cwd, "r");
        //      perror("FOPEN in main");
        while ((read = getline(&vote, &len, fp)) != -1) {
            char *tmp;
            tmp = malloc(1024);
            strcpy(tmp, vote);
            dispatchLine(tmp);
            getResponse();
        }
        fclose(fp);
        close(sock);
        fprintf(stdout, "Closed connection with server at %s:%d\n", server_IP, server_port);
    } else {

        perror("Connection failed!");
    }
}
