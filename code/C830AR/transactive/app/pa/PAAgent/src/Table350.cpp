/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table350.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table350.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include <ace/ACE.h>

using namespace TA_Base_Core;

const size_t TABLE_350_MESSAGE_SEQUENCE_ID_OFFSET   = 0;
const size_t TABLE_350_CHIME_OFFSET                 = 1;
const size_t TABLE_350_DVA_MESSAGE_1_OFFSET         = 2;
const size_t TABLE_350_DVA_MESSAGE_2_OFFSET         = 4;
const size_t TABLE_350_DVA_MESSAGE_3_OFFSET         = 6;
const size_t TABLE_350_DVA_MESSAGE_4_OFFSET         = 8;
const size_t TABLE_350_DWELL_TIME_OFFSET            = 10;

namespace TA_IRS_App
{
    Table350::Table350(TA_IRS_Bus::Scheduler& socketScheduler,
                       TA_IRS_Bus::Scheduler& processScheduler,
                       IPasEventSource& eventSource)
        : AbstractWriteablePASTable(350)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasEventSource(eventSource)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table350::setTableDataAndWrite(CORBA::Octet messageSeqId,
                                        bool hasChime,
                                        TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq& messages,
                                        unsigned short dwellTimeInSecs)
        //throw ( TA_Base_Core::PasWriteErrorException, TA_Base_Core::PasConnectionException )
    {
        LOG_SCOPE("Table350::setTableDataAndWrite");

        ACE_Future<int> future;
        WriteTable350* event = new WriteTable350(*this,
                                                 future,
                                                 messageSeqId,
                                                 hasChime,
                                                 messages,
                                                 dwellTimeInSecs,
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

    WriteTable350::WriteTable350(Table350& table,
                                 ACE_Future<int>& future,
                                 CORBA::Octet messageSeqId,
                                 bool hasChime,
                                 TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq& messages,
                                 unsigned short dwellTimeInSecs,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(table, future, socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_messageSeqId(messageSeqId)
        , m_hasChime(hasChime)
        , m_messages(messages)
        , m_dwellTimeInSecs(dwellTimeInSecs)
    {
    }

    WriteTable350::WriteTable350(ACE_Time_Value timeToExpire,
                                 Table350& table,
                                 ACE_Future<int>& future,
                                 CORBA::Octet messageSeqId,
                                 bool hasChime,
                                 TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq& messages,
                                 unsigned short dwellTimeInSecs,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(timeToExpire, table, future, socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_messageSeqId(messageSeqId)
        , m_hasChime(hasChime)
        , m_messages(messages)
        , m_dwellTimeInSecs(dwellTimeInSecs)
    {
    }

    void WriteTable350::writeTable()
    {
        LOG_SCOPE("WriteTable350::writeTable");

        TA_ASSERT(m_messages.length() > 0 && m_messages.length() <= TA_Base_Bus::IPAAgentCorbaDef::PA_MAXMSGSEQ, "messages contains 0 or more than PA_MAXMSGSEQ dva messages");
        TA_ASSERT(TA_Base_Bus::IPAAgentCorbaDef::PA_MAXMSGSEQ == 4, "Assumption TA_Base_Bus::IPAAgentCorbaDef::PA_MAXMSGSEQ == 4 is false");

        unsigned short messageIds[4];

        for (int i = 0; i < 4; ++i)
        {
            if (i < m_messages.length())
            {
                messageIds[i] = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageIdFromKey(m_messages[i]);
            }
            else
            {
                // No more messages - set to 0
                messageIds[i] = 0;
            }
        }

        // ReEntrant as this will also be guarded in the write() call.
        ThreadGuard guard(m_table.m_lock);

        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_350_MESSAGE_SEQUENCE_ID_OFFSET, m_messageSeqId);
        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_350_CHIME_OFFSET, m_hasChime ? 1 : 0);
        PasHelpers::instance().set16bit(m_table.m_buffer, TABLE_350_DVA_MESSAGE_1_OFFSET, messageIds[0]);
        PasHelpers::instance().set16bit(m_table.m_buffer, TABLE_350_DVA_MESSAGE_2_OFFSET, messageIds[1]);
        PasHelpers::instance().set16bit(m_table.m_buffer, TABLE_350_DVA_MESSAGE_3_OFFSET, messageIds[2]);
        PasHelpers::instance().set16bit(m_table.m_buffer, TABLE_350_DVA_MESSAGE_4_OFFSET, messageIds[3]);
        PasHelpers::instance().set16bit(m_table.m_buffer, TABLE_350_DWELL_TIME_OFFSET, m_dwellTimeInSecs);

        m_future.set(PASConnection::getInstance()->writeTable(m_table.TABLE_NUMBER,
                                                              m_table.m_buffer,
                                                              m_table.BUFFER_SIZE));
    }

    PASWriteEvent* WriteTable350::recreateEvent(ACE_Time_Value timeToExpire)
    {
        return new WriteTable350(timeToExpire,
                                 m_table,
                                 m_future,
                                 m_messageSeqId,
                                 m_hasChime,
                                 m_messages,
                                 m_dwellTimeInSecs,
                                 m_socketScheduler,
                                 m_processScheduler,
                                 m_pasEventSource);
    }
}
