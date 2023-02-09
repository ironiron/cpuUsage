#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "utils.h"

static stats_t *stats = NULL;
static double *cpu_percentage = NULL;
static unsigned int cpu_nbr;
static bool close_tasks = false;

static sem_t reader_sem;
static sem_t analyzer_sem;

static void *Reader(void *arg)
{
    (void)arg;
    while (1)
    {
        FILE *file = fopen("/proc/stat", "r");
        if (file == NULL)
        {
            perror("!!!Error opening /proc/stat!!!");
            return 0;
        }

        fscanf(file, "%*[^\n]\n ");// Ignore the first line (cpu total)
        for (unsigned int i = 0; i < cpu_nbr; i++)
        {
            fscanf(file,
                   "%*3s %u %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &stats[i].cpuID, &stats[i].user, &stats[i].nice, &stats[i].system,
                   &stats[i].idle, &stats[i].iowait, &stats[i].irq, &stats[i].softirq,
                   &stats[i].steal, &stats[i].quest, &stats[i].guestnice);
        }
        fclose(file);

        sem_post(&reader_sem);
        if (close_tasks == true)
        {
            break;
        }
        sleep(1);
    }
    return NULL;
}

static void *Analyzer(void *arg)
{
    (void)(arg);
    int sem_val = 0;
    unsigned long long int *prev_idle = malloc(sizeof(unsigned long long int) * (cpu_nbr));
    unsigned long long int *prev_total = malloc(sizeof(unsigned long long int) * (cpu_nbr));
    for (unsigned int i = 0; i < cpu_nbr; i++)
    {
        prev_idle[i] = 0;
        prev_total[i] = 0;
    }

    while (1)
    {
        sem_wait(&reader_sem);
        sem_getvalue(&reader_sem, &sem_val);
        if (sem_val != 0)
        {
            printf("Analyze task stalled for more than 1sec \n");
            return NULL;
        }
        for (unsigned int i = 0; i < cpu_nbr; i++)
        {
            cpu_percentage[i] = Calculate_Percentage(&stats[i], &prev_idle[i], &prev_total[i]);
        }
        sem_post(&analyzer_sem);
        if (close_tasks == true)
        {
            break;
        }
    }
    free(prev_idle);
    free(prev_total);
    return NULL;
}

static void *Printer(void *arg)
{
    (void)(arg);
    int disp_cnt = 0;
    int sem_val = 0;

    // print initial values, so it won't rewrite previous commands in terminal during erasing
    for (unsigned int i = 0; i < cpu_nbr; i++)
    {
        printf("cpu%d perc: % 1.1f \n", i, 0.0);
    }

    while (1)
    {
        sem_wait(&analyzer_sem);
        sem_getvalue(&analyzer_sem, &sem_val);
        // if (sem_val != 0)
        // {
        //     printf("Printer task stalled for more than 1sec");
        //     return NULL;
        // }

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
        printf("\n"); // flush and move cursor up
        printf("\r \033[A ");
        if (close_tasks == true)
        {
            break;
        }
    }
    return NULL;
}
//TODO fix too long exiting - send semaphores
int main()
{
    printf("\n    ---cppUsage program---  \n");

    cpu_nbr = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
    assert(cpu_nbr != 0);
    printf("This machine has %u cpus \n", cpu_nbr);

    // allocate memory for (number of cpus)
    stats = malloc(sizeof(stats_t) * (cpu_nbr));
    if (stats == NULL)
    {
        perror("fatal error cannot allocate memory for program \n");
    }
    cpu_percentage = malloc(sizeof(cpu_percentage) * cpu_nbr);
    if (cpu_percentage == NULL)
    {
        perror("fatal error cannot allocate memory for program \n");
    }
    
    int retval = sem_init(&reader_sem, false, 0);
    assert(!retval);
    retval = sem_init(&analyzer_sem, false, 0);
    assert(!retval);

    printf("    *press Enter to stop program*    \n");

    pthread_t reader_thread;
    pthread_t analyzer_thread;
    pthread_t printer_thread;
    retval = pthread_create(&reader_thread, NULL, Reader, NULL);
    assert(!retval);
    retval = pthread_create(&analyzer_thread, NULL, Analyzer, NULL);
    assert(!retval);
    retval = pthread_create(&printer_thread, NULL, Printer, NULL);
    assert(!retval);

    getchar(); // Wait for Enter key

    close_tasks = true;
    pthread_join(printer_thread, NULL);
    pthread_join(analyzer_thread, NULL);
    pthread_join(reader_thread, NULL);
    free(stats);
    free(cpu_percentage);

    printf("\n  Thank You for using this program     \n");
    return 0;
}
