/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/RightsChangedWindowNotificationHandler.cpp $
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

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/RightsChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "core/synchronisation/src/ThreadGuard.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

RightsChangedWindowNotificationHandler::RightsChangedWindowNotificationHandler(HWND hWnd)
    :
    m_notificationWnd(hWnd)
{
    PaRightsManager::getInstance().registerRightsChangeListener(*this);
}

RightsChangedWindowNotificationHandler::~RightsChangedWindowNotificationHandler()
{
    // This is not a safe condition, we were still posting notifications to this window
    TA_ASSERT(::IsWindow(m_notificationWnd), "Window no longer valid while in destructor");

    PaRightsManager::getInstance().deRegisterRightsChangeListener(*this);
}

void RightsChangedWindowNotificationHandler::onRightsChanged(const std::vector<unsigned long>& changedRightsGlobalIds)
{
    TA_ASSERT(::IsWindow(m_notificationWnd), "Window no longer valid");

    TA_Base_Core::ThreadGuard sync(m_lock);

    ::PostMessage(m_notificationWnd, WM_PA_RIGHTS_CHANGE, 0, 0);
}
