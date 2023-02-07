#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

long cpu_nbr;
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
} _stats;

_stats *stats = NULL;
sem_t reader_sem;

void *Reader(void *arg)
{
    (void)arg;
    while (1)
    {
        FILE *file = fopen("/proc/stat", "r");
        if (file == NULL)
        {
            perror("Error opening /proc/stat!");
            return 0;
        }

        // Ignore the first line (cpu total)
        for (int i = 0; i < cpu_nbr + 1; i++)
        {
            fscanf(file,
                   "%*3s %u %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &stats[i].cpuID, &stats[i].user, &stats[i].nice, &stats[i].system,
                   &stats[i].idle, &stats[i].iowait, &stats[i].irq, &stats[i].softirq,
                   &stats[i].steal, &stats[i].quest, &stats[i].guestnice);
        }
        sem_post(&reader_sem);

        fclose(file);
        sleep(1);
    }
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    pthread_exit(NULL);
}

void *Analyzer(void *arg)
{
    int sem_val = 0;
    (void)(arg);
    while (1)
    {

        printf("\n WAITING \n");
        sem_wait(&reader_sem);
        sem_getvalue(&reader_sem, &sem_val);
        if (sem_val != 0)
        {
            printf("Analyze task stalled for more than 1sec");
            return NULL;
        }
        printf(" cpuID: \n");
        printf("-%d-", stats[1].cpuID);
        printf("-%d-", stats[2].cpuID);
        printf("-%d-", stats[3].cpuID);
        printf("-%d-", stats[4].cpuID);
        printf(" user:  \n");
        printf("-%llu-", stats[1].user);
        printf("-%llu-", stats[2].user);
        printf("-%llu-", stats[3].user);
        printf("-%llu-", stats[4].user);
        sleep(3);
        // unsigned long long int non_idle =stats.user+stats.nice+stats.system+stats.irq+stats.softirq+stats.steal;
        // unsigned long long int idle=stats.idle+stats.iowait;
        // unsigned long long int totaltime = idle+non_idle;
    }
}

void *Printer(void *arg)
{
    (void)(arg);
    while (1)
    {
    }
}

#include <assert.h>
int main()
{
    // int a=0;
    printf("Hello World\n");

    cpu_nbr = sysconf(_SC_NPROCESSORS_ONLN);
    printf("number of proc %lu \n", cpu_nbr);
    // allocate memory for (number of cpus + cpu in total)
    stats = malloc(sizeof(_stats) * (cpu_nbr + 1));
    if (stats == NULL)
    {
        perror("fatal error cannot allocate memory for program");
    }
    int retval = sem_init(&reader_sem, false, 0);
    assert(!retval);

    // Reader();
    pthread_t reader_thread;
    pthread_t analyzer_thread;
    pthread_t printer_thread;
    retval = pthread_create(&reader_thread, NULL, Reader, NULL);
    assert(!retval);
    retval = pthread_create(&analyzer_thread, NULL, Analyzer, NULL);
    assert(!retval);
    retval = pthread_create(&printer_thread, NULL, Printer, NULL);
    assert(!retval);
    pthread_join(reader_thread, NULL);
    return 0;
}