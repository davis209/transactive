/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table630.cpp $
 * @author:  Ripple
 * @version: $Revision: #6 $
 *
 * Last modification: $DateTime: 2025/03/05 13:16:38 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table630.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include <ace/ACE.h>
#include <malloc.h>

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    Table630::Table630(TA_IRS_Bus::Scheduler& socketScheduler,
                       TA_IRS_Bus::Scheduler& processScheduler,
                       IPasEventSource& eventSource)
        : AbstractWriteablePASTable(630)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasEventSource(eventSource)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table630::setTableDataAndWrite(unsigned int messageKey, CORBA::Octet sourceId)
        //throw ( TA_Base_Core::PasWriteErrorException, TA_Base_Core::PasConnectionException )
    {
        LOG_SCOPE("Table630::setTableDataAndWrite");

        ACE_Future<int> future;
        WriteTable630* event = new WriteTable630(*this,
                                                 future,
                                                 messageKey,
                                                 sourceId,
                                                 m_socketScheduler,
                                                 m_processScheduler,
                                                 m_pasEventSource);
        addWriteEvent(*event);
        m_socketScheduler.post(event);

        int retVal = -1;
        ACE_Time_Value tv(10, 0);
        tv += ACE_OS::gettimeofday();

        if (-1 == future.get(retVal, &tv))
        {
            char msg[100] = {0};
            sprintf(msg, "Timeout on write operation on table %d", TABLE_NUMBER);
            TA_THROW(TA_Base_Core::PasWriteErrorException(retVal, msg));
        }

        if (retVal)
        {
            char msg[100] = {0};
            sprintf(msg, "Write operation on table %d returned an error state %d", TABLE_NUMBER, retVal);
            TA_THROW(TA_Base_Core::PasWriteErrorException(retVal, msg));
        }
    }

    WriteTable630::WriteTable630(Table630& table,
                                 ACE_Future<int>& future,
                                 unsigned int messageKey,
                                 CORBA::Octet sourceId,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(table, future, socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_messageKey(messageKey)
        , m_sourceId(sourceId)
    {
    }

    WriteTable630::WriteTable630(ACE_Time_Value timeToExpire,
                                 Table630& table,
                                 ACE_Future<int>& future,
                                 unsigned int messageKey,
                                 CORBA::Octet sourceId,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(timeToExpire, table, future, socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_messageKey(messageKey)
        , m_sourceId(sourceId)
    {
    }

    void WriteTable630::writeTable()
    {
        LOG_SCOPE("WriteTable630::writeTable");

        int adhocOffset = PasHelpers::instance().getAdhocOffset(m_messageKey);

        TA_ASSERT(adhocOffset < TA_Base_Bus::IPAAgentCorbaDef::PA_NBADHOC, "adhocOffset out of bounds");

        ThreadGuard guard(m_table.m_lock);

        for (auto i = 0; i < TA_Base_Bus::IPAAgentCorbaDef::PA_NBADHOC; ++i)
        {
            PasHelpers::instance().set8bit(m_table.m_buffer, i, 0);
        }

        PasHelpers::instance().set8bit(m_table.m_buffer, adhocOffset, m_sourceId);

        m_future.set(PASConnection::getInstance()->writeTable(m_table.TABLE_NUMBER,
                                                              m_table.m_buffer,
                                                              m_table.BUFFER_SIZE));
    }

    PASWriteEvent* WriteTable630::recreateEvent(ACE_Time_Value timeToExpire)
    {
        return new WriteTable630(timeToExpire,
                                 m_table,
                                 m_future,
                                 m_messageKey,
                                 m_sourceId,
                                 m_socketScheduler,
                                 m_processScheduler,
                                 m_pasEventSource);
    }
}
