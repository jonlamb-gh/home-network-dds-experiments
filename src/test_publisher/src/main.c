/**
 * @file main.c
 * @brief TODO.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "home_network/home_network.h"

#include "sensor.h"
#include "sensorPlugin.h"
#include "sensorSupport.h"

#define WRITE_INTERVAL (500ULL)

static volatile sig_atomic_t exit_signaled = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void sigint_handler(int sig)
{
    (void) sig;
    exit_signaled = 1;
}

static void timer_callback(union sigval data)
{
    int status;

    status = pthread_mutex_lock(&mutex);

    if(status == 0)
    {
        status = pthread_cond_signal(&cond);
    }

    if(status == 0)
    {
        status = pthread_mutex_unlock(&mutex);
    }

    if(status != 0)
    {
        exit(1);
    }
}

static DDS_ReturnCode_t create_publisher(
        sensor_infoDataWriter ** const type_dw_ref,
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;
    DDS_DataWriter *dw_ref = NULL;
    struct DDS_DataWriterListener dw_listener =
            DDS_DataWriterListener_INITIALIZER;

    if(ret == DDS_RETCODE_OK)
    {
        ret = hn_create_topic(
                "sensor_info",
                sensor_infoTypePlugin_get(),
                participant);
    }

    if(ret == DDS_RETCODE_OK)
    {
        ret = hn_create_publisher(
                "sensor_info",
                DDS_RELIABLE_RELIABILITY_QOS,
                &dw_listener,
                DDS_STATUS_MASK_NONE,
                &dw_ref,
                participant);
    }

    if(ret == DDS_RETCODE_OK)
    {
        *type_dw_ref = sensor_infoDataWriter_narrow(dw_ref);

        if(*type_dw_ref == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return ret;
}

int main(int argc, char **argv)
{
    int ret = 0;
    DDS_ReturnCode_t dds_ret = DDS_RETCODE_OK;
    sensor_infoDataWriter *type_dw = NULL;
    struct sigaction act;
    hn_participant_s participant;
    hn_timer_s timer;

    const struct itimerspec spec =
    {
        .it_value.tv_sec = 0,
        .it_value.tv_nsec = (1000ULL * 1000ULL * WRITE_INTERVAL),
        .it_interval.tv_sec = 0,
        .it_interval.tv_nsec = (1000ULL * 1000ULL * WRITE_INTERVAL)
    };

    (void) memset(&participant, 0, sizeof(participant));
    (void) memset(&timer, 0, sizeof(timer));

    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_RESTART;
    act.sa_handler = sigint_handler;

    ret = sigaction(SIGINT, &act, 0);

    if(ret == 0)
    {
        ret = hn_timer_create(
                timer_callback,
                NULL,
                &timer);
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = hn_create(&participant);
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = create_publisher(
                &type_dw,
                &participant);
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = hn_enable(&participant);
    }

    sensor_info * const sample = sensor_info_create();

    if(sample == NULL)
    {
        dds_ret = DDS_RETCODE_OUT_OF_RESOURCES;
    }

    if(ret == 0)
    {
        ret = hn_timer_set(
                &spec,
                &timer);
    }

    if((dds_ret != DDS_RETCODE_OK) || (ret != 0))
    {
        exit_signaled = 1;
    }

    while(exit_signaled == 0)
    {
        ret = pthread_mutex_lock(&mutex);

        if(ret == 0)
        {
            ret = pthread_cond_wait(&cond, &mutex);
            ret |= pthread_mutex_unlock(&mutex);
        }

        if((dds_ret == DDS_RETCODE_OK) && (ret == 0))
        {
            printf("write\n");

            dds_ret = sensor_infoDataWriter_write(
                    type_dw,
                    sample,
                    &DDS_HANDLE_NIL);
        }

        if((dds_ret != DDS_RETCODE_OK) || (ret != 0))
        {
            exit_signaled = 1;
        }
    }

    (void) hn_destroy(&participant);

    (void) hn_timer_destroy(&timer);

    if(dds_ret != DDS_RETCODE_OK)
    {
        if(ret == 0)
        {
            ret = 1;
        }
    }

    printf("\n'%s' exiting - return = %d\n", argv[0], ret);
    (void) fflush(stdout);

    return ret;
}
