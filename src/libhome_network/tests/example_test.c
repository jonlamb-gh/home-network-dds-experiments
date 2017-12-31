/**
 * @file example_test.c
 * @brief TODO.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "home_network/home_network.h"
#include "sensor.h"
#include "sensorPlugin.h"
#include "sensorSupport.h"

static DDS_ReturnCode_t test_topic(
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
        sensor_info * const sample = sensor_info_create();

        if(sample != NULL)
        {
            sensor_info_delete(sample);
        }
        else
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return ret;
}

static DDS_ReturnCode_t test_publish(
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;
    DDS_DataWriter *dw_ref = NULL;
    sensor_infoDataWriter *type_dw = NULL;
    struct DDS_DataWriterListener dw_listener =
            DDS_DataWriterListener_INITIALIZER;

    if(ret == DDS_RETCODE_OK)
    {
        ret = hn_create_publisher(
                "sensor_info",
                &dw_listener,
                DDS_STATUS_MASK_NONE,
                &dw_ref,
                participant);
    }

    if(ret == DDS_RETCODE_OK)
    {
        type_dw = sensor_infoDataWriter_narrow(dw_ref);

        if(type_dw == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    sensor_info * const sample = sensor_info_create();

    if(sample == NULL)
    {
        ret = DDS_RETCODE_OUT_OF_RESOURCES;
    }

    if(ret == DDS_RETCODE_OK)
    {
        ret = sensor_infoDataWriter_write(
                type_dw,
                sample,
                &DDS_HANDLE_NIL);
    }

    if(sample != NULL)
    {
        sensor_info_delete(sample);
    }

    return ret;
}

static DDS_ReturnCode_t test_subscribe(
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;
    DDS_DataReader *dr_ref = NULL;
    sensor_infoDataReader *type_dr = NULL;
    struct DDS_DataReaderListener dr_listener =
            DDS_DataReaderListener_INITIALIZER;

    if(ret == DDS_RETCODE_OK)
    {
        ret = hn_create_subscriber(
                "sensor_info",
                &dr_listener,
                DDS_STATUS_MASK_NONE,
                &dr_ref,
                participant);
    }

    if(ret == DDS_RETCODE_OK)
    {
        type_dr = sensor_infoDataReader_narrow(dr_ref);

        if(type_dr == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if(ret == DDS_RETCODE_OK)
    {
        // TODO - take
    }

    return ret;
}

int main(int argc, char **argv)
{
    hn_participant_s participant;

    (void) memset(&participant, 0, sizeof(participant));

    printf("---------->\n");
    printf("  'hn_create()'\n");
    DDS_ReturnCode_t ret_c = hn_create(&participant);
    printf("    %d\n", (int) ret_c);
    printf("<----------\n");

    printf("---------->\n");
    printf("  'test_topic()'\n");
    DDS_ReturnCode_t ret_t = test_topic(&participant);
    printf("    %d\n", (int) ret_t);
    printf("<----------\n");

    printf("---------->\n");
    printf("  'hn_enable()'\n");
    DDS_ReturnCode_t ret_e = hn_enable(&participant);
    printf("    %d\n", (int) ret_e);
    printf("<----------\n");

    printf("---------->\n");
    printf("  'test_subscribe()'\n");
    DDS_ReturnCode_t ret_s = test_subscribe(&participant);
    printf("    %d\n", (int) ret_s);
    printf("<----------\n");

    printf("---------->\n");
    printf("  'test_publish()'\n");
    DDS_ReturnCode_t ret_p = test_publish(&participant);
    printf("    %d\n", (int) ret_p);
    printf("<----------\n");

    printf("---------->\n");
    printf("  'hn_destroy()'\n");
    DDS_ReturnCode_t ret_d = hn_destroy(&participant);
    printf("    %d\n", (int) ret_d);
    printf("<----------\n");

    return (int) (ret_c | ret_t | ret_e | ret_p | ret_s | ret_t | ret_d);
}
