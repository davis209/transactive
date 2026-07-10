/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/GenericStatusModel.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * GenericStatusModel controls the shared business logic for broadcast status dialog screens
 */

#include "app/pa/PAManager/src/StdAfx.h"

#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/GenericStatusModel.h"
#include "app/pa/PAManager/src/PAManagerModel.h"
#include "app/pa/PAManager/src/StationStatusHelper.h"
#include "app/pa/PAManager/src/TrainStatusHelper.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

GenericStatusModel::GenericStatusModel(GenericStatusPage& page,
                                       const std::string& broadcastId,
                                       const EModelSubType& subType)
    :
    m_pageView(page),
    m_progressReportSubscriber(page, broadcastId),
    m_broadcastId(broadcastId),
    m_modelSubType(subType)
{
    FUNCTION_ENTRY("GenericStatusModel");
    FUNCTION_EXIT;
}

GenericStatusModel::~GenericStatusModel()
{
    FUNCTION_ENTRY("~GenericStatusModel");
    FUNCTION_EXIT;
}

TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState GenericStatusModel::processProgressUpdate()
{
    LOG_SCOPE("GenericStatusModel::processProgressUpdate");
    FUNCTION_ENTRY("processProgressUpdate");

    // Obtain the packet we need to access for processing - ValueNotSetException
    // may propogate from this call
    const LockedPacketProgressReport lockedProgressReport = m_progressReportSubscriber.getProgressReport();

    // Obtain direct reference to data
    const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& lockedData = lockedProgressReport.getLockedData();

    processProgressUpdateData(lockedData);

    FUNCTION_EXIT;
    return lockedData.broadcastState;
}

void GenericStatusModel::processProgressUpdateData(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& lockedData)
{
    LOG_SCOPE("GenericStatusModel::processProgressUpdateData");
    FUNCTION_ENTRY("processProgressUpdateData");

    try
    {
        // Update the page view model as required
        m_pageView.updateReportLists(lockedData);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Some of the items in the list won't have been updated properly
        PaErrorHandler::displayError(PaErrorHandler::DATABASE_ERROR);
    }

    FUNCTION_EXIT;
}

// AZ++ TD10377 -- added parameter to optionally display error message dialog or
// throw exception
void GenericStatusModel::forceProgressReportUpdate(bool pShowErrorMessageDialog)
{
    FUNCTION_ENTRY("forceProgressReportUpdate");

    try
    {
        m_progressReportSubscriber.forceProgressUpdate(CachedConfig::getInstance()->getAgentObject());
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        if (pShowErrorMessageDialog)
        {
            PaErrorHandler::displayError(PaErrorHandler::BROADCAST_ID_NOT_FOUND);
        }
        else
        {
            FUNCTION_EXIT;
            throw;
        }
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        // Progress report unable to be updated
        if (pShowErrorMessageDialog)
        {
            PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
        }
        else
        {
            FUNCTION_EXIT;
            throw;
        }
    }
    catch (const TA_Base_Core::AccessDeniedException&)
    {
        if (pShowErrorMessageDialog)
        {
            PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        }
        else
        {
            FUNCTION_EXIT;
            throw;
        }
    }
    catch (const CORBA::Exception&)
    {
        // Comms error
        if (pShowErrorMessageDialog)
        {
            PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
        }
        else
        {
            FUNCTION_EXIT;
            throw;
        }
    }

    FUNCTION_EXIT;
}

// ++AZ TD10377

void GenericStatusModel::refreshSubscription()
{
    FUNCTION_ENTRY("refreshSubscription");

    m_progressReportSubscriber.resetSubscription(getBroadcastId());

    FUNCTION_EXIT;
}

std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig> GenericStatusModel::getBroadcastConfig()
{
    LOG_SCOPE("DvaStatusModel::getBroadcastConfig");
    FUNCTION_ENTRY("getBroadcastConfig");

    CWaitCursor hourGlassCursor;

    TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* result = NULL;
    CORBA_CALL_RETURN(result, CachedConfig::getInstance()->getAgentObject(), getBroadcastConfig, (getBroadcastId().c_str()));

    FUNCTION_EXIT;
    return std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig>(result);
}

CString GenericStatusModel::formatDateIntoString(time_t time)
{
    FUNCTION_ENTRY("formatDateIntoString");

    // In format "17:30 - 8 Sep"
    CString dateFormatStr("%H:%M - %d %b");

    FUNCTION_EXIT;
    return CTime(time).Format(dateFormatStr);
}

CString GenericStatusModel::formatTimeOfDayIntoString(int hour, int min)
{
    FUNCTION_ENTRY("formatTimeOfDayIntoString");

    CString result;
    // In format "08:05"
    result.Format("%02d:%02d", hour, min);

    FUNCTION_EXIT;
    return result;
}

void GenericStatusModel::retryFailedBroadcast()
{
    FUNCTION_ENTRY("retryFailedBroadcast");

    retryBroadcast(FAILED);

    FUNCTION_EXIT;
}

void GenericStatusModel::retryAllBroadcast()
{
    FUNCTION_ENTRY("retryAllBroadcast");

    retryBroadcast(ALL);

    FUNCTION_EXIT;
}

void GenericStatusModel::retryBroadcast(const GenericStatusModel::ERetryMode& mode)
{
    FUNCTION_ENTRY("retryBroadcast");

    // Ensure progress report exists and is up to date, required
    // for retrieval of station zones or train IDs (obtained from the progress report)
    forceProgressReportUpdate(true);

    try
    {
        // We access the cached report data from m_progressReportSubscriber
        // as there isn't a direct query to collect zones in a specific broadcast group
        // Obtain the packet we need to access for processing (but don't purge it from queue)
        const LockedPacketProgressReport lockedProgressReport =
            m_progressReportSubscriber.getProgressReport(true);

        if (isStationModel())
        {
            StationStatusHelper::retryStationBroadcast(
                CachedConfig::getInstance()->getAgentObject(),
                getBroadcastId(),
                lockedProgressReport,
                mode);
        }
        else if (isTrainModel())
        {
            TrainStatusHelper::retryTrainBroadcast(
                CachedConfig::getInstance()->getAgentObject(),
                getBroadcastId(),
                lockedProgressReport,
                mode,
                getHasOverrideOption());
        }
    }
    catch (const TA_Base_Core::ValueNotSetException&)
    {
        // Progress report data not set, operation must be aborted (request ignored)
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }

    // Prompt for an immediate update of progress report (in an effort to become
    // up to date immediately)
    forceProgressReportUpdate(true);

    FUNCTION_EXIT;
}

bool GenericStatusModel::getHasOverrideOption()
{
    FUNCTION_ENTRY("getHasOverrideOption");

    if (!isTrainModel())
    {
        // Only the train models have an override option
        TA_ASSERT(FALSE, "Invalid context");

        FUNCTION_EXIT;
        return false;
    }
    else
    {
        // All train models need to define this function
        TA_ASSERT(FALSE, "No implementation for getHasOverrideOption");

        FUNCTION_EXIT;
        return false;
    }
}

void GenericStatusModel::terminateBroadcast()
{
    LOG_SCOPE("DvaStatusModel::terminateBroadcast");
    FUNCTION_ENTRY("terminateBroadcast");

    CWaitCursor hourGlassCursor;

    // Allow all exceptions to propogate upwards
    CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), terminateBroadcast, (
        getBroadcastId().c_str(),
        CachedConfig::getInstance()->getSessionId().c_str()));

    FUNCTION_EXIT;
}

bool GenericStatusModel::terminateAndRemoveBroadcast(EBroadcastOrigin bcc)
{
    FUNCTION_ENTRY("terminateAndRemoveBroadcast");

    std::vector<std::string> broadcastIds;
    broadcastIds.push_back(getBroadcastId());

    int result = PaManagerModel::terminateAndRemoveBroadcastIds(
        broadcastIds,
        CachedConfig::getInstance()->getAgentObject(),
        bcc);

    FUNCTION_EXIT;
    return result > 0;
}

const std::string GenericStatusModel::getStatusString(const TA_Base_Bus::IPAAgentCorbaDef::ELocalState& status)
{
    FUNCTION_ENTRY("getStatusString");

    switch (status)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS:
            FUNCTION_EXIT;
            return "In progress";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS_DWELL_TIME:
            FUNCTION_EXIT;
            return "In progress (dwell time)";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING:
            FUNCTION_EXIT;
            return "Connecting";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY:
            FUNCTION_EXIT;
            return "Train ready";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED:
            FUNCTION_EXIT;
            return "Continue required";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED:
            FUNCTION_EXIT;
            return "Completed";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_ABORTED:
            FUNCTION_EXIT;
            return "Aborted";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CYCLE:
            FUNCTION_EXIT;
            return "Failed cycle";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_ZONE:
            FUNCTION_EXIT;
            return "Failed zone";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_OVERRIDDEN:
            FUNCTION_EXIT;
            return "Overridden";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_TRAIN:
            FUNCTION_EXIT;
            return "Failed train";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONNECTION_RESET:
            FUNCTION_EXIT;
            return "Connection reset";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INTERRUPTED:
            FUNCTION_EXIT;
            return "Failed interrupted";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_VERSION_CONFLICT:
            FUNCTION_EXIT;
            return "Version conflict";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INVALID_REQUEST:
            FUNCTION_EXIT;
            return "Invalid request";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY:
            FUNCTION_EXIT;
            return "Busy";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY_WITH_OVERRIDE_OPTION:
            FUNCTION_EXIT;
            return "Busy with override option";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT:
            FUNCTION_EXIT;
            return "Live timeout";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED:
            FUNCTION_EXIT;
            return "First Cycle Cancelled";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_NO_PERMISSION:
            FUNCTION_EXIT;
            return "No Permission";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BAD_COMMAND:
            FUNCTION_EXIT;
            return "Bad Command";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_ORIGIN:
            FUNCTION_EXIT;
            return "Different Origin";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_VERSION:
            FUNCTION_EXIT;
            return "Different Version";

        case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_MPU_CHANGEOVER:
            FUNCTION_EXIT;
            return "MPU Changeover";

        default:
            LOG_ERROR("GenericStatusModel::getStatusString: Unrecognized status (%d)", int(status));

            //TA_THROW(TA_Base_Core::TransactiveException("Unrecognized local status"));
            FUNCTION_EXIT;
            return "???";
    }
}
