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

#define TOPIC_STRING_MAX (256)

#ifndef HN_TOPICS_MAX
#define HN_TOPICS_MAX (32)
#endif

#ifndef HN_PUBLISHERS_MAX
#define HN_PUBLISHERS_MAX (32)
#endif

// arbitrary limitation, type name == topic name
typedef struct
{
    DDS_Topic *topic;
    char name[TOPIC_STRING_MAX];
} hn_topic_s;

typedef struct
{
    hn_topic_s topics[HN_TOPICS_MAX];
    DDS_UnsignedLong len;
} hn_topic_data_s;

typedef struct
{
    const hn_topic_s *topic_ref;
    DDS_Publisher *publisher;
    DDS_DataWriter *datawriter;
} hn_publisher_s;

typedef struct
{
    hn_publisher_s publishers[HN_PUBLISHERS_MAX];
    DDS_UnsignedLong len;
} hn_publisher_data_s;

typedef struct
{
    DDS_DomainParticipant *dp;
    struct DDS_GUID_t dp_guid; // TODO
    hn_topic_data_s topic_data;
    hn_publisher_data_s pub_data;
} hn_participant_s;

DDS_ReturnCode_t hn_create(
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_enable(
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_create_topic(
        const char * const name,
        struct NDDS_Type_Plugin * const plugin,
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_create_publisher(
        const char * const topic_name,
        struct DDS_DataWriterListener * const dw_listener,
        const DDS_StatusMask dw_mask,
        DDS_DataWriter ** const dw_ref,
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_unregister_types(
        hn_participant_s * const participant);

DDS_ReturnCode_t hn_destroy(
        hn_participant_s * const participant);

#endif /* HOME_NETWORK_H */

