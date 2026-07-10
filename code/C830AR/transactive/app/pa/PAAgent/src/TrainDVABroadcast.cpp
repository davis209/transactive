/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/TrainDVABroadcast.cpp $
 * @author  A. Parker
 * @version $Revision: #4 $
 * Last modification : $DateTime: 2025/01/06 12:26:20 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#include "pch.h"
#include "AbstractBroadcastCircularList.h"
#include "BroadcastManager.h"
#include "TrainDVABroadcast.h"
#include "bus/trains/TrainCommonLibrary/src/TrainAccessDeniedException.h"
#include "bus/trains/TrainCommonLibrary/src/AgentCommunicationException.h"
#include "bus/trains/TrainCommonLibrary/src/InvalidParameterException.h"
#include "bus/trains/TrainCorbaProxy/src/PaTypesConversion.h"
#include "core/exceptions/src/AuthenticationAgentException.h"
#include "core/exceptions/src/AuthenticationSecurityException.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/exceptions/src/ThrowException.h"
#include "core/utilities/src/TAAssert.h"
#include <time.h>

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

namespace TA_IRS_App
{
    TrainDVABroadcast::TrainDVABroadcast(TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP,
                                         const std::string& sessionId)
        : AbstractTrainBroadcast(broadcastConfigP, sessionId),
        m_theSpecifics(broadcastConfigP->theBroadcastConfigSpecifics.theTrainDvaConfig()),
        m_terminateTimeLimit(0)
    {
        FUNCTION_ENTRY("TrainDVABroadcast");

        m_okTrainListWhileAborting.length(0);

        // Set the initial effective coverage to be the entire list of trains
        // The false flag prevents the sending of the synchronisation message
        setEffectiveCoverageIndices(
            calculateEffectiveCoverageIndices(m_theSpecifics.trains, m_theSpecifics.trains),
            false);

        FUNCTION_EXIT;
    }

    TrainDVABroadcast::~TrainDVABroadcast()
    {
        FUNCTION_ENTRY("~TrainDVABroadcast");
        FUNCTION_EXIT;
    }

    bool TrainDVABroadcast::isCyclic()
    {
        FUNCTION_ENTRY("isCyclic");

        FUNCTION_EXIT;
        return !((0 == m_theSpecifics.announcementPeriod.period) &&
                 (0 == m_theSpecifics.announcementPeriod.duration));
    }

    bool TrainDVABroadcast::getIsPartOfBroadcast(CORBA::Octet trainId, CORBA::Octet announcementId)
    {
        FUNCTION_ENTRY("getIsPartOfBroadcast");

        // check the announcement ID first
        if (announcementId == getAnnouncementId())
        {
            // find trainId

            for (int index = 0; index < m_theSpecifics.trains.length(); ++index)
            {
                if (m_theSpecifics.trains[index] == trainId)
                {
                    // found train id
                    FUNCTION_EXIT;
                    return true;
                }
            }
        }

        // not found
        FUNCTION_EXIT;
        return false;
    }

    void TrainDVABroadcast::validateBroadcastConfig() //throw(TA_Base_Core::BadParameterException)
    {
        LOG_SCOPE("TrainDVABroadcast::validateBroadcastConfig");
        FUNCTION_ENTRY("validateBroadcastConfig");

        ThreadGuard guard(m_lock);
        int i = 0;

        // At least one train must be specified
        if (m_theSpecifics.trains.length() == 0)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("No trains requested"));
        }

        // Test validity of message key in database
        try
        {
            TA_IRS_Bus::TrainDvaMessageRecord record = TA_IRS_Bus::CachedMaps::getInstance()->getTrainDvaMessageRecordFromKey(m_theSpecifics.messageKey);
        }
        catch (TA_Base_Core::CachedMappingNotFoundException&)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("A requested message does not exist in the database"));
        }

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::execute()
    {
        FUNCTION_ENTRY("execute");

        ThreadGuard guard(m_lock);

        auto effectiveTrains = getEffectiveTrainCoverage(m_theSpecifics.trains);

        // package up the list of trains for TrainAgent
        TA_IRS_Bus::CommonTypes::TrainIdList trainList;

        //trainList.length(effectiveTrains.length());
        for (int i = 0; i < effectiveTrains.length(); ++i)
        {
            trainList.insert(static_cast<unsigned char> (effectiveTrains[i]));
        }

        TA_IRS_Bus::TrainDvaMessageRecord record = TA_IRS_Bus::CachedMaps::getInstance()->getTrainDvaMessageRecordFromKey(m_theSpecifics.messageKey);
        unsigned long messageIdAkaTableOffset = record.m_pKeyOrId;

        // Initialise with a new announcementId
        try
        {
            setAnnouncementId(AbstractBroadcastCircularList::getInstance()->getNextAvailableTrainAnnouncementId());
        }
        catch (TA_Base_Core::NoFreeAnnounceIdException&)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_ANNOUNCE_ID);
            // Process next train if there are any left for execution
            FUNCTION_EXIT;
            return;
        }

        try
        {
            if (isCyclic())
            {
                m_trainPaCorbaProxy->cyclicAnnouncement(trainList,
                                                        getAnnouncementId(),
                                                        messageIdAkaTableOffset,
                                                        m_theSpecifics.libraryVersion,
                                                        TA_IRS_Bus::PaTypesConversion::convertToCpp(m_theSpecifics.announcementPeriod),
                                                        m_theSpecifics.hasOverrideOption,
                                                        getSessionId().c_str());
            }
            else
            {
                m_trainPaCorbaProxy->preRecordedAnnouncement(trainList,
                                                             getAnnouncementId(),
                                                             messageIdAkaTableOffset,
                                                             m_theSpecifics.libraryVersion,
                                                             m_theSpecifics.hasOverrideOption,
                                                             getSessionId().c_str());
            }
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (...)
        {
            // Unexpected exception...
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }

        // Clear any past errors only for the transition from Execution to Runtime
        // In the case of the transition from Execution to Termination to PendingRemoval,
        // we want to retain the GlobalState so that the PA Manager can see the
        // errors encountered.
        m_progressReportSentry->resetGlobalState();

        // Finished executing each train once so we move to the runtime state
        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Runtime);

        // Maochun Sun++
        // TD13789
        m_progressReportSentry->setExecutionPhase(TA_Base_Bus::IPAAgentCorbaDef::EXECUTE_PHASE_2);
        // ++Maochun Sun
        // TD13789

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::handleCommandResult(TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices& effectiveCoverageIndices,
                                                unsigned int trainIndexIndex,
                                                const TA_IRS_Bus::PaTypes::PaUpdateEvent& event)
    {
        FUNCTION_ENTRY("handleCommandResult");

        ThreadGuard guard(m_lock);

        switch (event.commandResult)
        {
            case TA_IRS_Bus::PaTypes::PaProceeding:
            {
                TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState broadcastState = m_progressReportSentry->getBroadcastState();

                switch (broadcastState)
                {
                    // these states set up the broadcast
                    case TA_Base_Bus::IPAAgentCorbaDef::Creation:
                    case TA_Base_Bus::IPAAgentCorbaDef::Execution:
                    case TA_Base_Bus::IPAAgentCorbaDef::Runtime:
                        m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY);
                        break;

                        // these states terminate it (with an abort command)
                    case TA_Base_Bus::IPAAgentCorbaDef::Termination:
                    case TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval:
                        m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_ABORTED);
                        break;

                    default:
                        break;
                }
            }
            break;

            case TA_IRS_Bus::PaTypes::PaInUse:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY);
                break;

            case TA_IRS_Bus::PaTypes::PaInUseOptionToOverride:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY_WITH_OVERRIDE_OPTION);
                break;

            case TA_IRS_Bus::PaTypes::PaInUseFirstCycleCancelled:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED);
                break;

            case TA_IRS_Bus::PaTypes::PaReset:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONNECTION_RESET);
                break;

            case TA_IRS_Bus::PaTypes::PaFailed:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INVALID_REQUEST);
                break;

            case TA_IRS_Bus::PaTypes::PaDifferentOrigin:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_ORIGIN);
                break;

            case TA_IRS_Bus::PaTypes::PaDifferentId:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INVALID_REQUEST);
                break;

            case TA_IRS_Bus::PaTypes::PaDifferentVersion:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_VERSION);
                break;

            case TA_IRS_Bus::PaTypes::BadCommandBadChecksum:
            case TA_IRS_Bus::PaTypes::BadCommandUnknownCommand:
            case TA_IRS_Bus::PaTypes::BadCommandBadParameter:
            case TA_IRS_Bus::PaTypes::BadCommandUnexpectedCommand:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BAD_COMMAND);
                break;

            case TA_IRS_Bus::PaTypes::InvalidTrainError:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_NO_PERMISSION);
                break;

            case TA_IRS_Bus::PaTypes::TransmissionFailureError:
            case TA_IRS_Bus::PaTypes::TransactionTimeoutError:
            case TA_IRS_Bus::PaTypes::TransactionCancelledError:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_TRAIN);
                break;

            case TA_IRS_Bus::PaTypes::MpuChangeoverError:
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_MPU_CHANGEOVER);
                break;

            default:
                // unknown enumeration value
                break;
        }

        // check to ensure that there is at least one train that is successful
        checkAllTrainStatus();

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::checkAllTrainStatus()
    {
        FUNCTION_ENTRY("checkAllTrainStatus");

        ThreadGuard guard(m_lock);

        auto effectiveCoverageIndices = getEffectiveCoverageIndices();

        // If no trains were successful, we terminate.
        bool foundSuccess = false;

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            if (m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]) ==
                TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY)
            {
                // Set back to InProgress to indicate the second phase
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS);

                // Found at least one successful train
                foundSuccess = true;
            }
            else if (m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]) ==
                     TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS)
            {
                // Found at least one successful train
                foundSuccess = true;
            }
            // ss++
            // TD13952
            else if (m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]) ==
                     TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED)
            {
                // Found at least one successful train
                foundSuccess = true;
            }//++ss
        }

        if ((false == foundSuccess) &&
            (TA_Base_Bus::IPAAgentCorbaDef::Termination != m_progressReportSentry->getBroadcastState()))
        {
            setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Termination);
        }

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::runtimeSpecifics()
    {
        FUNCTION_ENTRY("runtimeSpecifics");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
            getEffectiveCoverageIndices();

        // Cyclic and oneshot broadcasts are both tested in the same manner for termination

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            if (m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]) == TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS)
            {
                // Still running - wait for processRuntimeNotification() to flag the
                // remaining trains as completed OR until they fail
                FUNCTION_EXIT;
                return;
            }

            //ss++
            if (m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]) == TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED)
            {
                // Still running - wait for processRuntimeNotification() to flag the
                // remaining trains as completed OR until they fail
                FUNCTION_EXIT;
                return;
            }

            //++ss
        }

        // Got here - no trains in progress anymore - therefore we terminate

        // Reset to 0 as this completed naturally - indicates to terminate() that
        // it wasn't operator aborted.
        setAnnouncementId(0);

        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Termination);

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::terminate()
    {
        FUNCTION_ENTRY("terminate");

        ThreadGuard guard(m_lock);

        if (0 != getAnnouncementId())
        {
            if (0 == m_terminateTimeLimit)
            {
                // send the disconnect
                disconnect();
            }
        }

        // update the list of current active trains
        populateActiveTrains();

        // check if all train results have been received
        // or if the timeout has been exceeded
        if ((0 == getAnnouncementId()) ||
            (0 == m_okTrainListWhileAborting.length()) ||
            ((0 != m_terminateTimeLimit) &&
             (time(NULL) > m_terminateTimeLimit)
             ))
        {
            // Reset the announce id so that it can be used by new broadcasts
            setAnnouncementId(0);

            updateFinalProgressReport();

            setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval);
        }

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::operatorAbort()
    {
        FUNCTION_ENTRY("operatorAbort");

        ThreadGuard guard(m_lock);

        populateActiveTrains();

        // Do not call this, it sets everything to aborted regardless of the return status of the train
        //AbstractBroadcast::operatorAbort();
        // manually set to termination phase
        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Termination);

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::populateActiveTrains()
    {
        FUNCTION_ENTRY("populateActiveTrains");

        ThreadGuard guard(m_lock);

        // need to obtain a snapshot of all the trains currently active:

        auto effectiveCoverageIndices = getEffectiveCoverageIndices();

        // No such sequence.clear() method.  Assume everything will be overwritten

        m_okTrainListWhileAborting.length(effectiveCoverageIndices.length()); // resize back later
        int okTrainListLength = 0;

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            auto localState = m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]);

            if (localState == TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS ||
                localState == TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY ||
                localState == TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED)
            {
                // flag ones without error
                m_okTrainListWhileAborting[okTrainListLength++] =  static_cast<unsigned char>(m_theSpecifics.trains[effectiveCoverageIndices[indexIndex]]);
            }
        }

        m_okTrainListWhileAborting.length(okTrainListLength);

        FUNCTION_EXIT
    }

    void TrainDVABroadcast::disconnect()
    {
        FUNCTION_ENTRY("disconnect");

        ThreadGuard guard(m_lock);

        if (m_okTrainListWhileAborting.length() == 0)
        {
            FUNCTION_EXIT;
            return;
        }

        m_terminateTimeLimit = time(NULL); // now

        unsigned long maximumWaitTime = 120000; // default time of 2 minutes

        try
        {
            maximumWaitTime = m_trainPaCorbaProxy->getCommandTimeout();
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());
        }

        try
        {
            // Note that for the DVA cyclic, we are using the endLiveAnnouncement() CORBA method to
            // send the PaReset command.  Really we should generalise the method name or provide another
            // but we are trying to minimise interface changes

            TA_IRS_Bus::CommonTypes::TrainIdList trainIdList;

            for (unsigned int i = 0; i < m_okTrainListWhileAborting.length(); ++i)
            {
                trainIdList.insert(m_okTrainListWhileAborting[i]);
            }

            m_trainPaCorbaProxy->endBroadcast(trainIdList, getAnnouncementId(), getSessionId().c_str());

            // end triggered successfully, allow the time to complete
            m_terminateTimeLimit += maximumWaitTime;
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            // Unexpected timeout condition
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT);
            // terminating anyway
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            // Unexpected timeout condition
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT);
            // terminating anyway
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            // Unexpected timeout condition
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);
            // terminating anyway
        }
        catch (...)
        {
            // Unexpected exception...
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);
            // operation not allowed - abort and send back to the PA Manager
        }

        FUNCTION_EXIT;
    }

    void TrainDVABroadcast::processRuntimeNotification(const TA_IRS_Bus::PaTypes::PaUpdateEvent& event)
    {
        FUNCTION_ENTRY("processRuntimeNotification");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
            getEffectiveCoverageIndices();

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            if (m_theSpecifics.trains[effectiveCoverageIndices[indexIndex]] == event.trainId)
            {
                // Found
                switch (event.updateType)
                {
                    case TA_IRS_Bus::PaTypes::LiveAnnouncementReady:
                    case TA_IRS_Bus::PaTypes::LiveAnnouncementContinueRequired:
                    case TA_IRS_Bus::PaTypes::LiveAnnouncementContinueTimeout:

                        // Ignored as these are only for Live broadcasts
                        break;

                    case TA_IRS_Bus::PaTypes::AnnouncementReset:
                    {
                        if (TA_IRS_Bus::PaTypes::MpuChangeoverError == event.commandResult)
                        {
                            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_MPU_CHANGEOVER);
                        }
                    }
                    break;

                    case TA_IRS_Bus::PaTypes::CompletedAnnouncement:
                        if (TA_IRS_Bus::ProtocolPaTypes::Successful == event.completionStatus)
                        {
                            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED);
                        }
                        else
                        {
                            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INTERRUPTED);
                        }

                        break;

                    case TA_IRS_Bus::PaTypes::PaCommandResult:
                        // This is the result of the execute command
                        handleCommandResult(effectiveCoverageIndices, indexIndex, event);
                        break;

                    default:
                        LOG_ERROR("Unexpected train PANotificationType");
                }

                FUNCTION_EXIT;
                return;
            }
        }

        LOG_WARN("TrainId not matched.");

        FUNCTION_EXIT;
    }
}
