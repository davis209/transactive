/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASWriteEvent.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "PASWriteEvent.h"
#include "PasTableManager.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/exceptions/src/PasExceptions.h"
#include "core/threads/src/Thread.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    ACE_Future<int> PASWriteEvent::ignoredFuture;

    PASWriteEvent::PASWriteEvent(AbstractWriteablePASTable& writeableTable,
                                 ACE_Future<int> future,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASEvent(socketScheduler, processScheduler, eventSource),
        m_future(future),
        m_writeableTable(writeableTable)
    {
    }

    PASWriteEvent::PASWriteEvent(AbstractWriteablePASTable& writeableTable,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASEvent(socketScheduler, processScheduler, eventSource),
        m_future(ignoredFuture),
        m_writeableTable(writeableTable)
    {
    }

    PASWriteEvent::PASWriteEvent(ACE_Time_Value timeToExpire,
                                 AbstractWriteablePASTable& writeableTable,
                                 ACE_Future<int> future,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASEvent(timeToExpire, socketScheduler, processScheduler, eventSource)
        , m_future(future)
        , m_writeableTable(writeableTable)
    {
    }

    PASWriteEvent::PASWriteEvent(ACE_Time_Value timeToExpire,
                                 AbstractWriteablePASTable& writeableTable,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASEvent(timeToExpire, socketScheduler, processScheduler, eventSource)
        , m_future(ignoredFuture)
        , m_writeableTable(writeableTable)
    {
    }

    PASWriteEvent::~PASWriteEvent()
    {
        m_future.set(AbstractPASTable::PAS_ERR_GENERIC);
    }

    void PASWriteEvent::consume()
    {
        LOG_SCOPE("PASWriteEvent::consume");

        try
        {
            writeTable();
            m_writeableTable.removeWriteEvent(*this);
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
    }

    void PASWriteEvent::cancel()
    {
    }
}
