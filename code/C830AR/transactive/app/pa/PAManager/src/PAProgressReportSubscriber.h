/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAProgressReportSubscriber.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * PaProgressReportSubscriber bridges communications between the PA Manager
 * and CORBA broadcasts
 */

#if !defined(PaProgressReportSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
#define PaProgressReportSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_

#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/threads/src/Thread.h"
#include "core/message/src/SpecialisedMessageSubscriber.h"
#include "core/message/IDL/src/CommsMessageCorbaDef.h"
#include "core/exceptions/src/PasExceptions.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "bus/pa/common/src/LockedPacket.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "app/pa/PAManager/src/IWindow.h"

namespace TA_Base_Core
{
    class AuditMessageSender;
}

typedef TA_IRS_Bus::LockedPacket<const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData> LockedPacketProgressReport;

class PaProgressReportSubscriber
    : public TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>
{
    friend class UnitTestAccessBridge;

public:

    /**
     * PaProgressReportSubscriber (constructor)
     *
     * @param pageView the reference to the window that should be notified when
     * CORBA updates come through
     *
     * @param broadcastId filtering broadcast string
     *
     * @param lParam (optional) lParam value passed on when notifications
     *              sent to the attached window
     *
     */
    PaProgressReportSubscriber(IWindow& pageView,
                               const std::string& broadcastId,
                               LONG lParam = 0);

    virtual ~PaProgressReportSubscriber();

    /**
     * receiveSpecialisedMessage
     *
     * Required derived function to receive messages
     *
     */
    virtual void receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message);

    /**
     * resetSubscription
     *
     * @param broadcastId filtering broadcast string
     *
     */
    void resetSubscription(const std::string& broadcastId);

    /**
     * getProgressReport
     *
     * Returns the last received packet of data, wrapped in an object so the data is
     * guaranteed to remain unmodified until the client is finished with the data
     *
     * @param bPeek set to true if want to peek at the packet, but not allow the class
     *              to know it's been acknowledged
     *
     * @return the progress report item wrapped in a LockedPacket object
     *
     * @exception ValueNotSetException if the progress report doesn't actually exist
     *      yet (user called this function before a notification was posted that it exists)
     *
     */
    const LockedPacketProgressReport getProgressReport(bool bPeek = false);

    /**
     * forceProgressUpdate
     *
     * Forces an update of the progress report (rather than waiting for an asychronous packet)
     * by requesting an update directly via the supplied PA agent)
     *
     * @param paAgent the CORBA agent to use when requesting the progress report update
     *
     * @exception ObjectResolutionException if the supplied PA agent is invalid
     *
     * @exception AccessDeniedException (from PA Agent - insufficient rights)
     *
     *
     */
    void    forceProgressUpdate(PAAgentNamedObject& paAgent);

    /**
     * getBroadcastId (simple accessor)
     *
     */
    const std::string getBroadcastId() const
    {
        return m_broadcastId;
    }

private:

    // Copy constructor not used so made private
    PaProgressReportSubscriber(const PaProgressReportSubscriber& thePaProgressReportSubscriber);
    // Assignment not used so it is made private
    PaProgressReportSubscriber& operator=(const PaProgressReportSubscriber&);

    /**
     * processProgressReport
     *
     * Entry point for processing of BroadcastProgressUpdateData objects received
     *
     * @param progressReport the newly received BroadcastProgressUpdateData data
     *
     */
    void    processProgressReport(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& progressReport);

    // The window to receive notifications when new data arrives and is
    // ready to be processed via getProgressReport
    IWindow& m_receivingView;

    TA_Base_Core::NonReEntrantThreadLockable         m_packetDataAndNotificationLock;
    TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData>    m_progressReportCachedData;

    bool                                    m_notificationAcknowledged;

    // Broadcast ID to receive the progress reports from - the currently subscribed to broadcast
    std::string   m_broadcastId;

    LONG    m_lParamValue;
};

#endif // !defined(PaProgressReportSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
