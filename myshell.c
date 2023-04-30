#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include <myshell.h>

enum status jobStatus = idle;

int main() 
{
	puts("myshell: program initiated");

	signal(SIGTSTP, handleSIGTSTP);
	
	char input[BUFFER_SIZE];
	char *args[MAX_ARGS_SIZE];
	
	while(1) 
	{
		printPrompt();
		readInput(input);

		if (isJobIdle())
			jobStatus = idle;

		if (isEOF())
			break;
		if (isEmpty(input)) 
			continue;
		if (isFgBg(input) && jobStatus != idle)
		{
			resumeJob(input);
			continue;
		}
			
		if (isPiped(input))
		{
			tokenizeIntoArray(input, args, "|");
			execPipedCmd(args);
		}
		else
		{
			tokenizeIntoArray(input, args, " ");
			execCmd(args);
		}
		// end of while loop
	}

	puts("\nmyshell: program terminated");
	exit(0);
}
