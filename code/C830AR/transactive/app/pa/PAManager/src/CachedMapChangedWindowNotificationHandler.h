/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/CachedMapChangedWindowNotificationHandler.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Helper class to handle posting of notifications to windows
 * when a cache map element changes
 * Links in with CachedMap class by registering this class
 * as an observer, it posts notifications (using key as lParam)
 * whenever an update received
 *
 */

#ifndef _CachedMapChangedWindowNotificationHandler_HEADER_
#define _CachedMapChangedWindowNotificationHandler_HEADER_

#include "bus/pa/cachedmaps/src/ICachedMapObserver.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"

namespace TA_IRS_Bus
{
    class ICachedMapObserverRegistry;
}

class CachedMapChangedWindowNotificationHandler
    :
    public TA_IRS_Bus::ICachedMapObserver
{
public:

    /**
     * CachedMapChangedWindowNotificationHandler (constructor)
     *
     * Constructs a new observer which will post messages to target
     * window whenever if receives a onCachedMapChanged call
     *
     * @param hWnd the window to receive the update notifications
     *
     * @param msgId the WM_ message identifier (WM_APP + n)
     *
     * @param registry where we should register and deregister ourselves
     *              this reference must remain valid for lifetime of this object
     *              as it is used in destructor to deregister ourself
     *
     */
    CachedMapChangedWindowNotificationHandler(HWND hWnd,
                                              DWORD msgId,
                                              TA_IRS_Bus::ICachedMapObserverRegistry& registry);

    /**
     * ~CachedMapChangedWindowNotificationHandler (destructor)
     *
     * deregisters this observer from the registry reference passed to constructor
     *
     */
    virtual ~CachedMapChangedWindowNotificationHandler();

    /**
     * onCachedMapChange (interface implementation)
     *
     * @see ICachedMapObserver::onCachedMapChange for more details
     *
     */
    virtual void onCachedMapChanged(unsigned long pKey, bool changesMade);

    /**
     * setMustAcknowledgeBeforeResendFlag
     *
     * @param mustAcknowledge set to true if the acknowledgeMessage function
     *      must be called before another message will be posted out to
     *      the window associated with this object (use this if only want
     *      a single notification for a batch of changes that come through)
     *
     * @see acknowledgeMessage
     *
     */
    void setMustAcknowledgeBeforeResendFlag(bool mustAcknowledge);

    /**
     * acknowledgeMessage
     *
     * Call this function to acknowledge receipt of a notification message
     *  once this is called, a new notification will be sent out for any
     *  subsequent changes.  Hence this should be called from the client
     *  window BEFORE any processing (so if during processing another change
     *  occurs, it will receive this notification also)
     *
     * @pre message has not already been acknowledged (without a new
     *          changed notification being received)
     *
     * @pre setMustAcknowledgeBeforeResendFlag has been set to true
     *
     * @see setMustAcknowledgeBeforeResendFlag
     *
     */
    void acknowledgeMessage();

private:

    HWND    m_notificationWnd;
    DWORD   m_wmMessage;

    // The registry that handler registration and deregistration of this handler
    TA_IRS_Bus::ICachedMapObserverRegistry& m_observerRegistry;

    bool    m_mustAcknowledgeMessagesBeforePosting;
    int     m_acknowledgeMentsRequired;
    TA_Base_Core::NonReEntrantThreadLockable m_lock;
};

#endif // _CachedMapChangedWindowNotificationHandler_HEADER_
