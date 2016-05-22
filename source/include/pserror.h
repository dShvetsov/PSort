/**
 * @file pserror.h
 * @brief Библиотека с описаниями для файла pserror.c
 */

#ifndef pserror__h
#define pserror__h

/**
 * @brief обрабтка ошибок
 * 
 * На поток ошибок пишется сообщение об ошибке 
 *
 * @param error_type Тип ошибки
 * 
 * @param procces_type Тип процесса, в котором произошла ошибка
 * 1 -- процесс manager
 * 0 -- другие процессы
 */ 
extern void xerror(int error_type, int process_type);

/**
 * @brief обработчик сигнала SIGUSR2 для процесса manager
 * 
 * manager посылает всем сигнал SIGUSR1, и выводит сообщение, что выходные данные
 * могут содержать ошибки
 */

extern void handle(int s);


/** @brief коды ошибок */
enum {
/** @brief ошибка связанная с выделение память  */
	MEMORY_ERROR,
/** @brief ошибка связанная с созданием неименнового канала*/
	PIPE_ERROR,
/** @brief ошибка связанная с порождением процесса*/
	FORK_ERROR,
/** @brief ошибка связанная с дублированием файлового дескриптора*/
	DUP_ERROR,
/** @brief ошибка связанная с чтением записью */
	READ_WRITE_ERROR
};

extern pid_t pid_log;

#endif
