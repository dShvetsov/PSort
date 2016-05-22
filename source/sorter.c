#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "log.h"
#include "pserror.h"
#include "psortlib.h"

void swap(string_struct *string, int a, int b)
{
	string_struct tmp;
	tmp.size = string[a].size;
	tmp.str = string[a].str;
	string[a].size = string[b].size;
	string[a].str = string[b].str;
	string[b].size = tmp.size;
	string[b].str = tmp.str;
}

int xcmp(int order, char *a, char *b)
{
	return order * strcmp(b, a);
}

int sorter(int num, int in, int out, int order)
{
	int count, existed_el, is_sort = 0, i, j;
	string_struct *string;
	string = (string_struct *) calloc(4, sizeof(string_struct));
	existed_el = 4;
	for (count = 0; read(in, &string[count].size, sizeof(string[count].size)) > 0; count++)
	{
		if (count >= existed_el - 1)
		{
			existed_el = existed_el * 2 + 1;
			if ((string = realloc (string, existed_el * sizeof(string_struct))) == NULL)
			{
				xerror(MEMORY_ERROR, 0);
			}  
		}
		if ((string[count].str = (char *) calloc(string[count].size + 1, sizeof(char))) == NULL)
		{
			xerror(MEMORY_ERROR, 0);
		}
		string[count].buf_size = string[count].size;
		if (read(in, string[count].str, string[count].size) == -1)
		{
			log_send(ps_log_pipe[1], PS_LOG_SORTER | PS_LOG_ERROR | 0);
			xerror(READ_WRITE_ERROR, 0);
		}
	}
	if (count == 0)
	{
		close(out);
		close(in);
		return 0;
	}
	for (i = 0; i < count; i++)
	{
		is_sort = 1;
		for (j = 0; j < count - 1; j++)
		{
			if (xcmp(order, string[j + 1].str, string[j].str) > 0) //Определение в какую сторону сортировать
			{
				is_sort = 0;
				swap(string, j, j + 1);
			}
		}
		if (is_sort)
			break;
	}
	if (write(out, &string[0].size, sizeof(string[0].size)) == -1)
	{
		log_send(ps_log_pipe[1], PS_LOG_SORTER| PS_LOG_ERROR | 0);
		xerror(READ_WRITE_ERROR, 0);
	}
	if(write(out, string[0].str, string[0].size) == -1)
	{
		log_send(ps_log_pipe[1], PS_LOG_SORTER| PS_LOG_ERROR | 0);
		xerror(READ_WRITE_ERROR, 0);
	}
	for (i = 1; i < count; i++)
	{
		if(write(out, &string[i].size, sizeof(string[i].size)) == -1)
		{
		log_send(ps_log_pipe[1], PS_LOG_SORTER| PS_LOG_ERROR | 0);
		xerror(READ_WRITE_ERROR, 0);
		}
		if(write(out, string[i].str, string[i].size) == -1)
		{
			log_send(ps_log_pipe[1], PS_LOG_SORTER| PS_LOG_ERROR | 0);
			xerror(READ_WRITE_ERROR, 0);
		}
		free(string[i - 1].str);
	}
	close(out);
	close(in);
	free(string[count - 1].str);
	log_send(ps_log_pipe[1], PS_LOG_SORTER | PS_LOG_SUMM | 2, num, count); 
	return 0;
}
