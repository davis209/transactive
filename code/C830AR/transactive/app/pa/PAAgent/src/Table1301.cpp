/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table1301.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table1301.h"
#include "PASConnection.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include <ace/ACE.h>

using namespace TA_Base_Core;

const size_t TABLE_1301_MESSAGE_SEQUENCE_ID_OFFSET = 0;
const size_t TABLE_1301_ANNOUNCE_ID_OFFSET         = 1;
const size_t TABLE_1301_PERIOD_OFFSET              = 2;
const size_t TABLE_1301_START_TIME_OFFSET          = 4;
const size_t TABLE_1301_STOP_TIME_OFFSET           = 8;
const size_t TABLE_1301_EVENT_TRIGGERED_OFFSET     = 12;
const size_t TABLE_1301_GLOBAL_COVERAGE_OFFSET     = 13;

namespace TA_IRS_App
{
    // ExceptionChecked
    Table1301::Table1301(TA_IRS_Bus::Scheduler& socketScheduler,
                         TA_IRS_Bus::Scheduler& processScheduler,
                         IPasEventSource& eventSource)
        : AbstractWriteablePASTable(1301)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasEventSource(eventSource)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table1301::setTableDataAndWrite(CORBA::Octet messageSeqId,
                                         CORBA::Octet announceId,
                                         unsigned short periodInSecs,
                                         TA_Base_Core::DateTime startTime,
                                         TA_Base_Core::DateTime stopTime,
                                         bool isEventTriggered,
                                         const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones)
        //throw ( TA_Base_Core::PasWriteErrorException, TA_Base_Core::PasConnectionException )
    {
        LOG_SCOPE("Table1301::setTableDataAndWrite");

        ACE_Future<int> future;
        WriteTable1301* event = new WriteTable1301(*this,
                                                   future,
                                                   messageSeqId,
                                                   announceId,
                                                   periodInSecs,
                                                   startTime,
                                                   stopTime,
                                                   isEventTriggered,
                                                   zones,
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

    WriteTable1301::WriteTable1301(Table1301& table,
                                   ACE_Future<int>& future,
                                   CORBA::Octet messageSeqId,
                                   CORBA::Octet announceId,
                                   unsigned short periodInSecs,
                                   TA_Base_Core::DateTime startTime,
                                   TA_Base_Core::DateTime stopTime,
                                   bool isEventTriggered,
                                   const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones,
                                   TA_IRS_Bus::Scheduler& socketScheduler,
                                   TA_IRS_Bus::Scheduler& processScheduler,
                                   IPasEventSource& eventSource)
        : PASWriteEvent(table, future, socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_messageSeqId(messageSeqId)
        , m_announceId(announceId)
        , m_periodInSecs(periodInSecs)
        , m_startTime(startTime)
        , m_stopTime(stopTime)
        , m_isEventTriggered(isEventTriggered)
        , m_zones(zones)
    {
    }

    WriteTable1301::WriteTable1301(ACE_Time_Value timeToExpire,
                                   Table1301& table,
                                   ACE_Future<int>& future,
                                   CORBA::Octet messageSeqId,
                                   CORBA::Octet announceId,
                                   unsigned short periodInSecs,
                                   TA_Base_Core::DateTime startTime,
                                   TA_Base_Core::DateTime stopTime,
                                   bool isEventTriggered,
                                   const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones,
                                   TA_IRS_Bus::Scheduler& socketScheduler,
                                   TA_IRS_Bus::Scheduler& processScheduler,
                                   IPasEventSource& eventSource)
        : PASWriteEvent(timeToExpire, table, future, socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_messageSeqId(messageSeqId)
        , m_announceId(announceId)
        , m_periodInSecs(periodInSecs)
        , m_startTime(startTime)
        , m_stopTime(stopTime)
        , m_isEventTriggered(isEventTriggered)
        , m_zones(zones)
    {
    }

    void WriteTable1301::writeTable()
    {
        LOG_SCOPE("WriteTable1301::writeTable");

        // Need a buffer for PA_MAXSTNID x 32bits
        std::uint32_t* globalCoverageBitmaps = reinterpret_cast<std::uint32_t*>(::calloc(TA_Base_Bus::IPAAgentCorbaDef::PA_MAXSTNID, 4));

        PasHelpers::instance().coverageToGlobalCoverageBitmaps(m_zones, globalCoverageBitmaps);

        // ReEntrant as this will also be guarded in the write() call.
        ThreadGuard guard(m_table.m_lock);

        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_1301_MESSAGE_SEQUENCE_ID_OFFSET, m_messageSeqId);
        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_1301_ANNOUNCE_ID_OFFSET, m_announceId);
        PasHelpers::instance().set16bit(m_table.m_buffer, TABLE_1301_PERIOD_OFFSET, m_periodInSecs);
        PasHelpers::instance().set32bit(m_table.m_buffer, TABLE_1301_START_TIME_OFFSET, m_startTime);
        PasHelpers::instance().set32bit(m_table.m_buffer, TABLE_1301_STOP_TIME_OFFSET, m_stopTime);
        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_1301_EVENT_TRIGGERED_OFFSET, m_isEventTriggered ? 1 : 0);

        unsigned long offset = TABLE_1301_GLOBAL_COVERAGE_OFFSET;

        for (int stationId = 1; stationId <= TA_Base_Bus::IPAAgentCorbaDef::PA_MAXSTNID; ++stationId)
        {
            PasHelpers::instance().set32bit(m_table.m_buffer, offset, globalCoverageBitmaps[stationId - 1]);
            offset += 4;
        }

        // Free the buffer before anything bad can happen
        ::free(globalCoverageBitmaps);
        globalCoverageBitmaps = NULL;

        m_future.set(PASConnection::getInstance()->writeTable(m_table.TABLE_NUMBER,
                                                              m_table.m_buffer,
                                                              m_table.BUFFER_SIZE));
    }

    PASWriteEvent* WriteTable1301::recreateEvent(ACE_Time_Value timeToExpire)
    {
        return new WriteTable1301(timeToExpire,
                                  m_table,
                                  m_future,
                                  m_messageSeqId,
                                  m_announceId,
                                  m_periodInSecs,
                                  m_startTime,
                                  m_stopTime,
                                  m_isEventTriggered,
                                  m_zones,
                                  m_socketScheduler,
                                  m_processScheduler,
                                  m_pasEventSource);
    }
}
