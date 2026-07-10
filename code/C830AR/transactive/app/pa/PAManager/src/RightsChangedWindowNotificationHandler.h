/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/RightsChangedWindowNotificationHandler.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Helper class to handle posting of notifications (WM_PA_RIGHTS_CHANGE) to windows
 * when there is a change in rights
 *
 */

#pragma once

#include "app/pa/PaManager/src/IRightsChangeListener.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"

class RightsChangedWindowNotificationHandler
    :
    public IRightsChangeListener
{
public:

    /**
     * RightsChangedWindowNotificationHandler (constructor)
     *
     * Constructs a new observer which will post messages to target
     * window whenever if receives a onCachedMapChanged call
     *
     * @param hWnd the window to receive the update notifications
     *
     */
    RightsChangedWindowNotificationHandler(HWND hWnd);

    /**
     * ~CachedMapChangedWindowNotificationHandler (destructor)
     *
     * deregisters this observer from the rights changed registry
     *
     */
    virtual ~RightsChangedWindowNotificationHandler();

    /**
     * onRightsChanged
     *
     * Interface implementation, @see IRightsChangeListener::onRightsChanged
     *
     */
    void onRightsChanged(const std::vector<unsigned long>& changedRightsGlobalIds);

private:

    HWND    m_notificationWnd;
    TA_Base_Core::NonReEntrantThreadLockable m_lock;
};
