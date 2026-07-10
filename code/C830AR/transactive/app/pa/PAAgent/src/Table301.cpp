/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table301.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table301.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include <ace/ACE.h>

using namespace TA_Base_Core;

namespace
{
    const size_t TABLE_301_MESSAGE_SEQUENCE_ID_OFFSET   = 0;
    const size_t TABLE_301_ANNOUNCE_ID_OFFSET           = 1;
    const size_t TABLE_301_PERIOD_OFFSET                = 2;
    const size_t TABLE_301_START_TIME_OFFSET            = 4;
    const size_t TABLE_301_STOP_TIME_OFFSET             = 8;
    const size_t TABLE_301_EVENT_TRIGGERED_OFFSET       = 12;
    const size_t TABLE_301_LOCAL_COVERAGE_OFFSET        = 13;
}

namespace TA_IRS_App
{
    Table301::Table301(TA_IRS_Bus::Scheduler& socketScheduler,
                       TA_IRS_Bus::Scheduler& processScheduler,
                       IPasEventSource& eventSource)
        : AbstractWriteablePASTable(301)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasEventSource(eventSource)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table301::setTableDataAndWrite(CORBA::Octet messageSeqId,
                                        CORBA::Octet announceId,
                                        unsigned short periodInSecs,
                                        TA_Base_Core::DateTime startTime,
                                        TA_Base_Core::DateTime stopTime,
                                        bool isEventTriggered,
                                        const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones)
        //throw ( TA_Base_Core::PasWriteErrorException, TA_Base_Core::PasConnectionException )
    {
        LOG_SCOPE("Table301::setTableDataAndWrite");

        ACE_Future<int> future;
        WriteTable301* event = new WriteTable301(*this,
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

    WriteTable301::WriteTable301(Table301& table,
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

    WriteTable301::WriteTable301(ACE_Time_Value timeToExpire,
                                 Table301& table,
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

    void WriteTable301::writeTable()
    {
        LOG_SCOPE("WriteTable301::writeTable");
        ThreadGuard guard(m_table.m_lock);

        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_301_MESSAGE_SEQUENCE_ID_OFFSET, m_messageSeqId);
        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_301_ANNOUNCE_ID_OFFSET, m_announceId);
        PasHelpers::instance().set16bit(m_table.m_buffer, TABLE_301_PERIOD_OFFSET, m_periodInSecs);
        PasHelpers::instance().set32bit(m_table.m_buffer, TABLE_301_START_TIME_OFFSET, m_startTime);
        PasHelpers::instance().set32bit(m_table.m_buffer, TABLE_301_STOP_TIME_OFFSET, m_stopTime);
        PasHelpers::instance().set8bit(m_table.m_buffer, TABLE_301_EVENT_TRIGGERED_OFFSET, m_isEventTriggered ? 1 : 0);

        if (is_depot())
        {
            auto localCoverageBitmap = PasHelpers::instance().coverageToLocalCoverageBitmap<std::uint64_t>(m_zones);
            PasHelpers::instance().set64bit(m_table.m_buffer, TABLE_301_LOCAL_COVERAGE_OFFSET, localCoverageBitmap);
        }
        else
        {
            auto localCoverageBitmap = PasHelpers::instance().coverageToLocalCoverageBitmap<std::uint32_t>(m_zones);
            PasHelpers::instance().set32bit(m_table.m_buffer, TABLE_301_LOCAL_COVERAGE_OFFSET, localCoverageBitmap);
        }

        m_future.set(PASConnection::getInstance()->writeTable(m_table.TABLE_NUMBER,
                                                              m_table.m_buffer,
                                                              m_table.BUFFER_SIZE));
    }

    PASWriteEvent* WriteTable301::recreateEvent(ACE_Time_Value timeToExpire)
    {
        return new WriteTable301(timeToExpire,
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
