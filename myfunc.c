#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#include <myshell.h>

extern enum status jobStatus;

void handleError(char *victim)
{
	handleErrorWithoutExit(victim);
	exit(EXIT_FAILURE);
}

void handleErrorWithoutExit(char *victim)
{
	char tag[BUFFER_SIZE] = "myshell: ";
	strcat(tag, victim);
	perror(tag);
}

void printPrompt()
{
	char cwd[BUFFER_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL) 
		handleError("getcwd");

	printf("%s%% ", cwd);

	// comment the above printf line and uncomment the below block to print the job status as well as current working directory
	/*
	char *status;
	if (jobStatus == idle)
		status = "idle";
	else if (jobStatus == running)
		status = "running";
	else if (jobStatus == suspended)
		status = "suspended";
	printf("job status: %s\t%s%% ", status, cwd);
	*/

}

void readInput(char *input)
{
	char buffer[BUFFER_SIZE];
	fgets(buffer, sizeof(buffer), stdin);
  	int len = strlen(buffer);
	if (buffer[len-1] == '\n') { buffer[len-1] = '\0'; }
	strcpy(input, buffer);
	
	if (ferror(stdin)) 
		handleError("fgets");
}

bool isEOF() 
{
	bool result = false;
	if (feof(stdin))
	{
		printf("\nmyshell: EOF detected");
		result = true;
	}
	return result;
}

bool isEmpty(char *input)
{
	return strlen(input) == 0;
}

bool isPiped(char *input)
{
	return strstr(input, " | ") != NULL;
}

void tokenizeIntoArray(char *input, char **args, const char *delim)
{
	char *token;
	char *savePtr;
	int i = 0;
	
	clearArgs(args);
	
	token = strtok_r(input, delim, &savePtr);
	while (token != NULL)
	{
		args[i++] = token;
		token = strtok_r(NULL, delim, &savePtr);
	}
}

void clearArgs(char **args)
{
	int i;
	for (i = 0; i < MAX_ARGS_SIZE; i++)
		args[i] = NULL;
}

void execCmd(char **args)
{
	if (isInternalCmd(args))
	{
		execInternalCmd(args);
		return;
	}

	if (jobStatus == suspended || jobStatus == running)
	{
		puts("myshell: cannot start system command while job is unfinished");
		return;
	}


	if (args[0][0] == '.' && args[0][1] == '/')
		jobStatus = running;
	
	int pid, status;
  	switch (pid = fork())
	{
		case ERROR:
			handleError("fork");
			break;
		case 0: // in child process
		  	if (execvp(args[0], args) == ERROR) 
			{
		  		printf("myshell: Error executing '%s'\n", args[0]);
				jobStatus = idle;
			}
			exit(0);
			break;
		default: // in parent process
			waitJob(pid, &status);
	}
}

bool isInternalCmd(char **args)
{
	return 
	(
		strcmp(args[0], "cd") == 0 || 
		strcmp(args[0], "exit") == 0
	);
}

void execInternalCmd(char **args)
{
	char *cmd = args[0];
	if (strcmp(cmd, "cd") == 0)
	{
		change_dir(cmd, args);
	}
	else if (strcmp(cmd, "exit") == 0) 
	{
		printf("myshell: program terminated\n");
		exit(0);
	}
}

void change_dir(char *cmd, char **args)
{
	char *path;
	if (args[1] == NULL || args[2] != NULL)
	{
		perror("cd : wrong number of argument, one argument required");
		return;
	}
	path = args[1];
	if (chdir(path) == ERROR) { handleErrorWithoutExit("chdir"); }
}

void execPipedCmd(char **args)
{
	if (jobStatus == suspended || jobStatus == running)
	{
		puts("myshell: cannot start system command while job is unfinished");
		return;
	}

	int tempin = dup(0);			
	int tempout = dup(1);			
	int i = 0;
	int fdin, fdout; // fd = file descriptor


	fdin = dup(tempin);
	int pid;
	for (i=0; i < args_length(args); i++)
	{
		char *cargs[BUFFER_SIZE]; // cargs = current command or arguments to be exexuted
		tokenizeIntoArray(args[i], cargs, " ");
		dup2(fdin, 0);
		close(fdin);
		
		if (i == args_length(args)-1)
			fdout = dup(tempout);
		else
		{
			int fd[2];
			pipe(fd);
			fdout = fd[1];
			fdin = fd[0];
		}	

		dup2(fdout, 1);
		close(fdout);
		
	  	switch (pid = fork())
		{
			case ERROR:
				handleError("fork");
				break;
			case 0: // in child process
			  	if (execvp(cargs[0], cargs) == ERROR) 
			  		printf("myshell: Error executing '%s'\n", args[0]);
				exit(0);
				break;
			default: // in parent process
				wait(NULL); // wait for the child process to finish
		}
	}

	dup2(tempin, 0);
	dup2(tempout, 1);
	close(tempin);
	close(tempout);
}

int args_length(char **args)
{
	int i = 0;
	while (args[i] != NULL)
		i++;
	return i;
}

void handleSIGTSTP(int sig)
{
	puts("\nmyshell: SIGTSTP detected");
	switch (jobStatus)
	{
		case idle:
			puts("myshell: currently no job to suspend");
			break;
		case running:
			puts("myshell: suspended ongoing job process, type 'fg' to resume");
			jobStatus = suspended;
			break;
		case suspended:
			puts("myshell: cannot start system command while job is unfinished");
	}
		
}

bool isFgBg(char *input)
{
	return 
	(
		strcmp(input, "fg") == 0 ||
		strcmp(input, "bg") == 0
	);
}

void resumeJob(char *input)
{
	kill(0, SIGCONT);
	int status;
	jobStatus = running;
	if (strcmp(input, "fg") == 0)
		waitJob(0, &status);
}

void waitJob(int pid, int *status)
{
	waitpid(pid, status, WUNTRACED);
	if (*status == 0)
		jobStatus = idle;
	else 
		jobStatus = suspended;
}

bool isJobIdle()
{
	return waitpid(0, NULL, WNOHANG) > 0;
}

