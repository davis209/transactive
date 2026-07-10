/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainRadioPttMonitor.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Handles windows notification based monitoring of the PTT enabled state
 *     of the train radio microphone
 *
 */

#if !defined(AFX_TrainRadioPttMonitor_H__8420358B_D5E4_4D3F_B59C_AF38445C948D__INCLUDED_)
#define AFX_TrainRadioPttMonitor_H__8420358B_D5E4_4D3F_B59C_AF38445C948D__INCLUDED_

#include "bus/radio/PTTMonitorLibrary/src/IPTTMonitoring.h"

namespace TA_IRS_Bus
{
    class PTTButton;
}

class TrainRadioPttMonitor
    :
    public TA_IRS_Bus::IPTTMonitoring
{
public:

    /**
     * TrainRadioPttMonitor (constructor)
     *
     * @param notificationHwnd handle to window that is to receive
     *      the state change notifications
     *
     * @pre notificationHwnd is a valid, non null window handle
     *
     */
    TrainRadioPttMonitor(HWND notificationHwnd);
    virtual ~TrainRadioPttMonitor();

    /**
     * onPTT*(interface implementations)
     *
     * Most of these functions are not responded to, only care
     *  about when PTT state indicates operator can now talk
     *
     * @see IPTTMonitoring::onPTT* for more details*
     *
     */
    virtual void onPTTPressed() {}

    virtual void onPTTCease() {}

    virtual void onPTTQueued() {}

    virtual void onPTTBusy() {}

    virtual void onPTTMonitoring() {}

    /**
     * onPTTGranted
     *
     * Called when operator has PTT down and ready to talk state active
     *
     */
    virtual void onPTTGranted();

    /**
     * onPTTCease
     *
     * Called when PTT Released (after pressing down/talking)
     *
     */
    virtual void onPTTReleased();

private:

    // For monitoring the state of the train radio PTT button
    TA_IRS_Bus::PTTButton*  m_pttButtonMonitor;

    // Window to receive update notifications
    HWND m_notificationHwnd;
};

#endif // AFX_TrainRadioPttMonitor_H__8420358B_D5E4_4D3F_B59C_AF38445C948D__INCLUDED_