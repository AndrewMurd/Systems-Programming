/******************************************
 *
 *               lab2a
 *
 *  This program illustrates the use of the
 *  execve system call.
 *
 *****************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


extern char **environ;

int main(int argc, char **argv) {
	int pid;
	int ret;
	int status;

	if((pid = fork())) {
		if(pid < 0) {
			printf("Fork error: %s\n",strerror(errno));
			exit(1);
		}
		printf("Wait: %d\n", wait(&status));
	} else {
		int fin = open(argv[1], O_RDONLY);
		int fout = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		dup2(fin, 0);
		dup2(fout, 1);
		ret = execve("lab4a", argv, environ);
		if(ret < 0) {
			printf("Execve failed: %s\n", strerror(errno));
			exit(1);
		}
	}

	exit(0);
}
