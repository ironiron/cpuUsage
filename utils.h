#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define CHECK_MEMORY_ALLOCATION(x)                                           \
    if ((x) == NULL)                                                         \
    {                                                                        \
        printf("ERROR Could not reserve enough memory for %d \n", __LINE__); \
        printf("Shutting down... \n");                                       \
        exit(EXIT_FAILURE);                                                  \
    }

#define CHECK_RETVALUE(x)                                   \
    if ((x) != 0)                                           \
    {                                                       \
        printf("ERROR Problem occured in %d \n", __LINE__); \
        printf("Shutting down... \n");                      \
        exit(EXIT_FAILURE);                                 \
    }

#define THREADS_NBR 3
typedef enum
{
    READER = 0,
    ANALYZER = 1,
    PRINTER = 2,
} task_t;

typedef struct
{
    unsigned int cpuID;
    unsigned long long int user;
    unsigned long long int nice;
    unsigned long long int system;
    unsigned long long int idle;
    unsigned long long int iowait;
    unsigned long long int irq;
    unsigned long long int softirq;
    unsigned long long int steal;
    unsigned long long int quest;
    unsigned long long int guestnice;
} stats_t;

double Calculate_Percentage(stats_t *s, unsigned long long int *prev_idle,
                            unsigned long long int *prev_total);
void Send_Thread_Alive_Sig(task_t t);
bool Get_Thread_Alive_Status(task_t t);
void Clear_Thread_Alive_Status(task_t t);
