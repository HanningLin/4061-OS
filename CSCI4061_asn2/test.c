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
#include <ctype.h>
#define MAX_STR_SIZE 1024

char canidate_names[MAX_STR_SIZE][100];
int canidate_counts[100];




int main(){
  char path[]="Who_Won/Region_1/County_1/County_1.txt";
  char newpath = malloc(1024 * sizeof(char));
	//Allocate space for MAX_NODES to node pointer
	//getpath(path,newpath);
  char buf[]="A:2,B:1,C:1";
  int num;
  int **argvp;
  if ((num = makeargv(buf,",", &argvp)) == -1) {
      fprintf(stderr, "failed to parse the string you given:%s\n");
      return 1;
    }
char temp[1024];
  for (int i=0;i<num;i++){
    strcpy(temp,argvp[i]);
    //printf("num is %c\n",argvp[i][2]);
    //printf("num is %c\n",temp[2]);
    printf("num is %d\n",atoi(&temp[2]));
    int yi=9;
    int num = 100;
    char str;
    sprintf(str,"%d",yi);

    printf("itoa:%s\n",str);
}
  return 0;

	//Call execNodes on the root node


}

void getpath(char *path,char *newpath)
{
  char delim[]="/";
  int num;
  strcat(path,"\0");
  int totalnum= strlen(path);
  char **argvp;
  printf("path is %s\ntotalnum is %d\n",path,totalnum);
  if ((num = makeargv(path, delim, &argvp)) == -1) {
      fprintf(stderr, "failed to parse the string you given:%s\n");
      return 1;
    }
    printf("num is %d\n",num);
    printf("%s\n",argvp[num-1]);
  int denum=strlen(argvp[num-1])+1;
  int realnum=totalnum-denum;
  printf("realnum is %d\n",realnum);
  strncpy(newpath,path,realnum);
  return 0;
}

void listdir(const char *name, int indent)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2);
        } else {
            printf("%*s- %s\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
}
