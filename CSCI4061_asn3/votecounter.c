/*login: tessm061 lin00116
* date: 04/10/18
* name: Tyler Tessmann, Hanning Lin */

#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_STR_SIZE 4096
#define MAX_FILEPATH 4096

struct condQueue {
	pthread_cond_t* cond;
	pthread_mutex_t* mutex;
	int cur_threads;
};

typedef struct dir {
    char name[MAX_STR_SIZE];    //Name of node
    char path[MAX_STR_SIZE];    //Path of file
    struct dir *next;           //Next canidate node
    struct dir *prev;           // previous canidate node
} dir_node;

typedef struct candidateCount {
    char name[MAX_STR_SIZE];    //Name of node
    int  count;  								//Count of canidate
    struct dir *next;           //Next  node
    struct dir *prev;           // previous node
} counter;

dir_node *publicroot;
// counter *publiccounter;

pthread_mutex_t logMutex;
pthread_mutex_t queueMutex;
pthread_mutex_t rwMutex;

FILE *LogPTR;
char input_dir[MAX_FILEPATH];

char output_dir[MAX_FILEPATH];

char **DSQ;
int priority[4096];
int LENGTH = 0;
int currentGrab = 0;
int num_threads = 4;

//unction getpath() can get the path of the out put file of a node
int getPath(char*filename,char*out_s){
  dir_node *temp_node;
  temp_node = publicroot;
  int flag = 0;
  if (!strcmp(temp_node->name, filename)) {
      strcpy(out_s, temp_node->path);
      flag = 1;
  }
  while (temp_node->next != NULL) {
      temp_node = temp_node->next;
      if (!strcmp(temp_node->name, filename)) {
          strcpy(out_s, temp_node->path);
          flag = 1;
      }
  }
  if (flag == 0) {
    //printf("fatal filename %s\n",filename);
      return -1;
  } else {
      strcat(out_s, "/");
      strcat(out_s, filename);
      strcat(out_s, ".txt");
  }
	return 0;
}
//this function can get the name of the parent of current path
void getparentname(char* currentpath,char* parentname){
  char delim[] = "/";
  int numtokens;
  char *buf = malloc(MAX_STR_SIZE * sizeof(char));
  char **argvp;
  if ((numtokens = makeargv(currentpath, delim, &argvp)) == -1) {
      //f//printf(stderr, "failed to parse the string you given:%s\n", buf);
      return 1;       //argvp[0] is the parent node.
  }
  strcpy(parentname,argvp[numtokens-3]);
  return 0;
}
//function createCounter() is used to create a new node of the Counter.
counter* createCounter(char* name, counter* next, counter* prev,int count)
{
    counter* new_node = (counter*)malloc(sizeof(counter));
    if(new_node == NULL)
    {
        //printf("Error creating a new node.\n");
        _exit(0);
    }
    memset(new_node->name, '\0', sizeof(new_node->name));
    new_node->count = count;
    strcpy(new_node->name,name);
    new_node->next = next;
    new_node->prev = prev;
    return new_node;
}
//function used to aggregate the result of it's child directory
int aggregate(char *filename,counter* root_counter){
  //f//printf(stderr,"DEBUG:  filename(%s)\n",filename);
  char *file = malloc(4096);
  char *parentname = malloc(4096);
  char *readfilepath = malloc(4096);
  char *writefilepath = malloc(4096);
  char *readbuff = malloc(4096);
  char *writebuff = malloc(4096);
  char *nextname = malloc(4096);
  size_t len = MAX_STR_SIZE;
  ssize_t read_size;
  ssize_t write_size;
  int flag=0;// if flag==0 means there is no that node
  int c=0;//counter used to count the length of inrement linked list

  counter *temp_node;
  counter *temp_out;
  counter *root_out=createCounter("ROOTOUT",NULL,NULL,0);

  char delim[] = ":";
  char **argvp;
  int numtokens;
  if(!strcmp(filename,publicroot->name)){
    return -1;
  }
  //open nowfile
  if(getPath(filename,readfilepath)==-1){//if this node is not part of DAG
		fprintf(stderr,"Node %s is not part of DAG, ignore it.\n",filename);
		return 0;
	}
  pthread_mutex_lock(&rwMutex);//mutex lock, used to solve sychronization problem
  FILE *in = fopen(readfilepath, "r");
  //get the information
  if(in){
    while ((read_size = getline(&readbuff, &len, in)) != -1) {
      trimlastenter(readbuff);
      flag=0;
        //reading from the child file
        temp_node=root_counter;
        if(read_size==2)//this is the leaf file
        {//create the data linked list used for recursive all the way to the root node
          while (temp_node->next != NULL) {//if the candidate exist, add the counter
            temp_node = temp_node->next;
            if (!strcmp(temp_node->name, readbuff)) {
                temp_node->count++;
                flag=1;
            }
        }
        if (flag==0){//if there is no such node, create a new one
          temp_node->next=createCounter(readbuff,NULL,temp_node,1);
        }
      }
    }
    }
  else
  {
    printf(stderr,"File doesn't exist!\n",filename);
    perror(NULL);
    _exit(1);
  }
  //writing to parent's file
  getparentname(readfilepath,parentname);
  getPath(parentname,writefilepath);
	//try to open the file
  FILE *Fout = fopen(writefilepath, "r+");
  if (!Fout){//file don't exit
	//just create and write
    Fout = fopen(writefilepath, "w+");
    temp_node=root_counter;
    while (temp_node->next != NULL) {//write the data to the file
        temp_node = temp_node->next;
        sprintf(writebuff,"%s:%d\n",temp_node->name,temp_node->count);
        fwrite(writebuff,1, strlen(writebuff), Fout);
    }
    fclose(Fout);
  }else{
    //file exist
    //update the file
    temp_out=root_out;
    while ((read_size = getline(&readbuff, &len, Fout)) != -1) {
			//create a linked list of the new output of the file
      trimlastenter(readbuff);
        if ((numtokens = makeargv(readbuff,delim, &argvp)) == -1) {//parse the line
            return 1;   //parse failed
        }
        temp_node=root_counter;
        temp_out->next=createCounter("temp",NULL,temp_node,0);
        temp_out=temp_out->next;
        flag=0;
        while (temp_node->next != NULL) {
            temp_node = temp_node->next;
            if (!strcmp(temp_node->name, argvp[0])) {//cauculate the new count
                strcpy(temp_out->name,temp_node->name);
                temp_out->count=atoi(argvp[1])+temp_node->count;
                flag = 1;
                break;
            }
        }
        if (flag==0){//there is no such node
        strcpy(temp_out->name,argvp[0]);
        temp_out->count=atoi(argvp[1]);
        }
    }
    temp_out=root_out;
    while(temp_out->next!=NULL){
      temp_out=temp_out->next;//go to the end of the linked list,can be used for debug
    }
    fclose(Fout);
    FILE *Fout = fopen(writefilepath, "w");//create a blank file and write
    temp_out=root_out;
    while (temp_out->next != NULL) {//write the result to the file
        temp_out = temp_out->next;
        sprintf(writebuff,"%s:%d\n",temp_out->name,temp_out->count);
        fwrite(writebuff,1, strlen(writebuff), Fout);
    }
    fclose(Fout);
  }
pthread_mutex_unlock(&rwMutex);//unlock the mutex lock
  getparentname(readfilepath,parentname);
  fclose(in);
	//recursive, call same function on parent node
  if(aggregate(parentname,root_counter)==-1)
  {
    return 0;
  }
  //free heap

    free(file);
    free(parentname);
    free(readfilepath);
    free(writefilepath);
    free(readbuff);
    free(writebuff);
    free(nextname);
    return 0;
}

//Main funcito to read the input files
void readInput(char* filename)
{
    char *file = malloc(4096);
    char *out_s = malloc(4096);
    getPath(filename,out_s);

    //printf("Doing strcpy input\n");
    strcpy(file, input_dir);
    //printf("Done\n");
    strcat(file, filename);
    //printf("input is %s\nDoing strcpy of %s\n", file, output_dir);
    //strcpy(out_s,input_dir);
    //printf("Doing fopens\n");
    //printf("#######################log out[%s]\n", out_s);
    FILE *Fout = fopen(out_s, "w+");
    //printf("Writing to %s\n", out_s);
    FILE *in = fopen(file, "r");
    if (in && Fout) {
        char *buffer = malloc(4096);
        size_t len = 0;
        ssize_t read;
        while ((read = getline(&buffer, &len, in)) != -1) {
            //printf("About to decrypt\n");
            decryptString(buffer);
            //printf("Buffer is %s", buffer);
            strcat(buffer, "\n");
            fwrite(buffer, 1, strlen(buffer), Fout);
        }
        //printf("Done with file\n");
        fclose(in);
        //printf("Done with closing in\n");
        fclose(Fout);
    }
    //printf("Done with closing out\n");
}

//getter for the queue
//thread safe
char *getNameFromQueue()
{
    char *name;
    name = malloc(4096);
    pthread_mutex_lock(&(queueMutex));
    name = DSQ[currentGrab];
    currentGrab++;
    pthread_mutex_unlock(&(queueMutex));
    return name;
}

//This is the child thread
void runThread(void* arg)
{
    struct condQueue* cq = (struct condQueue*) arg;
    //printf("We are Thread #%d\n",cq->cur_threads);
    char *file;
    file = malloc(4096);
    file = getNameFromQueue();
    //printf("Got %s from queue\n", file);
    //TODO unlock mutux
    //printf("About to get TID\n");
    char tids[32];
    pid_t tid;
    tid = pthread_self();
    sprintf(tids, "%ld", tid);
    //printf("TID is %s\n", tids);
    char *log_entry = malloc(1000);
    strcpy(log_entry, file);
    strcat(log_entry, ":");
    strcat(log_entry, tids);
    strcat(log_entry, ":start\n");
    //printf("%s\nreading input now\n", log_entry);
    writeToLog(log_entry);
    readInput(file);
    counter *increment=createCounter("ROOTINCREMENT",NULL,NULL,0);
    aggregate(file,increment);
    strcpy(log_entry, file);
    strcat(log_entry, ":");
    strcat(log_entry, tids);
    strcat(log_entry, ":end\n");
    writeToLog(log_entry);
    //printf("deinc cur threads\n");
    cq->cur_threads--;
    //printf("cond signal\n");
    pthread_cond_signal(cq->cond);

}

/*
void tmp_Test(){
 pthread_t test;
 //printf("Making struct\n");
 struct regionStruct* dep = (struct regionStruct*) malloc(sizeof(struct regionStruct));
 //printf("Making filename\n");
 dep->filename = malloc(4096);
 strcpy(dep->filename,"Sub_County_1");
 //printf("Making thread\n");
 pthread_create(&test, NULL, runThread, (void*) dep);
 pthread_join(test, NULL);
 //printf("Done with closing thread\n");
}
*/
//Write a msg to log
void writeToLog(char *data)
{
    //printf("logptr is %d about to make thread test\n", LogPTR);
    pthread_mutex_lock(&(logMutex));
    //printf("logging %s of len %d\n", data, strlen(data));
    int i=fwrite(data, 1, strlen(data), LogPTR);
    if (!i)
      perror("fwrite is");
    pthread_mutex_unlock(&(logMutex));
}

//mimic rm -rf
void removeDIR(char *path)
{
    DIR *dir;
    if ((dir = opendir(path)) == NULL) {
     perror("Dir Open failed:");
    }
    struct dirent *ent;
    int i = 0;
    char *full_path;
    struct stat stat_data;
    while ((ent = readdir(dir)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;
        full_path = malloc(4096);
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, ent->d_name);
        stat(full_path, &stat_data);

        if (S_ISDIR(stat_data.st_mode) != 0) {
            removeDIR(full_path);
            continue;
        }
        if(unlink(full_path) != 0)
		perror("Could not delete file");
    }
     if(rmdir(path) != 0)
                perror("Could not remove dir");

     if(closedir(dir) != 0)
                perror("Could not close dir");

}
//check to see if input file is blank
int checkFileHasData(char *filename)
{
 FILE *fp;
 int size;
 fp = fopen(filename, "r");  //open the inputfile
    fseek (fp, 0, SEEK_END);
    size = ftell(fp);
    if (size < 1)
        return 0;

return 1;
}

//check if we can access the file
int checkFileAccess(char *filename)
{
    return (access(filename, F_OK) != -1);
}

//check if the directory exists
int checkDirAccess(char *path)
{
    DIR *dir = opendir(path);
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        return 1;
    }
    return 0;
}

//decyprt a string sent to it
void decryptString(char *str)
{
    char *fin;
    fin = malloc(sizeof(str));
		int position=0;
    for (int i = 0; str[i] != '\0' && str[i] != 10; i++) {
        //printf("Processing ..%d..\n", str[i]);
        int tmp = str[i];
        if (tmp == 'y')
            strcat(fin, "a");
        else if (tmp == 'z')
            strcat(fin, "b");
				else if (tmp == 'Y')
		            strcat(fin, "A");
		    else if (tmp == 'Z')
		            strcat(fin, "B");
        else {
            tmp = tmp + 2;
            fin[position] = tmp;
						fin[position+1]='\0';
						position++;
            //printf("ASC::%d::\n", tmp);
        }
    }
    //printf("Str(%s)::Fin(%s)::\n", str,fin);
    strcpy(str, fin);
    return fin;
}

//sort the queue with priority (EC)
void sortQueue()
{
char* tmp_string;
int tmp_int;


for (int i = 0; i < LENGTH; i++) {
  for (int j = 0; j < LENGTH; j++) {
        if(priority[i] < priority[j]){
          tmp_int = priority[i];
          tmp_string = DSQ[i];
	   priority[i] = priority[j];
           DSQ[i] =  DSQ[j];
          priority[j] = tmp_int;
          DSQ[j] = tmp_string;
        }
    }
 }

for (int i = 0; i < LENGTH; i++) {
        //printf("FINAL Item: %s has p#%d\n", DSQ[i],priority[i]);
    }
}

//Add a string to the queue
void addToQueue(char *name)
{
    char *fin;
    fin = malloc(sizeof(name));
    strcpy(fin, name);
    //printf("Calling addToQueue with %s\n", name);
    DSQ[LENGTH] = malloc(sizeof(fin));
    DSQ[LENGTH] = fin;

    priority[LENGTH] = malloc(sizeof(int));
    char delim[] = "_";
    int flag = 0;
    char **argvp;
    int numtokens;
    int pri = 2147483647;
    if ((numtokens = makeargv(fin, delim, &argvp)) == -1) {
                //f//printf(stderr, "failed to parse the string you given:%s\n", fin);
                return 1;       //argvp[0] is the parent node.
            }
     //printf("testing val %d\n",numtokens);
    if(numtokens > 2){
    if(!strcmp("p",argvp[numtokens-2])){
    pri = atoi(argvp[numtokens-1]);
    priority[LENGTH] = pri;
    }
     else{
    priority[LENGTH] = pri;
    }
    }
    else{
    priority[LENGTH] = pri;
    }


    LENGTH++;
    for (int i = 0; i < LENGTH; i++) {
        //printf("Item: %s has p#%d\n", DSQ[i],priority[i]);
    }
}

//go through dir, decrypt all files
void decryptAll()
{
    struct dirent *pDirent;
    DIR *pDir;
    pDir = opendir(input_dir);
    if (pDir == NULL) {
        perror("Dir cannot open");
        return 1;
    }
    while ((pDirent = readdir(pDir)) != NULL) {
        if (!strcmp(pDirent->d_name, ".") || !strcmp(pDirent->d_name, ".."))
            continue;
        else if (pDirent->d_type == DT_DIR)
            continue;
        else
            addToQueue(pDirent->d_name);
    }
    closedir(pDir);
    sortQueue();
    pthread_t test[100];
    pthread_cond_t cond[100];
    //printf("Making condQueue\n");
    struct condQueue* cq = (struct condQueue*) malloc(sizeof(struct condQueue));
	//cq->q = (struct queue*) malloc(sizeof(struct queue));
	cq->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
	cq->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(&(queueMutex), NULL);
    pthread_mutex_init((cq->mutex), NULL);
    pthread_cond_init(cq->cond, NULL);
    cq->cur_threads = 0;
    //printf("Entering for loop\n");
    //TODO EC
    if(LENGTH < 1)
     fprintf(stderr,"error: input directory is empty\n");

    for (int i = 0; i < LENGTH; i++) {
     //TODO
	 //printf("Locking mutex\n");
        pthread_mutex_lock((cq->mutex));
	 //printf("Locked mutex, waiting in cur\n");
        cq->cur_threads++;
        while(cq->cur_threads > num_threads)
         pthread_cond_wait((cq->cond),(cq->mutex));

 	//printf("Dispatched thread\n");
        pthread_create(&test[i], NULL, runThread, (void*) cq);
        pthread_mutex_unlock((cq->mutex));

     //TODOed_tpthread_t
    }
    for (int i = 0; i < LENGTH; i++) {
        pthread_join(test[i], NULL);
    }
}

//makes the log file
void initLog()
{
    //printf("initing log mutex\n");
    pthread_mutex_init(&(logMutex), NULL);
    char logname[4096];
    memset(logname, '\0', sizeof(logname));
    //printf("doing str cpy\n");
    //printf("output_dir is: %s\n", output_dir);
    strcpy(logname, output_dir);
    //printf("doing str cat\n");
    strcat(logname, "/log.txt");
    LogPTR = malloc(sizeof(FILE *));
    //printf("opening file %s\n", logname);
    LogPTR = fopen(logname, "w");
    chmod(logname, 0700);
    if(!LogPTR)
      perror("File open is");
    ////printf("logptr is %d about to make thread test\n",fp);
    //fwrite("Log Starting\n",1,sizeof("Log Starting\n"),LogPTR);
    //writeToLog("Log Starting\n");
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

//Create a new node of DAG node
dir_node *create(char *name, dir_node * prev, dir_node * next)
{
    //Use malloc to allocate space on the heap
    dir_node *new_node = (dir_node *) malloc(sizeof(dir_node));
    if (new_node == NULL) {
        //f//printf(stderr, "Error creating a new node.\n");
        _exit(0);
    }
    //set the name
    strcpy(new_node->name, name);
    new_node->next = next;
    new_node->prev = prev;
    return new_node;
}
//function used to find the path of a subdirectory
int makepath(char *oldpath, char *filename, char *newpath)
{
    strcpy(newpath, oldpath);
    strcat(newpath, "/");
    strcat(newpath, filename);
    return 0;
}
//function used to trim the last "\n" in a string
int trimlastenter(char *filename)
{
    int len = strlen(filename);
    char newname[4096];
    memset(newname, '\0', sizeof(newname));
    if (filename[len - 1] == '\n') {
        strncpy(newname, filename, len - 1);
        memset(filename, '\0', sizeof(filename));
        strcpy(filename, newname);
    }
    return 0;
}
int trimlastslash(char *filename)
{
    int len = strlen(filename);
    char newname[4096];
    memset(newname, '\0', sizeof(newname));
    if (filename[len - 1] == '/') {
        strncpy(newname, filename, len - 1);
        memset(filename, '\0', sizeof(filename));
        strcpy(filename, newname);
    }
    return 0;
}
//create the DAG directories based on the input.txt
dir_node *createDAG(char *filename, char *output_dir)
{
    //if (mkdir(output_dir, 0777) == -1) {
    //    perror("make directory\n");
    //}
    FILE *fp;
    int numtokens;
    int i = 0;
    fp = fopen(filename, "r");  //open the inputfile
    if (fp == NULL) {
        perror("Bad File Path");
    }
    chmod(filename, 0700);
    size_t len = MAX_STR_SIZE;
    ssize_t read;
    char *buf = malloc(MAX_STR_SIZE * sizeof(char));    //set aside a chunk of memory for the buffer
    if (buf == NULL) {
        perror("MALLOC FAILURE\n");
        _exit(1);
    }
    int linecounter = 0;        //keep a running count of what line we are on
    char delim[] = ":";
    int flag = 0;
    char **argvp;
    dir_node *temp_node;
    dir_node *root = (dir_node *) malloc(sizeof(dir_node));
    temp_node = root;
    char rootpath[MAX_STR_SIZE];
    while ((read = getline(&buf, &len, fp)) != -1) {
        if (buf[0] == '#') {    //skip comment lines
            //printf("Skipping comment %s \n", buf);
        } else {

            if ((numtokens = makeargv(buf, delim, &argvp)) == -1) {
                //f//printf(stderr, "failed to parse the string you given:%s\n", buf);
                return 1;       //argvp[0] is the parent node.
            }
            if (linecounter == 0) {//if this is the first line
                root->prev = NULL;
                root->next = NULL;
                strcpy(root->name, argvp[0]);
                makepath(output_dir, argvp[0], root->path);//set the first dir_node which is the root node
                for (i = 1; i < numtokens; i++) {//create the other nodes in the first line
                    trimlastenter(argvp[i]);
                    temp_node->next = create(argvp[i], temp_node, NULL);
                    temp_node = temp_node->next;
                    makepath(root->path, argvp[i], temp_node->path);
                }
            } else {            //not first line
                temp_node = root;//start from the root
                flag = 0;
                while (temp_node->next != NULL) {//find if this node exist
                    temp_node = temp_node->next;
                    if (!strcmp(temp_node->name, argvp[0])) {//if the parent node exist
                        strcpy(rootpath, temp_node->path);//set rootpath
                        flag = 1;
                    }
                }
                if (flag == 0) {//if the node is not found,there is a mistake
                    //printf("input file error\n");
                    exit(1);
                } else {
                    for (i = 1; i < numtokens; i++) {//create new node for the child nodes
                        trimlastenter(argvp[i]);
                        temp_node->next = create(argvp[i], temp_node, NULL);
                        temp_node = temp_node->next;
                        makepath(rootpath, argvp[i], temp_node->path);
                    }
                }
            }
            linecounter++;
        }
    }
    //make the DAG directory based on the link list
    temp_node = root;
    if (mkdir(temp_node->path, 0777) == -1) {
        perror("make directory\n");
    }
    while (temp_node->next != NULL) {
        temp_node = temp_node->next;
        if (mkdir(temp_node->path, 0777) == -1) {
            perror("make directory\n");
        }
    }
    return root;
}
void findwinner(){
  dir_node*root_dir=publicroot;
  char path[4096];
  char *readbuff=malloc(4096);
  char writebuff[4096];
  ssize_t read_size;
  size_t len = MAX_STR_SIZE;
  memset(path, '\0', sizeof(path));
  memset(writebuff,'\0',sizeof(writebuff));
  char delim[] = ":";
  char **argvp;
  int numtokens;
  char result_name[4096];
  memset(result_name, '\0', sizeof(result_name));
  int result_count=0;
	//set the path of the Who_Won file
  strcpy(path,root_dir->path);
  strcat(path,"/");
  strcat(path,root_dir->name);
  strcat(path,".txt");
  //f//printf(stderr,"FINDWINNER:path is [%s]\n",path);
  FILE*file =fopen(path,"r+");//open the output file of the root node
  if(file){
    while ((read_size = getline(&readbuff, &len, file)) != -1) {//read by line and find the winner
      trimlastenter(readbuff);
      if ((numtokens = makeargv(readbuff,delim, &argvp)) == -1){
      }

      if(atoi(argvp[1])>result_count){
        strcpy(result_name,argvp[0]);
        result_count=atoi(argvp[1]);
      }

    }
		//write the result of the winner into the file
    sprintf(writebuff,"WINNER:%s\n",result_name);
    fwrite(writebuff,1, strlen(writebuff), file);
  }
  else{
    //f//printf(stderr,"Findwinner read file %s failed!\n",path);
    perror(NULL);
    _exit(1);
  }

}


int main(int argc, char **argv)
{
    //Allocate space for MAX_NODES to node pointer
    if (argc < 4 || argc > 5) { //if there isnt proper args, fail now and show proper usage
        //printf("Usage: %s Program <DAG.txt> <input_dir> <output_dir> [num_threads] \n", argv[0]);
        //printf("Try %s DAG.txt in out\n", argv[0]);
        return -1;              //exit in failure
    }

    DSQ = malloc(4096);
    //priority = malloc(4096);

    char *filename = malloc(sizeof(argv[1]));
    filename = argv[1];

		memset(input_dir,"\0",sizeof(input_dir));
    strcpy(input_dir,argv[2]);
		trimlastslash(input_dir);
		strcat(input_dir,"/");

		memset(output_dir,"\0",sizeof(output_dir));
    strcpy(output_dir,argv[3]);
		trimlastslash(output_dir);

    if(argc > 4){
    num_threads = atoi(argv[4]);}

   int DAG_exists = checkFileAccess(filename);
    if (!DAG_exists) {
        perror("Input FILE does not exist");
        _exit(1);
    }

   int DAG_contents = checkFileHasData(filename);
    if (!DAG_contents) {
        perror("Input FILE does not have data");
        _exit(1);
    }

    int inputExists = checkDirAccess(input_dir);
    if (!inputExists) {
        perror("Input DIR does not exist");
        _exit(1);
    }

     int outputExists = checkDirAccess(output_dir);
     if(outputExists){
       removeDIR(output_dir);
     }

     int newFolder = mkdir(output_dir, 0700);
       if(newFolder == -1)
         perror("MKDIR Failed:");

    initLog();
    //Where we read in the dag, and make it
    publicroot = createDAG(filename, output_dir);

    pthread_mutex_init(&rwMutex,NULL);
    //End of DAG making
    decryptAll();
    findwinner();
    fclose(LogPTR);
    return 0;
}
