/**
 * @file home_network.h
 * @brief TODO.
 *
 * access dp GUID:
 * https://community.rti.com/forum-topic/accessing-domainparticipants-guid
 *
 */

#ifndef HOME_NETWORK_H
#define HOME_NETWORK_H

#include "rti_me_c.h"
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"
#include "netio/netio_udp.h"

#ifndef HN_TOPICS_MAX
#define HN_TOPICS_MAX (32)
#endif

// arbitrary limitation, type name == topic name
typedef struct
{
    DDS_Topic *topics[HN_TOPICS_MAX];
    char names[HN_TOPICS_MAX][256];
    DDS_UnsignedLong len;
} hn_topics_s;

typedef struct
{
    DDS_DomainParticipant *dp;
    struct DDS_GUID_t dp_guid; // TODO
    hn_topics_s topic_data;
} hn_participant_s;

DDS_ReturnCode_t hn_create(
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_enable(
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_create_topic(
        const char * const name,
        struct NDDS_Type_Plugin * const plugin,
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_unregister_types(
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_destroy(
        hn_participant_s * const participant);

#endif /* HOME_NETWORK_H */

