/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/ProgressReportSentry.cpp $
 * @author  A. Parker
 * @version $Revision: #4 $
 * Last modification : $DateTime: 2025/01/06 12:26:20 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#include "pch.h"
#include "CachedConfig.h"
#include "PasHelpers.h"
#include "ProgressReportSentry.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/stdutil/strings.h"
#include "core/utilities/src/MakeNameValuePairsString.h"
#include <time.h>

using namespace TA_Base_Core;

// Thread checked
ProgressReportSentry::ProgressReportSentry(const CORBA::String_member& broadcastId, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones)
{
    FUNCTION_ENTRY("ProgressReportSentry");

    // Initialise progress report
    m_progressReport.broadcastId = broadcastId;
    m_progressReport.broadcastState = TA_Base_Bus::IPAAgentCorbaDef::Creation;
    m_progressReport.globalState = TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE;
    // ++ Bug456
    m_progressReport.m_executionPhase = TA_Base_Bus::IPAAgentCorbaDef::EXECUTE_PHASE_1;
    // ++ Bug456
    m_inhibitAuditMessages = false;
    m_isStationBroadcast = true;

    m_progressReport.localProgresses.length(zones.length());

    for (int i = 0; i < zones.length(); ++i)
    {
        m_progressReport.localProgresses[i].zoneKeyOrTrainId = zones[i];
        resetZoneOrTrain(i);
    }

    FUNCTION_EXIT;
}

// Thread checked
ProgressReportSentry::ProgressReportSentry(const CORBA::String_member& broadcastId, const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains)
{
    FUNCTION_ENTRY("ProgressReportSentry");

    // Initialise progress report
    m_progressReport.broadcastId = broadcastId;
    m_progressReport.broadcastState = TA_Base_Bus::IPAAgentCorbaDef::Creation;
    m_progressReport.globalState = TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE;
    // TD 15621&16523
    m_progressReport.m_executionPhase = TA_Base_Bus::IPAAgentCorbaDef::EXECUTE_PHASE_1;
    // TD 15621&16523
    m_inhibitAuditMessages = false;
    m_isStationBroadcast = false;

    m_progressReport.localProgresses.length(trains.length());

    for (int i = 0; i < trains.length(); ++i)
    {
        m_progressReport.localProgresses[i].zoneKeyOrTrainId = trains[i];
        resetZoneOrTrain(i);
    }

    FUNCTION_EXIT;
}

// Thread checked
ProgressReportSentry::~ProgressReportSentry()
{
    FUNCTION_ENTRY("~ProgressReportSentry");
    FUNCTION_EXIT;
}

void ProgressReportSentry::setProgressReport(const TA_Base_Bus::IPAAgentCorbaDef::ProgressReport& progressReport)
{
    FUNCTION_ENTRY("setProgressReport");

    ThreadGuard guard(m_lock);

    m_progressReport = progressReport;

    FUNCTION_EXIT;
}

// Thread checked
void ProgressReportSentry::setBroadcastId(const std::string& newBroadcastId)
{
    FUNCTION_ENTRY("setBroadcastId");

    ThreadGuard guard(m_lock);

    // Note CORBA::String_members will automatically free and reallocate memory for const char* assignment
    m_progressReport.broadcastId = newBroadcastId.c_str();

    setHasProgressReportChanged(true);

    FUNCTION_EXIT;
}

// Thread checked
TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* ProgressReportSentry::getProgressReportCopy()
{
    FUNCTION_ENTRY("getProgressReportCopy");

    ThreadGuard guard(m_lock);

    auto newCopyP = new TA_Base_Bus::IPAAgentCorbaDef::ProgressReport();
    *newCopyP = m_progressReport; // DeepCopy
    FUNCTION_EXIT;
    return newCopyP;
}

// Thread checked
TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState ProgressReportSentry::getBroadcastState()
{
    FUNCTION_ENTRY("getBroadcastState");

    ThreadGuard guard(m_lock);

    FUNCTION_EXIT;
    return m_progressReport.broadcastState;
}

// Thread checked
void ProgressReportSentry::setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState state)
{
    LOG_SCOPE("ProgressReportSentry::setBroadcastState");
    FUNCTION_ENTRY("setBroadcastState");

    ThreadGuard guard(m_lock);

    submitBroadcastStateAuditMessage(m_progressReport.broadcastState, static_cast<const char*>(m_progressReport.broadcastId));

    if (m_progressReport.broadcastState == state)
    {
        FUNCTION_EXIT;
        return;
    }

    LOG_DEBUG("setBroadcastState(): %s", nvps(state));

    m_progressReport.broadcastState = state;
    setHasProgressReportChanged(true);

    FUNCTION_EXIT;
}

void ProgressReportSentry::resetGlobalState()
{
    FUNCTION_ENTRY("resetGlobalState");

    ThreadGuard guard(m_lock);

    if (m_progressReport.globalState == TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE)
    {
        FUNCTION_EXIT;
        return;
    }

    m_progressReport.globalState = TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE;
    setHasProgressReportChanged(true);

    FUNCTION_EXIT;
}

void ProgressReportSentry::setGlobalState(TA_Base_Bus::IPAAgentCorbaDef::EGlobalState globalState)
{
    LOG_SCOPE("ProgressReportSentry::setGlobalState");
    FUNCTION_ENTRY("setGlobalState");

    ThreadGuard guard(m_lock);

    if (m_progressReport.globalState == globalState)
    {
        FUNCTION_EXIT;
        return;
    }

    // We ignore GLOBAL_STATE_NO_FAILURE so that if an error is already encountered the only
    // way to revert back to GLOBAL_STATE_NO_FAILURE is to call resetGlobalState()
    //

    if (globalState == TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE)
    {
        FUNCTION_EXIT;
        return;
    }

    LOG_DEBUG("setGlobalState(): %s", nvps(globalState));

    submitBroadcastStateFailureAuditMessage(globalState, static_cast<const char*>(m_progressReport.broadcastId));

    m_progressReport.globalState = globalState;
    setHasProgressReportChanged(true);

    FUNCTION_EXIT;
}

void ProgressReportSentry::setLocalState(unsigned int index, TA_Base_Bus::IPAAgentCorbaDef::ELocalState localState)
{
    LOG_SCOPE("ProgressReportSentry::setLocalState");
    FUNCTION_ENTRY("setLocalState");

    ThreadGuard guard(m_lock);

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    TA_ASSERT(index < theLocalProgressSeq.length(), "index out of bounds");

    if (localState == theLocalProgressSeq[index].localState)
    {
        // No change
        FUNCTION_EXIT;
        return;
    }

    if (theLocalProgressSeq[index].localState == TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_ABORTED)
    {
        // Already aborted.  This overrides all other states until the zone/train is reset
        FUNCTION_EXIT;
        return;
    }

    LOG_DEBUG("setLocalState(): %s", nvps(index, localState));

    switch (localState)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_ABORTED:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CYCLE:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_ZONE:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_OVERRIDDEN:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_TRAIN:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONNECTION_RESET:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INTERRUPTED:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_VERSION_CONFLICT:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INVALID_REQUEST:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY_WITH_OVERRIDE_OPTION:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_NO_PERMISSION:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BAD_COMMAND:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_ORIGIN:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_VERSION:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_MPU_CHANGEOVER:

            theLocalProgressSeq[index].errorOccurred = true;
            theLocalProgressSeq[index].localState = localState;
            setHasProgressReportChanged(true);
            break;

            // ss++
            // TD13952
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED:
            theLocalProgressSeq[index].errorOccurred = false;
            theLocalProgressSeq[index].localState = localState;
            setHasProgressReportChanged(true);
            break;

            // ++ss
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS_DWELL_TIME:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY:
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT: // this indicates a possible failure only
            if (theLocalProgressSeq[index].errorOccurred == false)
            {
                theLocalProgressSeq[index].localState = localState;
                setHasProgressReportChanged(true);
            }
            else
            {
                // This zone/train has been flagged with an error, do not change it.
            }

            break;

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED:
            // Special case for this one, display this regardless of errorflag.
            theLocalProgressSeq[index].localState = localState;
            setHasProgressReportChanged(true);
            break;

        default:
            TA_ASSERT(false, "Unrecognised ELocalState");
            break;
    }

    // raise an audit message if there was an error
    if (theLocalProgressSeq[index].errorOccurred)
    {
        submitZoneBroadcastFailureAuditMessage(index, localState);
    }

    FUNCTION_EXIT;
}

// ++ Bug456
void ProgressReportSentry::setExecutionPhase(TA_Base_Bus::IPAAgentCorbaDef::EExecutePhase executionPhase)
{
    LOG_SCOPE("ProgressReportSentry::setExecutionPhase");
    FUNCTION_ENTRY("setExecutionPhase");

    ThreadGuard guard(m_lock);

    if (m_progressReport.m_executionPhase == executionPhase)
    {
        FUNCTION_EXIT;
        return;
    }

    LOG_DEBUG("setExecutionPhase(): %s", nvps(executionPhase));

    m_progressReport.m_executionPhase = executionPhase;
    setHasProgressReportChanged(true);

    FUNCTION_EXIT;
}

// ++ Bug456

TA_Base_Bus::IPAAgentCorbaDef::ELocalState ProgressReportSentry::getLocalState(unsigned int index)
{
    FUNCTION_ENTRY("getLocalState");

    ThreadGuard guard(m_lock);

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    TA_ASSERT(index < theLocalProgressSeq.length(), "index out of bounds");

    FUNCTION_EXIT;
    return theLocalProgressSeq[index].localState;
}

void ProgressReportSentry::setOverridingSourcePriority(unsigned int index, CORBA::Octet overridingSourcePriority)
{
    FUNCTION_ENTRY("setOverridingSourcePriority");

    ThreadGuard guard(m_lock);

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    TA_ASSERT(index < theLocalProgressSeq.length(), "index out of bounds");

    if (theLocalProgressSeq[index].overridingSourcePriorityForZonesOnly != overridingSourcePriority)
    {
        LOG_DEBUG("setOverridingSourcePriority(): %s", nvps(index, overridingSourcePriority));
        theLocalProgressSeq[index].overridingSourcePriorityForZonesOnly = overridingSourcePriority;
        setHasProgressReportChanged(true);
    }

    FUNCTION_EXIT;
}

bool ProgressReportSentry::getErrorOccurred(unsigned int index)
{
    FUNCTION_ENTRY("getErrorOccurred");

    // Implicitly threadsafe

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    TA_ASSERT(index < theLocalProgressSeq.length(), "index out of bounds");

    FUNCTION_EXIT;
    return theLocalProgressSeq[index].errorOccurred;
}

void ProgressReportSentry::resetZoneOrTrain(unsigned int index)
{
    FUNCTION_ENTRY("resetZoneOrTrain");

    ThreadGuard guard(m_lock);

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    TA_ASSERT(index < theLocalProgressSeq.length(), "index out of bounds");

    theLocalProgressSeq[index].localState = TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS;
    theLocalProgressSeq[index].overridingSourcePriorityForZonesOnly = 0;
    theLocalProgressSeq[index].errorOccurred = false;

    FUNCTION_EXIT;
}

void ProgressReportSentry::flagNonCompletedWithErrors()
{
    FUNCTION_ENTRY("flagNonCompletedWithErrors");

    ThreadGuard guard(m_lock);

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    for (int i = 0; i < theLocalProgressSeq.length(); ++i)
    {
        if (theLocalProgressSeq[i].localState != TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED)
        {
            theLocalProgressSeq[i].errorOccurred = true;
        }
    }

    FUNCTION_EXIT;
}

void ProgressReportSentry::flagNonCompletedWithErrorsAndAborted()
{
    FUNCTION_ENTRY("flagNonCompletedWithErrorsAndAborted");

    ThreadGuard guard(m_lock);

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    for (int i = 0; i < theLocalProgressSeq.length(); ++i)
    {
        if (theLocalProgressSeq[i].localState != TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED)
        {
            theLocalProgressSeq[i].localState = TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_ABORTED;
            theLocalProgressSeq[i].errorOccurred = true;
        }
    }

    FUNCTION_EXIT;
}

// TD17456
void ProgressReportSentry::flagNonValidWithFailure()
{
    FUNCTION_ENTRY("flagNonValidWithFailure");

    ThreadGuard guard(m_lock);

    auto& theLocalProgressSeq = m_progressReport.localProgresses;

    for (int i = 0; i < theLocalProgressSeq.length(); ++i)
    {
        theLocalProgressSeq[i].localState = TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_ZONE;
        theLocalProgressSeq[i].errorOccurred = true;
    }

    FUNCTION_EXIT;
}

// TD17456

void ProgressReportSentry::submitBroadcastStateAuditMessage(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, const std::string& broadcastId) const
{
    FUNCTION_ENTRY("submitBroadcastStateAuditMessage");

    if (!m_inhibitAuditMessages)
    {
        PasHelpers::instance().submitPaAgentBroadcastStateAuditMessage(broadcastId, newState);
    }

    FUNCTION_EXIT;
}

void ProgressReportSentry::submitBroadcastStateFailureAuditMessage(const TA_Base_Bus::IPAAgentCorbaDef::EGlobalState& newStateFailure, const std::string& broadcastId) const
{
    FUNCTION_ENTRY("submitBroadcastStateFailureAuditMessage");

    if (!m_inhibitAuditMessages)
    {
        PasHelpers::instance().submitPaAgentBroadcastStateFailureAuditMessage(broadcastId, newStateFailure);
    }

    FUNCTION_EXIT;
}

void ProgressReportSentry::submitZoneBroadcastFailureAuditMessage(unsigned long index, TA_Base_Bus::IPAAgentCorbaDef::ELocalState localState)
{
    FUNCTION_ENTRY("submitZoneBroadcastFailureAuditMessage");

    if (!m_inhibitAuditMessages)
    {
        std::string reason;

        switch (localState)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS:
                reason = "In progress";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS_DWELL_TIME:
                reason = "In progress dwell time";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING:
                reason = "Connecting";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY:
                reason = "Train ready";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED:
                reason = "Continue live announcement required";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED:
                reason = "Completed";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_ABORTED:
                reason = "Aborted";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CYCLE:
                reason = "Failed cycle";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_ZONE:
                reason = "Failed zone";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_OVERRIDDEN:
                reason = "Overridden";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_TRAIN:
                reason = "Failed train";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONNECTION_RESET:
                reason = "Connection reset";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INTERRUPTED:
                reason = "Failed interrupted";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_VERSION_CONFLICT:
                reason = "Version conflict";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INVALID_REQUEST:
                reason = "Invalid request";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY:
                reason = "Busy";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY_WITH_OVERRIDE_OPTION:
                reason = "Busy with override option";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT:
                reason = "TIMS continue request not received";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_NO_PERMISSION:
                reason = "No duty for this train at this location";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED:
                reason = "Train first cycle cancelled";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BAD_COMMAND:
                reason = "Failed bad command";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_ORIGIN:
                reason = "Failed different origin";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_VERSION:
                reason = "Failed different version";
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_MPU_CHANGEOVER:
                reason = "Failed mpu changeover";
                break;
        }

        // already locked by calling method
        auto& theLocalProgressSeq = m_progressReport.localProgresses;

        TA_ASSERT(index < theLocalProgressSeq.length(), "index out of bounds");

        // if its a station broadcast
        if (m_isStationBroadcast)
        {
            //TD10320
            //            PasHelpers::instance().submitPaZoneBroadcastFailureAuditMessage(
            //               theLocalProgressSeq[index].zoneKeyOrTrainId,
            //                reason );
        }
        else
        {
            // if its a train broadcast
            PasHelpers::instance().submitPaTrainBroadcastFailureAuditMessage(theLocalProgressSeq[index].zoneKeyOrTrainId,
                                                                             CachedConfig::getInstance()->getAgentLocationKey(),
                                                                             reason);
        }
    }

    FUNCTION_EXIT;
}
