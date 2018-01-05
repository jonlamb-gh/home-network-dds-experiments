/**
 * @file timer_test.c
 * @brief TODO.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "home_network/home_network.h"

#define TIMER_INTERVAL_MS (500ULL)

#define TEST_ADDR ((void*) 0x1234)

static void timer_callback(union sigval data)
{
    printf("\n*timer_callback p = %p*\n\n", data.sival_ptr);

    if(data.sival_ptr != TEST_ADDR)
    {
        exit(1);
    }
}

int main(int argc, char **argv)
{
    hn_timer_s timer;
    struct timespec tspec;

    (void) memset(&timer, 0, sizeof(timer));
    (void) memset(&tspec, 0, sizeof(tspec));

    // 500 ms interval
    const struct itimerspec spec =
    {
        .it_value.tv_sec = 0,
        .it_value.tv_nsec = (1000ULL * 1000ULL * TIMER_INTERVAL_MS),
        .it_interval.tv_sec = 0,
        .it_interval.tv_nsec = (1000ULL * 1000ULL * TIMER_INTERVAL_MS)
    };

    hn_timespec_set_ms(TIMER_INTERVAL_MS, &tspec);

    printf(
            "%llu ms == %lu s : %lu ns\n",
            TIMER_INTERVAL_MS,
            (unsigned long) tspec.tv_sec,
            (unsigned long) tspec.tv_nsec);

    printf("---------->\n");
    printf("  'hn_timer_create()'\n");
    const int ret_c = hn_timer_create(
            timer_callback,
            TEST_ADDR,
            &timer);
    printf("    %d\n", ret_c);
    printf("<----------\n");

    printf("---------->\n");
    printf("  'hn_timer_set()'\n");
    const int ret_s = hn_timer_set(
            &spec,
            &timer);
    printf("    %d\n", ret_s);
    printf("<----------\n");

    (void) sleep(2);

    printf("---------->\n");
    printf("  'hn_timer_destroy()'\n");
    const int ret_d = hn_timer_destroy(&timer);
    printf("    %d\n", ret_d);
    printf("<----------\n");

    return (ret_c | ret_s | ret_d);
}
