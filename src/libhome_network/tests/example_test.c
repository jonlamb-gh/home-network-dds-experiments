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
    printf("  'hn_destroy()'\n");
    DDS_ReturnCode_t ret_d = hn_destroy(&participant);
    printf("    %d\n", (int) ret_d);
    printf("<----------\n");

    return (int) (ret_c | ret_e | ret_t | ret_d);
}
