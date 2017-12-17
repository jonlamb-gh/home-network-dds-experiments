/**
 * @file example_test.c
 * @brief TODO.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "home_network/home_network.h"

int main(int argc, char **argv)
{
    hn_participant_s participant;

    (void) memset(&participant, 0, sizeof(participant));

    printf("---------->\n");
    DDS_ReturnCode_t ret_c = hn_create(&participant);
    printf("<----------\n");
    
    printf("---------->\n");
    DDS_ReturnCode_t ret_e = hn_enable(&participant);
    printf("<----------\n");
    
    printf("---------->\n");
    DDS_ReturnCode_t ret_d = hn_destroy(&participant);
    printf("<----------\n");

    printf(
            "ret_c %d - ret_e %d - ret_d %d\n",
            (int) ret_c,
            (int) ret_e,
            (int) ret_d);

    return (int) (ret_c | ret_e | ret_d);
}
