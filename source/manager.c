/**
 * @file manager.c
 * @brief Основной файл проекта
 */
 
/**
 * @mainpage Программа выполняющая параллельную сортировку строк
 * @autor Денис Швецов
 * МГУ им. М.В. Ломоносова
 * ф-т ВМК
 * группа 205
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "log.h"
#include "pserror.h"
#include "psortlib.h"

/** Вход в программу
 * psort [ключи] <число sorter-процессов>
 * ключи: -m число последовательностей, которые объеденяет merger-процесс (обязательный)
 * -r  сортировка в обратном порядке
 */

int main(int argc, char **argv)
{
	pid_t hlp;
    signal(SIGUSR1, SIG_IGN);
    if (pipe(ps_log_pipe) == -1)
        xerror(PIPE_ERROR, 1);
    signal(SIGUSR1, SIG_IGN);
    if (!(hlp = fork()))
    {
        close(ps_log_pipe[1]);
        logger(ps_log_pipe[0]);
        return 0;
    }
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, handle);
    if (hlp == -1)
        xerror(FORK_ERROR, 1);
    close(ps_log_pipe[0]);
    log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_RUN | 0);
    int option, sorter_cnt = 0, sqnc_cnt = 0, merger_cnt, order = 1, i, j, generation = 0, amount_merger = 0, srt_cnt = 0;
    int (*fdW)[2], (*fdR)[2];
    //На каждом этапе, fdW будет отвечать за указатель на массив (двумерный Ax2) пищущих дискрипторов
    //а fdR за указатель на массив читающих дискрипторов
    while ((option = getopt(argc, argv, "-m:r")) != -1)
    {
        //Разрбираем параметры
        switch (option) {
        case 1:
            sscanf(optarg, "%d", &sorter_cnt);
            srt_cnt = sorter_cnt;
            break;
        case 'm':
            sscanf(optarg, "%d", &sqnc_cnt);
            break;
        case 'r':
            order = -1;
            break;
        case '?':
            fprintf(stderr, "Error: unknown option");
            return -1;
            break;
        }
    }
    
    if (sorter_cnt <= 0 || sqnc_cnt <= 0)
    {
        fprintf(stderr, "Error: didn't define necessary parametr\n");
        return 0;
    }
    log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_PARAM | 3, sorter_cnt, sqnc_cnt, order == -1 );
    if ((fdW = (int (*)[2]) calloc (sorter_cnt, sizeof(int [2]))) == NULL)
    {
        log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
        xerror(MEMORY_ERROR, 1);
    }
    for (i = 0; i < sorter_cnt; i++)
    {
        if (pipe(fdW[i]) != 0)
        {
            log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
            xerror(PIPE_ERROR, 1);
        }
    }
    if (!(hlp = fork()))
    {//partitioner
        log_send(ps_log_pipe[1], PS_LOG_PARTITIONER | PS_LOG_RUN | 0);
        for (i = 0; i < sorter_cnt; i++)
        {
            close(fdW[i][0]);
        }
        partitioner(sorter_cnt, fdW);
        log_send(ps_log_pipe[1], PS_LOG_PARTITIONER | PS_LOG_END | 0);
        return 0;
    }
    if (hlp == -1)
    {
        log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
        xerror(FORK_ERROR, 1);
    }
    fdR = fdW;
    fdW = (int (*)[2]) calloc(sorter_cnt, sizeof(int [2]));
    if (fdW == NULL)
    {
        log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
        xerror(MEMORY_ERROR, 1);
    }
    for (i = 0; i < sorter_cnt; i++)
    {
        close(fdR[i][1]);
        if (pipe(fdW[i]) != 0)
        {
            log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
            xerror(PIPE_ERROR, 1);
        }
    }
    
    for (i = 0; i < sorter_cnt; i++)
    {
        //порождаем sorter ов
        if (!(hlp = fork()))
        {
            log_send(ps_log_pipe[1], PS_LOG_SORTER | PS_LOG_RUN | 1, i + 1);
            int fdrd = dup(fdR[i][0]), fdwr = dup(fdW[i][1]);
            if (fdrd == -1 || fdwr == -1)
            {
                log_send(ps_log_pipe[1], PS_LOG_SORTER | PS_LOG_ERROR | 1, i + 1);
                xerror(DUP_ERROR, 0);
            }
            for (j = 0; j < i; j++)
            {
                close(fdW[j][0]);
            }
            for (j = i; j < sorter_cnt ; j++) //sorter_cnt > merger_cnt always
            {
                close(fdR[j][0]);
                close(fdW[j][1]);
                close(fdW[j][0]);
            }
            free(fdW);
            free(fdR); // освобождаем неиспользуюмую память
            sorter(i, fdrd, fdwr, order);
            log_send(ps_log_pipe[1], PS_LOG_SORTER | PS_LOG_END | 1, i + 1);
            return 0;
        }
        if (hlp == -1)
        {
            log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
            xerror(FORK_ERROR, 1);
        }
        close(fdR[i][0]);
        close(fdW[i][1]);
    }
    free(fdR);
    //Далее считаем что sorter_cnt это количество отправляющих процессов
    // merger_cnt  количество принимающих процессов
    merger_cnt = sorter_cnt / sqnc_cnt + (sorter_cnt % sqnc_cnt != 0);
    do
    {
        generation++;
        fdR = fdW;
        fdW = (int (*)[2]) calloc (merger_cnt, sizeof(int [2]));
        if (fdW == NULL)
        {
            log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
            xerror(MEMORY_ERROR, 1);
        }
        for (i = 0; i < merger_cnt; i++)
        {
            if (pipe(fdW[i]) != 0)
            {
                log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
                xerror(PIPE_ERROR, 1);
            }
        }
        for (i = 0; i < merger_cnt; i++)
        {
            if (!(hlp = fork()))
            {//Merger
                int fdwr = dup(fdW[i][1]), *fdrd, number_in = 0;
                if (fdwr == -1)
                {
                    log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 2, generation, i + 1);
                    xerror(DUP_ERROR, 0);
                }
                if ((fdrd = (int *) calloc(sqnc_cnt, sizeof(int))) == NULL)
                {
                    log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 2, generation, i + 1);
                    xerror(MEMORY_ERROR, 0);
                }
                for (j = i; j < sorter_cnt; j += merger_cnt)
                {
                    if ((fdrd[number_in] = dup(fdR[j][0])) == -1)
                    {
                        log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 2, generation, i + 1);
                        xerror(DUP_ERROR, 0);
                    }
                    number_in++;
                }
                log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_RUN | 2, generation, i + 1);
                //закрываем все дескрипторы
                for (j = 0; j < i; j++)
                {
                    close(fdW[j][0]);
                }
                for (j = i; j < merger_cnt; j++)
                {
                    close(fdW[j][0]);
                    close(fdW[j][1]);
                    close(fdR[j][0]);
                }
                for (j = merger_cnt; j < sorter_cnt; j++)
                {
                    close(fdR[j][0]);
                }
                //освобождаем память мерждера
                free(fdR);
                free(fdW);
                merger(generation, i + 1, number_in, fdrd, fdwr, order, 0);
                log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_END | 2, generation, i + 1);
                return 0;
            }
            if (hlp == -1)
            {
                log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
                xerror(FORK_ERROR, 1);
            }
            close(fdR[i][0]);
            close(fdW[i][1]);
        }
        for (i = merger_cnt; i < sorter_cnt; i++)
        {
            close(fdR[i][0]);
        }
        free(fdR);
        log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_BURN | 2, generation, merger_cnt);
        amount_merger += merger_cnt;
        sorter_cnt = merger_cnt;
        merger_cnt = sorter_cnt / sqnc_cnt + (sorter_cnt % sqnc_cnt != 0);
    } while (merger_cnt != 1);

    if (!(hlp = fork()))
    {
        int *fdrd;
        if ((fdrd = (int *) calloc(sorter_cnt, sizeof(int))) == NULL)
        {
            log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_ERROR | 2, generation + 1, 1);
            xerror(MEMORY_ERROR, 0);
        }
        for (i = 0; i < sorter_cnt; i++)
        {
            if ((fdrd[i] = dup(fdW[i][0])) == -1)
                xerror(DUP_ERROR, 0);
            close(fdW[i][1]);
            close(fdW[i][0]);
        }
        free(fdW);
        log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_RUN | 2, generation + 1, 1);
        merger(generation + 1, 1, sorter_cnt, fdrd, 1, order, 1);
        log_send(ps_log_pipe[1], PS_LOG_MERGER | PS_LOG_END | 2, generation + 1, 1);
        return 0;
    }
    if (hlp == -1)
    {
        log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_ERROR | 0);
        xerror(PIPE_ERROR, 1);
    }
    log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_BURN | 2, generation + 1, 1);
    amount_merger++;
    for (i = 0; i < sorter_cnt; i++)
    {
        close(fdW[i][0]);
        close(fdW[i][1]);
    }
    log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_SUMM | 3, srt_cnt, amount_merger, generation);
    for (i = 0; i < 1 + srt_cnt + amount_merger; i++)
        wait(NULL);
    log_send(ps_log_pipe[1], PS_LOG_MANAGER | PS_LOG_END | 0);
    close(ps_log_pipe[1]);
    wait(NULL);
    return 0;
}
