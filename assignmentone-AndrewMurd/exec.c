/**************************************************
 *
 *                   exec.c
 *
 *  The exec function that executes a command
 *  pipeline.
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "mysh.h"

/*
 *  Execute a pipeline
 */

int exec(struct Pipeline *line) {
	struct Command *currentCMD = line->commands;	// Initialize pointer to current command in linked list
	int numCommands = 0, counter = 0;
	// Count number of commands in linked list
	while (currentCMD != NULL) {
		numCommands++;
		currentCMD = currentCMD->next;
	}
	currentCMD = line->commands;

	int fds[2 * numCommands];	// Initialize file descriptors for pipes
	// Create pipes
	for(int i = 0; i < numCommands; i++) {
		if(pipe(fds + i*2) < 0) {
            		printf("Error Creating Pipes!");
            		exit(0);
        	}
	}
	// Exit program
	if (strcmp(currentCMD->name, "exit") == 0) exit(0);
	// Change prompt
	if (strcmp(currentCMD->name, "prompt") == 0) {
		strcpy(Prompt, line->commands->args->name);
	} else {
		// If there is no pipeline
		if (numCommands <= 1) {
			int id = fork();	// Create child process
			if (id == 0) {
				char* argumentList[10]; 
				argumentList[0] = currentCMD->name;
				int i = 1;
				// Construct argument array
				struct Arg *currentArg = currentCMD->args;
				while (currentArg != NULL) {
					argumentList[i] = currentArg->name;
					currentArg = currentArg->next;
					i++;
				}
				argumentList[i] = NULL;
				// Handle Input redirection
				if (currentCMD->input != NULL) {
					int inputFile = open(currentCMD->input, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
					if (inputFile == -1) return -1;
					dup2(inputFile, 0);		// Asign output stream to output file's fd
					close(inputFile);
				}
				// Handle Output redirection
				if (currentCMD->output != NULL) {
					int outputFile = open(currentCMD->output, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
					if (outputFile == -1) return -1;
					dup2(outputFile, 1);		
					close(outputFile);
				}
				// Execute command with appropriate arguments
				int statusCode = execvp(currentCMD->name, argumentList); 
				if (statusCode == -1) {
					printf("Process did not terminate correctly!\n");
					exit(0);
				}
			} 
			wait(NULL); // Wait for child process to finish
		} else {
			// Loop over commands in pipeline
			while (currentCMD != NULL) {
				int id1 = fork();
				if (id1 == 0) {
					char* argumentList[10]; 
					argumentList[0] = currentCMD->name;
					int i = 1;

					struct Arg *currentArg = currentCMD->args;
					while (currentArg != NULL) {
						argumentList[i] = currentArg->name;
						currentArg = currentArg->next;
						i++;
					}
					argumentList[i] = NULL;

					if (currentCMD->input != NULL) {
						int inputFile = open(currentCMD->input, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
						if (inputFile == -1) return -1;
						dup2(inputFile, 0);
						close(inputFile);
					}
					if (currentCMD->output != NULL) {
						int outputFile = open(currentCMD->output, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
						if (outputFile == -1) return -1;
						dup2(outputFile, 1);
						close(outputFile);
					}
					// Child process gets input from the previous command, if it's not the first command 
					if (counter > 0) {
						if (dup2(fds[(counter-1)*2], 0) < 0) {
                			printf("Error routing pipe!");
							exit(0);
            			}
					}
					// Child process outputs to next command, if it's not the last command 
					if (currentCMD->next != NULL) {
						if (dup2(fds[counter*2+1], 1) < 0) {
                			printf("Error routing pipe!");
							exit(0);
            			}
					}
					// Close all pipe fds
					for(int i = 0; i < 2*numCommands; i++){
                    	close(fds[i]);
            		}

					int statusCode = execvp(currentCMD->name, argumentList); 
					if (statusCode == -1) {
						printf("Process did not terminate correctly!\n");
						exit(0);
					}
				}
				currentCMD = currentCMD->next;	// Increment the current command
				counter++;
			}
			// Parent closes the pipes and waits for children
			for(int i = 0; i < 2*numCommands; i++)
    				close(fds[i]);

			for(int i = 0; i < 2*numCommands; i++)
        			wait(NULL);
		}
	}
	return(0);
}
