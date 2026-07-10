/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/OperatorTerminalDvaPlaybackModel.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Provides a basic model which allows playback of a single DVA message
 *     at the operators workstation (using the special assigned zone)
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/OperatorTerminalDvaPlaybackModel.h"
#include "app/pa/PAManager/src/PaProgressReportSubscriber.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PAManagerModel.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "core/data_access_interface/src/DatabaseKey.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

OperatorTerminalDvaPlaybackModel::OperatorTerminalDvaPlaybackModel()
    :
    m_playDvaMessageProgressReportSubscriber(0)
{
}

OperatorTerminalDvaPlaybackModel::~OperatorTerminalDvaPlaybackModel()
{
    // Unsafe to leave this alive until destruction (since client
    // window is expected to be destroyed before we get here)
    TA_ASSERT(0 == m_playDvaMessageProgressReportSubscriber, "Client failed to clean up properly");
}

unsigned long OperatorTerminalDvaPlaybackModel::getOperatorTerminalBroadcastZoneKey()
{
    unsigned long zoneId = CachedConfig::getInstance()->getLocalConsolePlaybackZoneId();
    unsigned long locationKey = CachedConfig::getInstance()->getEntityLocationKey();

    // Allow cachedMappingNotFoundException to propogate..
    unsigned long pKey = TA_IRS_Bus::CachedMaps::getInstance()->
        getKeyFromPaZoneIdAndLocation(zoneId, locationKey);

    return pKey;
}

bool OperatorTerminalDvaPlaybackModel::isPlayingDvaMessage()
{
    return 0 != m_playDvaMessageProgressReportSubscriber;
}

bool OperatorTerminalDvaPlaybackModel::processPlayDvaNotification(
    UINT wParam,
    TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState>* broadcastState)
{
    LOG_SCOPE("OperatorTerminalDvaPlaybackModel::processPlayDvaNotification");

    if (0 == m_playDvaMessageProgressReportSubscriber)
    {
        // Progress report subscriber is destroyed, nothing to respond
        // to for this notification
        if (0 != broadcastState)
        {
            broadcastState->resetValue();
        }

        return true;
    }

    // Note: Important we don't hold onto the locked packet
    // retrieved here (because we delete m_playDvaMessageProgressReportSubscriber
    // when we call stopDvaPlayback)
    TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState bcState = m_playDvaMessageProgressReportSubscriber->getProgressReport().getLockedData().broadcastState;
    LOG_DEBUG("processPlayDvaNotification(): %s", nvps(bcState));

    if (0 != broadcastState)
    {
        broadcastState->setValue(bcState);
    }

    if (bcState == TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval)
    {
        // The progress report indicates playback is complete, perform cleanup
        stopDvaPlayback();
        return true;
    }
    else
    {
        // Progress report indicates playback continuing as normal
        return false;
    }
}

bool OperatorTerminalDvaPlaybackModel::stopDvaPlayback()
{
    if (!isPlayingDvaMessage())
    {
        TA_ASSERT(FALSE, "Already stopped");
        return true;
    }

    bool success = false;

    try
    {
        std::string broadcastId = m_playDvaMessageProgressReportSubscriber->getBroadcastId();

        if (0 == broadcastId.length())
        {
            // This happens when someone (maybe at another terminal) deleted broadcast
            // - although the broadcast should have self-terminated before getting here..
            // In any case, it means broadcast already deleted
            success = true;
        }
        else
        {
            // terminate and remove the broadcast
            std::vector<std::string> broadcastIds;
            broadcastIds.push_back(broadcastId);

            if (0 == PaManagerModel::terminateAndRemoveBroadcastIds(broadcastIds,
                                                                    CachedConfig::getInstance()->getAgentObject(),
                                                                    BO_PLAYBACK_BROADCAST))
            {
                // No removal failures
                success = true;
            }
        }
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
    }
    catch (const CORBA::Exception&)
    {
    }

    if (0 != m_playDvaMessageProgressReportSubscriber)
    {
        // We catch all exceptions above - if request failed, we ignore the failure
        // (ensuring we still removing subscriber)
        // - Leave it up to the user to attempt to remove broadcast manually (if required)
        delete m_playDvaMessageProgressReportSubscriber;
        m_playDvaMessageProgressReportSubscriber = 0;
    }

    return success;
}

void OperatorTerminalDvaPlaybackModel::playDvaMessage(unsigned long messageKey, IWindow& notificationWindow)
{
    LOG_SCOPE("OperatorTerminalDvaPlaybackModel::playDvaMessage");

    if (!PaRightsManager::getInstance().hasRightToPlaybackDvaMessage())
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        return;
    }

    if (isPlayingDvaMessage())
    {
        TA_ASSERT(FALSE, "Already playing a DVA message");
        return;
    }

    TA_ASSERT(TA_Base_Core::DatabaseKey::isValidKey(messageKey), "Invalid input message key");

    CWaitCursor hourGlassCursor;

    TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq messages;
    messages.length(1);
    messages[0] = messageKey;

    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
    zones.length(1);

    // Next call may throw an exception, make it propogate to indicate failure
    zones[0] = getOperatorTerminalBroadcastZoneKey();
    LOG_ERROR("Using zone ID pKey=%d", zones[0]);

    // Play the single message as would be done for a normal broadcast
    // (with parameters to have it play immediately, once off)
    std::string broadcastId;
    CORBA_CALL_RETURN(broadcastId, CachedConfig::getInstance()->getAgentObject(), broadcastStationDva,
                      (messages,
                       zones,
                       unsigned short(0),
                       false,
                       0,
                       0,
                       unsigned short(0),
                       false,
                       "",
                       CachedConfig::getInstance()->getSessionId().c_str()));

    // Upon success instantiate a new progress report subscriber
    // to check on the progress.. once it reaches PendingRemoval, it
    // should be removed with removeBroadcast
    m_playDvaMessageProgressReportSubscriber = new
        PaProgressReportSubscriber(notificationWindow,
                                   broadcastId,
                                   reinterpret_cast<LONG>(this));
}

void OperatorTerminalDvaPlaybackModel::onStatusNameChange(const std::string from, const std::string to)
{
    if (0 != m_playDvaMessageProgressReportSubscriber)
    {
        if (from == m_playDvaMessageProgressReportSubscriber->getBroadcastId())
        {
            // Our broadcast ID is changing, refresh subscription to the new broadcast ID
            // (in doing so this will update the broadcast ID, as returned by getBroadcastId())
            // If target name is empty string, this broadcast is being removed (no more
            // progress reports to listen out for)
            // resetSubscription handles empty length strings properly (by cancelling subscription)
            try
            {
                m_playDvaMessageProgressReportSubscriber->resetSubscription(to);
            }
            catch (...)
            {
                LOG_ERROR("onStatusNameChange: error updating playback object subscription to ", to);
            }
        }
    }
}

void OperatorTerminalDvaPlaybackModel::requestTermination()
{
    LOG_SCOPE("OperatorTerminalDvaPlaybackModel::requestTermination");

    if (0 != m_playDvaMessageProgressReportSubscriber)
    {
        if (m_playDvaMessageProgressReportSubscriber->getBroadcastId().length() > 0)
        {
            CWaitCursor hourGlassCursor;

            CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), terminateBroadcast, (
                m_playDvaMessageProgressReportSubscriber->getBroadcastId().c_str(),
                CachedConfig::getInstance()->getSessionId().c_str()));
        }
    }
}
