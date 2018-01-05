/**
 * @file main.c
 * @brief TODO.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "home_network/home_network.h"

#include "sensor.h"
#include "sensorPlugin.h"
#include "sensorSupport.h"

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
    hn_participant_s participant;

    (void) memset(&participant, 0, sizeof(participant));

    dds_ret = hn_create(&participant);

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

    while(dds_ret == DDS_RETCODE_OK)
    {
        if(dds_ret == DDS_RETCODE_OK)
        {
            printf("write\n");

            dds_ret = sensor_infoDataWriter_write(
                    type_dw,
                    sample,
                    &DDS_HANDLE_NIL);
        }

        (void) sleep(1);
    }

    (void) hn_destroy(&participant);

    if(dds_ret != DDS_RETCODE_OK)
    {
        ret = 1;
    }

    return ret;
}
