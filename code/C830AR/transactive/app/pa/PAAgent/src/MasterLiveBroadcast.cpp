/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/MasterLiveBroadcast.cpp $
 * @author  A. Parker
 * @version $Revision: #5 $
 * Last modification : $DateTime: 2025/01/20 18:40:08 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#include "pch.h"
#include "MasterLiveBroadcast.h"
#include "Table1200.h"
#include "Table1201.h"
#include "Table302.h"
#include "Table202.h"
#include "Table590.h"
#include "CachedConfig.h"
#include "BroadcastManager.h"
#include "AbstractBroadcastCircularList.h"
#include "PASConnection.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/exceptions/src/ThrowException.h"
#include <time.h>

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

namespace TA_IRS_App
{
    MasterLiveBroadcast::MasterLiveBroadcast(Table202& table202,
                                             Table302& table302,
                                             Table590& table590,
                                             Table1200& table1200,
                                             Table1201& table1201,
                                             Table1301& table1301,
                                             TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP,
                                             const std::string& sessionId)
        : AbstractStationBroadcast(broadcastConfigP, sessionId),
        m_theSpecifics(broadcastConfigP->theBroadcastConfigSpecifics.theStationLiveConfig()),
        m_table202(table202),
        m_table302(table302),
        m_table590(table590),
        m_table1200(table1200),
        m_table1201(table1201),
        m_table1301(table1301),
        m_isConnected(false)
    {
        TA_ASSERT(CachedConfig::getInstance()->getIsMaster(), "MasterLiveBroadcast is not available for the Station PA Agent");

        // Set the initial effective coverage to be the entire list of zones
        // The false flag prevents the sending of the synchronisation message
        setEffectiveCoverageIndices(
            calculateEffectiveCoverageIndices(m_theSpecifics.zones, m_theSpecifics.zones),
            false);
    }

    MasterLiveBroadcast::~MasterLiveBroadcast()
    {
    }

    void MasterLiveBroadcast::validateBroadcastConfig() //throw(TA_Base_Core::BadParameterException)
    {
        LOG_SCOPE("MasterLiveBroadcast::validateBroadcastConfig");

        ThreadGuard guard(m_lock);

        // Source Id needs to be from an OCC paging console or a spare:
        if (!PasHelpers::instance().zoneIsAPagingConsole(m_theSpecifics.sourceId, getSessionId())) // TD19669
        {
            LOG_DEBUG("The specified source is not a paging console");
            TA_THROW(BadParameterException("The specified source is not a paging console"));
        }

        // At least one zone must be specified
        if (m_theSpecifics.zones.length() == 0)
        {
            LOG_DEBUG("No zones requested");
            TA_THROW(BadParameterException("No zones requested"));
        }

        // Test validity of zone keys
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
                LOG_DEBUG("A requested zone does not exist in the database");
                TA_THROW(BadParameterException("A requested zone does not exist in the database"));
            }

            try
            {
                zoneLocationKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(zoneKey);
                stationId = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(zoneLocationKey);
            }
            catch (TA_Base_Core::CachedMappingNotFoundException&)
            {
                LOG_ERROR("[BadParameterException] An ISCS station location could not be resolved to a PAS location");
                TA_THROW(BadParameterException("An ISCS station location could not be resolved to a PAS location"));
            }
        }
    }

    void MasterLiveBroadcast::execute()
    {
        LOG_SCOPE("MasterLiveBroadcast::execute");

        if (!executePhaseOneAndReturnIsCompleted())
        {
            // retry next iteration
            return;
        }

        if (!executePhaseTwoAndReturnIsCompleted())
        {
            // retry next iteration
            return;
        }

        // All went well so we move to the runtime state
        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Runtime);
    }

    bool MasterLiveBroadcast::executePhaseOneAndReturnIsCompleted()
    {
        LOG_SCOPE("MasterLiveBroadcast::executePhaseOneAndReturnIsCompleted");

        ThreadGuard guard(m_lock);

        ///////////////////////////////////////////////////////////////////////////
        // Phase 1 of execution - PAS connection
        ///////////////////////////////////////////////////////////////////////////

        if (getIsConnected())
        {
            // Already completed
            return true;
        }

        setWasExecutedSuccessfully(false);

        try
        {
            setAnnounceId(AbstractBroadcastCircularList::getInstance()->getNextAvailableStationAnnounceId());
        }
        catch (TA_Base_Core::NoFreeAnnounceIdException&)
        {
            // Serious error - terminate
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_ANNOUNCE_ID);
            return false;
        }

        if (!PASConnection::getInstance()->isConnected())
        {
            LOG_ERROR("executePhaseOneAndReturnIsCompleted(): FEPC not available!");
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
            return false;
        }

        try
        {
            m_table1201.setTableDataAndWrite(m_theSpecifics.sourceId,
                                             getAnnounceId(),
                                             getEffectiveZoneCoverage(m_theSpecifics.zones));
        }
        catch (TA_Base_Core::PasWriteErrorException& e)
        {
            LOG_ERROR("executePhaseOneAndReturnIsCompleted(): PasWriteErrorException: %s", st::enum_to_string_value(static_cast<AbstractPASTable::EPasWriteErrors>(e.getErrorValue())));

            if (e.getErrorValue() == AbstractPASTable::PAS_ERR_INVALID_ANNOUNCE)
            {
                // Somehow this announceId has been tied up in a dead broadcast and
                // this Agent instance has no knowledge of it.  We will endeavour to
                // kill it and retry the broadcast

                try
                {
                    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage nullZones;
                    nullZones.length(0);

                    m_table1201.setTableDataAndWrite(m_table202.getSourceId(getAnnounceId()),
                                                     getAnnounceId(),
                                                     nullZones);

                    m_table1301.setTableDataAndWrite(m_table302.getSequenceId(getAnnounceId()),
                                                     0,
                                                     0,
                                                     0,
                                                     0,
                                                     false,
                                                     nullZones);
                }
                catch (TA_Base_Core::PasConnectionException&)
                {
                    // Serious error - terminate
                    abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
                    return false;
                }

                setAnnounceId(0);

                determineWhetherToRetryOrAbort(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_ANNOUNCE_ID);
                return false;
            }
            else
            {
                // PAS_ERR_GENERIC or anything else

                // Serious error - terminate
                // TD17456
                if (isAllFailureZones())
                {
                    LOG_ERROR("noneValidZoneTermination error %d", e.getErrorValue());
                    noneValidZoneTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_PA_AGENT);
                }
                else
                {
                    LOG_ERROR("abnormalTermination error %d", e.getErrorValue());
                    abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_PA_AGENT);
                }

                // TD17456
                return false;
            }
        }
        catch (TA_Base_Core::PasConnectionException&)
        {
            // Serious error - terminate
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
            return false;
        }

        // Flag to try to cleanup the announceId in terminate
        setWasExecutedSuccessfully(true);

        // Succeeded so we reset the GlobalState
        m_progressReportSentry->resetGlobalState();

        // Set this flag to true so we can hang around the execution state
        // without resending the PAS request until PTT is pressed.
        setIsConnected(true);

        return true;
    }

    bool MasterLiveBroadcast::executePhaseTwoAndReturnIsCompleted()
    {
        LOG_SCOPE("MasterLiveBroadcast::executePhaseTwoAndReturnIsCompleted");

        ThreadGuard guard(m_lock);

        ///////////////////////////////////////////////////////////////////////////
        // Phase 2 of execution - awaiting PTT
        ///////////////////////////////////////////////////////////////////////////

        bool pttState(false);

        try
        {
            pttState = m_table590.isPttStatusOn(m_theSpecifics.sourceId);
        }
        catch (TA_Base_Core::PasConnectionException&)
        {
            // Serious error - terminate
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
            return false;
        }

        // Succeeded so we reset the GlobalState
        m_progressReportSentry->resetGlobalState();

        // if the button is pressed, that means the switching of zones is complete
        if (pttState)
        {
            // Sleep for a poll cycle to ensure table 1200 is processed
            m_table1200.getCondition().timedWait(CachedConfig::getInstance()->getFastPollRateInMSecs());
            updateProgressReport(false); //isRuntime
        }

        return pttState;
    }

    void MasterLiveBroadcast::runtimeSpecifics()
    {
        LOG_SCOPE("MasterLiveBroadcast::runtimeSpecifics");

        ThreadGuard guard(m_lock);

        updateProgressReport(true); //isRuntime

        bool pttState;

        try
        {
            pttState = m_table590.isPttStatusOn(m_theSpecifics.sourceId);
        }
        catch (TA_Base_Core::PasConnectionException&)
        {
            determineWhetherToRetryOrAbort(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
            return;
        }

        if (pttState == true)
        {
            // Operator is still talking so we cannot terminate yet.
            return;
        }

        // The operator has released the PTT button so we move to the termination state
        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Termination);
        // terminate will be called by the BroadcastWorker().
    }

    void MasterLiveBroadcast::updateProgressReport(bool isRuntime)
    {
        LOG_SCOPE("MasterLiveBroadcast::updateProgressReport");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices = getEffectiveCoverageIndices();

        // ++ Bug456
        m_table1200.readTimedWait();

        // ++ Bug456
        // update progress report structure for each zone
        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            unsigned long zoneKey = m_theSpecifics.zones[effectiveCoverageIndices[indexIndex]];

            // CachedMappingNotFoundException will never happen because it has been tested
            // in validateBroadcastConfig()
            unsigned int zoneId = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneIdFromKey(zoneKey);
            unsigned long zoneLocationKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(zoneKey);
            unsigned int stationId = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(zoneLocationKey);

            ZoneStatus zoneStatus;

            try
            {
                zoneStatus = m_table1200.getZoneStatus(zoneId, stationId);
            }
            catch (TA_Base_Core::PasConnectionException&)
            {
                if (isRuntime)
                {
                    determineWhetherToRetryOrAbort(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
                    return;
                }
                else
                {
                    // Serious error - terminate
                    abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
                    return;
                }
            }

            if (zoneStatus.m_status)
            {
                // Faulty zone
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_ZONE);
            }
            else
            {
                // Zone functional
                if (zoneStatus.m_sourceId != 0 &&
                    zoneStatus.m_sourceId != m_theSpecifics.sourceId)
                {
                    // Overridden by another source
                    m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_OVERRIDDEN);
                    m_progressReportSentry->setOverridingSourcePriority(effectiveCoverageIndices[indexIndex], zoneStatus.m_priority);
                }
                else
                {
                    // All ok
                    m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS);
                    m_progressReportSentry->setOverridingSourcePriority(effectiveCoverageIndices[indexIndex], 0);
                }
            }
        } // for loop
    }

    void MasterLiveBroadcast::terminate()
    {
        LOG_SCOPE("MasterLiveBroadcast::terminate");

        ThreadGuard guard(m_lock);

        if (getWasExecutedSuccessfully())
        {
            try
            {
                m_table1201.setTableDataAndWrite(m_theSpecifics.sourceId,
                                                 0,
                                                 getEffectiveZoneCoverage(m_theSpecifics.zones));
            }
            catch (TA_Base_Core::PasWriteErrorException&)
            {
                // Not expected to get here
                m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_PA_AGENT);
                // Carry on regardless
            }
            catch (TA_Base_Core::PasConnectionException&)
            {
                determineWhetherToRetryOrAbort(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT);
                return;
            }
        }

        // Reset the connected flag so a new execution will send a new request to PAS
        setIsConnected(false);

        // Reset the announce id so that it can be used by new broadcasts
        setAnnounceId(0);

        updateFinalProgressReport();

        // All went well so we move to the pending removal state
        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval);
    }

    // TD17456
    bool MasterLiveBroadcast::isAllFailureZones()
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
