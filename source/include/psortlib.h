/**
 * @file psortlib.h
 * @brief Библиотека для функций partitioner, merger, sorter
 */

#ifndef psortlib__h
#define psortlib__h

/** @brief структура для хранения строки в памяти
 * Структура испоьзуется в функциях:
 * - int partitioner(int count_out, int (*fd)[2]);
 * - int merger(int generation, int num, int in_count, int *in, int out, int order, int last);
 * - int sorter(int num, int in, int out, int order);
 * - void swap(string_struct *string, int a, int b);
 */
typedef struct string_struct {
//! @brief размер строки
	int size;
//! @brief  размер буфера, выделенного под строку
	int buf_size;
//! @brief указатель на строку
	char *str;
} string_struct;

/**
 * @brief функция для partitioner-процесс
 * 
 * Функция читает со стандратного потока ввода строки (до перевода строки),
 * и рассылает их sorter-процессам согласно round-robin распределению
 * 
 * @param count_out количество sorter-процессов.
 * 
 * @param fd - указатель на массив из двух элементов, в которых находятся 
 * указатели на дескрипторы неименновых каналов которыми связаны partitioner-процесс и sorter-процессы
 */
extern int partitioner(int count_out, int (*fd)[2]);

/**
 * @brief функция сравнения двух строк
 * 
 * Функция лексиграфически сравнивет две строки, согласно порядку сортировки
 * 
 * @param order порядок сортировки: 1 -- прямой порядок, 2 -- обратный порядок
 * 
 * @param a, b - сравниваемые строки
 * 
 * @return число > 0, если a больше b, < 0, если a меньше b, 0 если a == b
 * согласно порядку сортировки
 */
extern int xcmp(int order, char *a, char *b);

/**
 * @brief функция для merger-процессов
 * 
 * Функция предназначена для merger-процесса
 * Объеденяет in_count последовательностей и пересылает полученную последовательность дальше
 * если merger последний выводит последовательность на стандартный поток вывода
 * Не последний merger-процесс записывает в выходной дескриптор строки, в виде
 * [int][char[]], т.е. сначала идет размер строки, записанный в типе int, а за ним последовательность символов
 * 
 * @param generation поколоение mergera (первое объеденяет последовательности sorter-процессов,
 * merger-процесс последнего поколения выводит последовательность на экран
 * 
 * @param num номер merger-процесса в последовательности
 * 
 * @param in_count количество последовательностей, которые должен объединить merger-процесс
 * 
 * @param in - указатель на массив дескрипторов из которых merger-процесс получает строки
 * 
 * @param out дескриптор в который пишет merger-процесс
 * 
 * @param order порядок в котором merger-процесс объеденяет последовательности
 * 
 * @param last = 1, если merger-процесс последний, 0 иначе 
 */ 
extern int merger(int generation, int num, int in_count, int *in, int out, int order, int last);

/**
 * @brief функция обмена двух элменета массива string
 * 
 * Функция меняет местами в массиве string элементы номер a и b
 * 
 * @param string массив типа string_struct в котором происходит обмен
 * 
 * @param a, b номера элементов в массиве, которые необходимо обменять
 */ 
extern void swap(string_struct *string, int a, int b);

/**
 * @brief Функция sorter-процесса
 * 
 * Функция принмает строки от partitioner-процесса, сортирует их пузырьком 
 * и отправляет их merger-процессу в виде
 * [int][char[]], т.е. сначала идет размер строки, записанный в типе int, а за ним последовательность символов
 * 
 * @param num номер сортера
 * 
 * @param in дескриптор чтения строки
 * 
 * @param out дескриптор записи строк
 * 
 * @param order порядок сортировки (1 - прямой порядок, -1 - обратный порядок
 */ 
extern int sorter(int num, int in, int out, int order);

#endif
