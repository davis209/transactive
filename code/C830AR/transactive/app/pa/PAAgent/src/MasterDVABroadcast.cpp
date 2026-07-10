/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/MasterDVABroadcast.cpp $
 * @author  A. Parker
 * @version $Revision: #5 $
 * Last modification : $DateTime: 2025/01/22 17:56:54 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#include "pch.h"
#include "MasterDVABroadcast.h"
#include "Table1200.h"
#include "Table1301.h"
#include "Table1610.h"
#include "Table302.h"
#include "Table202.h"
#include "Table350.h"
#include "BroadcastManager.h"
#include "CachedConfig.h"
#include "AbstractBroadcastCircularList.h"
#include "PASConnection.h"
#include "bus/signs_4669/tis_agent_access/src/TISAgentAccessFactory.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/exceptions/src/AuthenticationAgentException.h"
#include "core/exceptions/src/AuthenticationSecurityException.h"
#include "core/exceptions/src/ThrowException.h"
#include "core/naming/src/NamedObject.h"
#include "ace/OS.h"
#include <set>
#include <time.h>

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

namespace TA_IRS_App
{
    MasterDVABroadcast::MasterDVABroadcast(Table202& table202,
                                           Table302& table302,
                                           Table350& table350,
                                           Table1200& table1200,
                                           Table1201& table1201,
                                           Table1301& table1301,
                                           Table1610& table1610,
                                           TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP,
                                           const std::string& sessionId)
        : AbstractStationBroadcast(broadcastConfigP, sessionId),
        AbstractNonTrainDVABroadcast(table202, table302, table350, broadcastConfigP, sessionId),
        m_table1200(table1200),
        m_table1201(table1201),
        m_table1610(table1610),
        m_table1301(table1301)
    {
        LOG_SCOPE("MasterDVABroadcast");

        TA_ASSERT(CachedConfig::getInstance()->getIsMaster(), "MasterDVABroadcast is not available for the Master PA Agent");

        m_tisCoverageMap.clear();
    }

    MasterDVABroadcast::~MasterDVABroadcast()
    {
    }

    void MasterDVABroadcast::validateBroadcastConfigSpecifics() //throw(TA_Base_Core::BadParameterException)
    {
        LOG_SCOPE("MasterDVABroadcast::validateBroadcastConfigSpecifics");

        ThreadGuard guard(m_lock);
        int i = 0;

        // Test validity of zone keys in database (Global version)
        for (int index = 0; index < m_theSpecifics.zones.length(); ++index)
        {
            unsigned long zoneKey;
            unsigned int zoneId;
            unsigned long zoneLocationKey;
            unsigned int stationId;

            try
            {
                zoneKey = m_theSpecifics.zones[index];
                zoneId = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneIdFromKey(zoneKey);
            }
            catch (TA_Base_Core::CachedMappingNotFoundException&)
            {
                TA_THROW(BadParameterException("A requested zone does not exist in the database"));
            }

            try
            {
                zoneLocationKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(zoneKey);
                stationId = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(zoneLocationKey);
            }
            catch (TA_Base_Core::CachedMappingNotFoundException&)
            {
                TA_THROW(BadParameterException("An ISCS station location could not be resolved to a PAS location"));
            }
        }

        // Messages can only be from setA
        for (i = 0; i < m_theSpecifics.messages.length(); ++i)
        {
            TA_Base_Bus::IPAAgentCorbaDef::EDvaSet set = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageSetFromKey(m_theSpecifics.messages[i]);

            if (set != TA_Base_Bus::IPAAgentCorbaDef::SetA)
            {
                TA_THROW(BadParameterException("A requested message Id is not from set A"));
            }
        }
    }

    void MasterDVABroadcast::writeToMessagesSequenceBroadcastRequestTable(unsigned int announceId)
        // throws (PasWriteErrorException, PasConnectionException)
    {
        LOG_SCOPE(boost::format("MasterDVABroadcast::writeToMessagesSequenceBroadcastRequestTable[%d]") % announceId);

        m_table1301.setTableDataAndWrite(getMessageSeqId(),
                                         announceId,
                                         m_theSpecifics.periodInSecs,
                                         m_theSpecifics.startTime,
                                         m_theSpecifics.stopTime,
                                         m_broadcastConfigV->isEventTriggered,
                                         getEffectiveZoneCoverage(m_theSpecifics.zones));
    }

    void MasterDVABroadcast::performVersionCheck()
    {
        LOG_SCOPE("MasterDVABroadcast::performVersionCheck");

        //Maochun Sun++
        //TD13407
        unsigned long   zoneLocationKey;
        unsigned int    stationId;
        unsigned long   zoneKey;

        std::vector<unsigned int> stationIdList;

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage effectiveZoneCoverage = getEffectiveZoneCoverage(m_theSpecifics.zones);

        for (int i = 0; i < effectiveZoneCoverage.length(); i++)
        {
            zoneKey = effectiveZoneCoverage[i];
            zoneLocationKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(zoneKey);
            stationId = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(zoneLocationKey);

            if (std::find(stationIdList.begin(), stationIdList.end(), stationId) == stationIdList.end())
            {
                stationIdList.push_back(stationId);
            }
        }

        //if (!m_table1610.isPublicSetSynchronised())
        if (!m_table1610.isPublicSetSynchronised(stationIdList))
            //++Maochun Sun
            //TD13407
        {
            // Just a warning message
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_PUBLIC_DVA_SET_VERSION_CONFLICT);
        }
    }

    //Maochun Sun++
    //TD13411
    //void MasterDVABroadcast::updateCacheWithLatestZoneStatus(unsigned long zoneKey)
    void MasterDVABroadcast::updateCacheWithLatestZoneStatus()
        //++Maochun Sun
        //TD13411
    {
        //Maochun Sun++
        //TD13411
        /*
        // CachedMappingNotFoundException will never happen because it has been tested
        // in validateBroadcastConfig()
        unsigned int zoneId = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneIdFromKey(zoneKey);
        unsigned long zoneLocationKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(zoneKey);
        unsigned int stationId = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(zoneLocationKey);

        // ++ Bug456
        m_table1200.readTimedWait();
        // ++ Bug456
        m_lastZoneStatuses.insert(ZoneKeyToZoneStatusMap::value_type(zoneKey, m_table1200.getZoneStatus(zoneId, stationId)));
        */

        m_table1200.readTimedWait();

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices = getEffectiveCoverageIndices();

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            unsigned long zoneKey = m_theSpecifics.zones[effectiveCoverageIndices[indexIndex]];

            unsigned int zoneId = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneIdFromKey(zoneKey);
            unsigned long zoneLocationKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(zoneKey);
            unsigned int stationId = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(zoneLocationKey);

            m_lastZoneStatuses.insert(ZoneKeyToZoneStatusMap::value_type(zoneKey, m_table1200.getZoneStatus(zoneId, stationId)));
        }

        //++Maochun Sun
        //TD13411
    }

    void MasterDVABroadcast::setupAndValidateTisParameters(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones)
        //throw(TA_Base_Core::BadParameterException)
    {
        LOG_SCOPE("MasterDVABroadcast::setupAndValidateTisParameters");

        //    if (CachedConfig::getInstance()->getLocalTISAgentEntityName().empty())
        //    {
        //        PasHelpers::instance().throwBadParameterException("Local TisAgent is not configured at this location.", getBroadcastId());
        //    }
        try
        {
            // force the access factory to build an agent reference
            TA_Base_Bus::TISAgentAccessFactory::getInstance().getSTISAgent();
        }
        catch (...)
        {
            // can only be a DatabaseException or a DataException
            TA_THROW(BadParameterException("Local TisAgent is not configured at this location."));
        }

        TA_IRS_Bus::StationDvaMessageRecord record;

        try
        {
            record = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(m_theSpecifics.messages[0]);
        }
        catch (TA_Base_Core::CachedMappingNotFoundException&)
        {
            TA_THROW(BadParameterException("A requested message does not exist in the database"));
        }

        // Verify if all TIS synchronisation parameters are set

        if (record.m_tisMessageTag == -1 ||
            record.m_tisLibrarySection == -1 ||
            record.m_tisLibraryVersion == -1)
        {
            TA_THROW(BadParameterException("PA-TIS synchronisation configuration is not complete"));
        }

        // Determine PA Zone to PID Zone mapping

        std::set<std::string> uniquePidZones;

        for (int index = 0; index < zones.length(); ++index)
        {
            TA_IRS_Bus::PaZoneRecord record = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneRecordFromKey(zones[index]);

            std::list<std::string>::iterator it = record.m_equipmentEntities.begin();

            for (; it != record.m_equipmentEntities.end(); ++it)
            {
                uniquePidZones.insert(*it);
            }
        }

        std::map<std::string, unsigned long> locationNameToPidCount;

        std::set<std::string>::iterator itForPidZones = uniquePidZones.begin();

        for (; itForPidZones != uniquePidZones.end(); ++itForPidZones)
        {
            try
            {
                std::string locationName;
                std::string pidName;

                getEquipmentComponentAndVerify(*itForPidZones, locationName, pidName);

                unsigned long& locationCount = locationNameToPidCount[locationName];
                TA_Base_Core::STISDestination& locationDestination = m_tisCoverageMap[locationName];

                if (locationCount == 0)
                {
                    // Initialise new structure
                    locationDestination.station = locationName.c_str();
                    // Not used
                    locationDestination.levels.length(0);
                    // The max possible size.  We will resize later
                    locationDestination.pids.length(uniquePidZones.size());
                }

                locationDestination.pids[locationCount] = pidName.c_str();

                ++locationCount;
            }
            catch (TA_Base_Core::InvalidPasConfigurationException& e)
            {
                PasHelpers::instance().raiseInvalidPAConfigurationAlarm(e.what());
            }
        }

        // Resize in case some Pid Zones were invalid
        std::map<std::string, unsigned long>::iterator itForLocationCount = locationNameToPidCount.begin();

        for (; itForLocationCount != locationNameToPidCount.end(); ++itForLocationCount)
        {
            const std::string& locationName = itForLocationCount->first;
            unsigned long locationCount = itForLocationCount->second;

            m_tisCoverageMap[locationName].pids.length(locationCount);
        }
    }

    void MasterDVABroadcast::sendRequestToTisAgent(const TA_IRS_Bus::StationDvaMessageRecord& record,
                                                   const char* startTimeString,
                                                   const char* stopTimeString,
                                                   CORBA::UShort tisPriority)
    {
        LOG_SCOPE("MasterDVABroadcast::sendRequestToTisAgent");

        // Send a multi station broadcast
        TA_Base_Core::STISDestinationList destinationList;
        destinationList.length(m_tisCoverageMap.size());

        int i = 0;
        LocationNameToDestinationMapIt it = m_tisCoverageMap.begin();

        for (; it != m_tisCoverageMap.end(); ++it)
        {
            destinationList[i] = it->second;
            i++;
        }

        CORBA_CALL(TA_Base_Bus::TISAgentAccessFactory::getInstance().getSTISAgent(), submitPredefinedDisplayRequest, (
            destinationList,
            static_cast<TA_Base_Core::ELibrarySection>(record.m_tisLibrarySection),
            record.m_tisLibraryVersion,
            record.m_tisMessageTag,
            startTimeString,
            stopTimeString,
            tisPriority,
            getSessionId().c_str()));
    }

    void MasterDVABroadcast::getEquipmentComponentAndVerify(const std::string& equipmentName,
                                                            std::string& locationName,
                                                            std::string& pidName)
        //throw(TA_Base_Core::InvalidPasConfigurationException)
    {
        std::vector<std::string> tokens;

        PasHelpers::instance().tokenise(equipmentName, tokens, ".");

        if (tokens.size() != 4)
        {
            TA_THROW(InvalidPasConfigurationException("Associated PID zone equipment is invalid"));
        }

        // Determine StationSystem
        if (tokens[1] != EXPECTED_STATION_SYSTEM)
        {
            TA_THROW(InvalidPasConfigurationException("Associated PID zone equipment is invalid"));
        }

        locationName = tokens[0];
        pidName = tokens[3];
    }

    void MasterDVABroadcast::disconnectSourceId()
    {
        LOG_SCOPE("MasterDVABroadcast::disconnectSourceId");

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage nullZones;
        nullZones.length(0);

        m_table1201.setTableDataAndWrite(m_table202.getSourceId(getAnnounceId()),
                                         getAnnounceId(),
                                         nullZones);
    }

    void MasterDVABroadcast::disconnectSequenceId()
    {
        LOG_SCOPE("MasterDVABroadcast::disconnectSequenceId");

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage nullZones;
        nullZones.length(0);

        m_table1301.setTableDataAndWrite(m_table302.getSequenceId(getAnnounceId()),
                                         0,
                                         0,
                                         0,
                                         0,
                                         false,
                                         nullZones);
    }

    // jeffrey++ Bug457
    int MasterDVABroadcast::getZoneStatusTableIndex()
    {
        return m_table1200.m_readIndex;
    }

    // TD17456
    bool MasterDVABroadcast::isAllFailureZones()
    {
        bool isNonValidZone = true;

        m_table1200.readTimedWait();

        for (int i = 0; i < m_theSpecifics.zones.length(); ++i)
        {
            // get the zone id and status for every selected zone
            unsigned int zoneId = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneIdFromKey(m_theSpecifics.zones[i]);
            unsigned long zoneLocationKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(m_theSpecifics.zones[i]);
            unsigned int stationId = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(zoneLocationKey);

            ZoneStatus zoneSta = m_table1200.getZoneStatus(zoneId, stationId);

            // for zone status, if in normal status, value is 0, and in failure status, value is 1
            isNonValidZone = isNonValidZone && zoneSta.m_status;
        }

        return isNonValidZone;
    }
}
