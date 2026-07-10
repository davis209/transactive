/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/AdhocMessageRecordModel.cpp $
 * @author Jade Lee
 * @version $Revision: #4 $
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by: $Author: limin.zhu $
 *
 * Class to model the process of recording an ad-hoc message
 *
 */

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/AdhocMessageRecordPage.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaProgressReportSubscriber.h"
#include "app/pa/PAManager/src/PAManagerModel.h"
#include "app/pa/PAManager/src/PaRightsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

AdhocMessageRecordModel::AdhocMessageRecordModel(unsigned long pKey)
    :
    m_adhocMessageKey(pKey),
    m_progressReportSubscriber(0)
{
}

AdhocMessageRecordModel::~AdhocMessageRecordModel()
{
    // User should destroy attached window before destroying model, and
    // it's good practice to not rely on destructor to stop the recording
    // (because while recording, notifications sent out to attached window)
    TA_ASSERT(!isRecording(), "User should stop recording prior to destruction");
}

int AdhocMessageRecordModel::getRecordingSourceId() const
{
    return CachedConfig::getInstance()->getLocalConsoleRecordingSourceId();
}

bool AdhocMessageRecordModel::isRecording() const
{
    // If we have a valid subscriber, must still be recording
    return 0 != m_progressReportSubscriber;
}

void AdhocMessageRecordModel::beginRecording(IWindow& notificationWnd)
{
    LOG_SCOPE("AdhocMessageRecordModel::beginRecording");

    if (!PaRightsManager::getInstance().hasRightToRecordDvaMessage())
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        return;
    }

    if (isRecording())
    {
        TA_ASSERT(FALSE, "Cannot begin recording");
        return;
    }

    CWaitCursor hourGlassCursor;

    int sourceId = getRecordingSourceId();

    LOG_DEBUG("beginRecording(): IPAAgentCorbaDef::recordAdhocMessage: %s", nvps(sourceId, m_adhocMessageKey));

    // Allow exceptions to propogate
    std::string broadcastId;
    CORBA_CALL_RETURN(broadcastId, CachedConfig::getInstance()->getAgentObject(), recordAdhocMessage,
                      (sourceId, m_adhocMessageKey, CachedConfig::getInstance()->getSessionId().c_str()));

    LOG_DEBUG("beginRecording(): IPAAgentCorbaDef::recordAdhocMessage: %s", nvps(broadcastId));

    // Begin listening out for updates
    m_progressReportSubscriber = new PaProgressReportSubscriber(notificationWnd, broadcastId, reinterpret_cast<LONG>(this));
}

bool AdhocMessageRecordModel::processRecordAdhocNotification(UINT wParam, TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState>* broadcastState)
{
    LOG_SCOPE("AdhocMessageRecordModel::processRecordAdhocNotification");

    if (0 == m_progressReportSubscriber)
    {
        // Progress report subscriber is destroyed, nothing to respond
        // to for this notification (happens when recording is stopped, but
        // a few messages still lingering and get sent through)
        if (0 != broadcastState)
        {
            broadcastState->resetValue();
        }

        return true;
    }

    // Note: Important we don't hold onto the locked packet
    // retrieved here (because we delete m_progressReportSubscriber
    // when we call stopRecording - which is origin/owner of the lock)
    TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState bcState = m_progressReportSubscriber->getProgressReport().getLockedData().broadcastState;
    LOG_DEBUG("processRecordAdhocNotification(): %s", nvps(bcState));

    if (0 != broadcastState)
    {
        broadcastState->setValue(bcState);
    }

    if (TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval == bcState)
    {
        // The progress report indicates recording is complete, perform cleanup
        stopRecording();
        return true;
    }
    else
    {
        // Progress report indicates recording still in progress
        return false;
    }
}

bool AdhocMessageRecordModel::stopRecording()
{
    LOG_SCOPE("AdhocMessageRecordModel::stopRecording");

    if (!isRecording())
    {
        TA_ASSERT(FALSE, "Not currently recording");
        return true;
    }

    bool success = false;

    try
    {
        // terminate and remove the broadcast
        std::vector<std::string> broadcastIds;
        broadcastIds.push_back(m_progressReportSubscriber->getBroadcastId());

        if (0 == PaManagerModel::terminateAndRemoveBroadcastIds(broadcastIds,
                                                                CachedConfig::getInstance()->getAgentObject(),
                                                                BO_RECORDING_BROADCAST))
        {
            // No removal failures
            success = true;
        }
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
    }
    catch (const CORBA::Exception&)
    {
    }

    // We catch all exceptions above - if request failed, we ignore the failure
    // (ensuring we still removing subscriber)
    // - Leave it up to the user to attempt to remove broadcast manually (if required)
    delete m_progressReportSubscriber;
    m_progressReportSubscriber = 0;

    return success;
}

void AdhocMessageRecordModel::requestTermination()
{
    LOG_SCOPE("AdhocMessageRecordModel::requestTermination");

    if (0 != m_progressReportSubscriber)
    {
        if (m_progressReportSubscriber->getBroadcastId().length() > 0)
        {
            CWaitCursor hourGlassCursor;

            LOG_DEBUG("requestTermination(): IPAAgentCorbaDef::terminateBroadcast: %s", nvps(m_progressReportSubscriber->getBroadcastId()));

            CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), terminateBroadcast, (
                m_progressReportSubscriber->getBroadcastId().c_str(),
                CachedConfig::getInstance()->getSessionId().c_str()));
        }
    }
}

// jeffrey++ TES736
std::string AdhocMessageRecordModel::getBroadcastId()
{
    return m_progressReportSubscriber->getBroadcastId();
}

// ++jeffrey TES736
