/******************************************
 *
 *               lab2a
 *
 *  This program illustrates the use of the
 *  fork system call.
 *
 *****************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv) {
	int pid;
	int i;
	struct timespec request;

	if((pid = fork())) {
		if(pid < 0) {
			printf("Fork error: %s\n",strerror(errno));
			exit(1);
		}
		printf("Parent: child pid: %d\n",pid);
		request.tv_sec = 0;
		request.tv_nsec = 10; //it takes about 0 nanoseconds for child process to start
		nanosleep(&request, NULL);
		for(i=0; i<100; i++) {
			printf("Parent: %d\n",i);
		}
	} else {
		pid = getppid();
		printf("Child: parent pid: %d\n",pid);
		for(i=0; i<100; i++) {
			printf("Child: %d\n",i);
		}
	}

	exit(0);
}
