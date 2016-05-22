#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <time.h>
#include "log.h"
#include "pserror.h"

int ps_log_pipe[2];

void log_send(int fd, int msgid, ...)
{
	ps_log_prm_t prm[256];
	int i , *pnt = &msgid;
	prm[0].p = msgid;
	prm[1].t = time(NULL);
	for (i = 2; i < LOGPRMCNT(msgid) + 2; i++)
	{
		pnt ++;
		prm[i].p = *pnt;
	}
	if(write(fd, prm, sizeof(ps_log_prm_t) * i) == -1)
		xerror(READ_WRITE_ERROR, 0);
} 

void gettime(char *s, time_t tim)
{
	struct tm *tm = localtime(&tim);
	sprintf(s, "%02d:%02d:%02d %02d.%02d.%d " , tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
}

void logger(int in)
{
	int file = open("psort.log", O_CREAT | O_WRONLY | O_TRUNC, 0666);
	int i, hlp = 0;
	if (dup2(file, 1) == -1)
	{
		xerror(DUP_ERROR, 0);
	}
	close(file);
	unsigned msgid, prm[32];
	char msg_time[32], msg[256];
	time_t tim;
	while (read(in, &msgid, sizeof(ps_log_prm_t)) > 0)
	{
		if(read(in, &tim, sizeof(ps_log_prm_t)) == -1)
		{
			printf("/nlog : ERROR\n");
			xerror(READ_WRITE_ERROR,0);
		}
		gettime(msg_time, tim);
		sprintf(msg, "%s ", msg_time);
		switch (LOGPRTYPE(msgid))
		{
			case PS_LOG_MANAGER:
				sprintf(msg, "manager         : ");
				hlp = 0;
				break;
			case PS_LOG_PARTITIONER:
				sprintf(msg, "partitioner     : ");
				hlp = 0;
				break;
			case PS_LOG_SORTER:
				if(read(in, prm, sizeof(ps_log_prm_t)) == -1)
				{
					printf("/nlog : ERROR\n");
					xerror(READ_WRITE_ERROR,0);
				}
				sprintf(msg, "sorter [%d]     : ", prm[0]);
				hlp = 1;
				break;
			case PS_LOG_MERGER:
				if(read(in, prm, sizeof(ps_log_prm_t) * 2) == -1)
				{
					printf("/nlog : ERROR\n");
					xerror(READ_WRITE_ERROR,0);
				}
				sprintf(msg, "merger [%d - %d] : ", prm[0], prm[1]);
				hlp = 2;
				break;
		}
		switch(LOGMSGTYPE(msgid))
		{
			case PS_LOG_RUN:
				sprintf(msg, "%s run", msg);
				break;
			case PS_LOG_END:
				sprintf(msg, "%s end", msg);
				break;
			case PS_LOG_PARAM:
				sprintf(msg, "%s have parametrs : ", msg);
				if(read(in, prm, sizeof(ps_log_prm_t) * (LOGPRMCNT(msgid) - hlp)) == -1)
				{
					printf("/nlog : ERROR\n");
					xerror(READ_WRITE_ERROR,0);
				}
				for (i = 0; i < LOGPRMCNT(msgid) - hlp; i++)
				{
					sprintf(msg, "%s %d", msg, prm[i]);
				} 
				break;
			case PS_LOG_BURN:
				if (read(in, prm, sizeof(ps_log_prm_t) * 2) == -1)
				{
					printf("/nlog : ERROR\n");
					xerror(READ_WRITE_ERROR,0);
				}
				sprintf(msg, "%s created %d generation of mergers (%d mergers)", msg, prm[0], prm[1]);
				break;
			case PS_LOG_SUMM:
				if (LOGPRTYPE(msgid) == PS_LOG_MANAGER)
				{
					if (read(in, prm, sizeof(ps_log_prm_t) * 3) == -1)
					{
						printf("/nlog : ERROR\n");
						xerror(READ_WRITE_ERROR,0);
					}
					sprintf(msg, "%s (summary) created:  %d sorters, %d mergers from %d genetaions", msg, prm[0], prm[1], prm[2]);
				}
				else
				{
					if (read(in, prm, sizeof(ps_log_prm_t)) == -1)
					{
						printf("/nlog : ERROR\n");
						xerror(READ_WRITE_ERROR,0);
					}
					sprintf(msg, "%s prccess %d strings", msg, prm[0]);
				}
				break;
			case PS_LOG_ERROR:
				sprintf(msg, "%s !ERROR!", msg);
				break;
			case PS_LOG_ERROR2:
				
		}
		printf("%s %s\n",msg_time, msg);
	}
	return;
}
