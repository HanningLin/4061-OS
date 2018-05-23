#define _BSD_SOURCE
#define NUM_ARGS 3

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

void shallowCopy(char* old, char* new) {

	// TODO: Create links to all files in old in new.
	DIR* d_new;
	DIR* d;
	struct dirent *dir;
	char oldfilename[1024]="\0";
	char newfilename[1024]="\0";
	if((d=opendir(old))==NULL){
		printf("Old directory don\'t exist!");
	}

	if((d_new=opendir(new))==NULL)//Create new directory
	{
		if(mkdir(new,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1){
			printf("new directory creat failed!");
			exit(0);
		}
		if((d_new=opendir(new))==NULL){
			printf("new directory open failed!");
			exit(0);
		}
	}
	closedir(d_new);
	clock_t start = clock();
	while((dir=readdir(d))!=NULL){
		if(!(dir->d_type==DT_DIR)&&(strcmp(dir->d_name,".")!=0)&&(strcmp(dir->d_name,"..")!=0)){
			strcpy(oldfilename,"\0");
			strcat(oldfilename,old);
			strcat(oldfilename,"/\0");
			strcat(oldfilename,dir->d_name);
			strcpy(newfilename,"\0");
			strcat(newfilename,new);
			strcat(newfilename,"/\0");
			strcat(newfilename,dir->d_name);

			link(oldfilename, newfilename);
		}

	}//go thorough all the files


	clock_t end =(clock()- start)/CLOCKS_PER_SEC;
	printf("Time for shallow copy = %d\n",end);
	closedir(d);
}

void deepCopy(char* old, char* new) {
	//use read and write to do deep copy
	// TODO: Copy the contents of all files in old to new.
	DIR* d;
	DIR* d_new;
	struct dirent *dir;
	char oldfilename[1024]="\0";
	char newfilename[1024]="\0";
	char buff[1024]="\0";
	int len;
  int fd,fd2;
	if((d=opendir(old))==NULL){
		printf("Old directory don\'t exist!");
	}

	if((d_new=opendir(new))==NULL)//creat new directory
	{
		if(mkdir(new,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1){
			printf("new directory creat failed!");
			exit(0);
		}
		if((d_new=opendir(new))==NULL){
			printf("new directory open failed!");
			exit(0);
		}
	}
	closedir(d_new);
	clock_t start = clock();
		while((dir=readdir(d))!=NULL){
			strcpy(buff,"\0");
			if(!(dir->d_type==DT_DIR)&&(strcmp(dir->d_name,".")!=0)&&(strcmp(dir->d_name,"..")!=0)){
				strcpy(oldfilename,"\0");
				strcat(oldfilename,old);
				strcat(oldfilename,"/\0");
				strcat(oldfilename,dir->d_name);
				strcpy(newfilename,"\0");
				strcat(newfilename,new);
				strcat(newfilename,"/\0");
				strcat(newfilename,dir->d_name);
				if((fd = open(oldfilename,O_CREAT|O_RDONLY,0777)));
    		if((fd2 = open(newfilename, O_CREAT|O_WRONLY,0777)));{

				while(len = read(fd,buff,1024))
				{

					//printf("%s",buff);
					if(len == -1) {
						perror("ERROR");
						break;
					}
					write(fd2,buff,sizeof(buff));

					//printf("%d\n", len);
				}
			}
			//printf("%s finished\n",newfilename);
			}
	}
	//printf("HELLO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
	clock_t end =(clock()- start)/CLOCKS_PER_SEC;
	printf("Time for deep copy = %d\n",end);
	closedir(d);
	}

int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {

		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

// TODO: Time how long it takes to execute each copy and print the results.
	shallowCopy(argv[1],argv[2]);
	deepCopy(argv[1],argv[3]);
	return 0;
}
	strcat(oldfilename,dir->d_name);
				strcpy(newfilename,"\0");
				strcat(newfilename,new);
				strcat(newfilename,"/\0");
				strcat(newfilename,dir->d_name);
				if((fd = open(oldfilename,O_CREAT|O_RDONLY,0777)));
    		if((fd2 = open(newfilename, O_CREAT|O_WRONLY,0777)));{

				while(len = read(fd,buff,1024))
				{

					//printf("%s",buff);
					if(len == -1) {
						perror("ERROR");
						break;
					}
					write(fd2,buff,sizeof(buff));

					//printf("%d\n", len);
				}
			}
			//printf("%s finished\n",newfilename);
			}
	}
	//printf("HELLO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
	clock_t end =(clock()- start)/CLOCKS_PER_SEC;
	printf("Time for deep copy = %d\n",end);
	closedir(d);
	}

int main(int argc, char** argv) {

	if (argc < NUM_ARGS + 1) {

		pr