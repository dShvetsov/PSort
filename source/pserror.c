#include <stdio.h>
#include <signal.h>
#include "pserror.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "log.h"

char error_msg[] = "\n *** An error has occurred, the response contains incorrect data *** \n \0";

void handle(int s)
{
	signal(SIGUSR2, SIG_IGN);
	close(ps_log_pipe[1]);
	while (wait(NULL) != -1);
	if (write(2, error_msg, sizeof(error_msg)) != -1);
	exit(1);
}

void xerror(int a, int type)
{
	log_send(ps_log_pipe[1], type | PS_LOG_ERROR2 | 1, a);
	/*switch(a)
	{
		case PIPE_ERROR:
			fprintf(stderr, "ERROR WITH PIPE CREATING\n");
			break;
		case FORK_ERROR:
			fprintf(stderr, "ERROR WITH PROCCESS CREATING\n");
			break;
		case DUP_ERROR:
			fprintf(stderr, "ERROR WITH DUPLICATE FILE DESCRIPTOT\n");
			break;
		case MEMORY_ERROR:
			fprintf(stderr, "ERROR  WITH MEMORY\n");
			break;
		case READ_WRITE_ERROR:
			fprintf(stderr, "ERROR WITH READ/WRITE\n");
			break;
	} */
	if (type)
	{
		handle(0);
	}
	else
	{
		kill(getppid(), SIGUSR2);
	}
	exit(1);
}
