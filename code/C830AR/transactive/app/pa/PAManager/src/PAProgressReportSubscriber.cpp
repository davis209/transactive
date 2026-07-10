/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAProgressReportSubscriber.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * PaProgressReportSubscriber bridges communications between the PA Manager
 * and CORBA broadcasts
 */

#if defined(_MSC_VER)
#pragma warning(disable : 4786 4146 4018 4250 4290)
#endif // defined _MSC_VER

#include "app/pa/PAManager/src/StdAfx.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/RunParams.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaProgressReportSubscriber.h"
#include "core/utilities/src/StdUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;

///////////////////////////////////////////////
///////////////////////////////////////////////
// [Testing code]
/*
PaProgressReportSubscriber* g_PAPR = 0;

DWORD WINAPI Report_Tester(void* val)
{
    for(;;)
    {
        if (g_PAPR)
        {
            TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData pr;

            pr.broadcastId = CORBA::string_dup("");

            pr.zoneProgresses.length(5);

            for (int a = 0; a < 5; a ++)
            {
                TA_Base_Bus::IPAAgentCorbaDef::ZoneProgress zp;

                zp.overridingSourcePriority = 25;
                zp.zoneState = TA_Base_Bus::IPAAgentCorbaDef::EZoneState(
                                    TA_Base_Bus::IPAAgentCorbaDef::Connecting + rand() % 9);

                zp.zoneKey = 5;

                pr.zoneProgresses[a] = zp;
            }

            g_PAPR->processProgressReport(pr);
        }

        Sleep(500);
    };

    ExitThread(0);
    return 0;
}

DWORD g_prsthreadID = 0;
HANDLE g_prshThread = CreateThread(0, 0, Report_Tester, 0, 0, &g_prsthreadID);
*/
///////////////////////////////////////////////
///////////////////////////////////////////////

// ExceptionChecked
PaProgressReportSubscriber::PaProgressReportSubscriber
(
    IWindow& pageView,
    const std::string& broadcastId,
    LONG lParam
)
    :
    m_receivingView(pageView),
    m_notificationAcknowledged(true),
    m_lParamValue(lParam)
{
    FUNCTION_ENTRY("Constructor");

    // [Testing code] g_PAPR = this;

    // Begin subscription immediately
    try
    {
        resetSubscription(broadcastId);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // If unable to subscribe, do so at later date
        PaErrorHandler::displayError(PaErrorHandler::INIT_ERROR);
    }

    FUNCTION_EXIT;
}

// ExceptionChecked
PaProgressReportSubscriber::~PaProgressReportSubscriber()
{
    FUNCTION_ENTRY("Destructor");

    TA_Base_Core::ThreadGuard sync(m_packetDataAndNotificationLock);

    // This is not a safe condition, we were still posting notifications to this window
    TA_ASSERT(::IsWindow(m_receivingView.getHWnd()), "Window no longer valid while in destructor");

    try
    {
        resetSubscription("");
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }

    FUNCTION_EXIT;
}

// Verified to be threadsafe - discussions with CamR
// ExceptionChecked
void PaProgressReportSubscriber::resetSubscription(const std::string& broadcastId)
{
    FUNCTION_ENTRY("resetSubscription");

    m_broadcastId = broadcastId;

    // Harmless to unsubscribe if not subscribed
    TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(this);

    // Only need to resubscribe provided a valid name exists (if we take this code
    // out, a crash occurs during stopping of DVA playbacks, found while resolving TD #2966)
    if (broadcastId.length() > 0)
    {
        // Location key = region key
        long regionKey = CachedConfig::getInstance()->getEntityLocationKey();

        NameValuePair nvp(TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_ID_FILTER_STRING,
                          TA_Base_Core::NameValuePair::EQUALS,
                          broadcastId);

        std::vector<NameValuePair*> extraFilterableData;
        extraFilterableData.push_back(&nvp);

        // Subscribe for updates from the local PA Agent
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(
            PAAgentComms::BroadcastProgressUpdate,	               //context
            this,				                                    //SpecialisedMessageSubscriber
            0,	                                                    //unsigned long entityKey,
            0,		    											//unsigned long subsystemKey,
            regionKey,                                              //unsigned long regionKey);
            &extraFilterableData);    								//broadcastId
    }

    FUNCTION_EXIT;
}

// ExceptionChecked
void PaProgressReportSubscriber::receiveSpecialisedMessage(const CommsMessageCorbaDef& message)
{
    LOG_SCOPE("PaProgressReportSubscriber::receiveSpecialisedMessage");
    FUNCTION_ENTRY("receiveSpecialisedMessage");

    try
    {
        const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData* progressReport = 0;

        if (((message.messageState) >>= progressReport) == 0)
        {
            LOG_ERROR("Failed to interpret message");
            return;
        }

        LOG_DEBUG("receiveSpecialisedMessage(): %s", nvps(*progressReport));

        processProgressReport(*progressReport);
    }
    catch (...)
    {
        //submitAuditMessage("Failed to process information received from PA");
        LOG_ERROR("Exception thrown while receiving event");
    }

    FUNCTION_EXIT;
}

void PaProgressReportSubscriber::forceProgressUpdate(PAAgentNamedObject& paAgent)
{
    LOG_SCOPE("PaProgressReportSubscriber::forceProgressUpdate");
    FUNCTION_ENTRY("forceProgressUpdate");

    if (m_broadcastId.length() > 0)
    {
        CWaitCursor hourGlassCursor;
        // Call the agent directly, and pass result directly to process function
        // it will be treated identically to a report that was received asynchronously
        // The function is thread safe, so it's safe to call this forceProgressUpdate from
        // a separate thread - that's where it'll usually be called from..
        TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData* ret = NULL;
        CORBA_CALL_RETURN(ret, paAgent, getProgressReport, (m_broadcastId.c_str()));
        std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData> data(ret);

        processProgressReport(*data.get());
    }

    FUNCTION_EXIT;
}

void PaProgressReportSubscriber::processProgressReport(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& progressReport)
{
    LOG_SCOPE("PaProgressReportSubscriber::processProgressReport");
    FUNCTION_ENTRY("processProgressReport");
    TA_Base_Core::ThreadGuard sync(m_packetDataAndNotificationLock);

    // We will make a local copy of the message, if for whatever reason
    // the WM_PA_STN_STATUS_REPORT_UPDATE is not acknowledged, the packet will
    // be dropped (and overwritten) - fortunately we don't have to worry about this
    // because each packet provides full refresh information (we can just read
    // the latest packet and be up to date)
    m_progressReportCachedData.setValue(progressReport);

    // Post the relevant message to the window that constructed this subscriber
    // The local copy has been created and is now ready for processing by the thread
    // that created this subscribe

    // Only post the notification if the last notification has been responded to
    if (m_notificationAcknowledged)
    {
        if (0 != m_receivingView.getHWnd() &&
            ::IsWindow(m_receivingView.getHWnd()))
        {
            // Invalidate the acknowledged state, now that a new process request has been issued
            m_notificationAcknowledged = false;

            LOG_INFO("Posting message WM_PA_STN_STATUS_REPORT_UPDATE to window %d", m_receivingView.getHWnd());

            ::PostMessage(m_receivingView.getHWnd(),
                          WM_PA_STN_STATUS_REPORT_UPDATE,
                          0,
                          m_lParamValue);
        }
        else
        {
            // User didn't destroy subscriber before destroying attached window
            // The client code needs to be fixed
            TA_ASSERT(FALSE, "Specified window does not exist");
        }
    }
    else
    {
        // We get in here if the update requests are not being processed fast enough by the client
        // That is, we're receiving updates over the network faster than they can be processed
        // All that happens is we update the to the latest packet, and processing of current packet
        // doesn't occur
    }

    FUNCTION_EXIT;
}

const LockedPacketProgressReport PaProgressReportSubscriber::getProgressReport(bool bPeek)
{
    FUNCTION_ENTRY("getProgressReport");
    // Acquire the locked progress report immediately, initiating the lock on m_packetDataAndNotificationLock
    LockedPacketProgressReport lockedProgressReport(
        m_packetDataAndNotificationLock,
        m_progressReportCachedData.getValue());

    // Acknowledge the message, to ensure we don't queue up
    // message arrived notifications if we're slow to respond
    if (!bPeek)
    {
        LOG_INFO("Collecting message for window %d, %s", m_receivingView.getHWnd(), st::get_class_name_and_address(m_receivingView));

        // If notification already acknowledged, means the update was attempted
        // to be retrieved twice, happens when two
        TA_ASSERT(!m_notificationAcknowledged, "Mismatched getProgressReport call");

        m_notificationAcknowledged = true;
    }

    return lockedProgressReport;
}
