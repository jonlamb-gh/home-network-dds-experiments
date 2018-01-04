/**
 * @file hn_core.c
 * @brief TODO.
 *
 * TODO:
 * - collapse common things into fx/macro
 * - move create/destroy together ?
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "home_network/home_network.h"

static const hn_topic_s * hn_topic_find(
        const char * const name,
        const hn_topic_data_s * const topic_data)
{
    const hn_topic_s *ref = NULL;
    DDS_UnsignedLong idx;

    for(idx = 0; (idx < topic_data->len) && (ref == NULL); idx += 1)
    {
        if(strncmp(name, topic_data->topics[idx].name, TOPIC_STRING_MAX) == 0)
        {
            ref = &topic_data->topics[idx];
        }
    }

    return ref;
}

static DDS_ReturnCode_t register_reader_writer_history(
        RT_Registry_T * const registry)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    if(ret == DDS_RETCODE_OK)
    {
        const RTI_BOOL status = RT_Registry_register(
                registry,
                DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                WHSM_HistoryFactory_get_interface(),
                NULL,
                NULL);

        if(status == RTI_FALSE)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if(ret == DDS_RETCODE_OK)
    {
        const RTI_BOOL status = RT_Registry_register(
                registry,
                DDSHST_READER_DEFAULT_HISTORY_NAME,
                RHSM_HistoryFactory_get_interface(),
                NULL,
                NULL);

        if(status == RTI_FALSE)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return ret;
}

static DDS_ReturnCode_t register_udp_transport(
        RT_Registry_T * const registry)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    struct UDP_InterfaceFactoryProperty * const udp_property =
            (struct UDP_InterfaceFactoryProperty *) malloc(sizeof(*udp_property));

    if(udp_property == NULL)
    {
        ret = DDS_RETCODE_OUT_OF_RESOURCES;
    }

    if(ret == DDS_RETCODE_OK)
    {
        const RTI_BOOL status = RT_Registry_unregister(
                registry,
                NETIO_DEFAULT_UDP_NAME,
                NULL,
                NULL);

        if(status == RTI_FALSE)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    // only allow 'lo' for now
    if(ret == DDS_RETCODE_OK)
    {
        (void) memcpy(
                udp_property,
                &UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT,
                sizeof(UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT));

        REDA_StringSeq_set_maximum(&udp_property->allow_interface, 1);
        REDA_StringSeq_set_length(&udp_property->allow_interface, 1);

        *REDA_StringSeq_get_reference(&udp_property->allow_interface, 0) =
                DDS_String_dup("lo");

        const RTI_BOOL status = RT_Registry_register(
                registry,
                NETIO_DEFAULT_UDP_NAME,
                UDP_InterfaceFactory_get_interface(),
                (struct RT_ComponentFactoryProperty *) udp_property,
                NULL);

        if(status == RTI_FALSE)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }

        /*
        if(*REDA_StringSeq_get_reference(&udp_property->allow_interface, 0) != NULL)
        {
            REDA_String_free(*REDA_StringSeq_get_reference(&udp_property->allow_interface, 0));
            *REDA_StringSeq_get_reference(&udp_property->allow_interface, 0) = NULL;
        }
        */
    }

    return ret;
}

static DDS_ReturnCode_t register_discovery_plugin(
        RT_Registry_T * const registry)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;
    struct DPDE_DiscoveryPluginProperty discovery_plugin_properties =
            DPDE_DiscoveryPluginProperty_INITIALIZER;

    if(ret == DDS_RETCODE_OK)
    {
        const RTI_BOOL status = RT_Registry_register(
                registry,
                "dpde",
                DPDE_DiscoveryFactory_get_interface(),
                &discovery_plugin_properties._parent,
                NULL);

        if(status == RTI_FALSE)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return ret;
}

DDS_ReturnCode_t hn_create(
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    OSAPI_Log_set_verbosity(OSAPI_LOG_VERBOSITY_DEBUG);

    // retrive the singleton registry instances
    DDS_DomainParticipantFactory * const factory =
            DDS_DomainParticipantFactory_get_instance();

    RT_Registry_T * const registry =
            DDS_DomainParticipantFactory_get_registry(factory);

    if(ret == DDS_RETCODE_OK)
    {
        ret = register_reader_writer_history(registry);
    }

    if(ret == DDS_RETCODE_OK)
    {
        ret = register_udp_transport(registry);
    }

    // turn off 'auto-enable'
    if(ret == DDS_RETCODE_OK)
    {
        struct DDS_DomainParticipantFactoryQos dpf_qos =
                DDS_DomainParticipantFactoryQos_INITIALIZER;

        DDS_DomainParticipantFactory_get_qos(factory, &dpf_qos);
        dpf_qos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
        DDS_DomainParticipantFactory_set_qos(factory, &dpf_qos);
    }

    if(ret == DDS_RETCODE_OK)
    {
        ret = register_discovery_plugin(registry);
    }

    if(ret == DDS_RETCODE_OK)
    {
        // TODO configure resource limits, name, domain, etc

        struct DDS_DomainParticipantQos dp_qos =
                DDS_DomainParticipantQos_INITIALIZER;

        dp_qos.resource_limits.max_destination_ports = 32;
        dp_qos.resource_limits.max_receive_ports = 32;
        dp_qos.resource_limits.local_topic_allocation = 1;
        dp_qos.resource_limits.local_type_allocation = 1;
        dp_qos.resource_limits.local_reader_allocation = 1;
        dp_qos.resource_limits.local_writer_allocation = 1;
        dp_qos.resource_limits.remote_participant_allocation = 16;
        dp_qos.resource_limits.remote_reader_allocation = 8;
        dp_qos.resource_limits.remote_writer_allocation = 8;

        strcpy(dp_qos.participant_name.name, "Participant_1");

        // set initial peers to use the default multicast peer
        DDS_StringSeq_set_maximum(&dp_qos.discovery.initial_peers, 1);
        DDS_StringSeq_set_length(&dp_qos.discovery.initial_peers, 1);
        *DDS_StringSeq_get_reference(&dp_qos.discovery.initial_peers, 0) =
                DDS_String_dup("239.255.0.1");

        participant->dp = DDS_DomainParticipantFactory_create_participant(
                factory,
                0, // TODO domain_id,
                &dp_qos,
                NULL,
                DDS_STATUS_MASK_NONE);

        if(participant->dp == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }

        /*
        if(*REDA_StringSeq_get_reference(&dp_qos.discovery.initial_peers, 0) != NULL)
        {
            REDA_String_free(*REDA_StringSeq_get_reference(&dp_qos.discovery.initial_peers, 0));
            *REDA_StringSeq_get_reference(&dp_qos.discovery.initial_peers, 0) = NULL;
        }
        */
    }

    return ret;
}

DDS_ReturnCode_t hn_enable(
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    if((participant == NULL) || (participant->dp == NULL))
    {
        ret = DDS_RETCODE_BAD_PARAMETER;
    }

    if(ret == DDS_RETCODE_OK)
    {
        DDS_Entity * const entity = DDS_DomainParticipant_as_entity(
                participant->dp);

        ret = DDS_Entity_enable(entity);
    }

    return ret;
}

DDS_ReturnCode_t hn_create_topic(
        const char * const name,
        struct NDDS_Type_Plugin * const plugin,
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    if((participant == NULL) || (participant->dp == NULL))
    {
        ret = DDS_RETCODE_BAD_PARAMETER;
    }

    if(ret == DDS_RETCODE_OK)
    {
        if(participant->topic_data.len > HN_TOPICS_MAX)
        {
            ret = DDS_RETCODE_OUT_OF_RESOURCES;
        }
    }

    if(ret == DDS_RETCODE_OK)
    {
        const DDS_UnsignedLong t_idx = participant->topic_data.len;
        hn_topic_s * const t_ref = &participant->topic_data.topics[t_idx];

        (void) strncpy(
                t_ref->name,
                name,
                sizeof(t_ref->name));

        ret = DDS_DomainParticipant_register_type(
                participant->dp,
                name,
                plugin);

        if(ret == DDS_RETCODE_OK)
        {
            t_ref->topic =
                    DDS_DomainParticipant_create_topic(
                            participant->dp,
                            name,
                            name,
                            &DDS_TOPIC_QOS_DEFAULT,
                            NULL,
                            DDS_STATUS_MASK_NONE);

            if(t_ref->topic == NULL)
            {
                ret = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
        }

        participant->topic_data.len += 1;
    }

    return ret;
}

DDS_ReturnCode_t hn_create_publisher(
        const char * const topic_name,
        const DDS_ReliabilityQosPolicyKind reliability,
        struct DDS_DataWriterListener * const dw_listener,
        const DDS_StatusMask dw_mask,
        DDS_DataWriter ** const dw_ref,
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;
    const hn_topic_s *t_ref = NULL;

    if((participant == NULL) || (participant->dp == NULL))
    {
        ret = DDS_RETCODE_BAD_PARAMETER;
    }

    if(ret == DDS_RETCODE_OK)
    {
        if(participant->pub_data.len > HN_PUBLISHERS_MAX)
        {
            ret = DDS_RETCODE_OUT_OF_RESOURCES;
        }
    }

    if(ret == DDS_RETCODE_OK)
    {
        t_ref = hn_topic_find(topic_name, &participant->topic_data);

        if(t_ref == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if(ret == DDS_RETCODE_OK)
    {
        const DDS_UnsignedLong p_idx = participant->pub_data.len;
        hn_publisher_s * const p_ref = &participant->pub_data.publishers[p_idx];
        struct DDS_DataWriterQos dw_qos = DDS_DataWriterQos_INITIALIZER;

        p_ref->topic_ref = t_ref;

        // TODO - QoS, etc

        dw_qos.reliability.kind = reliability;

        p_ref->publisher = DDS_DomainParticipant_create_publisher(
                participant->dp,
                &DDS_PUBLISHER_QOS_DEFAULT,
                NULL,
                DDS_STATUS_MASK_NONE);

        if(p_ref->publisher == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }

        if(ret == DDS_RETCODE_OK)
        {
            p_ref->datawriter = DDS_Publisher_create_datawriter(
                    p_ref->publisher,
                    p_ref->topic_ref->topic,
                    &dw_qos,
                    dw_listener,
                    dw_mask);

            if(dw_ref != NULL)
            {
                *dw_ref = p_ref->datawriter;
            }

            if(p_ref->datawriter == NULL)
            {
                ret = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
        }

        (void) DDS_DataWriterQos_finalize(&dw_qos);

        participant->pub_data.len += 1;
    }

    return ret;
}

DDS_ReturnCode_t hn_create_subscriber(
        const char * const topic_name,
        const DDS_ReliabilityQosPolicyKind reliability,
        struct DDS_DataReaderListener * const dr_listener,
        const DDS_StatusMask dr_mask,
        DDS_DataReader ** const dr_ref,
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;
    const hn_topic_s *t_ref = NULL;

    if((participant == NULL) || (participant->dp == NULL))
    {
        ret = DDS_RETCODE_BAD_PARAMETER;
    }

    if(ret == DDS_RETCODE_OK)
    {
        if(participant->sub_data.len > HN_SUBSCRIBERS_MAX)
        {
            ret = DDS_RETCODE_OUT_OF_RESOURCES;
        }
    }

    if(ret == DDS_RETCODE_OK)
    {
        t_ref = hn_topic_find(topic_name, &participant->topic_data);

        if(t_ref == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if(ret == DDS_RETCODE_OK)
    {
        const DDS_UnsignedLong s_idx = participant->sub_data.len;
        hn_subscriber_s * const s_ref = &participant->sub_data.subscribers[s_idx];
        struct DDS_DataReaderQos dr_qos = DDS_DataReaderQos_INITIALIZER;

        s_ref->topic_ref = t_ref;

        // TODO - QoS, etc

        dr_qos.reliability.kind = reliability; 

        s_ref->subscriber = DDS_DomainParticipant_create_subscriber(
                participant->dp,
                &DDS_SUBSCRIBER_QOS_DEFAULT,
                NULL,
                DDS_STATUS_MASK_NONE);

        if(s_ref->subscriber == NULL)
        {
            ret = DDS_RETCODE_PRECONDITION_NOT_MET;
        }

        if(ret == DDS_RETCODE_OK)
        {
            s_ref->datareader = DDS_Subscriber_create_datareader(
                    s_ref->subscriber,
                    DDS_Topic_as_topicdescription(s_ref->topic_ref->topic),
                    &dr_qos,
                    dr_listener,
                    dr_mask);

            if(dr_ref != NULL)
            {
                *dr_ref = s_ref->datareader;
            }

            if(s_ref->datareader == NULL)
            {
                ret = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
        }

        (void) DDS_DataReaderQos_finalize(&dr_qos);

        participant->sub_data.len += 1;
    }

    return ret;
}

DDS_ReturnCode_t hn_unregister_types(
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    if(participant != NULL)
    {
        if(participant->dp != NULL)
        {
            DDS_UnsignedLong idx;

            for(idx = 0; idx < participant->topic_data.len; idx += 1)
            {
                const struct NDDS_Type_Plugin * const plugin =
                        DDS_DomainParticipant_unregister_type(
                                participant->dp,
                                participant->topic_data.topics[idx].name);

                if(plugin == NULL)
                {
                    ret = DDS_RETCODE_PRECONDITION_NOT_MET;
                }
            }
        }
    }

    return ret;
}

DDS_ReturnCode_t hn_destroy(
        hn_participant_s * const participant)
{
    DDS_ReturnCode_t ret = DDS_RETCODE_OK;

    if(participant != NULL)
    {
        if(participant->dp != NULL)
        {
            // TODO - improve this

            ret = DDS_DomainParticipant_delete_contained_entities(participant->dp);

            ret |= hn_unregister_types(participant);

            ret |= DDS_DomainParticipantFactory_delete_participant(
                    DDS_DomainParticipantFactory_get_instance(),
                    participant->dp);

            RT_Registry_T * const registry = DDS_DomainParticipantFactory_get_registry(
                    DDS_DomainParticipantFactory_get_instance());

            (void) RT_Registry_unregister(
                    registry,
                    "dpde",
                    NULL,
                    NULL);

            struct RT_ComponentFactoryProperty *udp_property = NULL;
            (void) RT_Registry_unregister(
                    registry,
                    NETIO_DEFAULT_UDP_NAME,
                    &udp_property,
                    NULL);

            (void) RT_Registry_unregister(
                    registry,
                    DDSHST_READER_DEFAULT_HISTORY_NAME,
                    NULL,
                    NULL);

            (void) RT_Registry_unregister(
                    registry,
                    DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                    NULL,
                    NULL);

            DDS_DomainParticipantFactory_finalize_instance();
            participant->dp = NULL;

            if(udp_property  != NULL)
            {
                free(udp_property);
            }
        }
    }

    return ret;
}
