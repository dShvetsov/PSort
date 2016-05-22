#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "log.h"
#include "pserror.h"
#include "psortlib.h"

int merger(int generation, int num, int in_count, int *in, int out, int order, int last)
{
	log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_PARAM | 5, generation, num, in_count, (order == -1), last);
	string_struct *string = (string_struct *) calloc(in_count, sizeof(string_struct));
	if (string == NULL)
	{
		xerror(MEMORY_ERROR, 0);
	}
	int i, live_count = 0, first = 1, max_i;
	int str_cnt = 0;
	for (i = 0; i < in_count; i++)
	{
		if (read(in[i], &string[i].size, sizeof(string[i].size)) == 0)
		{
			string[i].size = -1;
		}
		else
		{
			string[i].buf_size = string[i].size;
			if ((string[i].str = (char *) calloc(string[i].buf_size, sizeof(char))) == NULL)
			{
				xerror(MEMORY_ERROR, 0);
			}
			if(read(in[i], string[i].str, string[i].size) == -1)
			{
				log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 0);
				xerror(READ_WRITE_ERROR, 0);
			}
			live_count++;
			str_cnt++;
		}
	}
	
	while (live_count > 0)
	{
		first = 1;
		max_i = 0;
		for (i = 0; i < in_count; i++)
		{
			if (string[i].size != -1)
			{
				if (first)
				{
					max_i = i;
					first = 0;
				}
				else
				{
					if ((xcmp(order,string[i].str, string[max_i].str) > 0))
					{
						max_i = i;
					}
				}
			}
		}
		if (last)
		{
			printf("%s\n", string[max_i].str);
		}
		else
		{
			if(write(out, &string[max_i].size, sizeof(string[max_i].size)) == -1)
			{
				log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 0);
				xerror(READ_WRITE_ERROR, 0);
			}
			if(write(out, string[max_i].str, string[max_i].size) == -1)
			{
				log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 0);
				xerror(READ_WRITE_ERROR, 0);
			}
		}
		if (read(in[max_i], &string[max_i].size, sizeof(string[max_i].size)) > 0)
		{
			if (string[max_i].size > string[max_i].buf_size)
			{
				string[max_i].buf_size = string[max_i].size;
				if ((string[max_i].str = realloc(string[max_i].str, string[max_i].buf_size)) == NULL)
				{
					xerror(MEMORY_ERROR, 0);
				}
			}
			if(read(in[max_i], string[max_i].str, string[max_i].size) == -1)
			{
				log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 0);
				xerror(READ_WRITE_ERROR, 0);
			}
			str_cnt++;
		}
		else
		{
			string[max_i].size = -1;
			live_count--;
		}
	}
	log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_SUMM | 3, num, generation,  str_cnt);
	return 0;
}
