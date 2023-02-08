#include <stdio.h>
#include "utils.h"

stats_t stats[10];

void test_Calculate_Percentage_for_first_run(void)
{
    stats[0].user = 10;
    stats[0].nice = 10;
    stats[0].system = 10;
    stats[0].idle = 10;
    stats[0].iowait = 10;
    stats[0].irq = 10;
    stats[0].softirq = 10;
    stats[0].steal = 10;
    stats[0].quest = 10;
    stats[0].guestnice = 10;

    unsigned long long int prev_total = 0;
    unsigned long long int prev_idle = 0;
    float res = Calculate_Percentage(stats, &prev_idle, &prev_total);
    assert(prev_idle == 20);  // 10+10
    assert(prev_total == 80); // 8*10 (without quest and guestnice)
    assert(res > 74 && res < 76);
}

void test_Calculate_Percentage_for_consecutive_runs(void)
{
    stats[0].user = 10;
    stats[0].nice = 10;
    stats[0].system = 10;
    stats[0].idle = 10;
    stats[0].iowait = 10;
    stats[0].irq = 10;
    stats[0].softirq = 10;
    stats[0].steal = 10;
    stats[0].quest = 10;
    stats[0].guestnice = 10;

    unsigned long long int prev_total = 0;
    unsigned long long int prev_idle = 0;
    float res = 0.0;
    res = Calculate_Percentage(stats, &prev_idle, &prev_total);
    assert(prev_idle == 20);  // 10+10
    assert(prev_total == 80); // 8*10 (without quest and guestnice)
    assert(res > 74 && res < 76);

    res = Calculate_Percentage(stats, &prev_idle, &prev_total);
    assert(prev_idle == 20);  // 10+10
    assert(prev_total == 80); // 8*10 (without quest and guestnice)
    assert(res == 0.0);

    stats[0].user = 99; //
    stats[0].nice = 10;
    stats[0].system = 10;
    stats[0].idle = 22; //
    stats[0].iowait = 10;
    stats[0].irq = 10;
    stats[0].softirq = 10;
    stats[0].steal = 10;
    stats[0].quest = 10;
    stats[0].guestnice = 10;

    res = Calculate_Percentage(stats, &prev_idle, &prev_total);
    assert(prev_idle == 32);      // 22+10
    assert(prev_total == 181);    // 6*10+99+22 (without quest and guestnice)
    assert(res < 88.4 && res > 88.0); // 181-80 for total diff and 32-20 for idle diff -> (101-12)/101
}

int main(void)
{
    test_Calculate_Percentage_for_first_run();
    test_Calculate_Percentage_for_consecutive_runs();
    printf("\n    ALL TESTS DONE - SUCCESS    \n");
    return 0;
}