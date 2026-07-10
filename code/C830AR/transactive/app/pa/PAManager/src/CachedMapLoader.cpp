/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/CachedMapLoader.cpp $
 * @author Jade Lee
 * @version $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by: $Author: limin.zhu $
 *
 * Manages loading of cached maps (needs to be centralized, to allow the PA Agent
 * to be used to load cached map data where possible, rather than direct DB access)
 *
 */

#include "StdAfx.h"
#include "app/pa/PAManager/src/CachedMapLoader.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "bus/pa/cachedmaps/src/PaDvaVersionCachedMap.h"
#include "bus/pa/cachedmaps/src/PaTrainDvaMessageCachedMap.h"

#include "bus/pa/cachedmaps/src/PaDvaPublicVersionCachedMap.h"
#include "bus/pa/cachedmaps/src/PaDvaPrivateVersionCachedMap.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"

using namespace TA_Base_Core;
using namespace TA_IRS_Bus;

namespace CachedMapLoader
{
    class CachedMapLoader_GlobalFunction {};

    static CachedMapLoader_GlobalFunction* LOGGING_PLACEHOLDER;
#define this LOGGING_PLACEHOLDER

    void loadAllCachedMaps()
    {
        LOG_SCOPE("CachedMapLoader::loadAllCachedMaps");

        unsigned int lKey = CachedConfig::getInstance()->getEntityLocationKey();

        refreshKeyToTrainDvaMessageRecordMap();
        // song toan++
        // TD 4080
        // Only want to load up location specific data for these, for all locations
        refreshKeyToStationDvaMessageRecordMap(lKey);

        // Only want to load up our local zones/DVA Version data if at a station
        if (CachedConfig::getInstance()->getLocationType() == CachedConfig::LOCATION_TYPE_OCC)
        {
            refreshKeyToDvaVersionRecordMap();
        }
        else
        {
            refreshKeyToDvaVersionRecordMap(lKey);
        }

        // jeffrey++ TD11826
        //refreshKeyToPaZoneGroupRecordMap(-1);
        refreshKeyToPaZoneGroupRecordMap(lKey);
        // ++jeffrey TD11826
        refreshKeyToPaZoneRecordMap();
        TA_IRS_Bus::CachedMaps::getInstance()->refreshKeyToPaStationRecordMap();
        // ++song toan
        // TD 4080
    }

    void refreshKeyToPaZoneGroupRecordMap(unsigned long locationKey)
    {
        // Zone groups are not loaded by PA Agent, must load directly from DB
        TA_IRS_Bus::CachedMaps::getInstance()->refreshKeyToPaZoneGroupRecordMap(locationKey);
    }

    void refreshKeyToDvaVersionRecordMap(unsigned long locationKey)
    {
        LOG_SCOPE("CachedMapLoader::refreshKeyToDvaVersionRecordMap");
        FUNCTION_ENTRY("refreshKeyToDvaVersionRecordMap");

        // Want to load all data for these items, regardless of location
        if (CachedConfig::getInstance()->isAtOcc())
        {
            try
            {
                // Attempt to retrieve from agent
                TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* ret = NULL;
                CORBA_CALL_RETURN(ret, CachedConfig::getInstance()->getAgentObject(), getDvaPublicVersionRecords, ());
                std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords> messages(ret);

                // PaDvaVersionCachedMap& cachedMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaDvaVersionCachedMap();
                PaDvaPublicVersionCachedMap& cachedMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaDvaPublicVersionCachedMap();

                // Clear all entries before refreshing items
                cachedMap.clearKeyToRecordMap();

                for (unsigned int i = 0; i < messages->length(); i++)
                {
                    // const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecord& msg = (*messages)[i];
                    const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord& msg = (*messages)[i];

                    TA_ASSERT(-1 == locationKey || msg.m_locationKey == locationKey, "Unexpected result from agent");

                    // DvaVersionRecord record;
                    DvaPublicVersionRecord record;

                    record.m_pKey           = msg.m_pKey;
                    record.m_locationKey    = msg.m_locationKey;

                    //					record.m_privateAdhoc1 = msg.m_privateAdhoc1;
                    //					record.m_privateAdhoc2 = msg.m_privateAdhoc2;
                    //					record.m_privateAdhoc3 = msg.m_privateAdhoc3;
                    //					record.m_privateAdhoc4 = msg.m_privateAdhoc4;
                    //					record.m_privateAdhoc5 = msg.m_privateAdhoc5;
                    //					record.m_privateAdhoc6 = msg.m_privateAdhoc6;
                    //					record.m_privatePreRecorded = msg.m_privatePreRecorded;

                    record.m_publicAdhoc1 = msg.m_publicAdhoc1;
                    record.m_publicAdhoc2 = msg.m_publicAdhoc2;
                    record.m_publicAdhoc3 = msg.m_publicAdhoc3;
                    record.m_publicAdhoc4 = msg.m_publicAdhoc4;
                    record.m_publicAdhoc5 = msg.m_publicAdhoc5;
                    record.m_publicAdhoc6 = msg.m_publicAdhoc6;
                    record.m_publicPreRecorded = msg.m_publicPreRecorded;

                    // Insert the items directly into cached map
                    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
                }

                // Must tell the cached map it's been initialised, else it will assert
                cachedMap.flagAsInitialised();
            }
            catch (...)
            {
                // Fall back to direct DB access
                TA_IRS_Bus::CachedMaps::getInstance()->refreshKeyToDvaPublicVersionRecordMap(locationKey);
            }
        }
        else
        {
            try
            {
                // Attempt to retrieve from agent
                //            std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords> messages(
                //                 CachedConfig::getInstance()->getAgentObject()->getDvaVersionRecords());
                TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords* ret = NULL;
                CORBA_CALL_RETURN(ret, CachedConfig::getInstance()->getAgentObject(), getDvaPrivateVersionRecords, ());
                std::auto_ptr< TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords > messages(ret);

                // PaDvaVersionCachedMap& cachedMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaDvaVersionCachedMap();
                PaDvaPrivateVersionCachedMap& cachedMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaDvaPrivateVersionCachedMap();

                // Clear all entries before refreshing items
                cachedMap.clearKeyToRecordMap();

                for (unsigned int i = 0; i < messages->length(); i++)
                {
                    // const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecord& msg = (*messages)[i];
                    const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecord& msg = (*messages)[i];

                    TA_ASSERT(-1 == locationKey || msg.m_locationKey == locationKey, "Unexpected result from agent");

                    // DvaVersionRecord record;
                    DvaPrivateVersionRecord record;

                    record.m_pKey           = msg.m_pKey;
                    record.m_locationKey    = msg.m_locationKey;

                    record.m_privateAdhoc1 = msg.m_privateAdhoc1;
                    record.m_privateAdhoc2 = msg.m_privateAdhoc2;
                    record.m_privateAdhoc3 = msg.m_privateAdhoc3;
                    record.m_privateAdhoc4 = msg.m_privateAdhoc4;
                    record.m_privateAdhoc5 = msg.m_privateAdhoc5;
                    record.m_privateAdhoc6 = msg.m_privateAdhoc6;
                    record.m_privatePreRecorded = msg.m_privatePreRecorded;

                    //                record.m_publicAdhoc1 = msg.m_publicAdhoc1;
                    //                record.m_publicAdhoc2 = msg.m_publicAdhoc2;
                    //                record.m_publicAdhoc3 = msg.m_publicAdhoc3;
                    //                record.m_publicAdhoc4 = msg.m_publicAdhoc4;
                    //                record.m_publicAdhoc5 = msg.m_publicAdhoc5;
                    //                record.m_publicAdhoc6 = msg.m_publicAdhoc6;
                    //                record.m_publicPreRecorded = msg.m_publicPreRecorded;

                    // Insert the items directly into cached map
                    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
                }

                // Must tell the cached map it's been initialised, else it will assert
                cachedMap.flagAsInitialised();
            }
            catch (...)
            {
                // Fall back to direct DB access
                TA_IRS_Bus::CachedMaps::getInstance()->refreshKeyToDvaPrivateVersionRecordMap(locationKey);
            }
        }

        FUNCTION_EXIT;
    }

    void refreshKeyToPaZoneRecordMap(unsigned long locationKey)
    {
        LOG_SCOPE("CachedMapLoader::refreshKeyToPaZoneRecordMap");
        FUNCTION_ENTRY("refreshKeyToPaZoneRecordMap");

        try
        {
            // Attempt to retrieve from agent
            TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords* ret = NULL;
            CORBA_CALL_RETURN(ret, CachedConfig::getInstance()->getAgentObject(), getPaZoneRecords, ());
            std::auto_ptr< TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords > messages(ret);

            // Clear all entries before refreshing items
            PaZoneCachedMap& cachedMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneCachedMap();

            cachedMap.clearKeyToRecordMap();

            for (unsigned int i = 0; i < messages->length(); i++)
            {
                const TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecord& msg = (*messages)[i];

                // song toan++
                // TD4080
                //      PA Manager will listen to changes of all zones/groups
                //      regardless of location
                //
                //        TA_ASSERT(-1 == locationKey || msg.m_locationKey == locationKey, "Unexpected result from agent");
                // song toan++
                // TD4080

                PaZoneRecord record;

                record.m_pKey           = msg.m_pKey;
                record.m_locationKey    = msg.m_locationKey;
                record.m_locationName   = msg.m_locationName.in();
                record.m_occLocationName    = msg.m_occLocationName.in();
                record.m_id             = msg.m_id;
                // liuyu++ TD8849
                record.m_orderId        = msg.m_orderId;
                // ++liuyu TD8849
                record.m_label          = msg.m_label.in();
                record.m_area           = msg.m_area.in();
                record.m_stnZoneShortName   = msg.m_stnZoneShortName.in();
                record.m_occZoneShortName   = msg.m_occZoneShortName.in();

                record.m_stnEquipmentEntityName     = msg.m_stnEquipmentEntityName.in();
                record.m_occEquipmentEntityName     = msg.m_occEquipmentEntityName.in();

                // PA Manager doesn't really care about these next parameters, but fill them in anyway
                for (unsigned int a = 0; a < msg.m_equipmentEntities.length(); a++)
                {
                    record.m_equipmentEntities.push_back(msg.m_equipmentEntities[a].in());
                }

                // Insert the items directly into cached map
                cachedMap.updateRecordMapWithItem(record.m_pKey, record);
            }

            // Must tell the cached map it's been initialised, else it will assert
            cachedMap.flagAsInitialised();
        }
        catch (...)
        {
            // Fall back to direct DB access on failure
            TA_IRS_Bus::CachedMaps::getInstance()->refreshKeyToPaZoneRecordMap(locationKey);
        }

        FUNCTION_EXIT;
    }

    void refreshKeyToTrainDvaMessageRecordMap()
    {
        LOG_SCOPE("CachedMapLoader::refreshKeyToTrainDvaMessageRecordMap");
        FUNCTION_ENTRY("refreshKeyToTrainDvaMessageRecordMap");

        // Want to load all data for these items, regardless of location
        try
        {
            // Attempt to retrieve from agent
            TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords* ret = NULL;
            CORBA_CALL_RETURN(ret, CachedConfig::getInstance()->getAgentObject(), getTrainDvaMessageRecords, ());
            std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords> messages(ret);

            // Clear all entries before refreshing items
            PaTrainDvaMessageCachedMap& cachedMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaTrainDvaMessageCachedMap();

            cachedMap.clearKeyToRecordMap();

            for (unsigned int i = 0; i < messages->length(); i++)
            {
                const TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecord& msg = (*messages)[i];

                TrainDvaMessageRecord record;

                record.m_pKeyOrId   = msg.m_pKeyOrId;
                record.m_label      = msg.m_label.in();

                // Insert the items directly into cached map
                cachedMap.updateRecordMapWithItem(record.m_pKeyOrId, record);
            }

            // Must tell the cached map it's been initialised, else it will assert
            cachedMap.flagAsInitialised();
        }
        catch (...)
        {
            // Fall back to direct DB access on failure
            TA_IRS_Bus::CachedMaps::getInstance()->refreshKeyToTrainDvaMessageRecordMap();
        }

        FUNCTION_EXIT;
    }

    void refreshKeyToStationDvaMessageRecordMap(unsigned int locationKey)
    {
        LOG_SCOPE(boost::format("CachedMapLoader::refreshKeyToStationDvaMessageRecordMap[%d:%s]") % locationKey % LocationX { locationKey }.name());
        FUNCTION_ENTRY("refreshKeyToStationDvaMessageRecordMap");

        try
        {
            // Get all records local to the PA Agent, store those with matching location key
            // in our cache
            TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData* ret = NULL;
            CORBA_CALL_RETURN(ret, CachedConfig::getInstance()->getAgentObject(), getStationDvaMessageRecords, ());
            std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData> messages(ret);

            // Clear all entries before refreshing items
            PaStationDvaMessageCachedMap& cachedMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();

            cachedMap.clearKeyToRecordMap();

            for (unsigned int i = 0; i < messages->length(); i++)
            {
                const TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecord& msg = (*messages)[i];

                if (-1 != locationKey)
                {
                    // If a location key is set, reject any records returned by Agent with mismatched location key
                    if (locationKey != msg.m_locationKey)
                    {
                        continue;
                    }
                }

                LOG_SCOPE(boost::format("MESSAGE[%s]") % msg.m_id);
                LOG_DEBUG("refreshKeyToStationDvaMessageRecordMap(): %s", nvps(msg));

                TA_IRS_Bus::StationDvaMessageRecord record;

                record.m_pKey           = msg.m_pKey;
                record.m_locationKey    = msg.m_locationKey;
                record.m_id             = msg.m_id;
                record.m_label          = msg.m_label.in();
                record.m_type           = msg.m_type;
                record.m_tisMessageTag  = msg.m_tisMessageTag;
                record.m_tisLibrarySection = msg.m_tisLibrarySection;
                record.m_tisLibraryVersion = msg.m_tisLibraryVersion;

                // Insert the items directly into cached map
                cachedMap.updateRecordMapWithItem(record.m_pKey, record);
            }

            // Must tell the cached map it's been initialised, else it will assert
            cachedMap.flagAsInitialised();
        }
        catch (...)
        {
            // If there is any kind of failure, simply fall back to direct DB access
            TA_IRS_Bus::CachedMaps::getInstance()->refreshKeyToStationDvaMessageRecordMap(locationKey);
        }

        FUNCTION_EXIT;
    }
}
