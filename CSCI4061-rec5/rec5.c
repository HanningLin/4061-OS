#define _BSD_SOURCE

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




typedef struct file_node{
	char name[1024];
	int owner;
	int size;
	int dtime;
}file;


int printStats(char* path, int max) {
	file *firstfile = (file *)malloc(sizeof(file) * max);
	// TODO: Put code here.
	DIR* d;
	struct dirent *dir;
	struct stat statbuf;
	char next_dir[1024]="\0";
	char filename[1024]="\0";
	char current_dir[1024]="\0";
	long long int totalsize=0;
	float pro_size;
	int realcounter=0;
	d=opendir(path);//inside is the direction
	int counter=0;
	if(d!=NULL)
	{
		while((dir=readdir(d))!=NULL){
		time_t timep;
		time(&timep);
		time_t diff_time;
			if((dir->d_type==DT_DIR)&&(strcmp(dir->d_name,".")!=0)&&(strcmp(dir->d_name,"..")!=0)){//if this is a folder
				strcpy(next_dir,path);
				strcpy(current_dir,next_dir);
				strcat(next_dir,"/\0");
				strcat(next_dir,dir->d_name);
				totalsize=totalsize+printStats(next_dir, max);
			}
			else if(!(dir->d_type==DT_DIR)){
				strcpy(filename,"\0");
				strcat(filename,path);
				strcat(filename,"/\0");
				strcat(filename,dir->d_name);
				stat(filename,&statbuf);
				{
            if (S_ISREG(statbuf.st_mode)){
							if(counter<max){
								strcpy(firstfile[counter].name,dir->d_name);
								firstfile[counter].owner=dir->d_ino;
								firstfile[counter].size=statbuf.st_size;
								diff_time=timep-statbuf.st_atime;
								firstfile[counter].dtime=diff_time;
								counter++;
								realcounter=counter;
							}
							totalsize=totalsize+(long long)statbuf.st_size;
						}
					}
				}
		}
		for(counter=0;counter<realcounter;counter++)
		{
			printf("File Name:%s\n",firstfile[counter].name);
			printf("          Owner:%d\n",firstfile[counter].owner);
			pro_size=(float)firstfile[counter].size/(float)totalsize*100;
			if(totalsize==0)
				printf("          Size:0%%(This is an empty folder)\n");
			else
				printf("          Size:%f%%\n",pro_size);
			printf("          Time access (sec):%d\n",firstfile[counter].dtime);
		}
		closedir(d);
		free(firstfile);
		return totalsize;
		}
		else {
			fprintf(stderr, "Can`t open directory %s\n", dir);
			_exit(1);
	}

}

int main(int argc, char** argv) {

	if (argc < 3) {
		printf("Wrong number of args, expected 2, given %d", argc - 1);
		exit(1);
	}

	printStats(argv[1], atoi(argv[2]));
}
