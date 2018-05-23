#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv) {

	if (argc < 2) {

		printf("Wrong number of args, expected 1, given %d", argc - 1);
		exit(1);
	}

	// TODO: Your code goes here.
	int p[2];

		if(pipe(p)==-1)
		{
			fprintf(stderr,"pipe failed.\n");
			exit(0);
		}//creat pip

	pid_t p1=fork();
	if (p1<0)
	{
		fprintf(stderr,"Error: Unable to fork.\n");
		exit(EXIT_FAILURE);
	}
	else if (p1>0)
	{

		int fd;
		close(p[1]);
		char filename[1024];
		ssize_t r;
		char * line = NULL;
		size_t len = 1024;
		wait(NULL);//printf("parent");
		read(p[0],filename,1024);

		int last=0;
		fd= fopen(filename,"r+");
		while ((r = getline(&line, &len, fd)) != -1) {
        //printf("%s", line);
				if(atoi(line)<=last)
				{
					printf("Incorrect!\n");
					exit(0);
				}
				else
				{
					last=atoi(line);
				}
    }
		printf("Correct!\n");


	}
	else
	{
		int fd;
		char filename[]="output.txt";
		int len=strlen(filename);

		fd = open(filename,O_RDWR|O_CREAT,0644);
		if(fd==-1)
		{
						printf("open file error:%m\n");
						exit(-1);
		}

		if(dup2(fd,fileno(stdout))==-1)
			{fprintf(stderr,"Redirect standard output failed");
			exit(1);}
		close(p[0]);

		write(p[1],filename,1024);
		execl("./rec3.o","rec3.o",argv[1],(char*) NULL);
		
	}
}
