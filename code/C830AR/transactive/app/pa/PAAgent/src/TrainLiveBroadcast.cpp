/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/TrainLiveBroadcast.cpp $
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
#include "CachedConfig.h"
#include "PasHelpers.h"
#include "TrainLiveBroadcast.h"
#include "bus/trains/TrainCommonLibrary/src/TrainAccessDeniedException.h"
#include "bus/trains/TrainCommonLibrary/src/AgentCommunicationException.h"
#include "bus/trains/TrainCommonLibrary/src/GroupInUseException.h"
#include "bus/trains/TrainCommonLibrary/src/InvalidParameterException.h"
#include "bus/trains/TrainCommonLibrary/src/PaTypes.h"
#include "bus/trains/TrainCorbaProxy/src/ExceptionConversion.h"
#include "core/exceptions/src/ThrowException.h"
#include "core/exceptions/src/AuthenticationAgentException.h"
#include "core/exceptions/src/AuthenticationSecurityException.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/TAAssert.h"
#include <time.h>

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    TrainLiveBroadcast::TrainLiveBroadcast(TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP,
                                           const std::string& sessionId)
        : AbstractTrainBroadcast(broadcastConfigP, sessionId),
        m_theSpecifics(broadcastConfigP->theBroadcastConfigSpecifics.theTrainLiveConfig()),
        m_nextState(TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS),
        m_terminateTimeLimit(0)
    {
        FUNCTION_ENTRY("TrainLiveBroadcast");

        // Set the initial effective coverage to be the entire list of trains
        // The false flag prevents the sending of the synchronisation message
        setEffectiveCoverageIndices(
            calculateEffectiveCoverageIndices(m_theSpecifics.trains, m_theSpecifics.trains),
            false);

        // now upon startup, set the state to connecting.
        //
        // The states will be
        // 1. Connecting
        // 2. Ready for live announcement
        // 3. (Begin pressed) In Progress
        // etc
        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
            getEffectiveCoverageIndices();

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex],
                                                  TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING);
        }

        FUNCTION_EXIT;
    }

    TrainLiveBroadcast::~TrainLiveBroadcast()
    {
        FUNCTION_ENTRY("~TrainLiveBroadcast");
        FUNCTION_EXIT;
    }

    int TrainLiveBroadcast::getIndexForTrain(CORBA::Octet trainId)
    {
        FUNCTION_ENTRY("getIndexForTrain");

        // find trainId

        for (int index = 0; index < m_theSpecifics.trains.length(); ++index)
        {
            if (m_theSpecifics.trains[index] == trainId)
            {
                // found train id
                FUNCTION_EXIT;
                return index;
            }
        }

        // not found
        FUNCTION_EXIT;
        return -1;
    }

    bool TrainLiveBroadcast::getIsPartOfBroadcast(CORBA::Octet trainId, CORBA::Octet announcementId)
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

    void TrainLiveBroadcast::validateBroadcastConfig() //throw(TA_Base_Core::BadParameterException)
    {
        FUNCTION_ENTRY("validateBroadcastConfig");

        ThreadGuard guard(m_lock);

        // At least one train must be specified
        if (m_theSpecifics.trains.length() == 0)
        {
            FUNCTION_EXIT;
            TA_THROW(BadParameterException("No trains requested"));
        }

        FUNCTION_EXIT;
    }

    void TrainLiveBroadcast::retry(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains,
                                   CORBA::Boolean hasOverrideOption,
                                   const std::string& sessionId)
    {
        FUNCTION_ENTRY("retry");

        ThreadGuard guard(m_lock);

        // retry the broadcast
        AbstractTrainBroadcast::retry(trains, hasOverrideOption, sessionId);

        m_terminateTimeLimit = 0;

        // reset the train state

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
            getEffectiveCoverageIndices();

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex],
                                                  TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING);
        }

        FUNCTION_EXIT;
    }

    void TrainLiveBroadcast::execute()
    {
        FUNCTION_ENTRY("execute");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage effectiveTrains =
            getEffectiveTrainCoverage(m_theSpecifics.trains);

        // package up the list of trains for TrainAgent
        TA_IRS_Bus::CommonTypes::TrainIdList trainList;

        //trainList.length(effectiveTrains.length());
        for (int i = 0; i < effectiveTrains.length(); ++i)
        {
            trainList.insert(static_cast<TA_IRS_Bus::CommonTypes::TrainIdType>(effectiveTrains[i]));
        }

        try
        {
            setAnnouncementId(AbstractBroadcastCircularList::getInstance()->getNextAvailableTrainAnnouncementId());
        }
        catch (TA_Base_Core::NoFreeAnnounceIdException&)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_ANNOUNCE_ID);

            FUNCTION_EXIT;
            return;
        }

        // set the next state
        m_nextState = TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY;

        try
        {
            //Bohong++
            m_trainPaCorbaProxy->setupTimeOut(DGNA_TIMEOUT);
            //++Bohong

            m_trainPaCorbaProxy->setupLiveAnnouncement(trainList,
                                                       getAnnouncementId(),
                                                       m_theSpecifics.hasOverrideOption,
                                                       m_theSpecifics.staticGroupName.in(),
                                                       getSessionId().c_str());

            // the results for each train come in as comms messages
            // if the setup succeeded, a ready for live announcement comes in (processRuntimeNotification)
            // if the setup failed, a command result comes in (handleCommandResult)
        }
        catch (const TA_IRS_Bus::AgentCommunicationException&)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException&)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::InvalidParameterException&)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::GroupInUseException&)
        {
            abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_STATIC_GROUP_IN_USE);

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

        PasHelpers::instance().sendBroadcastProgressUpdate(this);

        FUNCTION_EXIT;
    }

    void TrainLiveBroadcast::handleCommandResult(TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices& effectiveCoverageIndices,
                                                 unsigned int trainIndexIndex,
                                                 const TA_IRS_Bus::PaTypes::PaUpdateEvent& event)
    {
        FUNCTION_ENTRY("handleCommandResult");

        ThreadGuard guard(m_lock);

        switch (event.commandResult)
        {
            case TA_IRS_Bus::PaTypes::PaProceeding:
            {
                // based on the broadcast phase, set the correct status
                TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState broadcastState = m_progressReportSentry->getBroadcastState();

                switch (broadcastState)
                {
                    // setting up the live announcement
                    case TA_Base_Bus::IPAAgentCorbaDef::Creation:
                    case TA_Base_Bus::IPAAgentCorbaDef::Execution:
                    case TA_Base_Bus::IPAAgentCorbaDef::Runtime:
                        m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY);
                        break;

                        // ending a live announcement
                    case TA_Base_Bus::IPAAgentCorbaDef::Termination:
                    case TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval:
                        m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED);
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
                // not a valid response for this operation
                m_progressReportSentry->setLocalState(effectiveCoverageIndices[trainIndexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_TRAIN);
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

        FUNCTION_EXIT;
    }

    TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage TrainLiveBroadcast::getTrainsRequiringContinue()
    {
        FUNCTION_ENTRY("getTrainsRequiringContinue");

        //See runtimeSpecifics
        //Get all train statuses, and return all those with the status LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED

        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trainCoverage;

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
            getEffectiveCoverageIndices();

        std::vector<unsigned int> temp;
        int i;

        for (i = 0; i < effectiveCoverageIndices.length(); ++i)
        {
            unsigned int index = effectiveCoverageIndices[i];

            if (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED == m_progressReportSentry->getLocalState(index))
            {
                temp.push_back(index);
            }
        }

        trainCoverage.length(temp.size());

        for (i = 0; i < temp.size(); ++i)
        {
            trainCoverage[i] = temp[i];
        }

        FUNCTION_EXIT;
        return trainCoverage;
    }

    void TrainLiveBroadcast::runtimeSpecifics()
    {
        FUNCTION_ENTRY("runtimeSpecifics");

        // The update of the progress report structure is acheived by the CORBA messaging threads
        // which call one of the following:
        //
        // beginTrainLiveAnnouncement()
        // continueTrainLiveAnnouncement()
        // endTrainLiveAnnouncement()
        // processRuntimeNotification()
        //

        // The sending of the progressReport is performed by the BroadcastWorker as soon as we finish here

        if (true == areAllTrainsFinished())
        {
            // Terminate
            setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Termination);
            PasHelpers::instance().sendBroadcastProgressUpdate(this);
        }

        // else stay in Runtime

        FUNCTION_EXIT;
    }

    void TrainLiveBroadcast::terminate()
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

        // check if all train results have been received
        // or if the timeout has been exceeded
        if ((0 == getAnnouncementId()) ||
            (true == areAllTrainsFinished()) ||
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

    void TrainLiveBroadcast::beginTrainLiveAnnouncement() //throw(TA_Base_Core::BadParameterException)
    {
        FUNCTION_ENTRY("beginTrainLiveAnnouncement");

        ThreadGuard guard(m_lock);

        try
        {
            //Bohong++
            m_trainPaCorbaProxy->setupTimeOut(DGNA_TIMEOUT);
            //++Bohong

            m_trainPaCorbaProxy->beginLiveAnnouncement(getAnnouncementId(),
                                                       getSessionId().c_str());

            // set the next state - if the begin completed ok
            m_nextState = TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS;

            TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
                getEffectiveCoverageIndices();

            // set each ready train to in progress
            for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
            {
                // only update the state to in progress if it was ready,
                if (m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]) ==
                    TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY)
                {
                    m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex],
                                                          TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS);
                }
            }
        }
        catch (const TA_IRS_Bus::AgentCommunicationException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::InvalidParameterException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }
        catch (const TA_IRS_Bus::TrainRadioException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_RADIO_CALL_RESET);
            FUNCTION_EXIT;
            return;
        }
        catch (...)
        {
            // Unexpected exception...
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);

            FUNCTION_EXIT;
            return;
        }

        FUNCTION_EXIT;
    }

    void TrainLiveBroadcast::continueTrainLiveAnnouncement()
        //throw(TA_Base_Core::BadParameterException)
    {
        ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trainsRequiringContinue = getTrainsRequiringContinue();

        try
        {
            m_trainPaCorbaProxy->continueLiveAnnouncement(getAnnouncementId(), getSessionId().c_str());
        }
        catch (const TA_IRS_Bus::AgentCommunicationException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT);
            return;
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT);
            return;
        }
        catch (const TA_IRS_Bus::InvalidParameterException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);
            return;
        }
        catch (...)
        {
            // Unexpected exception...
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);
            // operation not allowed - abort and send back to the PA Manager
            return;
        }

        for (int i = 0; i < trainsRequiringContinue.length(); ++i)
        {
            m_progressReportSentry->setLocalState(trainsRequiringContinue[i], m_nextState);

            PasHelpers::instance().submitTrainPAContinueLiveAnnouncementAuditMessage(
                getBroadcastId(),
                trainsRequiringContinue[i]);
        }

        PasHelpers::instance().sendBroadcastProgressUpdate(this);
    }

    void TrainLiveBroadcast::endTrainLiveAnnouncement() //throw(TA_Base_Core::BadParameterException)
    {
        FUNCTION_ENTRY("endTrainLiveAnnouncement");

        ThreadGuard guard(m_lock);

        // Do the actual disconnection work in terminate()

        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Termination);
        PasHelpers::instance().sendBroadcastProgressUpdate(this);

        FUNCTION_EXIT;
    }

    void TrainLiveBroadcast::disconnect()
    {
        FUNCTION_ENTRY("disconnect");

        ThreadGuard guard(m_lock);

        // set the next state
        m_nextState = TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED;

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
            m_trainPaCorbaProxy->endLiveAnnouncement(getAnnouncementId(),
                                                     getSessionId().c_str());

            // end triggered successfully, allow the time to complete
            m_terminateTimeLimit += maximumWaitTime;
        }
        catch (const TA_IRS_Bus::AgentCommunicationException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT);
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT);
        }
        catch (const TA_IRS_Bus::InvalidParameterException&)
        {
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);
        }
        catch (...)
        {
            // Unexpected exception...
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT);
            // operation not allowed - abort and send back to the PA Manager
        }

        FUNCTION_EXIT;
    }

    void TrainLiveBroadcast::processRuntimeNotification(const TA_IRS_Bus::PaTypes::PaUpdateEvent& event)
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

                        // Prevent multiple train ready messages resetting state
                        if (m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]) == TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING)
                        {
                            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY);
                        }

                        break;

                    case TA_IRS_Bus::PaTypes::LiveAnnouncementContinueRequired:

                        // continue required
                        if (TA_IRS_Bus::PaTypes::PaProceeding == event.commandResult)
                        {
                            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED);
                        }
                        else // TA_IRS_Bus::PaTypes::PaReset
                        {
                            // any continue timeout should be reset
                            if (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT == m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]))
                            {
                                m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], m_nextState);
                            }
                        }

                        break;

                    case TA_IRS_Bus::PaTypes::AnnouncementReset:

                        if (TA_IRS_Bus::PaTypes::MpuChangeoverError == event.commandResult)
                        {
                            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_MPU_CHANGEOVER);
                        }
                        else
                        {
                            m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONNECTION_RESET);
                        }

                        if (areAllTrainsReset())
                        {
                            setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Termination);
                            PasHelpers::instance().sendBroadcastProgressUpdate(this);
                        }

                        break;

                    case TA_IRS_Bus::PaTypes::CompletedAnnouncement:

                        // Ignored as only relevant to DVA messages
                        break;

                    case TA_IRS_Bus::PaTypes::LiveAnnouncementContinueTimeout:

                        m_progressReportSentry->setLocalState(effectiveCoverageIndices[indexIndex], TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT);
                        break;

                    case TA_IRS_Bus::PaTypes::PaCommandResult:
                        // This is the result of the setup command
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

    void TrainLiveBroadcast::processRuntimeNotification(const TA_IRS_Bus::PaTypes::PaAnnouncementUpdate& event)
    {
        FUNCTION_ENTRY("processRuntimeNotification");

        ThreadGuard guard(m_lock);

        // This means the call has been dropped
        if (event.updateType == TA_IRS_Bus::PaTypes::AudioCallLost)
        {
            // the broadcast must be set to the failed state
            m_progressReportSentry->setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_RADIO_CALL_RESET);
        }

        FUNCTION_EXIT;
    }

    bool TrainLiveBroadcast::areAllTrainsReset()
    {
        FUNCTION_ENTRY("areAllTrainsReset");

        bool allTrainsReset = true;

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
            getEffectiveCoverageIndices();

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            if (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONNECTION_RESET !=
                m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex]))
            {
                allTrainsReset = false;
            }
        }

        FUNCTION_EXIT;
        return allTrainsReset;
    }

    bool TrainLiveBroadcast::areAllTrainsFinished()
    {
        FUNCTION_ENTRY("areAllTrainsFinished");

        bool allTrainsFinished = true;

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices effectiveCoverageIndices =
            getEffectiveCoverageIndices();

        for (int indexIndex = 0; indexIndex < effectiveCoverageIndices.length(); ++indexIndex)
        {
            if ((TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING == m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex])) ||
                (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY == m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex])) ||
                (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED == m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex])) ||
                (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS == m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex])) ||
                (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT == m_progressReportSentry->getLocalState(effectiveCoverageIndices[indexIndex])))
            {
                allTrainsFinished = false;
                break;
            }
        }

        FUNCTION_EXIT;
        return allTrainsFinished;
    }
}
