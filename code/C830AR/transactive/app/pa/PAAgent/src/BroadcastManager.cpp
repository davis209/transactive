/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/BroadcastManager.cpp $
 * @author  A. Parker
 * @version $Revision: #10 $
 * Last modification : $DateTime: 2025/09/11 17:51:01 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#include "pch.h"
#include "BroadcastManager.h"
#include "BroadcastWorker.h"
#include "CachedConfig.h"
#include "DVARecordingBroadcast.h"
#include "MasterDVABroadcast.h"
#include "MasterLiveBroadcast.h"
#include "PasHelpers.h"
#include "StationDVABroadcast.h"
#include "StationLiveBroadcast.h"
#include "StationMusicBroadcast.h"
#include "Table100.h"
#include "Table1610.h"
#include "Table604.h"
#include "Table610.h"
#include "TrainDVABroadcast.h"
#include "TrainLiveBroadcast.h"
#include "bus/generic_agent/src/GenericAgent.h"
#include "bus/pa/cachedmaps/src/PaDvaPrivateVersionCachedMap.h"
#include "bus/pa/cachedmaps/src/PaDvaPublicVersionCachedMap.h"
#include "bus/pa/cachedmaps/src/PaDvaVersionCachedMap.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "bus/pa/cachedmaps/src/PaTrainDvaMessageCachedMap.h"
#include "bus/pa/cachedmaps/src/PaZoneCachedMap.h"
#include "bus/pa/cachedmaps/src/PaZoneGroupCachedMap.h"
#include "bus/security/access_control/actions/src/AccessControlledActions.h"
#include "bus/security/rights_library/src/RightsLibrary.h"
#include "bus/trains/TrainCorbaProxy/src/TrainAgentProxyFactory.h"
#include "bus/trains/TrainCorbaProxy/src/TrainInformationCorbaProxy.h"
#include "core/exceptions/src/ThrowException.h"
#include "core/exceptions/src/RightsException.h"
#include "core/naming/src/Naming.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/RunParams.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/MakeNameValuePairsString.h"
#include "core/utilities/src/stdutil/datetime.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"
#include <ace/OS.h>
#include <time.h>
#include <iostream>
#include <sstream>

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

namespace TA_IRS_App
{
    const std::string BroadcastManager::ATS_AGENT_SESSION_ID = "AtsAgentSessionId";
    // TD17372
    const std::string BroadcastManager::MANAGER_LAUNCHING_BROADCAST_ID = "HardwareConnectionTest";
    // TD17372

    BroadcastManager::BroadcastManager(PasTableManager& tableManager)
        : m_broadcastWorker(NULL),
        m_rightsLibrary(NULL),
        m_resourceId(0),
        m_allBroadcasts(NULL),
        m_tableManager(tableManager),
        m_MusicType(TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic)
    {
        FUNCTION_ENTRY("BroadcastManager");

        m_allBroadcasts = AbstractBroadcastCircularList::getInstance();

        m_broadcastWorker = new BroadcastWorker();
        m_broadcastWorker->start();

        std::ostringstream msg;
        msg << "Registering PA Agent CORBA object: "
            << CachedConfig::getInstance()->getAgentName();
        LOG_INFO(msg);

        activateServantWithName(CachedConfig::getInstance()->getAgentName());

        FUNCTION_EXIT;
    }

    BroadcastManager::~BroadcastManager()
    {
        FUNCTION_ENTRY("~BroadcastManager");

        try
        {
            deactivateServant();

            if (m_broadcastWorker)
            {
                if (m_broadcastWorker->getCurrentState() == Thread::THREAD_STATE_RUNNING)
                {
                    m_broadcastWorker->terminateAndWait();
                }

                delete m_broadcastWorker;
                m_broadcastWorker = NULL;
            }

            // Cleanup time for all broadcasts
            while (!m_allBroadcasts->isEmpty())
            {
                AbstractBroadcast* broadcast = m_allBroadcasts->getNext();
                m_allBroadcasts->remove(broadcast);
                delete broadcast;
            }

            m_allBroadcasts = NULL;

            if (m_rightsLibrary != NULL)
            {
                delete m_rightsLibrary;
                m_rightsLibrary = NULL;
            }
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown", "Exception caught in destructor");
        }

        FUNCTION_EXIT;
    }

    void BroadcastManager::enable()
    {
        m_broadcastWorker->enable();
    }

    void BroadcastManager::disable()
    {
        m_broadcastWorker->disable();
    }

    // IDL Interface functions
    // exception checked
    std::string BroadcastManager::getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType broadcastType, const std::string& extraInformation /* = "" */)
        // TA_THROW CorruptInternalStateDataException
    {
        FUNCTION_ENTRY("getNewBroadcastId");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        static std::string lastNowString = "";
        static unsigned long extraInfoForUniqueness = 0;

        std::string broadcastTypeString;

        switch (broadcastType)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::StationMusic:

                // jeffrey++ TD10420
                // Station or Depot
                if (CachedConfig::getInstance()->getIsDepot())
                {
                    broadcastTypeString = "DepotMusic at ";
                }
                else
                {
                    broadcastTypeString = "StationMusic at ";
                }

                //broadcastTypeString = "StationMusic at ";
                // ++jeffrey TD10420
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::StationLive:
                broadcastTypeString = "StationLive " + extraInformation + " at ";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::StationDva:
                broadcastTypeString = "StationDva " + extraInformation + " at ";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::TrainLive:
                broadcastTypeString = "TrainLive at ";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::TrainDva:
                broadcastTypeString = "TrainDva at ";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::StationRecording:
                broadcastTypeString = "StationRecording at ";
                break;

            default:
                TA_THROW(CorruptInternalStateDataException("Unexpected EBroadcastType"));
        }

        // Build time stamp

        time_t now = time(NULL);
        struct tm nowTm;
        ACE_OS::localtime_r(&now, &nowTm);

        char nowString[80] = {0};
        sprintf(nowString,
                "%02d:%02d:%02d",
                (nowTm.tm_hour),
                (nowTm.tm_min),
                (nowTm.tm_sec));

        std::string broadcastId;

        // Since we only have a resolution of 1 sec, we need to check if the request
        // was done within 1 second of the previous broadcast by comparing the new nowString
        // with the previous.  We just need to make sure that the broadcastIds are all
        // unique.

        if (lastNowString == nowString)
        {
            // Ensure broadcastIds are different by updating the suffix
            ++extraInfoForUniqueness;

            char suffix[10] = {0};
            sprintf(suffix, " (%d)", extraInfoForUniqueness);

            broadcastId = broadcastTypeString + nowString + suffix;
        }
        else
        {
            // Reset for the next time
            extraInfoForUniqueness = 0;

            broadcastId = broadcastTypeString + nowString;
        }

        lastNowString = nowString;

        FUNCTION_EXIT;
        return broadcastId;
    }

    // exception checked
    char* BroadcastManager::broadcastStationDva(const TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq& messages,
                                                const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones_,
                                                CORBA::UShort dwellInSecs,
                                                CORBA::Boolean hasChime,
                                                TA_Base_Core::DateTime startTime,
                                                TA_Base_Core::DateTime stopTime,
                                                CORBA::UShort periodInSecs,
                                                CORBA::Boolean isSynchronisedWithTis,
                                                const char* groupName,
                                                const char* sessionId)
        // raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::broadcastStationDva");
        FUNCTION_ENTRY("broadcastStationDva");

        LOGOUT("broadcastStationDva");
        LOG_DEBUG("broadcastStationDva(): %s", nvps(messages, zones_, dwellInSecs, hasChime, startTime, stopTime, periodInSecs, isSynchronisedWithTis, groupName, sessionId));
        LOG_DEBUG_IF(startTime || stopTime, "broadcastStationDva(): startTime=%s, stopTime=%s", st::get_time_str(startTime), st::get_time_str(stopTime));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        auto zones = zones_;

        if (!ThisLocation::is_depot())
        {
            zones = PasHelpers::instance().remove_zones_where_id_larger_than(zones_, 32);
        }

        if (ThisLocation::is_station_or_depot())
        {
            auto zone_ids = PasHelpers::instance().local_zone_coverage_to_zone_id(zones);
            LOG_DEBUG("broadcastStationDva(): %s", nvps(zone_ids));
        }

        // this is to build a bvroadcast ID with more information
        std::ostringstream extraInformation;
        extraInformation << groupName << " (";

        try
        {
            for (int i = 0; i < messages.length(); ++i)
            {
                extraInformation << TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messages[i]);

                if (i != (messages.length() - 1))
                {
                    // avoid adding comma for the last item
                    extraInformation << ",";
                }
            }
        }
        catch (TA_Base_Core::CachedMappingNotFoundException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

            FUNCTION_EXIT;
            TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
        }

        extraInformation << ")";

        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationDva, extraInformation.str());
        // TES 735
        // No audit message needed for local playback
        // check if this DVA message is destined for a paging console
        bool isLocalPlayback = PasHelpers::instance().zoneIsAPagingConsole(zones);

        // this will validate the parameters like the zones and messages
        // as part of the audit message submission
        if (!isLocalPlayback)
        {
            PasHelpers::instance().submitPaBroadcastStationDvaRequestAuditMessage(
                broadcastId,
                messages,
                dwellInSecs,
                hasChime,
                startTime,
                stopTime,
                periodInSecs,
                isSynchronisedWithTis,
                zones,
                isLocalPlayback,
                sessionId);
        }

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            LOG_ERROR("Request failed due to broadcast limit (%s)", CachedConfig::getInstance()->getMaxBroadcasts());

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        // jeffrey++ Bug457
        CORBA::UShort sourceId = 0;
        /*
        CORBA::UShort sourceId;
        if (CachedConfig::getInstance()->getIsMaster())
        {
            sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_DVA_SET_A;
        }
        else
        {
            // Station or Depot
            if (CachedConfig::getInstance()->getIsDepot())
            {
                sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_DVA_SET_D;
            }
            else
            {
                sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_DVA_SET_B;
            }
        }
        */
        // ++jeffrey Bug457

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::StationDvaConfig theSpecifics;

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::StationDva;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = false;
        newBroadcastConfigP->submitTime = time(NULL);

        theSpecifics.dwellInSecs = dwellInSecs;
        theSpecifics.hasChime = hasChime;
        theSpecifics.isSynchronisedWithTis = isSynchronisedWithTis;
        theSpecifics.messages = messages;
        theSpecifics.periodInSecs = periodInSecs;
        // jeffrey++ TES712
        //theSpecifics.priority                   = 0;
        // ++jeffrey TES712
        theSpecifics.sourceId = static_cast<CORBA::Octet>(sourceId);
        theSpecifics.startTime = startTime;
        theSpecifics.stopTime = stopTime;
        theSpecifics.zones = zones;
        // TES 735
        // No audit message needed for local playback
        theSpecifics.isLocalPlayback = isLocalPlayback;

        // Initialise the union inside BroadcastConfig
        newBroadcastConfigP->theBroadcastConfigSpecifics.theStationDvaConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException

            if (CachedConfig::getInstance()->getIsMaster())
            {
                LOG_INFO("broadcastStationDva(): Creating MasterDVABroadcast.");
                newBroadcast = new MasterDVABroadcast(m_tableManager.getTable202(),
                                                      m_tableManager.getTable302(),
                                                      m_tableManager.getTable350(),
                                                      m_tableManager.getTable1200(),
                                                      m_tableManager.getTable1201(),
                                                      m_tableManager.getTable1301(),
                                                      m_tableManager.getTable1610(),
                                                      newBroadcastConfigP, sessionId);
            }
            else
            {
                LOG_INFO("broadcastStationDva(): Creating StationDVABroadcast.");
                newBroadcast = new StationDVABroadcast(m_tableManager.getTable202(),
                                                       m_tableManager.getTable201(),
                                                       m_tableManager.getTable302(),
                                                       m_tableManager.getTable350(),
                                                       m_tableManager.getTable200(),
                                                       m_tableManager.getTable301(),
                                                       newBroadcastConfigP, sessionId);
            }

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& e)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(e);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
        return CORBA::string_dup(newBroadcastConfigP->broadcastId);
    }

    // jeffrey++ TD10377
    void BroadcastManager::agentLinkTest()
    {
        FUNCTION_ENTRY("agentLinkTest");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        // empty method for PA Manager link test only

        FUNCTION_EXIT;
    }

    // ++jeffrey TD10377

    // jeffrey++ TES736
    bool BroadcastManager::hardwareLinkTest(const char* broadcastId, const char* sessionId)
    {
        FUNCTION_ENTRY("hardwareLinkTest");

        // test hardware connection availability
        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        if (PASConnection::getInstance()->isConnected())
        {
            FUNCTION_EXIT;
            return true;
        }
        // TD17372
        else if (strcmp(broadcastId, MANAGER_LAUNCHING_BROADCAST_ID.c_str()) == 0)
        {
            FUNCTION_EXIT;
            return false;
        }
        // TD17372
        else
        {
            AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);
            broadcast->setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Execution);
            PasHelpers::instance().submitPaTerminateBroadcastAuditMessage(broadcastId, sessionId);
            broadcast->operatorAbort();

            FUNCTION_EXIT;
            return false;
        }
    }

    // ++jeffrey TES736

    // exception checked
    char* BroadcastManager::broadcastStationLive(CORBA::Octet sourceId,
                                                 const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones_,
                                                 const char* groupName,
                                                 const char* sessionId)
        // raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", boost::format("BroadcastManager::broadcastStationLive[sourceId=%d]") % (int)sourceId);
        FUNCTION_ENTRY("broadcastStationLive");

        LOGOUT("broadcastStationLive");
        LOG_DEBUG("broadcastStationLive(): %s", nvps(sourceId, zones_, groupName, sessionId));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        auto zones = zones_;

        if (!ThisLocation::is_depot())
        {
            zones = PasHelpers::instance().remove_zones_where_id_larger_than(zones_, 32);
        }

        if (ThisLocation::is_station_or_depot())
        {
            auto zone_ids = PasHelpers::instance().local_zone_coverage_to_zone_id(zones);
            LOG_DEBUG("broadcastStationLive(): %s", nvps(zone_ids));
        }

        // this is to build a bvroadcast ID with more information
        std::string extraInformation(groupName);
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationLive, extraInformation);

        // this will validate the parameters like the zones and messages
        // as part of the audit message submission
        PasHelpers::instance().submitPaBroadcastStationLiveRequestAuditMessage(
            broadcastId,
            sourceId,
            zones,
            sessionId);

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            LOG_ERROR("Request failed due to broadcast limit (%s)", CachedConfig::getInstance()->getMaxBroadcasts());

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::StationLiveConfig theSpecifics;

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::StationLive;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = false;
        newBroadcastConfigP->submitTime = time(NULL);
        // jeffrey++ TES712
        //theSpecifics.priority                   = 0;
        // ++jeffrey TES712
        theSpecifics.sourceId = sourceId;
        theSpecifics.zones = zones;

        newBroadcastConfigP->theBroadcastConfigSpecifics.theStationLiveConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException
            if (CachedConfig::getInstance()->getIsMaster())
            {
                LOG_INFO("Creating MasterLiveBroadcast.");
                newBroadcast = new MasterLiveBroadcast(m_tableManager.getTable202(),
                                                       m_tableManager.getTable302(),
                                                       m_tableManager.getTable590(),
                                                       m_tableManager.getTable1200(),
                                                       m_tableManager.getTable1201(),
                                                       m_tableManager.getTable1301(),
                                                       newBroadcastConfigP, sessionId);
            }
            else
            {
                LOG_INFO("Creating StationLiveBroadcast.");
                newBroadcast = new StationLiveBroadcast(m_tableManager.getTable202(),
                                                        m_tableManager.getTable302(),
                                                        m_tableManager.getTable590(),
                                                        m_tableManager.getTable200(),
                                                        m_tableManager.getTable201(),
                                                        m_tableManager.getTable301(),
                                                        newBroadcastConfigP, sessionId);
            }

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& ex)
        {
            LOG_DEBUG("BadParameterException:%s", ex.reason.in());
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(ex);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
        return CORBA::string_dup(newBroadcastConfigP->broadcastId);
    }

    // song toan++
    // TD10409
    TA_Base_Bus::IPAAgentCorbaDef::EMusicType BroadcastManager::getMusicStatus(const char* sessionId)
    {
        FUNCTION_ENTRY("getMusicStatus");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        FUNCTION_EXIT;
        return m_MusicType;
    }

    // ++song toan
    // TD10409

    void BroadcastManager::broadcastStationMusic(TA_Base_Bus::IPAAgentCorbaDef::EMusicType musicType, const char* sessionId)
        // raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", boost::format("BroadcastManager::broadcastStationMusic[%s]") % st::enum_to_string_value(musicType));
        FUNCTION_ENTRY("broadcastStationMusic");

        LOGOUT("broadcastStationMusic");
        LOG_DEBUG("broadcastStationMusic(): %s", nvps(musicType, sessionId));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationMusic);

        if (CachedConfig::getInstance()->getIsMaster())
        {
            LOG_ERROR("Background music broadcasts are not possible from OCC");

            FUNCTION_EXIT;
            TA_THROW(BadParameterException("Background music broadcasts are not possible from OCC"));
        }

        PasHelpers::instance().submitPaBroadcastStationMusicRequestAuditMessage(
            broadcastId,
            musicType,
            sessionId);

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            LOG_ERROR("Request failed due to broadcast limit (%s)", CachedConfig::getInstance()->getMaxBroadcasts());

            // submit audit message
            TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
            zones.length(0);
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        CORBA::Octet sourceId = 0;

        // Station or Depot
        switch (musicType)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::OccMusic:
                sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_18_OCR_BGM;
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LocalMusic:
                if (CachedConfig::getInstance()->getIsDepot())
                {
                    sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_20_DCR_BGM;
                }
                else
                {
                    sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_19_PSC_BGM;
                }

                break;

                // liuyu++ TD8857
                // case TA_Base_Bus::IPAAgentCorbaDef::NoMusic:
            case TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic:
                // Don't care
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::NoLocalMusic:
                // Don't care
                break;

                // liuyu++ TD8857
            default:

                LOG_ERROR("Invalid music type specified in request");

                // submit audit message
                TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
                zones.length(0);
                //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
                //            broadcastId, zones, true, sessionId);//TD10320

                FUNCTION_EXIT;
                TA_THROW(BadParameterException("Invalid music type specified in request"));
        }

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::StationMusicConfig theSpecifics;

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::StationMusic;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = false;
        newBroadcastConfigP->submitTime = time(NULL);

        theSpecifics.musicType = musicType;
        // jeffrey++ TES712
        //theSpecifics.priority                   = 0;
        // ++jeffrey TES712
        theSpecifics.sourceId = sourceId;

        newBroadcastConfigP->theBroadcastConfigSpecifics.theStationMusicConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException

            newBroadcast = new StationMusicBroadcast(m_tableManager.getTable201(),
                                                     m_tableManager.getTable202(),
                                                     m_tableManager.getTable301(),
                                                     m_tableManager.getTable302(),
                                                     newBroadcastConfigP, sessionId);

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& e)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
            zones.length(0);
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(e);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
            zones.length(0);
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // song toan++
        // TD10409
        switch (musicType)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::OccMusic:
            case TA_Base_Bus::IPAAgentCorbaDef::LocalMusic:
            case TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic:
            case TA_Base_Bus::IPAAgentCorbaDef::NoLocalMusic:
                m_MusicType = musicType;
                break;

            default:
                // impossible to reach here thanks to previous switch statement
                // which thrown an exception
                ;
        }

        // ++song toan
        // TD10409

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
    }

    char* BroadcastManager::recordAdhocMessage(CORBA::Octet sourceId, CORBA::ULong messageKey, const char* sessionId)
        // raises (TA_Base_Core::BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", boost::format("BroadcastManager::recordAdhocMessage[sourceId=%d,messageKey=%d]") % (unsigned int)sourceId % messageKey);
        FUNCTION_ENTRY("recordAdhocMessage");

        LOGOUT("recordAdhocMessage");
        LOG_DEBUG("recordAdhocMessage(): %s", nvps(sourceId, messageKey, sessionId));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationRecording);

        PasHelpers::instance().submitPaRecordAdhocRequestAuditMessage(
            broadcastId,
            sourceId,
            messageKey,
            sessionId);

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            LOG_ERROR("Request failed due to broadcast limit (%s)", CachedConfig::getInstance()->getMaxBroadcasts());

            // submit audit message
            TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
            zones.length(0);
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //           broadcastId, zones, true, sessionId);//TD10320

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::StationRecordingConfig theSpecifics;

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::StationRecording;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = false;
        newBroadcastConfigP->submitTime = time(NULL);

        theSpecifics.messageKey = messageKey;
        theSpecifics.sourceId = sourceId;

        newBroadcastConfigP->theBroadcastConfigSpecifics.theStationRecordingConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException

            newBroadcast = new DVARecordingBroadcast(m_tableManager.getTable590(),
                                                     m_tableManager.getTable630(),
                                                     newBroadcastConfigP, sessionId);

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& e)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
            zones.length(0);
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(e);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
            zones.length(0);
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
        return CORBA::string_dup(newBroadcastConfigP->broadcastId);
    }

    void BroadcastManager::retryStationBroadcast(const char* broadcastId, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones_, const char* sessionId)
        //raises (BroadcastIdNotFound, BadParameterException, TA_Base_Core::AccessDeniedException)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::retryStationBroadcast");
        FUNCTION_ENTRY("retryStationBroadcast");

        LOGOUT("retryStationBroadcast");
        LOG_DEBUG("retryStationBroadcast(): %s", nvps(broadcastId, zones_, sessionId));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        auto zones = zones_;

        if (!ThisLocation::is_depot())
        {
            zones = PasHelpers::instance().remove_zones_where_id_larger_than(zones_, 32);
        }

        if (ThisLocation::is_station_or_depot())
        {
            auto zone_ids = PasHelpers::instance().local_zone_coverage_to_zone_id(zones);
            LOG_DEBUG("retryStationBroadcast(): %s", nvps(zone_ids));
        }

        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Request failed due to invalid BroadcastId", broadcastId);
        }

        AbstractStationBroadcast* stationBroadcast = dynamic_cast<AbstractStationBroadcast*>(broadcast);

        if (stationBroadcast == NULL)
        {
            std::ostringstream msg;
            msg << "The specified broadcastId " << broadcastId << " is not a station broadcast";

            FUNCTION_EXIT;
            TA_THROW(BadParameterException(msg.str().c_str()));
        }

        PasHelpers::instance().submitPaRetryStationBroadcastRequestAuditMessage(
            broadcastId,
            stationBroadcast->getBroadcastType(),
            zones,
            sessionId);

        // Queue actual execution in BroadcastWorker thread
        stationBroadcast->retry(zones, sessionId);

        FUNCTION_EXIT;
    }

    char* BroadcastManager::broadcastTrainDva(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains,
                                              CORBA::ULong messageKey,
                                              CORBA::Short libraryVersion,
                                              const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod,
                                              CORBA::Boolean hasOverrideOption,
                                              const char* sessionId)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached);
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::broadcastTrainDva");
        FUNCTION_ENTRY("broadcastTrainDva");

        LOGOUT("broadcastTrainDva");
        LOG_DEBUG("broadcastTrainDva(): %s", nvps(trains, messageKey, libraryVersion, hasOverrideOption, sessionId));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::TrainDva);

        PasHelpers::instance().submitTrainPAPrerecordedAuditMessage(
            broadcastId,
            trains,
            messageKey,
            libraryVersion,
            announcementPeriod,
            sessionId);

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            std::ostringstream msg;
            msg << "Request failed due to broadcast limit ("
                << CachedConfig::getInstance()->getMaxBroadcasts() << ")";

            LOG_ERROR(msg);

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                msg.str(),
                sessionId);

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::TrainDvaConfig theSpecifics;

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::TrainDva;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = false;
        newBroadcastConfigP->submitTime = time(NULL);

        theSpecifics.hasOverrideOption = hasOverrideOption;
        theSpecifics.libraryVersion = libraryVersion;
        theSpecifics.messageKey = messageKey;
        theSpecifics.announcementPeriod = announcementPeriod;
        theSpecifics.trains = trains;

        newBroadcastConfigP->theBroadcastConfigSpecifics.theTrainDvaConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException

            newBroadcast = new TrainDVABroadcast(newBroadcastConfigP, sessionId);

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& e)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                std::string(e.reason),
                sessionId);

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(e);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                "",
                sessionId);

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
        return CORBA::string_dup(newBroadcastConfigP->broadcastId);
    }

    char* BroadcastManager::broadcastTrainLive(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::Boolean hasOverrideOption, const char* staticGroup, const char* sessionId)
    {
        FUNCTION_ENTRY("broadcastTrainLive");

        LOGOUT("broadcastTrainLive");
        LOG_DEBUG("broadcastTrainLive(): %s", nvps(trains, hasOverrideOption, staticGroup, sessionId));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::TrainLive);

        if (CachedConfig::getInstance()->getLocalTrainAgentEntityName().empty())
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The local Train Agent cannot be resolved uniquely in the database.  Train PA broadcasts are disabled."));
        }

        PasHelpers::instance().submitTrainPASetupLiveAnnouncementAuditMessage(
            broadcastId,
            trains,
            sessionId);

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            std::ostringstream msg;
            msg << "Request failed due to broadcast limit ("
                << CachedConfig::getInstance()->getMaxBroadcasts() << ")";

            LOG_ERROR(msg);

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                msg.str(),
                sessionId);

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::TrainLiveConfig theSpecifics;

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::TrainLive;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = false;
        newBroadcastConfigP->submitTime = time(NULL);

        theSpecifics.hasOverrideOption = hasOverrideOption;
        theSpecifics.trains = trains;
        theSpecifics.staticGroupName = staticGroup;

        newBroadcastConfigP->theBroadcastConfigSpecifics.theTrainLiveConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException

            newBroadcast = new TrainLiveBroadcast(newBroadcastConfigP, sessionId);

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& e)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                std::string(e.reason),
                sessionId);

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(e);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                "",
                sessionId);

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
        return CORBA::string_dup(newBroadcastConfigP->broadcastId);
    }

    void BroadcastManager::beginTrainLiveAnnouncement(const char* broadcastId)
        //raises (BadParameterException, BroadcastIdNotFound);
    {
        FUNCTION_ENTRY("beginTrainLiveAnnouncement");

        LOGOUT("beginTrainLiveAnnouncement");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        if (CachedConfig::getInstance()->getLocalTrainAgentEntityName().empty())
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The local Train Agent cannot be resolved uniquely in the database.  Train PA broadcasts are disabled."));
        }

        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to begin a train live announcement on a non-existent broadcast", broadcastId);
        }

        TrainLiveBroadcast* trainLiveBroadcast = dynamic_cast<TrainLiveBroadcast*>(broadcast);

        if (trainLiveBroadcast == NULL)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified broadcastId is not a train live broadcast"));
        }

        trainLiveBroadcast->beginTrainLiveAnnouncement();

        FUNCTION_EXIT;
    }

    void BroadcastManager::continueTrainLiveAnnouncement(const char* broadcastId)
    {
        FUNCTION_ENTRY("continueTrainLiveAnnouncement");

        LOGOUT("continueTrainLiveAnnouncement");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to continue a train live announcement on a non-existent broadcast", broadcastId);
        }

        TrainLiveBroadcast* trainLiveBroadcast = dynamic_cast<TrainLiveBroadcast*>(broadcast);

        if (trainLiveBroadcast == NULL)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified broadcastId is not a train live broadcast"));
        }

        //      PasHelpers::instance().submitTrainPAContinueLiveAnnouncementAuditMessage(
        //          broadcastId,
        //          trainId );

        trainLiveBroadcast->continueTrainLiveAnnouncement();

        FUNCTION_EXIT;
    }

    void BroadcastManager::endTrainLiveAnnouncement(const char* broadcastId)
        //raises (BadParameterException, BroadcastIdNotFound);
    {
        FUNCTION_ENTRY("endTrainLiveAnnouncement");

        LOGOUT("endTrainLiveAnnouncement");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to end a train live announcement on a non-existent broadcast", broadcastId);
        }

        TrainLiveBroadcast* trainLiveBroadcast = dynamic_cast<TrainLiveBroadcast*>(broadcast);

        if (trainLiveBroadcast == NULL)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified broadcastId is not a train live broadcast"));
        }

        // Extensive audit logging
        PasHelpers::instance().submitTrainPAEndLiveAnnouncementAuditMessage(
            broadcastId,
            trainLiveBroadcast->getTrainCoverage());

        trainLiveBroadcast->endTrainLiveAnnouncement();

        FUNCTION_EXIT;
    }

    void BroadcastManager::retryTrainBroadcast(const char* broadcastId,
                                               const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains,
                                               CORBA::Boolean hasOverrideOption,
                                               const char* sessionId)
        //raises (BadParameterException, BroadcastIdNotFound, TA_Base_Core::AccessDeniedException);
    {
        FUNCTION_ENTRY("retryTrainBroadcast");

        LOGOUT("retryTrainBroadcast");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to retry a non-existent broadcast", broadcastId);
        }

        AbstractTrainBroadcast* trainBroadcast = dynamic_cast<AbstractTrainBroadcast*>(broadcast);

        if (trainBroadcast == NULL)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified broadcastId is not a train broadcast"));
        }

        // log as a new broadcast - or change later to have a retry message
        switch (trainBroadcast->getBroadcastType())
        {
            case TA_Base_Bus::IPAAgentCorbaDef::TrainLive:
                PasHelpers::instance().submitTrainPASetupLiveAnnouncementAuditMessage(
                    broadcastId,
                    trains,
                    sessionId);
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::TrainDva:
                TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfig = trainBroadcast->getBroadcastConfigCopy();

                try
                {
                    PasHelpers::instance().submitTrainPAPrerecordedAuditMessage(
                        broadcastId,
                        trains,
                        broadcastConfig->theBroadcastConfigSpecifics.theTrainDvaConfig().messageKey,
                        broadcastConfig->theBroadcastConfigSpecifics.theTrainDvaConfig().libraryVersion,
                        broadcastConfig->theBroadcastConfigSpecifics.theTrainDvaConfig().announcementPeriod,
                        sessionId);
                }
                catch (const BadParameterException& bpe)
                {
                    PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                        trains,
                        CachedConfig::getInstance()->getAgentLocationKey(),
                        std::string(bpe.reason),
                        sessionId);

                    delete broadcastConfig;
                    broadcastConfig = nullptr;

                    FUNCTION_EXIT;
                    TA_RETHROW(bpe);
                }

                delete broadcastConfig;
                broadcastConfig = nullptr;
                break;
        }

        // ToDo - use new session id?
        // Queue actual execution in BroadcastWorker thread
        trainBroadcast->retry(trains, hasOverrideOption, sessionId);

        FUNCTION_EXIT;
    }

    void BroadcastManager::terminateBroadcast(const char* broadcastId, const char* sessionId)
        //raises (BroadcastIdNotFound, TA_Base_Core::AccessDeniedException)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::terminateBroadcast");
        FUNCTION_ENTRY("terminateBroadcast");

        LOGOUT("terminateBroadcast");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to terminate a non-existent broadcast ", broadcastId);
        }

        // TES 735 - dont display event for playback
        // jeffrey++ TD10333
        // Merge the conditions to make the playback aborting just return without
        // submitting message. Otherwise, submit terminating message.
        if (broadcast->getBroadcastType() == TA_Base_Bus::IPAAgentCorbaDef::StationDva)
        {
            TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* config = broadcast->getBroadcastConfigCopy();

            /*    if (! config->theBroadcastConfigSpecifics.theStationDvaConfig().isLocalPlayback)

                {
                    PasHelpers::instance().submitPaTerminateBroadcastAuditMessage( broadcastId, sessionId );
                }

                delete config;
            }
            else
            {
                PasHelpers::instance().submitPaTerminateBroadcastAuditMessage( broadcastId, sessionId );
            }

            if (broadcast->getBroadcastState() != TA_Base_Bus::IPAAgentCorbaDef::Termination &&
                broadcast->getBroadcastState() != TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval)
            {
                // Queue actual termination in BroadcastWorker thread
                broadcast->operatorAbort();
            }
            */
            if (!config->theBroadcastConfigSpecifics.theStationDvaConfig().isLocalPlayback)
            {
                PasHelpers::instance().submitPaTerminateBroadcastAuditMessage(broadcastId, sessionId);
            }

            delete config;
        }
        else
        {
            PasHelpers::instance().submitPaTerminateBroadcastAuditMessage(broadcastId, sessionId);
        }

        if (broadcast->getBroadcastState() != TA_Base_Bus::IPAAgentCorbaDef::Termination &&
            broadcast->getBroadcastState() != TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval)
        {
            // Queue actual termination in BroadcastWorker thread
            broadcast->operatorAbort();
        }

        // ++jeffrey TD10333

        FUNCTION_EXIT;
    }

    void BroadcastManager::removeBroadcast(const char* broadcastId, const char* sessionId)
        //raises (BroadcastIdNotFound, BadParameterException, TA_Base_Core::AccessDeniedException)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::removeBroadcast");
        FUNCTION_ENTRY("removeBroadcast");

        LOGOUT("removeBroadcast");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to remove a non-existent broadcast", broadcastId);
        }

        if ((broadcast->getBroadcastState() == TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval) ||
            (broadcast->getBroadcastState() == TA_Base_Bus::IPAAgentCorbaDef::Termination))
        {
            // TES 735 - no audit for local playback
            if (broadcast->getBroadcastType() == TA_Base_Bus::IPAAgentCorbaDef::StationDva)
            {
                TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* config = broadcast->getBroadcastConfigCopy();

                if (!config->theBroadcastConfigSpecifics.theStationDvaConfig().isLocalPlayback)
                {
                    PasHelpers::instance().submitPaRemoveBroadcastAuditMessage(broadcastId, sessionId);
                }

                delete config;
            }
            else
            {
                PasHelpers::instance().submitPaRemoveBroadcastAuditMessage(broadcastId, sessionId);
            }

            // Flag the table for deletion when the BroadcastWorker is ready
            broadcast->setIsTaggedForRemoval(true);
        }
        else
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("Broadcast must be terminated first"));
        }

        FUNCTION_EXIT;
    }

    // jeffrey++ TD10853
    //void BroadcastManager::setAdhocType
    bool BroadcastManager::setAdhocType(CORBA::ULong messageKey, CORBA::Char messageType, const char* sessionId)
        //raises (BadParameterException, FepcCommsFailure, DatabaseCommsFailure, TA_Base_Core::AccessDeniedException);
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::setAdhocType");
        FUNCTION_ENTRY("setAdhocType");
        LOGOUT("setAdhocType");
        LOG_DEBUG("setAdhocType(): %s", nvps(messageKey, messageType, sessionId));

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        // Check the validity of the new messageType
        if (messageType != 'E' &&
            messageType != 'N' &&
            messageType != 'X')
        {
            TA_THROW(BadParameterException("The specified messageType is not recognised"));
        }

        // Check the validity of the messageKey

        TA_IRS_Bus::StationDvaMessageRecord record;

        try
        {
            record = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
        }
        catch (TA_Base_Core::CachedMappingNotFoundException&)
        {
            TA_THROW(BadParameterException("The specified message does not exist in the database"));
        }

        // Check if the messageKey is from this location
        if (CachedConfig::getInstance()->getAgentLocationKey() != record.m_locationKey)
        {
            TA_THROW(BadParameterException("The specified message does not belong to this location"));
        }

        // Check if the messageKey is an adhoc message
        int adhocOffset = PasHelpers::instance().getAdhocOffset(messageKey);

        if (adhocOffset < 0 || adhocOffset >= TA_Base_Bus::IPAAgentCorbaDef::PA_NBADHOC)
        {
            // out of range
            TA_THROW(BadParameterException("The specified message is not an ad-hoc message"));
        }

        // dont set the type if it is being set to the same thing
        if (record.m_type == messageType)
        {
            LOG_INFO("Received a request to set Adhoc Type to the current Adhoc type - Ignoring.");

            // jeffrey++ TD10853
            //return;
            return false;
            // ++jeffrey TD10853
        }

        try
        {
            m_tableManager.getTable604().setTableDataAndWrite(messageKey, messageType);

            // audit the change
            PasHelpers::instance().submitPaSetAdhocTypeAuditMessage(
                record.m_type,
                messageType,
                messageKey,
                sessionId);
        }
        catch (TA_Base_Core::PasWriteErrorException& e)
        {
            LOG_ERROR("setAdhocType(): PasWriteErrorException: %s", st::enum_to_string_value(static_cast<AbstractPASTable::EPasWriteErrors>(e.getErrorValue())));

            if (e.getErrorValue() == AbstractPASTable::PAS_ERR_GENERIC)
            {
                TA_THROW(BadParameterException("The request was not recognised by the PAS FEPC"));
            }

            // not expected to get here
            PasHelpers::instance().throwFepcCommsFailureException("Unexpected response from the PAS FEPC");
        }
        catch (TA_Base_Core::PasConnectionException&)
        {
            PasHelpers::instance().throwFepcCommsFailureException("Failed to write to table 604");
        }

        // Update CachedMaps and the database

        try
        {
            record.m_type = messageType;
            TA_IRS_Bus::CachedMaps::getInstance()->setStationDvaMessageRecord(record, true);
        }
        catch (...)
        {
            // Expecting either :
            // TA_Base_Core::DataException
            // TA_Base_Core::DatabaseException
            // TA_Base_Core::DataConfigurationException

            TA_THROW(DatabaseErrorException());
        }

        // Success - now notify all open PA Managers

        TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData updateData;

        // Only one entry
        updateData.length(1);

        updateData[0].m_pKey = record.m_pKey;
        updateData[0].m_id = record.m_id;
        updateData[0].m_locationKey = record.m_locationKey;
        updateData[0].m_label = record.m_label.c_str();
        updateData[0].m_type = record.m_type;
        updateData[0].m_tisMessageTag = record.m_tisMessageTag;
        updateData[0].m_tisLibrarySection = record.m_tisLibrarySection;
        updateData[0].m_tisLibraryVersion = record.m_tisLibraryVersion;

        PasHelpers::instance().sendDvaMessagesUpdate(updateData);

        FUNCTION_EXIT;
        // jeffrey++ TD10853
        return true;
        // ++jeffrey TD10853
    }

    void BroadcastManager::setAdhocLabel(CORBA::ULong messageKey, const char* label, const char* sessionId)
        //raises (BadParameterException, DatabaseCommsFailure, TA_Base_Core::AccessDeniedException);
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::setAdhocLabel");
        FUNCTION_ENTRY("setAdhocLabel");
        LOG_DEBUG("setAdhocLabel(): %s", nvps(messageKey, label, sessionId));

        LOGOUT("setAdhocLabel");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        // Check the validity of the label
        if (strlen(label) > TA_Base_Bus::IPAAgentCorbaDef::MAX_ADHOC_LABEL_LENGTH)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified label is greater than 120 characters"));
        }

        // Check the validity of the messageKey
        TA_IRS_Bus::StationDvaMessageRecord record;

        try
        {
            record = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
        }
        catch (TA_Base_Core::CachedMappingNotFoundException&)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified message does not exist in the database"));
        }

        // Check if the messageKey is from this location
        if (CachedConfig::getInstance()->getAgentLocationKey() != record.m_locationKey)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified message does not belong to this location"));
        }

        // Check if the messageKey is an adhoc message
        int adhocOffset = PasHelpers::instance().getAdhocOffset(messageKey);

        if (adhocOffset < 0 || adhocOffset >= TA_Base_Bus::IPAAgentCorbaDef::PA_NBADHOC)
        {
            // out of range

            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The specified message is not an ad-hoc message"));
        }

        // dont set the label if it is being set to the same thing
        if (record.m_label == label)
        {
            LOG_INFO("Received a request to set Adhoc Label to the current Adhoc Label - Ignoring.");

            FUNCTION_EXIT;
            return;
        }

        // Good to go

        try
        {
            std::string oldLabel = record.m_label;

            record.m_label = label;
            TA_IRS_Bus::CachedMaps::getInstance()->setStationDvaMessageRecord(record, true);

            PasHelpers::instance().submitPaSetAdhocLabelAuditMessage(
                oldLabel,
                label,
                messageKey,
                sessionId);
        }
        catch (...)
        {
            // Expecting either :
            // TA_Base_Core::DataException
            // TA_Base_Core::DatabaseException
            // TA_Base_Core::DataConfigurationException

            FUNCTION_EXIT;
            TA_THROW(DatabaseErrorException());
        }

        // Success - now notify all open PA Managers

        TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData updateData;

        // Only one entry
        updateData.length(1);

        updateData[0].m_pKey = record.m_pKey;
        updateData[0].m_id = record.m_id;
        updateData[0].m_locationKey = record.m_locationKey;
        updateData[0].m_label = record.m_label.c_str();
        updateData[0].m_type = record.m_type;
        updateData[0].m_tisMessageTag = record.m_tisMessageTag;
        updateData[0].m_tisLibrarySection = record.m_tisLibrarySection;
        updateData[0].m_tisLibraryVersion = record.m_tisLibraryVersion;
        PasHelpers::instance().sendDvaMessagesUpdate(updateData);

        FUNCTION_EXIT;
    }

    TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq* BroadcastManager::getBroadcastIdTypes()
    {
        FUNCTION_ENTRY("getBroadcastIdTypes");

        LOGOUT("GetBroadcastIdsgetBroadcastIdTypes");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq* broadcastIdTypes = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq();

        m_allBroadcasts->getAllBroadcastIdTypes(*broadcastIdTypes);

        FUNCTION_EXIT;
        return broadcastIdTypes;
    }

    void BroadcastManager::changeBroadcastId(const char* fromBroadcastId, const char* toBroadcastId, const char* sessionId)
        // raises (BroadcastIdNotFound, BadParameterException, TA_Base_Core::AccessDeniedException)
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::changeBroadcastId");
        LOG_SCOPE("BroadcastManager::changeBroadcastId");
        FUNCTION_ENTRY("changeBroadcastId");
        LOG_DEBUG("changeBroadcastId(): %s", nvps(fromBroadcastId, toBroadcastId, sessionId));

        LOGOUT("changeBroadcastId");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(fromBroadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to change a non-existent broadcast", fromBroadcastId);
        }

        AbstractBroadcast* nonExistingBroadcast = m_allBroadcasts->getBroadcast(toBroadcastId);

        if (nonExistingBroadcast != NULL)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("NewBroadcastId is used by another broadcast"));
        }

        broadcast->setBroadcastId(toBroadcastId);

        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        updateData.fromBroadcastId = fromBroadcastId;
        updateData.toBroadcastId = toBroadcastId;
        updateData.broadcastType = broadcast->getBroadcastType();
        updateData.sessionId = broadcast->getSessionId().c_str();

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentRenamedBroadcastUpdate(updateData);

        PasHelpers::instance().submitPaChangeBroadcastIdAuditMessage(fromBroadcastId,
                                                                     toBroadcastId,
                                                                     sessionId);

        FUNCTION_EXIT;
    }

    TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* BroadcastManager::getBroadcastConfig(const char* broadcastId)
        // raises (BroadcastIdNotFound)
    {
        FUNCTION_ENTRY("getBroadcastConfig");

        LOGOUT("getBroadcastConfig");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to return the broadcast config for a non-existent broadcast", broadcastId);
        }

        // Return the deep copy
        FUNCTION_EXIT;
        return broadcast->getBroadcastConfigCopy();
    }

    TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* BroadcastManager::getProgressReport(const char* broadcastId)
        // raises (BroadcastIdNotFound)
    {
        FUNCTION_ENTRY("getProgressReport");

        LOGOUT("getProgressReport");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(broadcastId);

        if (broadcast == NULL)
        {
            PasHelpers::instance().throwBroadcastIdNotFoundException("Failed to return the broadcast config for a non-existent broadcast", broadcastId);
        }

        // Return the deep copy
        FUNCTION_EXIT;
        return broadcast->getProgressReportCopy();
    }

    TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* BroadcastManager::getPriorityScheme()
        // raises (FepcCommsFailure)
    {
        FUNCTION_ENTRY("getPriorityScheme");

        LOGOUT("getPriorityScheme");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* newScheme = NULL;

        try
        {
            newScheme = m_tableManager.getTable100().getPrioritySchemeCopy();
        }
        catch (TA_Base_Core::PasConnectionException&)
        {
            PasHelpers::instance().throwFepcCommsFailureException("Failed to request priority scheme data from the PAS FEPC");
        }

        FUNCTION_EXIT;
        return newScheme;
    }

    TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecords* BroadcastManager::getStationDvaMessageRecords()
    {
        FUNCTION_ENTRY("getStationDvaMessageRecords");

        LOGOUT("getStationDvaMessageRecords");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecords* newData = NULL;

        TA_IRS_Bus::PaStationDvaMessageCachedMap& theMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
        std::list<TA_IRS_Bus::StationDvaMessageRecord> allRecords = theMap.getAllRecords();
        std::list<TA_IRS_Bus::StationDvaMessageRecord>::iterator it = allRecords.begin();

        newData = new TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData();
        newData->length(allRecords.size());

        for (int index = 0; it != allRecords.end(); ++it, ++index)
        {
            (*newData)[index].m_pKey = (*it).m_pKey;
            (*newData)[index].m_id = (*it).m_id;
            (*newData)[index].m_locationKey = (*it).m_locationKey;
            (*newData)[index].m_label = (*it).m_label.c_str();
            (*newData)[index].m_type = (*it).m_type;
            (*newData)[index].m_tisMessageTag = (*it).m_tisMessageTag;
            (*newData)[index].m_tisLibrarySection = (*it).m_tisLibrarySection;
            (*newData)[index].m_tisLibraryVersion = (*it).m_tisLibraryVersion;
        }

        // There should be no exceptions thrown above
        FUNCTION_EXIT;
        return newData;
    }

    TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords* BroadcastManager::getTrainDvaMessageRecords()
    {
        FUNCTION_ENTRY("getTrainDvaMessageRecords");

        LOGOUT("getTrainDvaMessageRecords");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords* newData = NULL;

        TA_IRS_Bus::PaTrainDvaMessageCachedMap& theMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaTrainDvaMessageCachedMap();
        std::list<TA_IRS_Bus::TrainDvaMessageRecord> allRecords = theMap.getAllRecords();
        std::list<TA_IRS_Bus::TrainDvaMessageRecord>::iterator it = allRecords.begin();

        newData = new TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords();
        newData->length(allRecords.size());

        for (int index = 0; it != allRecords.end(); ++it, ++index)
        {
            (*newData)[index].m_pKeyOrId = (*it).m_pKeyOrId;
            (*newData)[index].m_label = (*it).m_label.c_str();
        }

        // There should be no exceptions thrown above
        FUNCTION_EXIT;
        return newData;
    }

    TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords* BroadcastManager::getDvaVersionRecords()
    {
        FUNCTION_ENTRY("getDvaVersionRecords");

        LOGOUT("getDvaVersionRecords");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords* newData = NULL;

        TA_IRS_Bus::PaDvaVersionCachedMap& theMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaDvaVersionCachedMap();
        std::list<TA_IRS_Bus::DvaVersionRecord> allRecords = theMap.getAllRecords();
        std::list<TA_IRS_Bus::DvaVersionRecord>::iterator it = allRecords.begin();

        newData = new TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords();
        newData->length(allRecords.size());

        for (int index = 0; it != allRecords.end(); ++it, ++index)
        {
            (*newData)[index].m_pKey = (*it).m_pKey;
            (*newData)[index].m_locationKey = (*it).m_locationKey;
            (*newData)[index].m_privateAdhoc1 = (*it).m_privateAdhoc1.c_str();
            (*newData)[index].m_privateAdhoc2 = (*it).m_privateAdhoc2.c_str();
            (*newData)[index].m_privateAdhoc3 = (*it).m_privateAdhoc3.c_str();
            (*newData)[index].m_privateAdhoc4 = (*it).m_privateAdhoc4.c_str();
            (*newData)[index].m_privateAdhoc5 = (*it).m_privateAdhoc5.c_str();
            (*newData)[index].m_privateAdhoc6 = (*it).m_privateAdhoc6.c_str();
            (*newData)[index].m_privatePreRecorded = (*it).m_privatePreRecorded.c_str();
            (*newData)[index].m_publicAdhoc1 = (*it).m_publicAdhoc1.c_str();
            (*newData)[index].m_publicAdhoc2 = (*it).m_publicAdhoc2.c_str();
            (*newData)[index].m_publicAdhoc3 = (*it).m_publicAdhoc3.c_str();
            (*newData)[index].m_publicAdhoc4 = (*it).m_publicAdhoc4.c_str();
            (*newData)[index].m_publicAdhoc5 = (*it).m_publicAdhoc5.c_str();
            (*newData)[index].m_publicAdhoc6 = (*it).m_publicAdhoc6.c_str();
            (*newData)[index].m_publicPreRecorded = (*it).m_publicPreRecorded.c_str();
        }

        // There should be no exceptions thrown above
        FUNCTION_EXIT;
        return newData;
    }

    TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* BroadcastManager::getDvaPublicVersionRecords()
    {
        FUNCTION_ENTRY("getDvaPublicVersionRecords");

        LOGOUT("getDvaPublicVersionRecords");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* newData = NULL;

        TA_IRS_Bus::PaDvaPublicVersionCachedMap& theMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaDvaPublicVersionCachedMap();
        std::list<TA_IRS_Bus::DvaPublicVersionRecord> allRecords = theMap.getAllRecords();
        std::list<TA_IRS_Bus::DvaPublicVersionRecord>::iterator it = allRecords.begin();

        newData = new TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords();
        newData->length(allRecords.size());

        for (int index = 0; it != allRecords.end(); ++it, ++index)
        {
            (*newData)[index].m_pKey = (*it).m_pKey;
            (*newData)[index].m_locationKey = (*it).m_locationKey;
            (*newData)[index].m_publicAdhoc1 = (*it).m_publicAdhoc1.c_str();
            (*newData)[index].m_publicAdhoc2 = (*it).m_publicAdhoc2.c_str();
            (*newData)[index].m_publicAdhoc3 = (*it).m_publicAdhoc3.c_str();
            (*newData)[index].m_publicAdhoc4 = (*it).m_publicAdhoc4.c_str();
            (*newData)[index].m_publicAdhoc5 = (*it).m_publicAdhoc5.c_str();
            (*newData)[index].m_publicAdhoc6 = (*it).m_publicAdhoc6.c_str();
            (*newData)[index].m_publicPreRecorded = (*it).m_publicPreRecorded.c_str();
        }

        // There should be no exceptions thrown above
        FUNCTION_EXIT;
        return newData;
    }

    TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords* BroadcastManager::getDvaPrivateVersionRecords()
    {
        FUNCTION_ENTRY("getDvaPrivateVersionRecords");

        LOGOUT("getDvaPrivateVersionRecords");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords* newData = NULL;

        TA_IRS_Bus::PaDvaPrivateVersionCachedMap& theMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaDvaPrivateVersionCachedMap();
        std::list<TA_IRS_Bus::DvaPrivateVersionRecord> allRecords = theMap.getAllRecords();
        std::list<TA_IRS_Bus::DvaPrivateVersionRecord>::iterator it = allRecords.begin();

        newData = new TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords();
        newData->length(allRecords.size());

        for (int index = 0; it != allRecords.end(); ++it, ++index)
        {
            (*newData)[index].m_pKey = (*it).m_pKey;
            (*newData)[index].m_locationKey = (*it).m_locationKey;
            (*newData)[index].m_privateAdhoc1 = (*it).m_privateAdhoc1.c_str();
            (*newData)[index].m_privateAdhoc2 = (*it).m_privateAdhoc2.c_str();
            (*newData)[index].m_privateAdhoc3 = (*it).m_privateAdhoc3.c_str();
            (*newData)[index].m_privateAdhoc4 = (*it).m_privateAdhoc4.c_str();
            (*newData)[index].m_privateAdhoc5 = (*it).m_privateAdhoc5.c_str();
            (*newData)[index].m_privateAdhoc6 = (*it).m_privateAdhoc6.c_str();
            (*newData)[index].m_privatePreRecorded = (*it).m_privatePreRecorded.c_str();
        }

        // There should be no exceptions thrown above
        FUNCTION_EXIT;
        return newData;
    }

    TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords* BroadcastManager::getPaZoneRecords()
    {
        FUNCTION_ENTRY("getPaZoneRecords");

        LOGOUT("getPaZoneRecords");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords* newData = NULL;

        TA_IRS_Bus::PaZoneCachedMap& theMap = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneCachedMap();
        std::list<TA_IRS_Bus::PaZoneRecord> allRecords = theMap.getAllRecords();
        std::list<TA_IRS_Bus::PaZoneRecord>::iterator it = allRecords.begin();

        newData = new TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords();
        newData->length(allRecords.size());

        for (int index = 0; it != allRecords.end(); ++it, ++index)
        {
            (*newData)[index].m_pKey = (*it).m_pKey;
            (*newData)[index].m_locationKey = (*it).m_locationKey;
            (*newData)[index].m_locationName = (*it).m_locationName.c_str();
            (*newData)[index].m_occLocationName = (*it).m_occLocationName.c_str();
            (*newData)[index].m_id = (*it).m_id;
            // liuyu++ TD8849
            (*newData)[index].m_orderId = (*it).m_orderId;
            // ++liuyu TD8849
            (*newData)[index].m_label = (*it).m_label.c_str();
            (*newData)[index].m_area = (*it).m_area.c_str();
            (*newData)[index].m_stnZoneShortName = (*it).m_stnZoneShortName.c_str();
            (*newData)[index].m_occZoneShortName = (*it).m_occZoneShortName.c_str();
            (*newData)[index].m_stnEquipmentEntityName = (*it).m_stnEquipmentEntityName.c_str();
            (*newData)[index].m_occEquipmentEntityName = (*it).m_occEquipmentEntityName.c_str();

            // Copy the list of equipment entity names

            const std::list<std::string>& equipmentEntities = (*it).m_equipmentEntities;

            (*newData)[index].m_equipmentEntities.length(equipmentEntities.size());
            std::list<std::string>::const_iterator eqit = equipmentEntities.begin();

            for (int eqindex = 0; eqit != equipmentEntities.end(); ++eqit, ++eqindex)
            {
                (*newData)[index].m_equipmentEntities[eqindex] = (*eqit).c_str();
            }
        }

        // There should be no exceptions thrown above
        FUNCTION_EXIT;
        return newData;
    }

    void BroadcastManager::broadcastEventTriggeredStationDva(CORBA::ULong messageKey,
                                                             const char* messageVersionNumber,
                                                             CORBA::Boolean hasChime,
                                                             CORBA::Boolean isSynchronisedWithTis,
                                                             CORBA::ULong durationInSecs,
                                                             CORBA::UShort periodInSecs,
                                                             const char* sessionId,
                                                             const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones_)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredStationDva");
        FUNCTION_ENTRY("broadcastEventTriggeredStationDva");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();

        auto zones = zones_;

        if (!ThisLocation::is_depot())
        {
            zones = PasHelpers::instance().remove_zones_where_id_larger_than(zones_, 32);
        }

        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationDva);

        // Check station DVA message version
        std::string currentVersionNumber =
            // TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessageVersionFromStationDvaMessageKey(messageKey);
            TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessagePrivateVersionFromStationDvaMessageKey(messageKey);

        if (currentVersionNumber != messageVersionNumber)
        {
            // Only log the error as an audit message

            FUNCTION_EXIT;
            TA_THROW(BadParameterException("Event triggered station DVA message broadcast failed - version inconsistency"));
        }

        PasHelpers::instance().submitPaBroadcastEventTriggeredStationDvaRequestAuditMessage(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            zones,
            sessionId);

        broadcastEventTriggeredStationDvaInternal(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            zones,
            sessionId);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredStationDvaInternal(const std::string& broadcastId,
                                                                     const CORBA::ULong& messageKey,
                                                                     const char* messageVersionNumber,
                                                                     const CORBA::Boolean& hasChime,
                                                                     const CORBA::Boolean& isSynchronisedWithTis,
                                                                     const CORBA::ULong& durationInSecs,
                                                                     const CORBA::UShort& periodInSecs,
                                                                     const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones_,
                                                                     const char* sessionId)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredStationDvaInternal");
        FUNCTION_ENTRY("broadcastEventTriggeredStationDvaInternal");

        LOGOUT("broadcastEventTriggeredStationDva");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        auto zones = zones_;

        if (!ThisLocation::is_depot())
        {
            zones = PasHelpers::instance().remove_zones_where_id_larger_than(zones_, 32);
        }

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            LOG_ERROR("Request failed due to broadcast limit (%s)", CachedConfig::getInstance()->getMaxBroadcasts());

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        // jeffrey++ Bug457
        CORBA::UShort sourceId = 0;
        /*
        CORBA::UShort sourceId;
        if (CachedConfig::getInstance()->getIsMaster())
        {
            sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_DVA_SET_A;
        }
        else
        {
            // Station or Depot
            if (CachedConfig::getInstance()->getIsDepot())
            {
                sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_DVA_SET_D;
            }
            else
            {
                sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_DVA_SET_B;
            }
        }
        */
        // ++jeffrey Bug457

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::StationDvaConfig theSpecifics;

        // Setup the structure for only one message as event triggered broadcasts are
        // never multi-broadcasts
        TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq messages;
        messages.length(1);
        messages[0] = messageKey;

        time_t startTime;
        time_t stopTime;

        if (durationInSecs == 0)
        {
            // Oneshot starting from now
            startTime = 0;
            stopTime = 0;
        }
        else
        {
            startTime = time(NULL);
            stopTime = startTime + durationInSecs;
        }

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::StationDva;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = true;
        newBroadcastConfigP->submitTime = time(NULL);

        theSpecifics.dwellInSecs = 0;
        theSpecifics.hasChime = hasChime;
        theSpecifics.isSynchronisedWithTis = isSynchronisedWithTis;
        theSpecifics.messages = messages;
        theSpecifics.periodInSecs = periodInSecs;
        // jeffrey++ TES712
        //theSpecifics.priority                   = 0;
        // ++jeffrey TES712
        theSpecifics.sourceId = static_cast<CORBA::Octet>(sourceId);
        theSpecifics.startTime = startTime;
        theSpecifics.stopTime = stopTime;
        theSpecifics.zones = zones;
        theSpecifics.isLocalPlayback = false;

        // Initialise the union inside BroadcastConfig
        newBroadcastConfigP->theBroadcastConfigSpecifics.theStationDvaConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException

            if (CachedConfig::getInstance()->getIsMaster())
            {
                LOG_INFO("Creating MasterDVABroadcast.");
                newBroadcast = new MasterDVABroadcast(m_tableManager.getTable202(),
                                                      m_tableManager.getTable302(),
                                                      m_tableManager.getTable350(),
                                                      m_tableManager.getTable1200(),
                                                      m_tableManager.getTable1201(),
                                                      m_tableManager.getTable1301(),
                                                      m_tableManager.getTable1610(),
                                                      newBroadcastConfigP, sessionId);
            }
            else
            {
                LOG_INFO("Creating StationLiveBroadcast.");
                newBroadcast = new StationDVABroadcast(m_tableManager.getTable202(),
                                                       m_tableManager.getTable201(),
                                                       m_tableManager.getTable302(),
                                                       m_tableManager.getTable350(),
                                                       m_tableManager.getTable200(),
                                                       m_tableManager.getTable301(),
                                                       newBroadcastConfigP, sessionId);
            }

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& e)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(e);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            //        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            //            broadcastId, zones, true, sessionId);//TD10320

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredStationDvaForStation(CORBA::ULong messageKey,
                                                                       const char* messageVersionNumber,
                                                                       CORBA::Boolean hasChime,
                                                                       CORBA::Boolean isSynchronisedWithTis,
                                                                       CORBA::ULong durationInSecs,
                                                                       CORBA::UShort periodInSecs,
                                                                       const char* sessionId)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredStationDvaForStation");
        FUNCTION_ENTRY("broadcastEventTriggeredStationDvaForStation");

        LOGOUT("broadcastEventTriggeredStationDvaForStation");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationDva);

        // Check station DVA message version
        std::string currentVersionNumber =
            // TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessageVersionFromStationDvaMessageKey(messageKey);
            TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessagePrivateVersionFromStationDvaMessageKey(messageKey);

        if (currentVersionNumber != messageVersionNumber)
        {
            // Only log the error as an audit message

            FUNCTION_EXIT;
            TA_THROW(BadParameterException("Event triggered station DVA message broadcast failed - version inconsistency"));
        }

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;

        // Find all valid PA Zones for this location
        TA_IRS_Bus::PaZoneRecords records =
            TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneRecordsFromLocationKey(CachedConfig::getInstance()->getAgentLocationKey());

        zones.length(records.size());

        TA_IRS_Bus::PaZoneRecordsIt it = records.begin();
        int index = 0;
        int removedVirtualZones = 0;

        for (; it != records.end(); ++it, ++index)
        {
            if ((*it).m_stnEquipmentEntityName.empty() &&
                (*it).m_occEquipmentEntityName.empty())
            {
                // These zones are ignored because they are for paging console outputs.
                ++removedVirtualZones;
                continue;
            }

            zones[index] = (*it).m_pKey;
        }

        // Resize if zones removed;
        if (removedVirtualZones)
        {
            zones.length(records.size() - removedVirtualZones);
        }

        PasHelpers::instance().submitPaBroadcastEventTriggeredStationDvaForStationRequestAuditMessage(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            CachedConfig::getInstance()->getAgentLocationKey(),
            sessionId);

        broadcastEventTriggeredStationDvaInternal(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            zones,
            sessionId);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredStationDvaForArea(CORBA::ULong messageKey,
                                                                    const char* messageVersionNumber,
                                                                    CORBA::Boolean hasChime,
                                                                    CORBA::Boolean isSynchronisedWithTis,
                                                                    CORBA::ULong durationInSecs,
                                                                    CORBA::UShort periodInSecs,
                                                                    const char* sessionId,
                                                                    const char* zoneArea)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredStationDvaForArea");
        FUNCTION_ENTRY("broadcastEventTriggeredStationDvaForArea");

        LOGOUT("broadcastEventTriggeredStationDvaForArea");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationDva);

        // Check station DVA message version
        std::string currentVersionNumber =
            // TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessageVersionFromStationDvaMessageKey(messageKey);
            TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessagePrivateVersionFromStationDvaMessageKey(messageKey);

        if (currentVersionNumber != messageVersionNumber)
        {
            // Only log the error as an audit message

            FUNCTION_EXIT;
            TA_THROW(BadParameterException("Event triggered station DVA message broadcast failed - version inconsistency"));
        }

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;

        // Find all valid PA Zones for the given area
        TA_IRS_Bus::PaZoneRecords records = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneRecordsFromArea(zoneArea);

        zones.length(records.size());

        TA_IRS_Bus::PaZoneRecordsIt it = records.begin();
        int index = 0;

        for (; it != records.end(); ++it, ++index)
        {
            zones[index] = (*it).m_pKey;
        }

        PasHelpers::instance().submitPaBroadcastEventTriggeredStationDvaForAreaRequestAuditMessage(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            zoneArea,
            sessionId);

        broadcastEventTriggeredStationDvaInternal(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            zones,
            sessionId);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredStationDvaForGroup(CORBA::ULong messageKey,
                                                                     const char* messageVersionNumber,
                                                                     CORBA::Boolean hasChime,
                                                                     CORBA::Boolean isSynchronisedWithTis,
                                                                     CORBA::ULong durationInSecs,
                                                                     CORBA::UShort periodInSecs,
                                                                     const char* sessionId,
                                                                     const char* zoneGroupLabel)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredStationDvaForGroup");
        FUNCTION_ENTRY("broadcastEventTriggeredStationDvaForGroup");
        LOG_DEBUG("broadcastEventTriggeredStationDvaForGroup(): %s", nvps(messageKey, messageVersionNumber, hasChime, isSynchronisedWithTis, durationInSecs, periodInSecs, sessionId, zoneGroupLabel));

        LOGOUT("broadcastEventTriggeredStationDvaForGroup");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::StationDva);

        // TD 17531
        // remove the version check temporarily, not sure whether need the value in CE when doing ats
        // trigger event broadcast.

        //    // Check station DVA message version
        //    std::string currentVersionNumber =
        //        // TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessageVersionFromStationDvaMessageKey(messageKey);
        //          TA_IRS_Bus::CachedMaps::getInstance()->getDvaMessagePrivateVersionFromStationDvaMessageKey(messageKey);
        //    if (currentVersionNumber != messageVersionNumber)
        //    {
        //        // Only log the error as an audit message
        //        TA_THROW(BadParameterException("Event triggered station DVA message broadcast failed - version inconsistency"));
        //    }
        // TD 17531

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;

        // Find all PA Zones for the given group

        try
        {
            std::list<unsigned long> zoneKeys =
                TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneGroupCachedMap().getZonesAssociatedWithGroup(
                    CachedConfig::getInstance()->getAgentLocationKey(),
                    zoneGroupLabel);

            zones.length(zoneKeys.size());

            std::list<unsigned long>::iterator it = zoneKeys.begin();
            int index = 0;

            for (; it != zoneKeys.end(); ++it, ++index)
            {
                zones[index] = *it;
            }
        }
        catch (...)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("Event triggered Station broadcast failed - zone group details could not be retrieved"));
        }

        PasHelpers::instance().submitPaBroadcastEventTriggeredStationDvaForGroupRequestAuditMessage(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            zoneGroupLabel,
            sessionId);

        broadcastEventTriggeredStationDvaInternal(
            broadcastId,
            messageKey,
            messageVersionNumber,
            hasChime,
            isSynchronisedWithTis,
            durationInSecs,
            periodInSecs,
            zones,
            sessionId);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredTrainDva(CORBA::ULong messageKey,
                                                           CORBA::Short libraryVersion,
                                                           const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod,
                                                           const char* sessionId,
                                                           const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredTrainDva");
        FUNCTION_ENTRY("broadcastEventTriggeredTrainDva");

        LOGOUT("broadcastEventTriggeredTrainDva");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::string broadcastId = getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::TrainDva);

        // Check if Train Agent is available
        if (CachedConfig::getInstance()->getLocalTrainAgentEntityName().empty())
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The local Train Agent cannot be resolved uniquely in the database.  Train PA broadcasts are disabled."));
        }

        PasHelpers::instance().submitTrainPAPrerecordedAuditMessage(
            broadcastId,
            trains,
            messageKey,
            libraryVersion,
            announcementPeriod,
            sessionId);

        // Check the broadcast limit after sending the audit messages for tracability
        if (m_allBroadcasts->getSize() >= CachedConfig::getInstance()->getMaxBroadcasts())
        {
            std::ostringstream msg;
            msg << "Request failed due to broadcast limit ("
                << CachedConfig::getInstance()->getMaxBroadcasts() << ")";

            LOG_ERROR(msg);

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                msg.str(),
                sessionId);

            PasHelpers::instance().throwBroadcastLimitReachedException(broadcastId);
        }

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newBroadcastConfigP = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig();
        TA_Base_Bus::IPAAgentCorbaDef::TrainDvaConfig theSpecifics;

        // Note: string members will make a deep copy from a const char*

        newBroadcastConfigP->broadcastType = TA_Base_Bus::IPAAgentCorbaDef::TrainDva;
        newBroadcastConfigP->broadcastId = broadcastId.c_str();
        newBroadcastConfigP->isEventTriggered = true;
        newBroadcastConfigP->submitTime = time(NULL);

        theSpecifics.hasOverrideOption = false;
        theSpecifics.libraryVersion = libraryVersion;
        theSpecifics.messageKey = messageKey;
        theSpecifics.announcementPeriod = announcementPeriod;
        theSpecifics.trains = trains;

        newBroadcastConfigP->theBroadcastConfigSpecifics.theTrainDvaConfig(theSpecifics);

        AbstractBroadcast* newBroadcast = NULL;

        try
        {
            // Can throw TA_Base_Core::BadParameterException

            newBroadcast = new TrainDVABroadcast(newBroadcastConfigP, sessionId);

            newBroadcast->validateBroadcastConfig();
        }
        catch (TA_Base_Core::BadParameterException& e)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                std::string(e.reason),
                sessionId);

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW(e);
        }
        catch (...)
        {
            delete newBroadcast;
            newBroadcast = NULL;

            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");

            // submit audit message
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessages(
                trains,
                CachedConfig::getInstance()->getAgentLocationKey(),
                "",
                sessionId);

            // Pass on to the client

            FUNCTION_EXIT;
            TA_RETHROW();
        }

        m_allBroadcasts->insert(newBroadcast);

        // Notify all PA Managers
        TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData updateData;

        // Note: string members will make a deep copy from a const char*
        updateData.fromBroadcastId = const_cast<const char*>("");
        updateData.toBroadcastId = newBroadcastConfigP->broadcastId;
        updateData.broadcastType = newBroadcastConfigP->broadcastType;
        updateData.sessionId = sessionId;

        PasHelpers::instance().sendCurrentBroadcastsUpdate(updateData);
        PasHelpers::instance().sendPAAgentNewBroadcastUpdate(*newBroadcastConfigP);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredTrainDvaForStation(CORBA::ULong messageKey,
                                                                     CORBA::Short libraryVersion,
                                                                     const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod,
                                                                     const char* sessionId)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, TrainAgentCommsFailure, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredTrainDvaForStation");
        FUNCTION_ENTRY("broadcastEventTriggeredTrainDvaForStation");

        LOGOUT("broadcastEventTriggeredTrainDvaForStation");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        // Locked in broadcastEventTriggeredTrainDva()

        if (CachedConfig::getInstance()->getLocalTrainAgentEntityName().empty())
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The local Train Agent cannot be resolved uniquely in the database.  Train PA broadcasts are disabled."));
        }

        TA_IRS_Bus::TrainInformationCorbaProxyPtr trainAgentInfo =
            TA_IRS_Bus::TrainAgentProxyFactory::getInstance().createTrainInformationCorbaProxy();

        TA_IRS_Bus::TrainInformationTypes::TrainDetailsList trainListV;

        try
        {
            trainListV = trainAgentInfo->getTrainList();
        }
        catch (...)
        {
            PasHelpers::instance().throwTrainAgentCommsFailureException();
        }

        // Initialise the trains to the maximum potential size
        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains;
        trains.length(trainListV.size());

        unsigned long locationKey = CachedConfig::getInstance()->getAgentLocationKey();
        int count = 0;

        for (int index = 0; index < trainListV.size(); ++index)
        {
            // Only filter according to location if we are not Occ
            if (!CachedConfig::getInstance()->getIsMaster())
            {
                if (trainListV[index].currentLocation != locationKey)
                {
                    continue;
                }
            }

            // Only want trains in this location

            trains[count++] = trainListV[index].trainId;
        }

        // Resize to the actual number of trains after filtering
        trains.length(count);

        broadcastEventTriggeredTrainDva(messageKey,
                                        libraryVersion,
                                        announcementPeriod,
                                        sessionId,
                                        trains);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredTrainDvaForInbound(CORBA::ULong messageKey,
                                                                     CORBA::Short libraryVersion,
                                                                     const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod,
                                                                     const char* sessionId)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, TrainAgentCommsFailure, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredTrainDvaForInbound");
        FUNCTION_ENTRY("broadcastEventTriggeredTrainDvaForInbound");

        LOGOUT("broadcastEventTriggeredTrainDvaForInbound");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        // Locked in broadcastEventTriggeredTrainDva()

        if (CachedConfig::getInstance()->getLocalTrainAgentEntityName().empty())
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The local Train Agent cannot be resolved uniquely in the database.  Train PA broadcasts are disabled."));
        }

        TA_IRS_Bus::TrainInformationCorbaProxyPtr trainAgentInfo =
            TA_IRS_Bus::TrainAgentProxyFactory::getInstance().createTrainInformationCorbaProxy();

        TA_IRS_Bus::TrainInformationTypes::TrainDetailsList trainListV;

        try
        {
            trainListV = trainAgentInfo->getTrainList();//limin
        }
        catch (...)
        {
            PasHelpers::instance().throwTrainAgentCommsFailureException();
        }

        // Initialise the trains to the maximum potential size
        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains;
        trains.length(trainListV.size());

        unsigned long locationKey = CachedConfig::getInstance()->getAgentLocationKey();
        int count = 0;

        for (int index = 0; index < trainListV.size(); ++index)
        {
            // Only filter according to location if we are not Occ
            if (!CachedConfig::getInstance()->getIsMaster())
            {
                if (trainListV[index].currentLocation != locationKey)
                {
                    continue;
                }
            }

            if (TA_IRS_Bus::TrainInformationTypes::AtsTrackInner != trainListV[index].track)
            {
                continue;
            }

            // Only want inbound trains in this location

            trains[count++] = trainListV[index].trainId;
        }

        // Resize to the actual number of trains after filtering
        trains.length(count);

        broadcastEventTriggeredTrainDva(messageKey,
                                        libraryVersion,
                                        announcementPeriod,
                                        sessionId,
                                        trains);

        FUNCTION_EXIT;
    }

    void BroadcastManager::broadcastEventTriggeredTrainDvaForOutbound(CORBA::ULong messageKey,
                                                                      CORBA::Short libraryVersion,
                                                                      const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod,
                                                                      const char* sessionId)
        //raises (BadParameterException, TA_Base_Core::AccessDeniedException, TrainAgentCommsFailure, BroadcastLimitReached)
    {
        LOG_SCOPE("BroadcastManager::broadcastEventTriggeredTrainDvaForOutbound");
        FUNCTION_ENTRY("broadcastEventTriggeredTrainDvaForOutbound");

        LOGOUT("broadcastEventTriggeredTrainDvaForOutbound");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        // Locked in broadcastEventTriggeredTrainDva()

        if (CachedConfig::getInstance()->getLocalTrainAgentEntityName().empty())
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("The local Train Agent cannot be resolved uniquely in the database.  Train PA broadcasts are disabled."));
        }

        TA_IRS_Bus::TrainInformationCorbaProxyPtr trainAgentInfo =
            TA_IRS_Bus::TrainAgentProxyFactory::getInstance().createTrainInformationCorbaProxy();

        TA_IRS_Bus::TrainInformationTypes::TrainDetailsList trainListV;

        try
        {
            trainListV = trainAgentInfo->getTrainList();//limin
        }
        catch (...)
        {
            PasHelpers::instance().throwTrainAgentCommsFailureException();
        }

        // Initialise the trains to the maximum potential size
        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains;
        trains.length(trainListV.size());

        unsigned long locationKey = CachedConfig::getInstance()->getAgentLocationKey();
        int count = 0;

        for (int index = 0; index < trainListV.size(); ++index)
        {
            // Only filter according to location if we are not Occ
            if (!CachedConfig::getInstance()->getIsMaster())
            {
                if (trainListV[index].currentLocation != locationKey)
                {
                    continue;
                }
            }

            if (TA_IRS_Bus::TrainInformationTypes::AtsTrackInner != trainListV[index].track)
            {
                continue;
            }

            // Only want outbound trains in this location

            trains[count++] = trainListV[index].trainId;
        }

        // Resize to the actual number of trains after filtering
        trains.length(count);

        broadcastEventTriggeredTrainDva(messageKey,
                                        libraryVersion,
                                        announcementPeriod,
                                        sessionId,
                                        trains);

        FUNCTION_EXIT;
    }

    void BroadcastManager::throwAccessDeniedExceptionIfCannotPerformAction(unsigned long action, const std::string& sessionId, const std::string& broadcastId)
        //throw(TA_Base_Core::AccessDeniedException)
    {
        FUNCTION_ENTRY("throwAccessDeniedExceptionIfCannotPerformAction");

        if (!isRightsCheckingAvailable())
        {
            PasHelpers::instance().
                throwAccessDeniedException("Failed to determine rights for this action", broadcastId);
        }

        TA_Base_Bus::EDecisionModule decisionModule = TA_Base_Bus::UNDEFINED_DM;
        std::string reason;
        bool canPerformAction = false;

        try
        {
            canPerformAction = m_rightsLibrary->isActionPermittedOnResource(
                sessionId,
                m_resourceId,
                action,
                reason,
                decisionModule);
        }
        catch (...)
        {
            PasHelpers::instance().
                throwAccessDeniedException("Failed to determine rights for this action", broadcastId);
        }

        if (!canPerformAction)
        {
            PasHelpers::instance().
                throwAccessDeniedException("Action is not permitted", broadcastId);
        }

        FUNCTION_EXIT;
    }

    TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq* BroadcastManager::getCompleteBroadcastStateSeqCopy()
    {
        FUNCTION_ENTRY("getCompleteBroadcastStateSeqCopy");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq* allBroadcasts =
            new TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq;

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq broadcastIdTypeSeq;
        m_allBroadcasts->getAllBroadcastIdTypes(broadcastIdTypeSeq);

        allBroadcasts->length(broadcastIdTypeSeq.length());
        int numberOfsomehowRemovedBroadcasts = 0;

        for (int index = 0; index < broadcastIdTypeSeq.length(); ++index)
        {
            AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(static_cast<const char*>(broadcastIdTypeSeq[index].broadcastId));

            if (broadcast == NULL)
            {
                ++numberOfsomehowRemovedBroadcasts;
                continue;
            }

            TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfig =
                broadcast->getBroadcastConfigCopy();

            TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* progressReport =
                broadcast->getProgressReportCopy();

            TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* broadcastParameters =
                broadcast->getBroadcastParametersCopy();

            (*allBroadcasts)[index].broadcastId =
                broadcast->getBroadcastId().c_str();
            (*allBroadcasts)[index].theBroadcastConfig = *broadcastConfig;
            (*allBroadcasts)[index].theProgressReport = *progressReport;
            (*allBroadcasts)[index].broadcastParameters = *broadcastParameters;

            delete broadcastConfig;
            delete progressReport;
            delete broadcastParameters;
        }

        // resize for lost broadcasts

        allBroadcasts->length(broadcastIdTypeSeq.length() - numberOfsomehowRemovedBroadcasts);

        FUNCTION_EXIT;
        return allBroadcasts;
    }

    void BroadcastManager::processPAAgentNewBroadcastUpdate(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* updateData)
        // throw CorruptInternalStateDataException
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::processPAAgentNewBroadcastUpdate");
        FUNCTION_ENTRY("processPAAgentNewBroadcastUpdate");

        if (!updateData)
        {
            TA_THROW(CorruptInternalStateDataException("updateData is NULL"));
        }

        AbstractBroadcast* broadcast = NULL;
        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* newConfig =
            new TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig;

        // Solid copy
        *newConfig = *updateData;

        if (updateData->broadcastType == TA_Base_Bus::IPAAgentCorbaDef::StationMusic)
        {
            // song toan++
            // TD10409

            // copy the music status from the "control" agent
            m_MusicType = updateData->theBroadcastConfigSpecifics.theStationMusicConfig().musicType;

            // ++song toan
            // TD10409

            broadcast = new StationMusicBroadcast(m_tableManager.getTable201(),
                                                  m_tableManager.getTable202(),
                                                  m_tableManager.getTable301(),
                                                  m_tableManager.getTable302(),
                                                  newConfig, "");
        }
        else if (updateData->broadcastType == TA_Base_Bus::IPAAgentCorbaDef::StationLive)
        {
            if (CachedConfig::getInstance()->getIsMaster())
            {
                LOG_INFO("Creating MasterLiveBroadcast.");
                broadcast = new MasterLiveBroadcast(m_tableManager.getTable202(),
                                                    m_tableManager.getTable302(),
                                                    m_tableManager.getTable590(),
                                                    m_tableManager.getTable1200(),
                                                    m_tableManager.getTable1201(),
                                                    m_tableManager.getTable1301(),
                                                    newConfig, "");
            }
            else
            {
                LOG_INFO("Creating StationLiveBroadcast.");
                broadcast = new StationLiveBroadcast(m_tableManager.getTable202(),
                                                     m_tableManager.getTable302(),
                                                     m_tableManager.getTable590(),
                                                     m_tableManager.getTable200(),
                                                     m_tableManager.getTable201(),
                                                     m_tableManager.getTable301(),
                                                     newConfig, "");
            }
        }
        else if (updateData->broadcastType == TA_Base_Bus::IPAAgentCorbaDef::StationDva)
        {
            if (CachedConfig::getInstance()->getIsMaster())
            {
                LOG_INFO("Creating MasterDVABroadcast.");
                broadcast = new MasterDVABroadcast(m_tableManager.getTable202(),
                                                   m_tableManager.getTable302(),
                                                   m_tableManager.getTable350(),
                                                   m_tableManager.getTable1200(),
                                                   m_tableManager.getTable1201(),
                                                   m_tableManager.getTable1301(),
                                                   m_tableManager.getTable1610(),
                                                   newConfig, "");
            }
            else
            {
                LOG_INFO("Creating StationDVABroadcast.");
                broadcast = new StationDVABroadcast(m_tableManager.getTable202(),
                                                    m_tableManager.getTable201(),
                                                    m_tableManager.getTable302(),
                                                    m_tableManager.getTable350(),
                                                    m_tableManager.getTable200(),
                                                    m_tableManager.getTable301(),
                                                    newConfig, "");
            }
        }
        else if (updateData->broadcastType == TA_Base_Bus::IPAAgentCorbaDef::StationRecording)
        {
            broadcast = new DVARecordingBroadcast(m_tableManager.getTable590(),
                                                  m_tableManager.getTable630(),
                                                  newConfig, "");
        }
        else if (updateData->broadcastType == TA_Base_Bus::IPAAgentCorbaDef::TrainLive)
        {
            broadcast = new TrainLiveBroadcast(newConfig, "");
        }
        else if (updateData->broadcastType == TA_Base_Bus::IPAAgentCorbaDef::TrainDva)
        {
            broadcast = new TrainDVABroadcast(newConfig, "");
        }
        else
        {
            LOG_WARN("NewBroadcastUpdate message processing failed: Unknown broadcastType");

            FUNCTION_EXIT;
            return;
        }

        m_allBroadcasts->insert(broadcast);

        LOG_INFO("NewBroadcastUpdate message processed: BroadcastId=%s", broadcast->getBroadcastId());

        FUNCTION_EXIT;
    }

    void BroadcastManager::processPAAgentRemovedBroadcastUpdate(const char* updateData)
        // throw CorruptInternalStateDataException
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::processPAAgentRemovedBroadcastUpdate");
        FUNCTION_ENTRY("processPAAgentRemovedBroadcastUpdate");

        if (!updateData)
        {
            TA_THROW(CorruptInternalStateDataException("updateData is NULL"));
        }

        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(updateData);

        if (broadcast)
        {
            m_allBroadcasts->remove(broadcast);

            LOG_INFO("RemovedBroadcastUpdate message processed: BroadcastId=%s", updateData);

            delete broadcast;
        }
        else
        {
            LOG_WARN("RemovedBroadcastUpdate message processing failed: BroadcastId=%s unknown", updateData);
        }

        FUNCTION_EXIT;
    }

    void BroadcastManager::processPAAgentRenamedBroadcastUpdate(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData* updateData)
        // throw CorruptInternalStateDataException
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::processPAAgentRenamedBroadcastUpdate");
        FUNCTION_ENTRY("processPAAgentRenamedBroadcastUpdate");

        if (!updateData)
        {
            TA_THROW(CorruptInternalStateDataException("updateData is NULL"));
        }

        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(static_cast<const char*>(updateData->fromBroadcastId));

        if (broadcast)
        {
            broadcast->setBroadcastId(static_cast<const char*>(updateData->toBroadcastId));

            LOG_INFO("RenamedBroadcastUpdate message processed: BroadcastId=%s", static_cast<const char*>(updateData->toBroadcastId));
        }
        else
        {
            LOG_WARN("RenamedBroadcastUpdate message processing failed: BroadcastId=%s unknown", static_cast<const char*>(updateData->fromBroadcastId));
        }

        FUNCTION_EXIT;
    }

    void BroadcastManager::processPAAgentBroadcastParameterUpdate(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter* updateData)
        // throw CorruptInternalStateDataException
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::processPAAgentBroadcastParameterUpdate");
        FUNCTION_ENTRY("processPAAgentBroadcastParameterUpdate");

        if (!updateData)
        {
            TA_THROW(CorruptInternalStateDataException("updateData is NULL"));
        }

        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(static_cast<const char*>(updateData->broadcastId));

        if (broadcast)
        {
            // Logging included
            broadcast->setBroadcastParameter(*updateData);
        }
        else
        {
            LOG_WARN("BroadcastParameterUpdate message processing failed: BroadcastId=%s unknown", static_cast<const char*>(updateData->broadcastId));
        }

        FUNCTION_EXIT;
    }

    void BroadcastManager::processPAAgentBroadcastProgressUpdate(const TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* updateData)
        // throw CorruptInternalStateDataException
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::processPAAgentBroadcastProgressUpdate");
        FUNCTION_ENTRY("processPAAgentBroadcastProgressUpdate");

        if (!updateData)
        {
            TA_THROW(CorruptInternalStateDataException("updateData is NULL"));
        }

        AbstractBroadcast* broadcast = m_allBroadcasts->getBroadcast(static_cast<const char*>(updateData->broadcastId));

        if (broadcast)
        {
            broadcast->setProgressReport(*updateData);
            LOG_INFO("BroadcastProgressUpdate message processed: BroadcastId=%s", static_cast<const char*>(updateData->broadcastId));
        }
        else
        {
            LOG_WARN("BroadcastProgressUpdate message processing failed: BroadcastId=%s unknown", static_cast<const char*>(updateData->broadcastId));
        }

        FUNCTION_EXIT;
    }

    void BroadcastManager::processPAAgentEntireAgentStateUpdate(const TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq* updateData)
        // throw CorruptInternalStateDataException
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::processPAAgentEntireAgentStateUpdate");
        FUNCTION_ENTRY("processPAAgentEntireAgentStateUpdate");

        if (!updateData)
        {
            TA_THROW(CorruptInternalStateDataException("updateData is NULL"));
        }

        LOG_INFO("Processing EntireAgentStateUpdate message");

        for (int index = 0; index < updateData->length(); ++index)
        {
            processPAAgentNewBroadcastUpdate(&((*updateData)[index].theBroadcastConfig));
            processPAAgentBroadcastProgressUpdate(&((*updateData)[index].theProgressReport));

            for (int parameterIndex = 0; parameterIndex < (*updateData)[index].broadcastParameters.length(); ++parameterIndex)
            {
                processPAAgentBroadcastParameterUpdate(&((*updateData)[index].broadcastParameters[parameterIndex]));
            }
        }

        FUNCTION_EXIT;
    }

    bool BroadcastManager::isRightsCheckingAvailable()
    {
        FUNCTION_ENTRY("isRightsCheckingAvailable");

        if (m_rightsLibrary)
        {
            FUNCTION_EXIT;
            return true;
        }

        TA_Base_Bus::RightsLibraryFactory rightsLibraryFactory;
        IResource* resource = NULL;

        try
        {
            if (TA_Base_Core::RunParams::getInstance().isSet("DebugAlwaysTrueRightsLib"))
            {
                m_rightsLibrary = rightsLibraryFactory.buildRightsLibrary(TA_Base_Bus::ALWAYS_TRUE_RL);
            }
            else
            {
                m_rightsLibrary = rightsLibraryFactory.buildRightsLibrary();
            }

            // We must figure out the entity key as we do not yet have access to the ConfigurationEditorGUI
            // object to retrieve the key.

            resource = TA_Base_Core::ResourceAccessFactory::getInstance().getResourceFromEntity(
                CachedConfig::getInstance()->getAgentKey());
            m_resourceId = resource->getKey();

            delete resource;
            resource = NULL;
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "Database Error", "Could not initialise ready to check rights.");

            if (m_rightsLibrary != NULL)
            {
                delete m_rightsLibrary;
                m_rightsLibrary = NULL;
            }

            if (resource != NULL)
            {
                delete resource;
                resource = NULL;
            }

            FUNCTION_EXIT;
            return false;
        }

        FUNCTION_EXIT;
        return true;
    }

    void BroadcastManager::setModeChangingFlag(bool flag)
    {
        m_broadcastWorker->setModeChangingFlag(flag);
        LOG_DEBUG("setModeChangingFlag(): %s", nvps(flag));
    }

    //wuzhongyi CL-19136
    void BroadcastManager::forceTerminateAllExistPA()
    {
        LOG_SCOPE("CORBA", "IPAAgentCorbaDef", "BroadcastManager::forceTerminateAllExistPA");
        FUNCTION_ENTRY("forceTerminateAllExistPA");
        TA_Base_Bus::GenericAgent::ensureControlMode();

        try
        {
            LOG_INFO("before forceTerminateAllExistPA");
            m_tableManager.forceTerminateAllPA();
            LOG_INFO("after forceTerminateAllExistPA");
        }
        catch (TA_Base_Core::PasWriteErrorException& err)
        {
            TA_THROW(err);
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown", "Unknown exception when forceTerminateAllExistPA()");
            TA_RETHROW();
        }

        FUNCTION_EXIT;
    }
}
