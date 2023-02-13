/**
 ******************************************************************************
 * @file    main.c
 * @author  Rafał Mazurkiewicz
 * @brief   Cpu usage tracker - program made for interview 
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <assert.h>
#include "utils.h"

//Global variables to exchange data between threads
static stats_t *stats = NULL;
static double *cpu_percentage = NULL;

static unsigned int cpu_nbr; ///< Number of cpus for this process

static sem_t reader_sem;
static sem_t analyzer_sem;

static pthread_t reader_thread;
static pthread_t analyzer_thread;
static pthread_t printer_thread;
static pthread_t watchdog_thread;

/// @brief Performs threads cancelation and free global resources.
/// @param failure Indicates type of deinitialization. If True watchdog task is NOT cancelled.
static void Deinit(bool failure)
{
    pthread_cancel(printer_thread);
    pthread_cancel(reader_thread);
    pthread_cancel(analyzer_thread);
    sem_post(&reader_sem);
    sem_post(&analyzer_sem);
    if (failure == false)
    {
        pthread_cancel(watchdog_thread);
    }
    sleep(1); // give time for closing threads
    free(stats);
    free(cpu_percentage);
}

/// @brief Wrapper to close file, only for pthread_cleanup_push function 
/// @param arg pointer to file
static inline void Close_Wrapper(void *arg)
{
    fclose(arg);
}

static void *Reader(void *arg)
{
    (void)arg;
    while (1)
    {
        FILE *file = fopen("/proc/stat", "r");
        if (file == NULL)
        {
            perror("!!!Error opening /proc/stat!!! \n");
            return NULL;
        }
        pthread_cleanup_push(Close_Wrapper, file)

        fscanf(file, "%*[^\n]\n "); // Ignore the first line (cpu total)
        for (unsigned int i = 0; i < cpu_nbr; i++)
        {
            fscanf(file,
                   "%*3s %u %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &stats[i].cpuID, &stats[i].user, &stats[i].nice, &stats[i].system,
                   &stats[i].idle, &stats[i].iowait, &stats[i].irq, &stats[i].softirq,
                   &stats[i].steal, &stats[i].quest, &stats[i].guestnice);
        }
        pthread_cleanup_pop(1);
        sem_post(&reader_sem);
        Send_Thread_Alive_Sig(READER);
        sleep(1);
    }
    return NULL;
}

static void *Analyzer(void *arg)
{
    (void)(arg);
    unsigned long long int *prev_idle = malloc(sizeof(unsigned long long int) * (cpu_nbr));
    unsigned long long int *prev_total = malloc(sizeof(unsigned long long int) * (cpu_nbr));
    CHECK_MEMORY_ALLOCATION(prev_idle)
    CHECK_MEMORY_ALLOCATION(prev_total)
    pthread_cleanup_push(free, prev_idle)
    pthread_cleanup_push(free, prev_total)

    for (unsigned int i = 0; i < cpu_nbr; i++)
    {
        prev_idle[i] = 0;
        prev_total[i] = 0;
    }

    while (1)
    {
        sem_wait(&reader_sem);

        for (unsigned int i = 0; i < cpu_nbr; i++)
        {
            cpu_percentage[i] = Calculate_Percentage(&stats[i], &prev_idle[i], &prev_total[i]);
        }

        sem_post(&analyzer_sem);
        Send_Thread_Alive_Sig(ANALYZER);
    }
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
    return NULL;
}

static void *Printer(void *arg)
{
    (void)(arg);
    int disp_cnt = 0;

    // print initial values, so it won't rewrite previous commands in terminal during erasing
    for (unsigned int i = 0; i < cpu_nbr; i++)
    {
        printf("cpu%d perc: % 1.1f \n", i, 0.0);
    }

    while (1)
    {
        sem_wait(&analyzer_sem);
        Send_Thread_Alive_Sig(PRINTER);

        for (unsigned int i = 0; i < cpu_nbr; i++)
        {
            printf("\r \33[2K  \033[A \r"); // Erase previous entries
        }
        for (unsigned int i = 0; i < cpu_nbr; i++)
        {
            printf("cpu%d perc: % 1.1f \n", i, cpu_percentage[i]);
        }
        switch (disp_cnt) // Visualize that program is alive
        {
        case 0:
            printf("|");
            disp_cnt++;
            break;
        case 1:
            printf("/");
            disp_cnt++;
            break;
        case 2:
            printf("-");
            disp_cnt++;
            break;
        default:
            printf("\\");
            disp_cnt = 0;
            break;
        }
        printf("\n"); // flush and move cursor up one line
        printf("\r \033[A ");
    }
    return NULL;
}

static void *Watchdog(void *arg)
{
    (void)arg;
    while (1)
    {
        sleep(2);
        for (unsigned int i = 0; i < THREADS_NBR; i++)
        {
            if (Get_Thread_Alive_Status(i) == true)
            {
                Clear_Thread_Alive_Status(i);
            }
            else
            {
                Deinit(true);
                printf("\n########################\n");
                printf("########################\n");
                printf("########################\n");
                printf("Abnormal behaviour detected - program has stalled\n");
                printf("Task ID %d \n", i);
                printf("Shutting down\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/// @brief SIGTERM handler
/// @param signum signal ID
static void terminate(int signum)
{
    if (signum == SIGTERM)
    {
        Deinit(false);
        exit(EXIT_SUCCESS);
    }
}

int main()
{
    int retval;
    cpu_nbr = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
    assert(cpu_nbr != 0);

    stats = malloc(sizeof(stats_t) * (cpu_nbr));
    CHECK_MEMORY_ALLOCATION(stats)
    cpu_percentage = malloc(sizeof(cpu_percentage) * cpu_nbr);
    CHECK_MEMORY_ALLOCATION(stats)

    retval = sem_init(&reader_sem, false, 0);
    CHECK_RETVALUE(retval)
    retval = sem_init(&analyzer_sem, false, 0);
    CHECK_RETVALUE(retval)

    printf("\n    ---cppUsage program---  \n");
    printf("This machine has %u cpus \n", cpu_nbr);
    printf("    *press Enter to stop program*    \n");

    struct sigaction action = {0};
    action.sa_handler = terminate;
    sigaction(SIGTERM, &action, NULL);

    retval = pthread_create(&reader_thread, NULL, Reader, NULL);
    CHECK_RETVALUE(retval)
    retval = pthread_create(&analyzer_thread, NULL, Analyzer, NULL);
    CHECK_RETVALUE(retval)
    retval = pthread_create(&printer_thread, NULL, Printer, NULL);
    CHECK_RETVALUE(retval)
    retval = pthread_create(&watchdog_thread, NULL, Watchdog, NULL);
    CHECK_RETVALUE(retval)

    pthread_detach(reader_thread);
    pthread_detach(analyzer_thread);
    pthread_detach(printer_thread);
    pthread_detach(watchdog_thread);

    getchar(); // wait for user interaction
    Deinit(false);
    printf("\n  Thank You for using this program     \n");
    exit(EXIT_SUCCESS);
}
