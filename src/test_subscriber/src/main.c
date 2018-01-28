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

static volatile sig_atomic_t exit_signaled = 0;

static void sigint_handler(int sig)
{
    (void) sig;
    exit_signaled = 1;
}

static DDS_ReturnCode_t create_subscriber(
        sensor_infoDataReader ** const type_dr_ref,
        DDS_DataReader ** const dr_ref,
        struct DDS_DataReaderListener * const dr_listener,
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    if(ret == DDS_RETCODE_OK)
    {
        ret = hn_create_topic(
                "sensor_info",
                sensor_infoTypePlugin_get(),
                participant);
    }

    if(ret == DDS_RETCODE_OK)
    {
        ret = hn_create_subscriber(
                "sensor_info",
                DDS_RELIABLE_RELIABILITY_QOS,
                dr_listener,
                DDS_STATUS_MASK_NONE,
                dr_ref,
                participant);
    }

    if(ret == DDS_RETCODE_OK)
    {
        *type_dr_ref = sensor_infoDataReader_narrow(*dr_ref);

        if(*type_dr_ref == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return ret;
}

static DDS_ReturnCode_t wait_for_data(
        DDS_WaitSet * const waitset,
        struct DDS_ConditionSeq * const active_conditions,
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;
    const struct DDS_Duration_t wait_timeout =
    {
        .sec = 4,
        .nanosec = 0
    };

    ret = DDS_WaitSet_wait(
            waitset,
            active_conditions,
            &wait_timeout);

    return ret;
}

int main(int argc, char **argv)
{
    int ret = 0;
    DDS_ReturnCode_t dds_ret = DDS_RETCODE_OK;
    DDS_DataReader *datareader = NULL;
    sensor_infoDataReader *type_dr = NULL;
    struct DDS_DataReaderListener dr_listener =
            DDS_DataReaderListener_INITIALIZER;
    DDS_StatusCondition *dr_condition = NULL;
    struct DDS_ConditionSeq active_conditions =
            DDS_SEQUENCE_INITIALIZER;
    struct sigaction act;
    hn_participant_s participant;

    (void) memset(&participant, 0, sizeof(participant));

    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_RESTART;
    act.sa_handler = sigint_handler;

    ret = sigaction(SIGINT, &act, 0);

    if(ret != 0)
    {
        dds_ret = DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    DDS_WaitSet * const waitset = DDS_WaitSet_new();

    if(waitset == NULL)
    {
        dds_ret = DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = hn_create(&participant);
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = create_subscriber(
                &type_dr,
                &datareader,
                &dr_listener,
                &participant);
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        if(DDS_ConditionSeq_initialize(&active_conditions) == RTI_FALSE)
        {
            dds_ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        if(DDS_ConditionSeq_set_maximum(&active_conditions, 1) == RTI_FALSE)
        {
            dds_ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dr_condition = DDS_Entity_get_statuscondition(
                DDS_DataReader_as_entity(datareader));

        if(dr_condition == NULL)
        {
            dds_ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = DDS_StatusCondition_set_enabled_statuses(
                dr_condition,
                DDS_DATA_AVAILABLE_STATUS);
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = DDS_WaitSet_attach_condition(
                waitset,
                DDS_StatusCondition_as_condition(dr_condition));
    }

    if(dds_ret == DDS_RETCODE_OK)
    {
        dds_ret = hn_enable(&participant);
    }

    if((dds_ret != DDS_RETCODE_OK) || (ret != 0))
    {
        exit_signaled = 1;
    }

    while(exit_signaled == 0)
    {
        dds_ret = wait_for_data(
                waitset,
                &active_conditions,
                &participant);

        if(dds_ret == DDS_RETCODE_TIMEOUT)
        {
            printf("timeout\n");
            dds_ret = DDS_RETCODE_OK;
        }
        else if(dds_ret == DDS_RETCODE_OK)
        {
            printf("check\n");
        }

        if((dds_ret != DDS_RETCODE_OK) || (ret != 0))
        {
            exit_signaled = 1;
        }
    }

    if(waitset != NULL)
    {
        if(dr_condition != NULL)
        {
            (void) DDS_WaitSet_detach_condition(
                    waitset,
                    DDS_StatusCondition_as_condition(dr_condition));
        }

        (void) DDS_WaitSet_delete(waitset);
    }

    DDS_ConditionSeq_finalize(&active_conditions);

    (void) hn_destroy(&participant);

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
