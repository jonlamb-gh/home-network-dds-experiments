/**
 * @file home_network.c
 * @brief TODO.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "home_network/home_network.h"

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

    // only allow 'eth0' for now
    if(ret == DDS_RETCODE_OK)
    {
        (void) memcpy(
                udp_property,
                &UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT,
                sizeof(UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT));

        REDA_StringSeq_set_maximum(&udp_property->allow_interface, 1);
        REDA_StringSeq_set_length(&udp_property->allow_interface, 1);

        *REDA_StringSeq_get_reference(&udp_property->allow_interface, 0) =
                DDS_String_dup("eth0");

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
