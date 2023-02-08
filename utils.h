#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

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

float Calculate_Percentage(stats_t* s, unsigned long long int* prev_idle,
 unsigned long long int* prev_total );

void *Reader(void *arg);
void *Analyzer(void *arg);
void* Printer(void *arg);