#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "log.h"
#include "pserror.h"
#include "psortlib.h"


enum {INIT_STR_SIZE = 4};

int partitioner(int count_out, int (*fd)[2])
{
	log_send(ps_log_pipe[1], PS_LOG_PARTITIONER | PS_LOG_PARAM | 1, count_out);
	string_struct string;
	unsigned int i = 0, str_cnt = 0;
	char *hlp;
	string.str = calloc(INIT_STR_SIZE, sizeof(char));
	if (string.str == NULL)
	{
		xerror(MEMORY_ERROR, 0);
	}
	string.buf_size = INIT_STR_SIZE;
	while (fgets(string.str, string.buf_size, stdin) != NULL)
	{
		string.size = strlen(string.str);
		while (string.str[string.size -1] != '\n' && hlp != NULL)
		{
			string.buf_size *=2;
			string.str = realloc(string.str, string.buf_size * sizeof(char));
			if (string.str == NULL)
			{
				xerror(MEMORY_ERROR, 0);
			}
			hlp = fgets(string.str + string.size, string.buf_size - string.size, stdin);
			string.size = strlen(string.str); 
		}
		if (string.str[string.size - 1] == '\n')
			string.str[string.size - 1] = '\0';
		else
		{
			string.size++;
		}
		if (write(fd[i][1], &string.size, sizeof(int)) == -1)
		{
			log_send(ps_log_pipe[1], PS_LOG_PARTITIONER | PS_LOG_ERROR | 0);
			xerror(READ_WRITE_ERROR, 0);
		};
		
		if (write(fd[i][1], string.str, string.size) == -1)
		{
			log_send(ps_log_pipe[1], PS_LOG_PARTITIONER | PS_LOG_ERROR | 0);
			xerror(READ_WRITE_ERROR, 0);
		}
		str_cnt++;
		i = (i + 1) % count_out;
	}
	log_send(ps_log_pipe[1], PS_LOG_PARTITIONER | PS_LOG_SUMM | 1, str_cnt);
	for (i = 0; i < count_out; i++)
		close(fd[i][1]);
	return 0;
}

