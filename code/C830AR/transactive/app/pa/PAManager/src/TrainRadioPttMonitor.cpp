/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainRadioPttMonitor.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Handles windows notification based monitoring of the PTT enabled state
 *     of the train radio microphone
 *
 */

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/TrainRadioPttMonitor.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "bus/radio/PTTMonitorLibrary/src/PTTButton.h"
#include "core/utilities/src/DebugUtil.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

TrainRadioPttMonitor::TrainRadioPttMonitor(HWND notificationHwnd)
    :
    m_notificationHwnd(notificationHwnd)
{
    TA_ASSERT(m_notificationHwnd != 0 && ::IsWindow(m_notificationHwnd),
    "Invalid window handle");

    try
    {
        m_pttButtonMonitor = new TA_IRS_Bus::PTTButton(*this);

        // Call ID is deprecated, so use -1
        // We don't handle changes in session ID during a train live broadcast...
        m_pttButtonMonitor->startMonitoring(-1, CachedConfig::getInstance()->getSessionId().c_str());
    }
    catch (...)
    {
        LOG( SourceInfo,
             TA_Base_Core::DebugUtil::ExceptionCatch,
             "Unknown",
             "Caught in TrainRadioPttMonitor constructor");

        PaErrorHandler::displayError(PaErrorHandler::INIT_ERROR);
    }
}

TrainRadioPttMonitor::~TrainRadioPttMonitor()
{
    // The window handle should still be valid up until destruction
    // (because will still be attempting to send out notifications)
    TA_ASSERT(m_notificationHwnd != 0 && ::IsWindow(m_notificationHwnd),
              "Invalid window handle");

    try
    {
        if (0 != m_pttButtonMonitor)
        {
            m_pttButtonMonitor->endMonitoring();
            delete m_pttButtonMonitor;
            m_pttButtonMonitor = 0;
        }
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }
}

void TrainRadioPttMonitor::onPTTGranted()
{
    if (m_notificationHwnd != 0 && ::IsWindow(m_notificationHwnd))
    {
        // Post and ON notification (wParam==1)
        ::PostMessage(m_notificationHwnd, WM_PA_TRAIN_PTT_CHANGE, 1, 0);
    }
    else
    {
        TA_ASSERT(FALSE, "Invalid window handle");
    }
}

void TrainRadioPttMonitor::onPTTReleased()
{
    if (m_notificationHwnd != 0 && ::IsWindow(m_notificationHwnd))
    {
        // Post an OFF notification (wParam==0)
        ::PostMessage(m_notificationHwnd, WM_PA_TRAIN_PTT_CHANGE, 0, 0);
    }
    else
    {
        TA_ASSERT(FALSE, "Invalid window handle");
    }
}