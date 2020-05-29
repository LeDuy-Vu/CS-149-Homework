#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXLINE 80 /* The maximum length of a command line */

int main(void)
{
	char input[MAXLINE] ; // string to store command line
	char *args[MAXLINE / 2 + 1] ; //command line with max 40 arguments
	int should_run = 1 ; // flag to determine when to exit program
	int pid ;
	char *last ; // handle string modification
	printf("CS149 Shell from Le Duy Vu\n") ;

	while (should_run)
	{
		printf("LeDuy-539> ") ; //prompt
		fflush(stdout) ;
		fgets(input, sizeof input, stdin) ; //assign command to input

		//handle empty line and exit command
		if (!strcmp(input, "\n"))
			continue;
		if (!strcmp(input, "exit\n"))
		{
			should_run = 0 ;
			continue ;
		}

		//remove new line symbol at the end of command line
		last = strrchr(input, '\n') ;
		*last = 0 ;

		//remove '&' (if exist)
		last = strrchr(input, ' ') ;
		if (last != NULL)
		{
			if (!strcmp(last + 1, "&"))
			{
				*last = 0 ;
				last++ ;
			}
			else
				last = NULL ;
		}

		pid = fork() ; //create new process
		//handle fork fail
		if (pid < 0)
		{
			puts("Fork() failed. Please try again") ;
			continue ;
		}

		if (pid == 0) //child process
		{
			//tokenize command line
			int i = 0 ;
			char *ptr = strtok(input, " ") ;
			while (ptr != NULL)
			{
				//handle too many arguments
				if (i == MAXLINE / 2)
				{
					puts("Too many arguments. Please try a different command") ;
					exit(1) ;
				}

				args[i] = ptr ;
				i++ ;
				ptr = strtok(NULL, " ") ;
			}
			args[i] = 0 ; //put 0 as the last element of args[]
			execvp(args[0], args) ; //exec command

			//handle exec fail
			puts("Exec() error. Please try again") ;
			exit(1) ;
		}
		else //parent process
			//if there's no &, wait
			if (last == NULL)
				wait(NULL) ;
	}

	return 0;
}
