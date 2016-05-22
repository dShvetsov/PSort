#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_STR 256

int
    nostrcmp(const void *a,const void *b)
{
    return -strcmp(*(char**)a, *(char **)b);
}

int
    xstrcmp(const void *a, const void *b)
{
    return strcmp(*(char**)a, *(char **)b);
}

int
    main(int argc, char **argv)
{
    int len, numb, k, i;
    long long str_size;
    char *str, **array;
    str_size = SIZE_STR;
    str = calloc(str_size, sizeof(*str));
    if (str == NULL)
        return -1;
    numb = 1;
    k = 0;
    array = calloc(1, sizeof(*array));
    while(fgets(str, str_size, stdin) != NULL) {
        while ((strlen(str) == str_size - 1) && (str[str_size - 2] != '\n')) {
            str = realloc(str, 2 * str_size * sizeof(*str));
            if (fgets(str + str_size - 1, str_size + 1, stdin) == NULL) {
                str[str_size - 1] = '\n';
                str[str_size ] = 0;
            }
            str_size *= 2;
        }
        len = strlen(str) + 1;
        str = realloc(str, len * sizeof(*str));
        array[k] = str;
        k++;
        if (k == numb) {
            numb *= 2;
            array = realloc(array, numb * sizeof(*array));
        }
        str_size = SIZE_STR;
        str = calloc(str_size, sizeof(*str));
    }
    array = realloc(array, k * sizeof(*array));
    if (argc == 1)
        qsort(array, k, sizeof(*array), (&xstrcmp));
    else
        qsort(array, k, sizeof(*array), (&nostrcmp));
    for (i = 0; i < k; i++)
        printf("%s", array[i]);
    return 0;
}

