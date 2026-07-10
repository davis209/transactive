/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASReadEvent.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "PASReadEvent.h"
#include "PasTableManager.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/exceptions/src/PasExceptions.h"
#include "core/threads/src/Thread.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    PASReadEvent::PASReadEvent(TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASEvent(socketScheduler, processScheduler, eventSource)
    {
    }

    PASReadEvent::PASReadEvent(ACE_Time_Value timeToExpire,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASEvent(timeToExpire, socketScheduler, processScheduler, eventSource)
    {
    }

    void PASReadEvent::consume()
    {
        LOG_SCOPE("PASReadEvent::consume");

        try
        {
            readTable();
        }
        catch (TA_Base_Core::PasConnectionException& pce)
        {
            PASConnectionExceptionReason reason = pce.getReason();
            LOG_ERROR("consume(): PASConnectionExceptionReason: reason=%d", reason);

            // Only set the future to an error if the pas connection could
            // not reconnect
            switch (reason)
            {
                case UNABLE_TO_CONNECT:
                    m_socketScheduler.clear();
                    m_socketScheduler.post(m_pasEventSource.getDisconnectEvent());
                    m_processScheduler.post(m_pasEventSource.getInvalidateEvent());
                    m_socketScheduler.post(m_pasEventSource.getConnectEvent());
                    break;

                case DISCONNECTED:
                    m_socketScheduler.clear();
                    m_processScheduler.post(m_pasEventSource.getInvalidateEvent());
                    m_socketScheduler.post(m_pasEventSource.getConnectEvent());
                    break;
            }
        }

        delete this;
    }

    ACE_Time_Value PASReadEvent::getNextTimeToExpire()
    {
        return getNextTimeToExpire(CachedConfig::getInstance()->getFastPollRateInMSecs());
    }
}
