/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/OperatorTerminalDvaPlaybackModel.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Provides a basic model which allows playback of a single DVA message
 *     at the operators workstation (using the special assigned zone)
 * The user should ensure all playback is terminated before destroying
 * any window handles supplied to the instance (otherwise the destroyed windows
 * will continue to be sent notifications)
 *
 */

#if !defined(AFX_OperatorTerminalDvaPlaybackModel_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define AFX_OperatorTerminalDvaPlaybackModel_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include "app/pa/PAManager/src/IWindow.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"

class PaProgressReportSubscriber;

class OperatorTerminalDvaPlaybackModel
{
public:

    OperatorTerminalDvaPlaybackModel();
    virtual ~OperatorTerminalDvaPlaybackModel();

    /**
     * isPlayingDvaMessage
     *
     * @return true if the broadcast started up by a call to playDvaMessage
     *          is still active
     *
     */
    bool isPlayingDvaMessage();

    /**
     * processPlayDvaNotification
     *
     * The owner window calls this function whenever it gets a progress report
     *  updates (subscribed to by this model)
     *
     * @return true if the progress report indicated playback is complete, and
     *          the model has destroyed the broadcast and subscriber (cleaned up)
     *
     * @param broadcastState [OUT] (optional) current state of broadcast,
     *                   or unset if broadcast no longer exists
     *
     */
    bool processPlayDvaNotification(UINT wParam,
                                    TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState>* broadcastState = 0);

    /**
     * stopDvaPlayback
     *
     * Stops the playback of the DVA message that was previously started with
     *  a call to playDvaMessage
     *
     * @pre isPlayingDvaMessage() returns true
     *
     * @return false if there was an error terminating the broadcast via the PA
     *      agent (but the broadcast will still be removed from this class)
     *
     */
    bool stopDvaPlayback();

    /**
     * playDvaMessage
     *
     * Plays a DVA Message to the operator (using special zone corresponding
     *  to the operator terminal).  The user must call stopDvaPlayback before
     *  the window defined by notificationWindow is destroyed
     *
     * @pre TA_Base_Core::DatabaseKey::isValidKey(messageKey)
     *
     * @pre isPlayingDvaMessage() returns false
     *
     * @param dvaMessageKey the primary key of the DVA Message to play back
     *
     * @param notificationWindow the window to receives progress report update
     *          notifications (this information must be passed on to this model,
     *          using the function processPlayDvaNotification)
     *
     * @exception BadParameterException if there is an internal error processing the request
     *
     * @exception AccessDeniedException (from PA Agent - insufficient rights)
     *
     * @exception CORBA::Exception in the event of comms failure
     *
     * @exception CachedMappingNotFoundException upon database config error (couldn't
     *          determine the zone to play back to)
     *
     */
    void playDvaMessage(unsigned long messageKey, IWindow& notificationWindow);

    /**
     * onStatusNameChange
     *
     * Called whenever there is a change in the broadcast ID from the global
     *  status name change subscriber - meaning we need to re-subscribe the
     *  underlying progress report object to the new broadcast ID
     *
     * @param from the old status name
     *
     * @param to the new status name
     *
     */
    void onStatusNameChange(const std::string from, const std::string to);

    /**
     * requestTermination
     *
     * Forwards a request to the agent to terminate this playback
     *  The broadcast should promptly enter a PendingRemoval state
     *
     * @exception CORBA::Exception on comms error
     * @exception TransactiveException on unexpected error
     *
     */
    void requestTermination();

private:

    /**
     * getOperatorTerminalBroadcastZone
     *
     * @return the primary key of the zone corresponding to the
     *      current operator terminal (used for local playback of messages)
     *
     * @exception CachedMappingNotFoundException if couldn't find the key
     *
     */
    unsigned long getOperatorTerminalBroadcastZoneKey();

    // Listens to progress report updates for when a single DVA item playback is in progress
    PaProgressReportSubscriber* m_playDvaMessageProgressReportSubscriber;
};

#endif // AFX_OperatorTerminalDvaPlaybackModel_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_
