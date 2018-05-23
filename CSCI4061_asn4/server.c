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
#include <string.h>

#define MAX_CONNECTIONS 100
#define MAX_STR_SIZE 1024

char* server_IP;
int server_port;

char* client_IP;
int client_port;

int open_and_filled = 0;

struct client_info {
  char client_name[4096];
	int socket;
};
typedef struct candidateCount {
    char name[MAX_STR_SIZE];    //Number of votes for the canidate
    int  count;  //Name of canidate
    struct candidateCount *next;           //Next canidate node
    struct candidateCount *prev;           // previous canidate node
} counter;

typedef struct candidate_node {
    char name[MAX_STR_SIZE];    //Number of votes for the canidate
    int  count;  //Num of canidate
    struct candidate_node *next;           //Next canidate node
    struct candidate_node *prev;           // previous canidate node
    struct candidate_node **children; // the position of child nodes in linked list
    int num_children;
    int open;//flag whether it's open. 0 close, 1 open
    int closed;//flag whether it has benn changed
    struct candidateCount * root_counter;
    int hasdata;

} node;

pthread_mutex_t* mutex;
node *PUBLICROOT;//this is the first node, empty.
// char PUBLICPATH[1024];
char *trimwhitespace(char *str)
{
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0)  // All spaces
    return str;
  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  // Write new null terminator
  *(end+1) = 0;

  return str;
}

void formatoutput(char*str){//format output str
  char temp[1024];
  memset(temp,'\0',strlen(temp)+1);
  char delim[] = ";";
  char **argvp;
  int i=0;
  int numtokens;
  if ((numtokens = makeargv(str, delim, &argvp)) == -1) {
      fprintf(stderr, "failed to parse the string you given:%s\n", str);
      return -1;       //argvp[0] is the parent node.
  }
  for(i=0;i<numtokens;i++){
    strcat(temp,argvp[i]);
    strcat(temp," ");
  }
  if(numtokens==1){
    strcat(temp,"(null)");
  }
  memset(str,'\0',strlen(str)-1);
  strcpy(str,temp);
}
void formatinput(char*str){//format input str
  char temp[1024];
  memset(temp,'\0',strlen(temp)+1);
  char delim[] = ";";
  char **argvp;
  int i=0;
  int numtokens;
  trimwhitespace(str);
  if ((numtokens = makeargv(str, delim, &argvp)) == -1) {
      fprintf(stderr, "failed to parse the string you given:%s\n", str);
      return -1;       //argvp[0] is the parent node.
  }
  for(i=0;i<numtokens;i++){
    trimwhitespace(argvp[i]);
    strcat(temp,argvp[i]);
    strcat(temp," ");
  }
  if(numtokens==1){
    strcat(temp,"(null) (null)");
  }
  if(numtokens==2){
    strcat(temp,"(null)");
  }
  memset(str,'\0',strlen(str)-1);
  strcpy(str,temp);
}


node *create(char *name, node * prev, node * next)//create a new node
{
    //Use malloc to allocate space on the heap
    node *new_node = (node *) malloc(sizeof(node));
    if (new_node == NULL) {
        fprintf(stderr, "Error creating a new node.\n");
        _exit(0);
    }
    //Give it one vote
    //set the name
    strcpy(new_node->name, name);
    new_node->next = next;
    new_node->prev = prev;
    return new_node;
}

counter* createCounter(char* name, counter* next, counter* prev,int count)
{
    counter* new_node = (counter*)malloc(sizeof(counter));
    if(new_node == NULL)
    {
        printf("Error creating a new node.\n");
        _exit(0);
    }
    memset(new_node->name, '\0', sizeof(new_node->name));
    new_node->count = count;
    strcpy(new_node->name,name);
    new_node->next = next;
    new_node->prev = prev;
    return new_node;
}




int Return_Winner(char **argvp,int sock){
  char node_name[1024];
  node* temp_root=PUBLICROOT;
  strcpy(node_name,temp_root->next->name);
  node*allclosed_node=PUBLICROOT;
  counter* temp_counter;
  int found_flag=0;
  int buf_count=0;
  char buf_name[1024];
  memset(buf_name,'\0',sizeof(buf_name));
  char buffer[1024];
  memset(buffer,'\0',strlen(buffer)+1);
  while(temp_root->next!=NULL){
    temp_root=temp_root->next;
    if(!strcmp(node_name,temp_root->name)){
      found_flag=1;
      break;
    }
  }
  if(found_flag){
    int allclose=1;
    while(allclosed_node->next!=NULL){
      allclosed_node=allclosed_node->next;
      if(allclosed_node->num_children==0){//this is leaf
        if(!((allclosed_node->open==0)&&(allclosed_node->closed==1))){
          allclose=0;
          break;
        }
      }
    }
    if(!((temp_root->open==0)&&(temp_root->closed==1))||(allclose==0)){
      sprintf(buffer,"RO;%s\n",temp_root->name);
      send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
      formatoutput(buffer);
      printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
      return 0;
    }
    aggregate(temp_root,sock);
    temp_counter=temp_root->root_counter;
    while(temp_counter->next!=NULL){
      temp_counter=temp_counter->next;
      if(temp_counter->count > buf_count){
        strcpy(buf_name,temp_counter->name);
        buf_count=temp_counter->count;
      }
    }
    if(temp_root->hasdata){
      sprintf(buffer,"SC;Winner:%s",buf_name);
      send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
      formatoutput(buffer);
      printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
    }
    else{
      sprintf(buffer,"SC;No votes.");
      send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
      formatoutput(buffer);
      printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
    }

  }else{
    sprintf(buffer,"NR;%s",trimwhitespace(node_name));
    send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
    formatoutput(buffer);
    printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);//Region does not exist

    return -1;
  }
}
int aggregate(node* temp_root,int sock){
  //this function caculate the counter of a node, if child has node, add, if not recursiuve
  node* temp_node;
  counter* root_counter;
  int found;
  counter* root_temp_counter;
  counter* child_counter;
  for(int i=0;i<temp_root->num_children;i++){
    temp_node=(temp_root->children)[i];//this is the children
    if(!(temp_node->hasdata)){
      if(temp_node->num_children>0){
        aggregate(temp_node,sock);
      }
      else{
        continue;
      }
    }
    // else{
      root_counter=temp_root->root_counter;//counter of parent
      child_counter=temp_node->root_counter;//counter of child
      while(child_counter->next!=NULL){//all count in a node
        child_counter=child_counter->next;
        root_temp_counter=root_counter;
        found=0;
        if(temp_root->hasdata){
          while((root_temp_counter->next!=NULL)){//all count in parent
            root_temp_counter=root_temp_counter->next;
            if(!strcmp(root_temp_counter->name,child_counter->name)){
              found=1;//found a match
              break;//root_temp_counter is now the match one
            }
          }//
        }
        if(found){//found, root_temp_counter is now the match one
          root_temp_counter->count=root_temp_counter->count+child_counter->count;
        }
        else{//not found, root_temp_counter is the last node
          temp_root->hasdata=1;
          root_temp_counter->next=malloc(sizeof(counter));
          (root_temp_counter->next)->prev=root_temp_counter;
          root_temp_counter=root_temp_counter->next;
          strcpy(root_temp_counter->name,child_counter->name);
          root_temp_counter->count=child_counter->count;
        }
      }
    // }
  }
  return temp_root->hasdata;
}
int Count_Votes(char **argvp,int sock){
  char node_name[1024];
  memset(node_name,'\0',sizeof(node_name));
  strcpy(node_name,trimwhitespace(argvp[1]));
  char buf[1024];
  memset(buf,'\0',sizeof(buf));
  char buffer[1024];
  memset(buffer,'\0',strlen(buffer)+1);
  char output[1024];
  memset(output,'\0',sizeof(output));
  node* temp_root=PUBLICROOT;
  counter* temp_counter;
  int i;
  int flag=0;
  int found_flag=0;
  while(temp_root->next!=NULL){
    temp_root=temp_root->next;
    if(!strcmp(node_name,temp_root->name)){
      found_flag=1;
      break;
    }
  }
  if(found_flag){
    if(temp_root->hasdata){
      temp_counter=temp_root->root_counter;
      while(temp_counter->next!=NULL){
        if(flag){
          strcat(output,",");
        }else{
          flag=1;
        }
        temp_counter=temp_counter->next;
        sprintf(buf,"%s:%d",temp_counter->name,temp_counter->count);
        strcat(output,buf);
      }
      sprintf(buffer,"SC;%s",output);
      send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
      formatoutput(buffer);
      printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);

      return 0;
    }
    else{
      if(aggregate(temp_root,sock)==0){
        sprintf(buffer,"SC;No votes.");
        send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
        formatoutput(buffer);
        printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);

        return 0;
      }
      else{
        temp_counter=temp_root->root_counter;
        while(temp_counter->next!=NULL){
          if(flag){
            strcat(output,",");
          }else{
            flag=1;
          }
          temp_counter=temp_counter->next;
          sprintf(buf,"%s:%d",temp_counter->name,temp_counter->count);
          strcat(output,buf);
        }
        sprintf(buffer,"SC;%s",output);
        send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
        formatoutput(buffer);
        printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
        return 0;
      }
    }
  }
  else{
    sprintf(buffer,"NR;%s",trimwhitespace(argvp[1]));
    send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
    formatoutput(buffer);
    printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);//Region does not exist
    return -1;
  }
}

int Open_Polls(char *name){
  char node_name[1024];
  memset(node_name,'\0',sizeof(node_name));
  strcpy(node_name,trimwhitespace(name));
  node* temp_node=PUBLICROOT;
  int found_flag=0;
  int f;
  while(temp_node->next!=NULL){
    temp_node=temp_node->next;
    if(!strcmp(node_name,temp_node->name)){
      found_flag=1;
      break;
    }
  }
  if(found_flag){
    if((temp_node->open)){
      return 0;
    }
    else if(temp_node->closed){
      return -1;
    }
    else{//can be opened
      temp_node->open=1;
      if(temp_node->num_children>0){
        for(int i=0;i<temp_node->num_children;i++){
          f=Open_Polls(((temp_node->children)[i])->name);
        }
      }

    }
  }
  else{
    return -2;//0 is PF
  }
  return 1;

}

int Add_Votes(char **argvp){
  //this is input,need to parse to get node
  char input[1024];
  memset(input,'\0',sizeof(input));
  strcpy(input,trimwhitespace(argvp[2]));
  char node_name[1024];
  memset(node_name,'\0',sizeof(node_name));
  strcpy(node_name,trimwhitespace(argvp[1]));
  node* temp_node=PUBLICROOT;
  int found_flag=0;
  while(temp_node->next!=NULL){
    temp_node=temp_node->next;
    if(!strcmp(node_name,temp_node->name)){
      found_flag=1;
      break;
    }
  }
  if(found_flag){
    //create counter linked list
    //check if poll opened
    if(temp_node->num_children>0){
      return -3;
    }
    if(temp_node->open==0||temp_node->closed==1){
      return 0;
    }


    counter* temp_counter =temp_node->root_counter;
    //get info to make node
    char delim[] = ",";
    char **res_temp;
    int add_flag=0;
    int numtokens_temp;
    if ((numtokens_temp = makeargv(input, delim, &res_temp)) == -1) {
        fprintf(stderr, "failed to parse the string you given:%s\n", input);
        return -1;       //argvp[0] is the parent node.
    }
    strcpy(delim,":");
    char **res;
    int numstokens;
    for(int i=0;i<numtokens_temp;i++){
      if ((numstokens = makeargv(res_temp[i], delim, &res)) == -1) {
          fprintf(stderr, "failed to parse the string you given:%s\n", res_temp[i]);
          return -1;       //argvp[0] is the parent node.
      }
      //now get useful info of a node
      //create nodes
      add_flag=0;
      while(temp_counter->next!=NULL){
        temp_counter=temp_counter->next;
        if(!strcmp(temp_counter->name,res[0])){
          temp_counter->count=temp_counter->count+atoi(res[1]);
          add_flag=1;
          break;
        }
      }
      if(add_flag==0){
        temp_counter->next=malloc(sizeof(counter));
        (temp_counter->next)->prev=temp_counter;
        temp_counter=temp_counter->next;
        strcpy(temp_counter->name,res[0]);
        temp_counter->count=atoi(res[1]);
      }
    }
    temp_node->hasdata=1;
    temp_counter->next;
    return 1;
  }
  else{
    return -2;
  }

}
int Add_Region(char **argvp,int sock){
  char parentname[1024];
  memset(parentname,'\0',strlen(parentname)+1);
  strcpy(parentname,trimwhitespace(argvp[1]));
  char childname[1024];
  memset(childname,'\0',strlen(childname)+1);
  strcpy(childname,trimwhitespace(argvp[2]));
  char buffer[1024];
  int i;
  memset(buffer,'\0',strlen(buffer)+1);
  node *root = PUBLICROOT;
  node* temp_node=root;
  node* parent_node;
  int found_parent=0;
  while(temp_node->next!=NULL){//find parent node
    temp_node=temp_node->next;
    if(!(strcmp(temp_node->name,parentname))){
      found_parent=1;
      break;
    }
  }
  parent_node=temp_node;
  temp_node=PUBLICROOT;
  if(found_parent){
    while(temp_node->next!=NULL){//find the end of linked list
      temp_node=temp_node->next;//temp node is the last;
    }
    temp_node->next = create(childname, temp_node, NULL);
    temp_node = temp_node->next;
    temp_node ->root_counter=malloc(sizeof(counter));
    parent_node->num_children++;
    node** temp=parent_node->children;
    parent_node->children=(node **)malloc((parent_node->num_children)*sizeof(node*));
    for (i = 0; i < parent_node->num_children-1; i++){
      parent_node->children[i]=temp[i];
    }
    (parent_node->children)[parent_node->num_children-1]=temp_node;
    sprintf(buffer,"SC;%s\n",childname);
    send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
    formatoutput(buffer);
    printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
    return 0;
  }
  else{
    sprintf(buffer,"NR;%s",parentname);
    send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
    formatoutput(buffer);
    printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
    return 0;
  }
}

void Remove_Votes(char **argvp,int sock){//need to change,modify counter,not totally clean
  char input[1024];
  memset(input,'\0',sizeof(input));
  strcpy(input,trimwhitespace(argvp[2]));
  char buffer[1024];
  memset(buffer,'\0',strlen(buffer)+1);
  char node_name[1024];
  memset(node_name,'\0',sizeof(node_name));
  strcpy(node_name,trimwhitespace(argvp[1]));
  node* temp_node=PUBLICROOT;
  int found_flag=0;
  int found2;
  while(temp_node->next!=NULL){
    temp_node=temp_node->next;
    if(!strcmp(node_name,temp_node->name)){
      found_flag=1;
      break;
    }
  }
  if(found_flag){
    if(temp_node->num_children!=0){
      sprintf(buffer,"NL;%s",temp_node->name);
      send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
      formatoutput(buffer);
      printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
      return 0;
    }
    if(temp_node->open==0||temp_node->closed==1){
      sprintf(buffer,"RC;%s",trimwhitespace(argvp[1]));
      send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
      formatoutput(buffer);
      printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
      return 0;
    }
    counter* temp_counter =temp_node->root_counter;
    counter* root_counter =temp_node->root_counter;
    char delim[] = ",";
    char **res_temp;
    int numtokens_temp;
    if ((numtokens_temp = makeargv(input, delim, &res_temp)) == -1) {
        fprintf(stderr, "failed to parse the string you given:%s\n",client_IP, client_port, input);
        sprintf(buffer,"UE;");
        send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
        formatoutput(buffer);
        printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);

        return -1;       //argvp[0] is the parent node.
    }
    strcpy(delim,":");
    char **res;
    int numstokens;
    int is=0;
    memset(buffer,'\0',strlen(buffer));
    sprintf(buffer,"IS;");
    for(int i=0;i<numtokens_temp;i++){
      found2=0;
      if ((numstokens = makeargv(res_temp[i], delim, &res)) == -1) {
          fprintf(stderr, "failed to parse the string you given:%s\n", res_temp[i]);
          sprintf(buffer,"UE;");
          send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
          formatoutput(buffer);
          printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
          return -1;       //argvp[0] is the parent node.
        }
        temp_counter =temp_node->root_counter;
        while(temp_counter->next!=NULL){
          temp_counter=temp_counter->next;
          if(!(strcmp(temp_counter->name,res[0]))){//found2
            found2=1;
            break;
          }
        }
        if((found2==0)||(temp_counter->count<atoi(res[1]))){
          is=1;
          strcat(buffer,res[0]);
          strcat(buffer,",");

        }
    }

    if(is){
      char buf_out[1024];
      memset(buf_out,'\0',strlen(buf_out)+1);
      strncpy(buf_out,buffer,strlen(buffer)-1);
      buf_out[strlen(buffer)-1]='\0';
      send(sock,(const void*)buf_out,strlen(buf_out)+1,NULL);
      formatoutput(buf_out);
      printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buf_out);

      return 0;
    }
    for(int i=0;i<numtokens_temp;i++){
      if ((numstokens = makeargv(res_temp[i], delim, &res)) == -1) {
          fprintf(stderr, "failed to parse the string you given:%s\n", res_temp[i]);
          sprintf(buffer,"UE;");
          send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
          formatoutput(buffer);
          printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);

          return -1;       //argvp[0] is the parent node.
        }
      temp_counter =temp_node->root_counter;
      while(temp_counter->next!=NULL){
        temp_counter=temp_counter->next;
        if(!(strcmp(temp_counter->name,res[0]))){//found2
          found2=1;

          break;
        }
      }
      if((found2==0)||(temp_counter->count<atoi(res[1]))){
        sprintf(buffer,"IS;%s",res[0]);
        send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
        formatoutput(buffer);
        printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
        return 0;
      }
      else{
        temp_counter->count=temp_counter->count-atoi(res[1]);
      }
    }
    sprintf(buffer,"SC;");
    send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
    formatoutput(buffer);
    printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
    return 0;
  }
  else{
    sprintf(buffer,"NR;%s",trimwhitespace(argvp[1]));
    send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
    formatoutput(buffer);
    printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
    return -1;
  }
}

int Close_Polls(char *name){

  char node_name[1024];
  memset(node_name,'\0',sizeof(node_name));
  strcpy(node_name,trimwhitespace(name));
  node* temp_node=PUBLICROOT;
  int found_flag=0;
  int f;
  while(temp_node->next!=NULL){
    temp_node=temp_node->next;
    if(!strcmp(node_name,temp_node->name)){
      found_flag=1;
      break;
    }
  }
  if(found_flag==0){
    return -2;
  }
  if(found_flag){
    if((temp_node->closed)){//already closed before
      return 0;//0 is PF:closing an already closed region
    }
    else if(!(temp_node->open)){//not opend yet
      return -1;
    }
    else if(temp_node->open&&!(temp_node->closed)){
      temp_node->closed=1;
      temp_node->open=0;
      if(temp_node->num_children>0){
        for(int i=0;i<temp_node->num_children;i++){
          f=Close_Polls(((temp_node->children)[i])->name);
          }
        }
      }
      return 1;
    }
}


void Invalid_Code(char **argvp,int sock){
  char buffer[1024];
  memset(buffer,'\0',strlen(buffer)+1);
  sprintf(buffer,"UC;%s",argvp[0]);
  send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
  printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
  formatoutput(buffer);
}


void dispatchCode(char* str,int sock){
  trimlastenter(str);
  char tempb[1024];
  memset(tempb,'\0',strlen(tempb)+1);
  strcat(tempb,str);
  formatinput(tempb);
  fprintf(stdout,"\nRequest received from client at %s:%d,%s\n",client_IP, client_port,trimwhitespace(tempb));
  //parse the Code
  char buffer[1024];
  memset(buffer,'\0',strlen(buffer)+1);
  int numtokens;
  int f;
  char delim[] = ";";
  char **argvp;
  if ((numtokens = makeargv(str, delim, &argvp)) == -1) {
      fprintf(stderr, "failed to parse the string you given:%s\n", str);
      return 1;       //argvp[0] is the parent node.
  }

  switch(str[0]){
	case 'R' :
	        if(str[1] == 'W')
		            Return_Winner(argvp,sock);
	        else if(str[1] == 'V')
		            Remove_Votes(argvp,sock);
	        else
		Invalid_Code(argvp,sock);
	break;
	case 'C' :
	        if(str[1] == 'V')
                Count_Votes(argvp,sock);
	        else if(str[1] == 'P'){
            f=Close_Polls(argvp[1]);
            if(f==1){
              sprintf(buffer,"SC;");
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==0){
              sprintf(buffer,"PF;%s;closed",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==-1){
              sprintf(buffer,"PF;%s;unopened.",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==-2){
              sprintf(buffer,"NR;%s",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
          }

          else
                Invalid_Code(argvp,sock);
	break;
	case 'O' :
          if(str[1] == 'P'){
            f=Open_Polls(argvp[1]);
            if(f==1){
              sprintf(buffer,"SC;");
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==0){
              sprintf(buffer,"PF;%s Open",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==-1){
              sprintf(buffer,"RR;%s",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==-2){
              sprintf(buffer,"NR;%s",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
          }
          else
                Invalid_Code(argvp,sock);
	break;
	case 'A' :
          if(str[1] == 'V'){
            f=Add_Votes(argvp);
            if(f==1){
              sprintf(buffer,"SC;");
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==0){
              sprintf(buffer,"RC;%s",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);

            }
            else if(f==-1){
              sprintf(buffer,"UE;%s",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==-2){
              sprintf(buffer,"NR;%s",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
            else if(f==-3){
              sprintf(buffer,"NL;%s",trimwhitespace(argvp[1]));
              send(sock,(const void*)buffer,strlen(buffer)+1,NULL);
              formatoutput(buffer);
              printf("Sending response to client at %s:%d, %s\n",client_IP, client_port,buffer);
            }
          }

          else if(str[1] == 'R')
                Add_Region(argvp,sock);
          else
                Invalid_Code(argvp,sock);
	break;
	default :
	  Invalid_Code(argvp,sock);
	}
}

void runThread(void* arg)
{
    struct client_info* info = (struct client_info*) arg;
    int sock = info->socket;
    //printf("Running thread on socket %d\n",info->socket);
    int open_and_filled = 0;
    char buffer[1];
    char *buffer_r;
	  buffer_r = malloc(4096);
    strcpy(buffer_r,"");
    int n = -1;
    while(!open_and_filled){
      while ((n=read(sock,buffer,1)) > 0){
		  strcat(buffer_r,buffer);
	    if(buffer[0] == '\0'){
         pthread_mutex_lock(mutex);
	       dispatchCode(buffer_r,sock);
	       strcpy(buffer_r,"");
	       pthread_mutex_unlock(mutex);
	      }
      }
      if(client_port!=0){
        fprintf(stdout,"Closed connection with client at %s,%d\n",client_IP,client_port);
      }
      close(sock);//close socket
      break;
    }


}
int makeargv(const char *s, const char *delimiters, char ***argvp)
{
// this function splits your string with specified delimiter
    int error;
    int i;
    int numtokens;
    const char *snew;
    char *t;

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

    if (strtok(t, delimiters) != NULL)  // count number of tokens in s
        for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

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
        **argvp = strtok(t, delimiters);
        for (i = 1; i < numtokens; i++)
            *((*argvp) + i) = strtok(NULL, delimiters);
    }

    *((*argvp) + numtokens) = NULL;     // put in final NULL pointer

    return numtokens;
}

int trimlastenter(char *filename)
{
    int len = strlen(filename);
    char newname[1024];
    memset(newname, '\0', sizeof(newname));
    if (filename[len - 1] == '\n') {
        strncpy(newname, filename, len - 1);
        memset(filename, '\0', sizeof(filename));
        strcpy(filename, newname);
    }
    return 0;
}

node *createDAG(char *filename){
  FILE *fp;
  //open the inputfile
  chmod(filename, 0700);
  fp = fopen(filename, "r");
  if (fp == NULL) {
      perror("Bad File Path");
  }
  //parse data
  size_t len = MAX_STR_SIZE;
  ssize_t read;
  char *buf = malloc(MAX_STR_SIZE * sizeof(char));//set aside a chunk of memory for the buffer
  if (buf == NULL) {
      perror("MALLOC FAILURE\n");
      _exit(1);
  }
  int linecounter = 0;        //keep a running count of what line we are on
  int numtokens;
  int i;
  char delim[] = ":";
  int flag = 0;
  char **argvp;
  node *temp_node;
  node *temp_root;
  // node *temp_node_nwp;//clean all wild pointer
  node *root = (node *) malloc(sizeof(node));
  temp_node = root;//always the last one in linked list
  temp_root = root;//the root node of each line
  while ((read = getline(&buf, &len, fp)) != -1) {
    if (buf[0] == '#') {    //skip comment lines
        printf("Skipping comment %s \n", buf);
    }
    else{
      if ((numtokens = makeargv(buf, delim, &argvp)) == -1) {
          fprintf(stderr, "failed to parse the string you given:%s\n", buf);
          return 1;       //argvp[0] is the parent node.
      }
      if (linecounter == 0) {
        temp_node->prev = NULL;
        temp_node->next = NULL;
        memset(temp_node->name,'\0',sizeof(temp_node->name));
        strcpy(temp_node->name, argvp[0]);
        temp_root->num_children=numtokens-1;
        (temp_root->children)=(node **)malloc((temp_root->num_children)*sizeof(node*));
        for (i = 1; i < numtokens; i++){
          trimlastenter(argvp[i]);
          temp_node->next = create(argvp[i], temp_node, NULL);
          temp_node = temp_node->next;
          (temp_root->children)[i-1]=temp_node;
        }
      }
      else{//not the first line
        temp_root = root;
        flag = 0;
        //find if this aggregate node exist in linled list
        while (temp_root->next != NULL){
          temp_root = temp_root->next;
          if (!strcmp(temp_root->name, argvp[0])){//this node exist
            flag = 1;
            break;
          }
        }
        if (flag == 0){//not in
          printf("input file error,aggregate node doesn't exist\n");
          exit(1);
        }
        else{//in
          temp_root->num_children=numtokens-1;
          (temp_root->children)=(node **)malloc((temp_root->num_children)*sizeof(node*));
          for (i = 1; i < numtokens; i++) {
              trimlastenter(argvp[i]);
              temp_node->next = create(argvp[i], temp_node, NULL);
              temp_node = temp_node->next;
              (temp_root->children)[i-1]=temp_node;
          }
        }
      }
    }
    linecounter++;
  }


  return root;
}

int checkFileAccess(char *filename)
{
    return (access(filename, F_OK) != -1);
}

void test_dag(){//used to test if dag good
  node *temp=PUBLICROOT;
  node *subtemp;
  int i;
  while(temp->next!=NULL){
    temp=temp->next;
    printf("^^^^^^^^^^^^%s:",temp->name);
    if(temp->num_children==0){
      printf("\n");
    }
    else{
      for(i=0;i<temp->num_children;i++){
        printf("^^^^^^^^^^^%s,",((temp->children)[i])->name);
      }
      printf("\n");
    }
  }
}

int main(int argc, char** argv) {

	if (argc > NUM_ARGS + 1 || argc < NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}
  mutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex,NULL);
  pthread_t test[100];
  //create DAG
  PUBLICROOT=(node*)malloc(sizeof(node));
  node*root=PUBLICROOT;
  node*temp_node;
  int i=0;
  char *filename = malloc(sizeof(argv[1]));
  filename = argv[1];
  int DAG_exists = checkFileAccess(filename);
   if (!DAG_exists) {
       perror("Input FILE does not exist");
       _exit(1);
   }
   temp_node=createDAG(filename);//create DAG data struct.

   root->next=temp_node;
   temp_node=root;
   while(temp_node->next!=NULL){
       temp_node= temp_node->next;
       temp_node->root_counter=malloc(sizeof(counter));//root_counter,empty
   }

  // set Socket
  server_port = atoi(argv[2]);
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		printf("Socket create failed.\n");
		return EXIT_FAILURE;
	}

	{
		int opt = 1;
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof (server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(server_port);
	if (bind(server_socket, (struct sockaddr*) &server_addr, sizeof (server_addr)))
	{
		printf("Bind port %d failed.\n", server_port);
		return EXIT_FAILURE;
	}
	// start listen
	if (listen(server_socket, MAX_CONNECTIONS))
	{
		 printf("Server Listen Failed!");
		 return EXIT_FAILURE;
	}
	fprintf(stdout,"Server listening on port %d\n",server_port);
	// connect to client
	while (1)
	{
    // printf("Waiting for connection.......\n");
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);
		// establish connection
		int client_conn = accept(server_socket, (struct sockaddr*) &client_addr, &length);
    client_port = ntohs(client_addr.sin_port);
		client_IP = malloc(1024);
    client_IP = inet_ntoa(client_addr.sin_addr);
    printf("Connection initiated from client at %s:%d\n", client_IP,client_port);
  	if (client_conn < 0)
		{
			printf("Server Accept Failed!\n");
			return EXIT_FAILURE;
		}
    struct client_info* info = (struct client_info*) malloc(sizeof(struct client_info));
    info->socket = client_conn;
		int pthread_err = pthread_create(&test[i], NULL, (void *)runThread, (void *) info);//new thread
    // fprintf(stdout,"Closed connection with client at %s,%d\n",client_IP,client_port);
		if (pthread_err != 0)
		{
			printf("Create thread Failed!\n");
			return EXIT_FAILURE;
		}
    pthread_join(test[i],NULL);
    i++;
	}
	close(server_socket);
	return (EXIT_SUCCESS);

}
