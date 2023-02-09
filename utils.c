#include <stdio.h>
#include "utils.h"

double Calculate_Percentage(stats_t* s, unsigned long long int* prev_idle,
 unsigned long long int* prev_total )
{
    while(1)
    {
        unsigned long long int non_idle =s->user+s->nice+s->system+s->irq+s->softirq+s->steal;
        unsigned long long int idle=s->idle+s->iowait;
        unsigned long long int total = idle+non_idle;

        unsigned long long int totald = total-*prev_total;
        unsigned long long int idled = idle-*prev_idle;
        *prev_idle=idle;
        *prev_total=total;

        if(totald==0)
        {
            return (double)0.0;
        }
        else
        {
            return 100.0*(double)(totald-idled)/(double)totald;
        }
    } 
}
