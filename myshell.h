#ifndef myshell
#define myshell

#define BUFFER_SIZE 256
#define MAX_ARGS_SIZE 101
#define ERROR -1

enum status 
{
	idle = 0,
	running = 1,
	suspended = 2
};

void handleError(char *);
void handleErrorWithoutExit(char *);
void printPrompt(void);
void readInput(char *);
bool isEOF(void);
bool isEmpty(char *);
bool isPiped(char *);
void tokenizeIntoArray(char *, char **, const char *);
void clearArgs(char **);
void execCmd(char **);
bool isInternalCmd(char **);
void execInternalCmd(char **);
void change_dir(char *, char **);
void execPipedCmd(char **);
int args_length(char **);
void handleSIGTSTP(int);
bool isFgBg(char *);
void resumeJob(char *);
void waitJob(int, int *);
bool isJobIdle(void);

#endif

