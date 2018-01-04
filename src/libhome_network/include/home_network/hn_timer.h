/**
 * @file hn_timer.h
 * @brief TODO.
 *
 */

#ifndef HN_TIMER_H
#define HN_TIMER_H

#include <signal.h>
#include <time.h>

typedef void (*hn_timer_cb)(union sigval data);

typedef struct
{
    timer_t timer_id;
    struct sigevent event;
} hn_timer_s;

int hn_timer_create(
        hn_timer_cb cb,
        void * const cb_data,
        hn_timer_s * const timer);

int hn_timer_destroy(
        hn_timer_s * const timer);

int hn_timer_set(
        const struct itimerspec * const spec,
        hn_timer_s * const timer);

#endif /* HN_TIMER_H */

