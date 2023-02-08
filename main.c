#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "utils.h"


stats_t *stats = NULL;
float *cpu_percentage=NULL;
long cpu_nbr;

sem_t reader_sem;
sem_t analyzer_sem;


///////////
#include <time.h>

///////


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

       ////////// // Ignore the first line (cpu total)
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
}



void *Analyzer(void *arg)
{
    int sem_val = 0;
    (void)(arg);
///
srand(time(NULL));   // Initialization, should only be called once.

///
    assert(cpu_nbr!=0);
    unsigned long long int* prev_idle = malloc(sizeof(unsigned long long int) *(cpu_nbr+1));
    unsigned long long int* prev_total = malloc(sizeof(unsigned long long int) *(cpu_nbr+1));
    cpu_percentage=malloc(sizeof(float) *(cpu_nbr+1));;
    for(int i=0;i<cpu_nbr+1;i++)
    {
        prev_idle[i]=0;
        prev_total[i]=0;
        cpu_percentage[i]=-200;
    }

    while (1)
    {
        sem_wait(&reader_sem);
        sem_getvalue(&reader_sem, &sem_val);
        if (sem_val != 0)
        {
            printf("Analyze task stalled for more than 1sec");
            return NULL;
        }
        // printf(" cpuID: \n");
        // printf("-%d-", stats[1].cpuID);
        // printf("-%d-", stats[2].cpuID);
        // printf("-%d-", stats[3].cpuID);
        // printf("-%d-", stats[4].cpuID);
        // printf(" user:  \n");
        // printf("-%llu-", stats[1].user);
        // printf("-%llu-", stats[2].user);
        // printf("-%llu-", stats[3].user);
        // printf("-%llu-", stats[4].user);

        for (int i=0;i<cpu_nbr+1;i++)
        {
            // cpu_percentage[i]=rand();
            cpu_percentage[i]=Calculate_Percentage(&stats[i], &prev_idle[i], &prev_total[i]);
        }
        // unsigned int r1=Calculate_Percentage(&stats[1], &prev_idle[1], &prev_total[1]);
        // unsigned int r2 =Calculate_Percentage(&stats[2], &prev_idle[2], &prev_total[2]);
        // printf("\n r1: %u \n",r1);
        // printf("\n r2: %u \n",r2);
        // sleep(3);
        sem_post(&analyzer_sem);
    }
}

void* Printer(void *arg)
{
    int sem_val = 0;
    (void)(arg);
        for (int i=1;i<cpu_nbr+1;i++)
        {
            printf("cpu%d perc: % 1.1f \n",i-1,0.0);
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
// printf("aaaaaaaaaa");
// printf("bbbbbb");
// printf("ccccccc");
// printf("dddddddd");
// printf("\r \33[2K  \033[A ");
// printf("\r \33[2K  \033[A ");
// sleep(2);
// clean previous entries
                for (int i=1;i<cpu_nbr+1;i++)
        {
            printf("\r \33[2K  \033[A \r");
        }
        for (int i=1;i<cpu_nbr+1;i++)
        {
            printf("cpu%d perc: % 1.1f \n",i-1,cpu_percentage[i]);
        }
    }
}


int main()
{
    // int a=0;
    printf("Hello World\n");

    cpu_nbr = sysconf(_SC_NPROCESSORS_ONLN);
    printf("number of proc %lu \n", cpu_nbr);
    // allocate memory for (number of cpus + one more for total cpu usage)
    stats = malloc(sizeof(stats_t) * (cpu_nbr + 1));
    if (stats == NULL)
    {
        perror("fatal error cannot allocate memory for program1");
    }

        cpu_percentage = malloc(sizeof(unsigned int) *cpu_nbr);
    if (cpu_percentage == NULL)
    {
        perror("fatal error cannot allocate memory for program2");
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