/**
 ******************************************************************************
 * @file    utils.h
 * @author  Rafał Mazurkiewicz
 * @brief   Utilities header file
 ******************************************************************************
 */

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

#define THREADS_NBR 3 ///< number of threads excluding watchdog
typedef enum
{
    READER = 0,
    ANALYZER = 1,
    PRINTER = 2,
} task_t; ///< Used to identifie thread make sure that macro THREADS_NBR has the same number of threads

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
} stats_t; ///< Holds info from /proc/stat file

/// @brief Calculates CPU usage based on previous and current stats and 'prev_idle' and 'prev_total' with current values
/// @param s current readings. Can be of value stats_t
/// @param prev_idle previous idle time measured in units of USER_HZ (refer to linux man)
/// @param prev_total previous total time measured in units of USER_HZ (refer to linux man)
/// @return cpu usage in percents
double Calculate_Percentage(stats_t *s, unsigned long long int *prev_idle,
                            unsigned long long int *prev_total);
/// @brief Functions should be used only by a thread to signalize that it is running
/// @param t thread ID should be of type task_t
void Send_Thread_Alive_Sig(task_t t);
/// @brief Returns status flag of a thread
/// @param t thread ID should be of type task_t
bool Get_Thread_Alive_Status(task_t t);
/// @brief After invoking this function status flag is cleared and thread has no external signs of running
/// @param t thread ID should be of type task_t
void Clear_Thread_Alive_Status(task_t t);
